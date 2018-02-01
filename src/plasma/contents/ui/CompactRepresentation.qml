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

    property bool horizontalLayout: currentActivityName.height + currentActivityTime.height > parent.height

    Layout.minimumWidth: horizontalLayout ? units.gridUnit * 12 : units.gridUnit * 8
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
        anchors.leftMargin: units.smallSpacing
        height: paintedHeight
        elide: Text.ElideRight
        text: activityModel.currentActivityName
    }

    PlasmaComponents.Label {
        id: currentActivityTime
        anchors.leftMargin: units.smallSpacing
        height: paintedHeight
        elide: Text.ElideRight
        font.pointSize: theme.smallestFont.pointSize
        opacity: 0.6
        text: plasmoid.configuration.show_total_activity_time? activityModel.currentActivityTime + " | Σ = " + activityModel.totalActivityTime : activityModel.currentActivityTime
    }

    PlasmaCore.SvgItem {
        id: separator

        anchors {
            bottom: parent.bottom
            right: currentActivityTime.left
            rightMargin: units.smallSpacing
            top: parent.top
        }
        visible: horizontalLayout
        width: lineSvg.elementSize("vertical-line").width; height: parent.height
        elementId: "vertical-line"
        svg: PlasmaCore.Svg { id: lineSvg; imagePath: "widgets/line" }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: plasmoid.expanded = !plasmoid.expanded
    }

    states: [
        State {
            name: "horizontalLayout"
            when: horizontalLayout
            AnchorChanges {
                target: currentActivityName
                anchors.bottom: undefined
                anchors.left: currentActivityIcon.right
                anchors.right: separator.left
                anchors.verticalCenter: parent.verticalCenter
            }

            AnchorChanges {
                target: currentActivityTime
                anchors.left: undefined
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
            }
        },

        State {
            name: "verticalLayout"
            when: !horizontalLayout
            AnchorChanges {
                target: currentActivityName
                anchors.bottom: currentActivityIcon.verticalCenter
                anchors.left: currentActivityIcon.right
                anchors.right: parent.right
                anchors.verticalCenter: undefined
            }

            AnchorChanges {
                target: currentActivityTime
                anchors.left: currentActivityIcon.right
                anchors.right: parent.right
                anchors.top: currentActivityName.bottom
                anchors.verticalCenter: undefined
            }
        }
    ]
}
