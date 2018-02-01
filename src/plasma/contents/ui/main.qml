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
import org.kde.plasma.timekeeper 0.2 as PlasmaTimekeeper

Item {
    id: mainWindow

    Plasmoid.icon: "utilities-system-monitor"
    Plasmoid.toolTipMainText: activityModel.currentActivityName
    Plasmoid.toolTipSubText:  activityModel.currentActivityTime
    Plasmoid.switchWidth: units.gridUnit * 12
    Plasmoid.switchHeight: units.gridUnit * 12

    PlasmaTimekeeper.ActivityModel {
        id: activityModel
        resetOnSuspend: plasmoid.configuration.reset_on_suspend
        resetOnShutdown: plasmoid.configuration.reset_on_shutdown
    }

    PlasmaTimekeeper.ActivitySortModel {
        id: activitySortModel
        sourceModel: activityModel
    }

    Plasmoid.compactRepresentation: CompactRepresentation { }
    Plasmoid.fullRepresentation: PopupDialog { }
}
