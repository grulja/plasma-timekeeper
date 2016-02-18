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

#include <KLocalizedString>
#include <KWindowSystem>

#include <QLoggingCategory>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>

Q_DECLARE_LOGGING_CATEGORY(PLASMA_ACTIVITY_MONITOR)

Q_LOGGING_CATEGORY(PLASMA_ACTIVITY_MONITOR, "plasma-timekeeper")

/*                          ActivityModelItem                              *
 * ----------------------------------------------------------------------- */

ActivityModelItem::ActivityModelItem(QObject* parent)
    : QObject(parent)
{
}

ActivityModelItem::~ActivityModelItem()
{
}

void ActivityModelItem::setActivityIcon(const QPixmap& icon)
{
    m_activityIcon = icon;
}

QPixmap ActivityModelItem::activityIcon() const
{
    return m_activityIcon;
}

void ActivityModelItem::setActivityName(const QString& name)
{
    m_activityName = name;
}

QString ActivityModelItem::activityName() const
{
    return m_activityName;
}

void ActivityModelItem::setActivityTime(const QTime& time)
{
    m_activityTime = time;
}

QTime ActivityModelItem::activityTime() const
{
    return m_activityTime;
}

void ActivityModelItem::addSeconds(int secs)
{
    m_activityTime = m_activityTime.addSecs(secs);
}

/*                          ActivityModel                                  *
 * ----------------------------------------------------------------------- */

ActivityModel::ActivityModel(QObject* parent)
    : QAbstractListModel(parent),
      m_timer(new QTimer(this)),
      m_timeTrackingEnabled(true),
      m_screenLocked(false)
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
            m_screenLocked = reply.value();
        }
    });

    // Process the currently active window
    activeWindowChanged(KWindowSystem::activeWindow());

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &ActivityModel::activeWindowChanged, Qt::UniqueConnection);
    connect(m_timer, &QTimer::timeout, this, &ActivityModel::updateCurrentActivityTime);

    QDBusConnection::sessionBus().connect(QStringLiteral("org.kde.ksmserver"),
                                          QStringLiteral("/ScreenSaver"),
                                          QStringLiteral("org.freedesktop.ScreenSaver"),
                                          QStringLiteral("ActiveChanged"),
                                          this,
                                          SLOT(lockscreenActivityChanged(bool)));
}

ActivityModel::~ActivityModel()
{
}

QVariant ActivityModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();

    if (row >= 0 && row < m_list.count()) {
        ActivityModelItem * item = m_list.at(row);

        switch (role) {
            case ActivityIconRole:
                return item->activityIcon();
                break;
            case ActivityNameRole:
                return item->activityName();
                break;
            case ActivityTimeRole:
                return item->activityTime().toString(QLatin1String("hh:mm:ss"));
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
    return m_list.count();
}

QHash< int, QByteArray > ActivityModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[ActivityIconRole] = "ActivityIcon";
    roles[ActivityNameRole] = "ActivityName";
    roles[ActivityTimeRole] = "ActivityTime";

    return roles;
}

bool ActivityModel::timeTrackingEnabled() const
{
    return m_timeTrackingEnabled;
}

void ActivityModel::setTimeTrackingEnabled(bool enabled)
{
    m_timeTrackingEnabled = enabled;
    Q_EMIT timeTrackingEnabledChanged(m_timeTrackingEnabled);

    stopTracking(!m_timeTrackingEnabled);
}

void ActivityModel::resetTimeStatistics()
{
    Q_FOREACH (ActivityModelItem * item, m_list) {
        item->setActivityTime(QTime(0, 0, 0));
        updateItem(item);
    }

    resetCurrentActiveWindow();
    activeWindowChanged(KWindowSystem::activeWindow());
}

void ActivityModel::activeWindowChanged(WId window)
{
    KWindowInfo info = KWindowInfo(window, NET::WMName | NET::WMIconName, NET::WM2WindowClass);

    qCDebug(PLASMA_ACTIVITY_MONITOR) << "Active window changed to " << info.windowClassName();

    if (info.windowClassName().isEmpty()) {
        return;
    }

    if (!m_timeTrackingEnabled) {
        qCDebug(PLASMA_ACTIVITY_MONITOR) << "Monitoring disabled, ignoring new active window";
        return;
    }

    // Find if the activity already exists and if not add it to the model
    auto activityExist = std::find_if(m_list.constBegin(), m_list.constEnd(), [info] (ActivityModelItem * activityItem) {
        return activityItem->activityName() == info.windowClassName();
    });

    if (activityExist == m_list.constEnd()) {
        qCDebug(PLASMA_ACTIVITY_MONITOR) << "Adding new activity item " << info.windowClassName();
        ActivityModelItem *item = new ActivityModelItem();
        item->setActivityName(info.windowClassName());
        item->setActivityIcon(KWindowSystem::icon(window, 64, 64, true));
        item->setActivityTime(QTime(0, 0, 0));

        const int index = m_list.count();
        beginInsertRows(QModelIndex(), index, index);
        m_list << item;
        endInsertRows();
    }

    // Process the current activity
    updateCurrentActivityTime();

    // Process the next activity
    if (!m_timeTrackingEnabled && !m_screenLocked) {
        // Start timer to update the time every minute
        m_timer->stop();
        // TODO make this configurable
        m_timer->start(60000);
    }

    // Save current time and activity
    m_currentActiveWindow = info.windowClassName();
    m_currentTime = QTime::currentTime();
}

void ActivityModel::lockscreenActivityChanged(bool active)
{
    m_screenLocked = active;

    stopTracking(m_screenLocked);
}

void ActivityModel::updateCurrentActivityTime()
{
    Q_FOREACH (ActivityModelItem * item, m_list) {
        if (m_currentActiveWindow == item->activityName()) {
            item->addSeconds(m_currentTime.secsTo(QTime::currentTime()));
            updateItem(item);
        }
    }

    m_currentTime = QTime::currentTime();
    m_timer->start(60000);
}

void ActivityModel::stopTracking(bool stop)
{
    if (stop) {
        // Add remaining seconds
        updateCurrentActivityTime();
        // Reset current item and stop the timer
        resetCurrentActiveWindow();
    } else {
        // Start again with current active window
        activeWindowChanged(KWindowSystem::activeWindow());
    }
}

void ActivityModel::resetCurrentActiveWindow()
{
    // Reset current activity and time
    m_currentActiveWindow = QString();
    m_currentTime = QTime::currentTime();
    m_timer->stop();
}

void ActivityModel::updateItem(ActivityModelItem* item)
{
    const int row = m_list.indexOf(item);

    if (row >= 0) {
        QModelIndex index = createIndex(row, 0);
        Q_EMIT dataChanged(index, index);
    }
}