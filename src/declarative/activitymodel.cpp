/*
    Copyright 2016 Jan Grulich <jgrulich@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "activitymodel.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <KWindowSystem>

#include <QLoggingCategory>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusUnixFileDescriptor>

Q_DECLARE_LOGGING_CATEGORY(PLASMA_TIMEKEEPER)

Q_LOGGING_CATEGORY(PLASMA_TIMEKEEPER, "plasma-timekeeper")

const static QString LOGIN1_DBUS_SERVICE = QStringLiteral("org.freedesktop.login1");
const static QString LOGIN1_DBUS_PATH = QStringLiteral("/org/freedesktop/login1");
const static QString LOGIN1_DBUS_MANAGER_INTERFACE = QStringLiteral("org.freedesktop.login1.Manager");


/*                     ActivityModelItem::Private                          *
 * ----------------------------------------------------------------------- */
class ActivityModelItem::Private
{
public:
    Private()
    { }

    QPixmap activityIcon;
    QPixmap activityDefaultIcon;
    QString activityName;
    QTime activityTime;
};

/*                          ActivityModelItem                              *
 * ----------------------------------------------------------------------- */

ActivityModelItem::ActivityModelItem(QObject* parent)
    : QObject(parent),
      d(new Private())
{
}

ActivityModelItem::~ActivityModelItem()
{
    delete d;
}

void ActivityModelItem::setActivityIcon(const QPixmap& icon)
{
    d->activityIcon = icon;
}

QPixmap ActivityModelItem::activityIcon() const
{
    return d->activityIcon;
}

void ActivityModelItem::setActivityDefaultIcon(const QPixmap& icon)
{
    d->activityDefaultIcon = icon;
}

QPixmap ActivityModelItem::activityDefaultIcon() const
{
    return d->activityDefaultIcon;
}

void ActivityModelItem::setActivityName(const QString& name)
{
    d->activityName = name;
}

QString ActivityModelItem::activityName() const
{
    return d->activityName;
}

void ActivityModelItem::setActivityTime(const QTime& time)
{
    d->activityTime = time;
}

QTime ActivityModelItem::activityTime() const
{
    return d->activityTime;
}

void ActivityModelItem::addSeconds(int secs)
{
    d->activityTime = d->activityTime.addSecs(secs);
}

/*                     ActivityModel::Private                              *
 * ----------------------------------------------------------------------- */
class ActivityModel::Private
{
public:
    Private()
    : preparingForSleep(false),
      preparingForShutdown(false),
      resetOnSuspend(false),
      resetOnShutdown(false),
      screenLocked(false),
      timeTrackingEnabled(true),
      timer(new QTimer())
    { }

    ~Private()
    {
        delete timer;
    }

    bool preparingForSleep;
    bool preparingForShutdown;
    bool resetOnSuspend;
    bool resetOnShutdown;
    bool screenLocked;
    bool timeTrackingEnabled;

    // Current activity and time when the activity was updated for the last time
    QString currentActiveWindow;
    QTime currentTime;

    // List of activities
    QList<ActivityModelItem*> list;

    // List of ignored activities
    QStringList ignoredActivitiesList;

    // Timer
    QTimer* timer;

    QDBusUnixFileDescriptor inhibitFileDescriptor;
};

/*                          ActivityModel                                  *
 * ----------------------------------------------------------------------- */

ActivityModel::ActivityModel(QObject* parent)
    : QAbstractListModel(parent),
      d(new Private())
{
    QLoggingCategory::setFilterRules(QStringLiteral("plasma-timekeeper.debug = false"));

    QDBusInterface iface(QStringLiteral("org.kde.ksmserver"),
                         QStringLiteral("/ScreenSaver"),
                         QStringLiteral("org.freedesktop.ScreenSaver"),
                         QDBusConnection::sessionBus());

    // I guess there is a minimum chance that the applet will be started while the system
    // is locked, but it's better to be sure (e.g. plasmashell crash)
    QDBusPendingCall dbusCall = iface.asyncCall(QStringLiteral("GetActive"));
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(dbusCall);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, [this] (QDBusPendingCallWatcher* watcher) {
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isValid()) {
            d->screenLocked = reply.value();
        }
    });

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &ActivityModel::activeWindowChanged, Qt::UniqueConnection);
    connect(d->timer, &QTimer::timeout, this, &ActivityModel::updateCurrentActivityTime);

    // TODO check if logind is running

    QDBusConnection::sessionBus().connect(QStringLiteral("org.kde.ksmserver"),
                                          QStringLiteral("/ScreenSaver"),
                                          QStringLiteral("org.freedesktop.ScreenSaver"),
                                          QStringLiteral("ActiveChanged"),
                                          this,
                                          SLOT(lockscreenActivityChanged(bool)));

    QDBusConnection::systemBus().connect(LOGIN1_DBUS_SERVICE,
                                         LOGIN1_DBUS_PATH,
                                         LOGIN1_DBUS_MANAGER_INTERFACE,
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(prepareForSleepChanged(bool)));

    QDBusConnection::systemBus().connect(LOGIN1_DBUS_SERVICE,
                                         LOGIN1_DBUS_PATH,
                                         LOGIN1_DBUS_MANAGER_INTERFACE,
                                         QStringLiteral("PrepareForShutdown"),
                                         this,
                                         SLOT(prepareForShutdownChanged(bool)));
    inhibit();

    // Load previous values
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("plasma-timekeeper"), KConfig::SimpleConfig);
    Q_FOREACH (const QString& groupName, config->groupList()) {
        KConfigGroup group(config, groupName);
        if (group.isValid()) {
            if (groupName == QLatin1String("general")) {
                d->timeTrackingEnabled = group.readEntry<bool>("trackingEnabled", true);
                d->ignoredActivitiesList = group.readEntry<QStringList>("ignoredActivities", QStringList());
                continue;
            }

            ActivityModelItem *item = new ActivityModelItem();
            item->setActivityName(groupName);
            item->setActivityDefaultIcon(QIcon::fromTheme(QLatin1String("xorg")).pixmap(QSize(64, 64)));
            item->setActivityTime(QTime::fromString(group.readEntry("time")));

            const int index = d->list.count();
            beginInsertRows(QModelIndex(), index, index);
            d->list << item;
            endInsertRows();
        }
    }

    // Process the currently active window
    activeWindowChanged(KWindowSystem::activeWindow());
}

ActivityModel::~ActivityModel()
{
    delete d;
}

QVariant ActivityModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();

    if (row >= 0 && row < d->list.count()) {
        ActivityModelItem * item = d->list.at(row);

        switch (role) {
            case ActivityIconRole:
                return item->activityIcon().isNull() ? item->activityDefaultIcon() : item->activityIcon();
                break;
            case ActivityNameRole:
                return item->activityName();
                break;
            case ActivityTimeRole:
                return item->activityTime().toString(Qt::RFC2822Date);
                break;
            default:
                break;
        }
    }

    return QVariant();
}

int ActivityModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->list.count();
}

QHash< int, QByteArray > ActivityModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[ActivityIconRole] = "ActivityIcon";
    roles[ActivityNameRole] = "ActivityName";
    roles[ActivityTimeRole] = "ActivityTime";

    return roles;
}

QPixmap ActivityModel::currentActivityIcon() const
{
    if (d->currentActiveWindow.isEmpty()) {
        return QIcon::fromTheme(QLatin1String("xorg")).pixmap(QSize(64, 64));
    }

    QList<ActivityModelItem*>::const_iterator it;
    for (it = d->list.constBegin(); it != d->list.constEnd(); ++it) {
        if ((*it)->activityName() == d->currentActiveWindow) {
            return (*it)->activityIcon().isNull() ? (*it)->activityDefaultIcon() : (*it)->activityIcon();
        }
    }

    return QIcon::fromTheme(QLatin1String("xorg")).pixmap(QSize(64, 64));
}

QString ActivityModel::currentActivityName() const
{
    if (d->currentActiveWindow.isEmpty()) {
        return i18n("No active window");
    }

    return d->currentActiveWindow;
}

QString ActivityModel::currentActivityTime() const
{
    if (d->currentActiveWindow.isEmpty()) {
        return QString();
    }

    QList<ActivityModelItem*>::const_iterator it;
    for (it = d->list.constBegin(); it != d->list.constEnd(); ++it) {
        if ((*it)->activityName() == d->currentActiveWindow) {
            return (*it)->activityTime().toString(Qt::RFC2822Date);
        }
    }

    return QString();
}

bool ActivityModel::timeTrackingEnabled() const
{
    return d->timeTrackingEnabled;
}

void ActivityModel::setTimeTrackingEnabled(bool enabled)
{
    d->timeTrackingEnabled = enabled;

    updateTrackingState();

    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("plasma-timekeeper"), KConfig::SimpleConfig);
    KConfigGroup group(config, "general");
    if (group.isValid()) {
        group.writeEntry<bool>("trackingEnabled", enabled);
    }
}

void ActivityModel::setResetOnSuspend(bool reset)
{
    d->resetOnSuspend = reset;
}

void ActivityModel::setResetOnShutdown(bool reset)
{
    d->resetOnShutdown = reset;
}

void ActivityModel::ignoreActivity(const QString& activityName)
{
    if (!d->ignoredActivitiesList.contains(activityName)) {
        d->ignoredActivitiesList.append(activityName);

        KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("plasma-timekeeper"), KConfig::SimpleConfig);
        KConfigGroup group(config, "general");
        if (group.isValid()) {
            group.writeEntry<QStringList>("ignoredActivities", d->ignoredActivitiesList);
        }

        Q_FOREACH (ActivityModelItem* item, d->list) {
            if (item->activityName() == activityName) {
                config->deleteGroup(item->activityName());

                const int row = d->list.indexOf(item);
                if (row >= 0) {
                    beginRemoveRows(QModelIndex(), row, row);
                    d->list.removeAt(row);
                    item->deleteLater();
                    endRemoveRows();
                }
            }
        }

        if (d->currentActiveWindow == activityName) {
            // Reset current item
            d->currentActiveWindow = QString();
            d->currentTime = QTime::currentTime();
            Q_EMIT currentActivityChanged();
        }
    }
}

void ActivityModel::inhibit()
{
    if (d->inhibitFileDescriptor.isValid()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(LOGIN1_DBUS_SERVICE,
                                                          LOGIN1_DBUS_PATH,
                                                          LOGIN1_DBUS_MANAGER_INTERFACE,
                                                          QStringLiteral("Inhibit"));

    message.setArguments(QVariantList({QStringLiteral("shutdown:sleep"),
                         i18n("Plasma Timekeeper"),
                         i18n("Ensuring that the statistics get reseted on suspend or shutdown"),
                         QStringLiteral("delay")}));
    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher *inhibitWatcher = new QDBusPendingCallWatcher(reply, this);
    connect(inhibitWatcher, &QDBusPendingCallWatcher::finished, this,
        [this](QDBusPendingCallWatcher *self) {
            QDBusPendingReply<QDBusUnixFileDescriptor> reply = *self;
            self->deleteLater();
            if (!reply.isValid()) {
                return;
            }
            reply.value().swap(d->inhibitFileDescriptor);
        }
    );
}

void ActivityModel::uninhibit()
{
    if (!d->inhibitFileDescriptor.isValid()) {
        return;
    }

    d->inhibitFileDescriptor = QDBusUnixFileDescriptor();
}

void ActivityModel::resetTimeStatistics()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("plasma-timekeeper"), KConfig::SimpleConfig);

    Q_FOREACH (ActivityModelItem* item, d->list) {
        config->deleteGroup(item->activityName());

        const int row = d->list.indexOf(item);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            d->list.removeAt(row);
            item->deleteLater();
            endRemoveRows();
        }
    }

    // Reset current item
    d->currentActiveWindow = QString();
    d->currentTime = QTime::currentTime();
    Q_EMIT currentActivityChanged();

    // If time tracking is not enabled we don't need to start it again
    if (d->timeTrackingEnabled) {
        activeWindowChanged(KWindowSystem::activeWindow());
    }
}

void ActivityModel::activeWindowChanged(WId window)
{
    KWindowInfo info = KWindowInfo(window, NET::WMName | NET::WMIconName, NET::WM2WindowClass);

    qCDebug(PLASMA_TIMEKEEPER) << "Active window changed to " << info.windowClassName();

    if (info.windowClassName().isEmpty()) {
        return;
    }

    // Process the current activity
    updateCurrentActivityTime();

    if (!d->timeTrackingEnabled || d->ignoredActivitiesList.contains(info.windowClassName())) {
        // Reset current item
        d->currentActiveWindow = QString();
        d->currentTime = QTime::currentTime();
        Q_EMIT currentActivityChanged();
        qCDebug(PLASMA_TIMEKEEPER) << "Activity " << info.windowClassName() << " is ignored";
        return;
    }

    // Find if the activity already exists and if not add it to the model
    QList<ActivityModelItem*>::const_iterator it;
    for (it = d->list.constBegin(); it != d->list.constEnd(); ++it) {
        if ((*it)->activityName() == info.windowClassName()) {
            break;
        }
    }

    if (it == d->list.constEnd()) {
        qCDebug(PLASMA_TIMEKEEPER) << "Adding new activity item " << info.windowClassName();
        ActivityModelItem *item = new ActivityModelItem();
        item->setActivityName(info.windowClassName());
        item->setActivityIcon(KWindowSystem::icon(window, 64, 64, true));
        item->setActivityTime(QTime(0, 0, 0));

        const int index = d->list.count();
        beginInsertRows(QModelIndex(), index, index);
        d->list << item;
        endInsertRows();
    } else if ((*it)->activityIcon().isNull()) { // Update icon to avoid using the default one
        (*it)->setActivityIcon(KWindowSystem::icon(window, 64, 64, true));
        const int row = d->list.indexOf((*it));
        if (row >= 0) {
            QModelIndex index = createIndex(row, 0);
            Q_EMIT dataChanged(index, index);
        }
    }

    // Process the next activity
    if (!d->timeTrackingEnabled && !d->screenLocked) {
        // Start timer to update the time every minute
        d->timer->stop();
        // TODO make this configurable
        d->timer->start(60000);
    }

    // Save current time and activity
    d->currentActiveWindow = info.windowClassName();
    d->currentTime = QTime::currentTime();
    Q_EMIT currentActivityChanged();
}

void ActivityModel::lockscreenActivityChanged(bool active)
{
    d->screenLocked = active;

    updateTrackingState();
}

void ActivityModel::prepareForSleepChanged(bool sleep)
{
    d->preparingForSleep = sleep;

    updateTrackingState();

    if (d->preparingForSleep && d->resetOnSuspend) {
        resetTimeStatistics();
    }

    if (d->preparingForSleep) {
        uninhibit();
    } else {
        // Inhibit again to be sure that the next suspend will also reset and update the stats
        inhibit();
    }
}

void ActivityModel::prepareForShutdownChanged(bool shutdown)
{
    // NOTE
    // Might not work when rebooting/turning of the computer from kickoff in Plasma
    // See https://quickgit.kde.org/?p=plasma-workspace.git&a=commit&h=b5e814a7b2867914327c889794b1088027aaafd6

    d->preparingForShutdown = shutdown;

    updateTrackingState();

    if (d->preparingForShutdown && d->resetOnShutdown) {
        resetTimeStatistics();
        uninhibit();
    }

    // Probably no reason to start the inhibitor again as the applet will
    // be reloaded completely with already running inhibitor
}

void ActivityModel::updateCurrentActivityTime()
{
    Q_FOREACH (ActivityModelItem * item, d->list) {
        if (d->currentActiveWindow == item->activityName()) {
            item->addSeconds(d->currentTime.secsTo(QTime::currentTime()));

            const int row = d->list.indexOf(item);
            if (row >= 0) {
                QModelIndex index = createIndex(row, 0);
                Q_EMIT dataChanged(index, index);
            }

            // Store the new updated value
            KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("plasma-timekeeper"), KConfig::SimpleConfig);
            KConfigGroup group(config, item->activityName());
            if (group.isValid()) {
                group.writeEntry("time", item->activityTime().toString(Qt::RFC2822Date));
            }
        }
    }

    Q_EMIT currentActivityChanged();

    if (d->timeTrackingEnabled) {
        d->currentTime = QTime::currentTime();
        d->timer->start(60000);
    }
}

void ActivityModel::updateTrackingState()
{
    if (d->timeTrackingEnabled && !d->screenLocked && !d->preparingForSleep && !d->preparingForShutdown) {
        // Start again with current active window
        activeWindowChanged(KWindowSystem::activeWindow());
    } else {
        // Add remaining seconds
        updateCurrentActivityTime();

        // Reset current item and stop the timer
        d->currentActiveWindow = QString();
        d->currentTime = QTime::currentTime();
        d->timer->stop();
    }
}