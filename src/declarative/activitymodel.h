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

#ifndef PLASMA_ACTIVITY_MONITOR_ACTIVITY_MODEL_H
#define PLASMA_ACTIVITY_MONITOR_ACTIVITY_MODEL_H

#include <QAbstractListModel>
#include <QTime>
#include <QTimer>
#include <QWindow>

#include <KWindowSystem>

/*                          ActivityModelItem                              *
 * ----------------------------------------------------------------------- */

class Q_DECL_EXPORT ActivityModelItem : public QObject
{
Q_OBJECT
public:
    explicit ActivityModelItem(QObject* parent = 0);
    virtual ~ActivityModelItem();

    void setActivityIcon(const QPixmap& icon);
    QPixmap activityIcon() const;

    void setActivityName(const QString& name);
    QString activityName() const;

    void setActivityTime(const QTime& time);
    QTime activityTime() const;

    void addSeconds(int secs);

private:
    QPixmap m_activityIcon;
    QString m_activityName;
    QTime m_activityTime;
};

/*                          ActivityModel                                  *
 * ----------------------------------------------------------------------- */

class Q_DECL_EXPORT ActivityModel : public QAbstractListModel
{
Q_OBJECT
Q_PROPERTY(bool trackingEnabled READ trackingEnabled WRITE setTrackingEnabled NOTIFY trackingEnabledChanged)
public:

    explicit ActivityModel(QObject* parent = 0);
    virtual ~ActivityModel();

    enum ItemRole {
        ActivityIconRole = Qt::UserRole + 1,
        ActivityNameRole,
        ActivityTimeRole
    };

    int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    virtual QHash< int, QByteArray > roleNames() const Q_DECL_OVERRIDE;

    bool trackingEnabled() const;
    void setTrackingEnabled(bool enable);

public Q_SLOTS:
    void reset(const QString& activity = QString());

private Q_SLOTS:
    void activeWindowChanged(WId window);
    void updateTime();

Q_SIGNALS:
    void trackingEnabledChanged(bool enabled);

private:
    QList<ActivityModelItem*> m_list;
    QString m_currentActivity;
    QTime m_currentTime;
    QTimer* m_timer;
    bool m_trackingEnabled;

    void updateItem(ActivityModelItem* item);
};

#endif // PLASMA_ACTIVITY_MONITOR_ACTIVITY_MODEL_H
