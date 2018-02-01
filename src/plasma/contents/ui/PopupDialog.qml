/***************************************************************************
 *   Copyright (C) 2016-2018 by Jan Grulich <jgrulich@redhat.com>          *
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
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.timekeeper 0.2 as PlasmaTimekeeper

Item {
    id: fullRepresentation

    Layout.minimumWidth: units.gridUnit * 12
    Layout.minimumHeight: units.gridUnit * 12

    PlasmaExtras.ScrollArea {
        id: connectionScrollView

        anchors {
            bottom: buttonRow.top
            bottomMargin: Math.round(units.gridUnit / 3)
            left: parent.left
            right: parent.right
            top: parent.top
        }

        ListView {
            id: activityList

            property int currentVisibleButtonIndex: -1

            anchors.fill: parent
            clip: true
            model: activitySortModel
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            delegate: ActivityItem { }
        }
    }

    Row {
        id: buttonRow

        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        spacing: units.smallSpacing

        PlasmaComponents.Button {
            id: enableTrackingButton
            property bool timeTrackingEnabled: activityModel.timeTrackingEnabled
            iconSource: timeTrackingEnabled ? "media-playback-stop" : "media-record"
            text: timeTrackingEnabled ? i18n("Stop") : i18n("Start")

            onClicked: {
                timeTrackingEnabled = !timeTrackingEnabled
                activityModel.timeTrackingEnabled = timeTrackingEnabled
            }
        }

        PlasmaComponents.Button {
            id: resetTrackingButton
            iconSource: "view-refresh"
            text: i18n("Reset")

            onClicked: {
                activityModel.resetTimeStatistics()
            }
        }
    }
}
