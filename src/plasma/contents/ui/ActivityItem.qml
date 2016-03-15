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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

PlasmaComponents.ListItem {
    id: activityItem

    enabled: true
    checked: activityItem.containsMouse
    height: activityName.height + activityTime.height + Math.round(units.gridUnit / 2)

    KQuickControlsAddons.QPixmapItem {
        id: activityIcon

        anchors {
            left: parent.left
            leftMargin: Math.round(units.gridUnit / 3)
            verticalCenter: parent.verticalCenter
        }
        pixmap: ActivityIcon
        height: parent.height; width: height
    }

    PlasmaComponents.Label {
        id: activityName

        anchors {
            bottom: activityIcon.verticalCenter
            left: activityIcon.right
            leftMargin: Math.round(units.gridUnit / 2)
            right: ignoreActivityButton.left
        }
        height: paintedHeight
        elide: Text.ElideRight
        font.weight: ActivityName == activityModel.currentActivityName ? Font.DemiBold : Font.Normal
        text: ActivityName
    }

    PlasmaComponents.Label {
        id: activityTime

        anchors {
            left: activityIcon.right
            leftMargin: Math.round(units.gridUnit / 2)
            right: ignoreActivityButton.left
            top: activityName.bottom
        }
        height: paintedHeight
        elide: Text.ElideRight
        font.pointSize: theme.smallestFont.pointSize
        opacity: 0.6
        text: i18n("%1 (%2%)", ActivityTime, ActivityPercentualUsage)
    }

    PlasmaComponents.ToolButton {
        id: ignoreActivityButton

        anchors {
            right: parent.right
            rightMargin: Math.round(units.gridUnit / 3)
            verticalCenter: parent.verticalCenter
        }
        iconSource: "list-remove"
        tooltip: i18n("Stop monitoring this activity")
        opacity: activityList.currentVisibleButtonIndex == index && ActivityName != i18n("other applications") ? 1 : 0
        visible: opacity != 0

        onClicked: {
            activityModel.ignoreActivity(ActivityName)
        }
    }

    onContainsMouseChanged: {
        if (activityItem.containsMouse) {
            activityList.currentVisibleButtonIndex = index
        }
    }
}
