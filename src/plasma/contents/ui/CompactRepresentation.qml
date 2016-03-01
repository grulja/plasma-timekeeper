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
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Item {
    id: compactRepresentation

    Layout.minimumWidth: units.gridUnit * 8
    Layout.minimumHeight: units.iconSizes.small

    KQuickControlsAddons.QPixmapItem {
        id: currentActivityIcon

        anchors {
            bottom: parent.bottom
            bottomMargin: units.smallSpacing
            left: parent.left
            leftMargin: units.smallSpacing
            top: parent.top
            topMargin: units.smallSpacing
        }
        pixmap: activityModel.currentActivityIcon
        height: parent.height; width: height
    }

    PlasmaComponents.Label {
        id: currentActivityName

        anchors {
            bottom: currentActivityIcon.verticalCenter
            left: currentActivityIcon.right
            leftMargin: units.smallSpacing
            right: parent.right
        }
        height: paintedHeight
        elide: Text.ElideRight
        text: activityModel.currentActivityName
    }

    PlasmaComponents.Label {
        id: currentActivityTime

        anchors {
            left: currentActivityIcon.right
            leftMargin: units.smallSpacing
            right: parent.right
            top: currentActivityName.bottom
        }
        height: paintedHeight
        elide: Text.ElideRight
        font.pointSize: theme.smallestFont.pointSize
        opacity: 0.6
        text: activityModel.currentActivityTime
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: plasmoid.expanded = !plasmoid.expanded
    }
}