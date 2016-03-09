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
import QtQuick.Controls 1.3
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: iconsPage
    width: childrenRect.width
    height: childrenRect.height
    implicitWidth: pageColumn.implicitWidth
    implicitHeight: pageColumn.implicitHeight

    property alias cfg_reset_on_suspend: resetOnSuspendCheckbox.checked
    property alias cfg_reset_on_shutdown: resetOnShutdownCheckbox.checked

    Label {
        id: label
        anchors {
            left: parent.left
            top: parent.top
        }
        text: i18n("Reset statistics:")
    }

    Column {
        id: pageColumn
        anchors {
            left: parent.left
            top: label.bottom
            topMargin: Math.round(units.gridUnit / 3)
        }
        spacing: units.smallSpacing

        CheckBox {
            id: resetOnSuspendCheckbox
            text: i18n("On suspend or hibernation")
        }

        CheckBox {
            id: resetOnShutdownCheckbox
            text: i18n("On shutdown or restart")
        }
    }
}
