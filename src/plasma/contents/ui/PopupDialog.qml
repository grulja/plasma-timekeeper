/***************************************************************************
 *   Copyright (C) 2016 by Jan Grulich <jgrulich@redhat.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.activitymonitor 0.2 as PlasmaActivityMonitor

FocusScope {

    PlasmaActivityMonitor.ActivityModel {
        id: activityModel
    }

    PlasmaActivityMonitor.ActivitySortModel {
        id: activitySortModel
        sourceModel: activityModel
    }

    PlasmaExtras.ScrollArea {
        id: connectionScrollView

        anchors {
            bottom: enableTrackingButton.top
            bottomMargin: Math.round(units.gridUnit / 3)
            left: parent.left
            right: parent.right
            top: parent.top
        }

        ListView {
            id: activityList

            anchors.fill: parent
            clip: true
            model: activitySortModel
            boundsBehavior: Flickable.StopAtBounds
            delegate: ActivityItem { }
        }
    }

    PlasmaComponents.Button {
        id: enableTrackingButton

        property bool trackingEnabled: activityModel.trackingEnabled

        anchors {
            bottom: parent.bottom
            bottomMargin: Math.round(units.gridUnit / 3)
            left: parent.left
            leftMargin: Math.round(units.gridUnit / 3)
        }
        width: parent.width / 2

        iconSource: trackingEnabled ? "media-record" : "media-playback-stop"
        text: trackingEnabled ? i18n("Start tracking") : i18n("Stop tracking")

        onClicked: {
            trackingEnabled = !trackingEnabled
            activityModel.trackingEnabled = trackingEnabled
        }
    }

    PlasmaComponents.Button {
        id: resetTrackingButton

        anchors {
            bottom: parent.bottom
            bottomMargin: Math.round(units.gridUnit / 3)
            left: enableTrackingButton.right
            leftMargin: Math.round(units.gridUnit / 3)
            right: parent.right
            rightMargin: Math.round(units.gridUnit / 3)
        }
        width: parent.width / 2

        iconSource: "view-refresh"
        text: i18n("Reset")

        onClicked: {
            activityModel.reset()
        }
    }
}