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

#ifndef PLASMA_TIMEKEEPER_ACTIVITY_MODEL_H
#define PLASMA_TIMEKEEPER_ACTIVITY_MODEL_H

#include <QAbstractListModel>
#include <QTime>
#include <QTimer>
#include <QWindow>

#include <KWindowSystem>

/*                          ActivityModelItem                              *
 * ----------------------------------------------------------------------- */

class ActivityModelItem : public QObject
{
Q_OBJECT
public:
    explicit ActivityModelItem(QObject* parent = 0);
    virtual ~ActivityModelItem();

    void setActivityIcon(const QPixmap& icon);
    QPixmap activityIcon() const;

    void setActivityDefaultIcon(const QPixmap& icon);
    QPixmap activityDefaultIcon() const;

    void setActivityName(const QString& name);
    QString activityName() const;

    void setActivityTime(const QTime& time);
    QTime activityTime() const;

    void addSeconds(int secs);

private:
    class Private;
    Private *const d;
};

/*                          ActivityModel                                  *
 * ----------------------------------------------------------------------- */

class ActivityModel : public QAbstractListModel
{
Q_OBJECT
Q_PROPERTY(QPixmap currentActivityIcon READ currentActivityIcon NOTIFY currentActivityChanged)
Q_PROPERTY(QString currentActivityName READ currentActivityName NOTIFY currentActivityChanged)
Q_PROPERTY(QString currentActivityTime READ currentActivityTime NOTIFY currentActivityChanged)
Q_PROPERTY(bool timeTrackingEnabled READ timeTrackingEnabled WRITE setTimeTrackingEnabled NOTIFY timeTrackingEnabledChanged)
Q_PROPERTY(bool resetOnSuspend WRITE setResetOnSuspend)
Q_PROPERTY(bool resetOnShutdown WRITE setResetOnShutdown)
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

    QPixmap currentActivityIcon() const;
    QString currentActivityName() const;
    QString currentActivityTime() const;

    bool timeTrackingEnabled() const;
    void setTimeTrackingEnabled(bool enabled);

    void setResetOnSuspend(bool reset);
    void setResetOnShutdown(bool reset);

public Q_SLOTS:
    void inhibit();
    void uninhibit();
    void resetTimeStatistics();

private Q_SLOTS:
    void activeWindowChanged(WId window);
    void lockscreenActivityChanged(bool active);
    void prepareForSleepChanged(bool sleep);
    void prepareForShutdownChanged(bool shutdown);
    void updateCurrentActivityTime();
    void updateTrackingState();

Q_SIGNALS:
    void currentActivityChanged();
    void timeTrackingEnabledChanged(bool enabled);

private:
    class Private;
    Private *const d;
};

#endif // PLASMA_TIMEKEEPER_ACTIVITY_MODEL_H
