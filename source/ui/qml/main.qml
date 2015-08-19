/*
 * main.qml
 * Copyright (C) 2014-2015  Michał Garapich <michal@garapich.pl>
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
 * 
 */

import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

ApplicationWindow {
    id: vatsinatorWindow
    objectName: "vatsinatorWindow"
    
    title: "Vatsinator"
    width: 640
    height: 480
    
    MapPage {
        id: mapPage
    }
    
    StackView {
        id: stackView
        anchors.fill: parent
        focus: true
        
        /* Implements back key navigation */
        Keys.onReleased: if (event.key === Qt.Key_Back && stackView.depth > 1) {
                             stackView.pop();
                             event.accepted = true;
                         }
        
        initialItem: mapPage
    }
    
    NavigationDrawer {
        id: navigationDrawer
        objectName: "navigationDrawer"
        
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        position: Qt.LeftEdge
        visualParent: stackView
        
        MainMenu {
            id: mainMenu
            objectName: "mainMenu"
            onClicked: {
                navigationDrawer.hide()
                stackView.push(Qt.resolvedUrl(page))
            }
        }
    }
    
    /* Semi-transparent overlay */
    Rectangle {
        anchors.fill: parent
        color: "#262626"
        opacity: navigationDrawer.panelProgress * 0.6
    }
    
    style: ApplicationWindowStyle {
        background: Rectangle {
            color: "#00ffffff"
        }
    }
    
    Component.onCompleted: {
        visible = true
        android.navigationBarColor = palette.byHue(500)
        android.statusBarColor = palette.byHue(700)
    }
    
}