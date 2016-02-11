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

Q_DECLARE_LOGGING_CATEGORY(PLASMA_ACTIVITY_MONITOR)

Q_LOGGING_CATEGORY(PLASMA_ACTIVITY_MONITOR, "plasma-activity-monitor")

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
      m_trackingEnabled(false)
{
    QLoggingCategory::setFilterRules(QStringLiteral("plasma-activity-monitor.debug = false"));

    // Process the currently active window
    activeWindowChanged(KWindowSystem::activeWindow());

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &ActivityModel::activeWindowChanged, Qt::UniqueConnection);
    connect(m_timer, &QTimer::timeout, this, &ActivityModel::updateTime);
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

bool ActivityModel::trackingEnabled() const
{
    return m_trackingEnabled;
}

void ActivityModel::setTrackingEnabled(bool enable)
{
    // Process the currently active window
    activeWindowChanged(KWindowSystem::activeWindow());

    m_trackingEnabled = enable;

    Q_EMIT trackingEnabledChanged(m_trackingEnabled);
}

void ActivityModel::reset(const QString& activity)
{
    Q_FOREACH (ActivityModelItem * item, m_list) {
        if (activity.isEmpty() || (!activity.isEmpty() && activity == item->activityName())) {
            item->setActivityTime(QTime(0, 0, 0));
            updateItem(item);
        }
    }

    m_currentActivity = QString();
    m_currentTime = QTime::currentTime();

    // Process the currently active window
    activeWindowChanged(KWindowSystem::activeWindow());
}

void ActivityModel::activeWindowChanged(WId window)
{
    KWindowInfo info = KWindowInfo(window, NET::WMName | NET::WMIconName, NET::WM2WindowClass);

    qCDebug(PLASMA_ACTIVITY_MONITOR) << "Active window changed to " << info.windowClassName();

    if (info.windowClassName().isEmpty()) {
        return;
    }

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

    if (!m_trackingEnabled) {
        // Update previous activity
        Q_FOREACH (ActivityModelItem * item, m_list) {
            if (m_currentActivity == item->activityName()) {
                item->addSeconds(m_currentTime.secsTo(QTime::currentTime()));
                updateItem(item);
            }
        }

        // Start timer to update the time every minute
        m_timer->stop();
        m_timer->start(60000);
    }

    // Save current time and activity
    m_currentActivity = info.windowClassName();
    m_currentTime = QTime::currentTime();
}

void ActivityModel::updateTime()
{
    Q_FOREACH (ActivityModelItem * item, m_list) {
        if (m_currentActivity == item->activityName()) {
            item->addSeconds(60);
            updateItem(item);
        }
    }
    m_currentTime = m_currentTime.addSecs(60);
    m_timer->start(60000);
}

void ActivityModel::updateItem(ActivityModelItem* item)
{
    const int row = m_list.indexOf(item);

    if (row >= 0) {
        QModelIndex index = createIndex(row, 0);
        Q_EMIT dataChanged(index, index);
    }
}