/* -*- coding: utf-8-unix -*-
 *
 * Copyright (C) 2018-2020 Rinigus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.KeepAlive 1.2
import "."

ApplicationWindow {
    allowedOrientations: defaultAllowedOrientations
    _defaultPageOrientations: defaultAllowedOrientations // required to fix issue #219
    cover: Cover {}
    initialPage: null

    property real   compassOrientationOffset: 0
    property var    pages: StackPL { }
    property bool   running: applicationActive || (cover && cover.active) || keepAliveBackground
    property int    screenHeight: Screen.height
    property bool   screenLarge: Screen.sizeCategory >= Screen.Large
    property int    screenWidth: Screen.width
    property string title
    property bool   keepAlive: false
    property bool   keepAliveBackground: false

    DisplayBlanking {
        preventBlanking: applicationActive && keepAlive
    }

    KeepAlive {
        enabled: keepAliveBackground
    }

    Component.onCompleted: {
        //pages.ps = pageStack;
        //updateOrientation()
    }

    function clearPages() {
        // not used in the platforms with menu shown
        // as a page in a stack
    }

    function initPages() {
    }
}
