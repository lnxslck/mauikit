/*
 *   Copyright 2018 Camilo Higuita <milo.h@aol.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.0 as Kirigami
import org.kde.mauikit 1.0 as Maui
import "private"

Maui.Item
{
    id: control    
      
    height: iconSizes.big
    
    property string url : ""
    
    signal pathChanged(string path)
    signal homeClicked()
    signal placeClicked(string path)
    
	onUrlChanged: append()
	
    Rectangle
    {
        id: pathBarBG
        anchors.fill: parent
        z: -1
        color: pathEntry.visible ? colorScheme.viewBackgroundColor : colorScheme.backgroundColor
        radius: radiusV
        opacity: 1
        border.color: colorScheme.borderColor
        border.width: unit
    }
    
    RowLayout
    {
        id: pathEntry
        anchors.fill:  parent
        visible: false
        
        Maui.TextField
        {
            id: entry
            text: control.url
            height: parent.height
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.leftMargin: contentMargins
            Layout.alignment: Qt.AlignVCenter
            colorScheme.textColor: control.colorScheme.textColor
            colorScheme.backgroundColor: "transparent"
			colorScheme.borderColor: "transparent"
            horizontalAlignment: Qt.AlignLeft
            onAccepted:
            {
                pathChanged(text)
                showEntryBar()
            }
        }
        
        Item
        {
            Layout.fillHeight: true
            Layout.leftMargin: space.small
            Layout.rightMargin: space.small
            width: iconSize
            
            Maui.ToolButton
            {
                anchors.centerIn: parent
                iconName: "go-next"
                iconColor: control.colorScheme.textColor
                onClicked:
                {
                    pathChanged(entry.text)
                    showEntryBar()
                }
            }
        }
    }
    
    RowLayout
    {
        id: pathCrumbs
        anchors.fill: parent
        spacing: 0
        
        Item
        {
            Layout.fillHeight: true
            Layout.leftMargin: space.small
            Layout.rightMargin: space.small
            width: iconSize
            
            Maui.ToolButton
            {
                anchors.centerIn: parent
                iconName: "go-home"
                iconColor: control.colorScheme.textColor                
                onClicked: homeClicked()
            }
        }
        
        Kirigami.Separator
        {
            Layout.fillHeight: true
            color: colorScheme.borderColor
        }
        
        ListView
        {
            id: pathBarList
            Layout.fillHeight: true
            Layout.fillWidth: true
            
            orientation: ListView.Horizontal
            clip: true
            spacing: 0
            
            focus: true
            interactive: true
            boundsBehavior: isMobile ?  Flickable.DragOverBounds : Flickable.StopAtBounds
            
            model: ListModel{}
            
            delegate: PathBarDelegate
            {
                id: delegate
                height: pathBar.height - (unit*2)
                width: iconSizes.big * 3
                Connections
                {
                    target: delegate
                    onClicked:
                    {
                        pathBarList.currentIndex = index
                        placeClicked(pathBarList.model.get(index).path)
                    }
                }
            }
            
            MouseArea
            {
                anchors.fill: parent
                onClicked: showEntryBar()
                z: -1
            }
        }
        
        Item
        {
            Layout.fillHeight: true
            Layout.leftMargin: space.small
            Layout.rightMargin: space.small
            width: iconSize
            Maui.ToolButton
            {
                anchors.centerIn: parent
                iconName: "filename-space-amarok"
                iconColor: control.colorScheme.textColor                
                onClicked: showEntryBar()
            }
        }
        
        //        MouseArea
        //        {
        //            anchors.fill: parent
        //            propagateComposedEvents: true
        //            onClicked: showEntryBar()
        //        }
        
        
        
    }
    
    function append()
    {
        pathBarList.model.clear()
        var places = control.url.split("/")
        var url = ""
        for(var i in places)
        {
            url = url + places[i] + "/"
            if(places[i].length > 1)
                pathBarList.model.append({label : places[i], path: url})
        }
        
        pathBarList.currentIndex = pathBarList.count-1
        pathBarList.positionViewAtEnd()
    }
     
    function showEntryBar()
    {
        pathEntry.visible = !pathEntry.visible
        pathCrumbs.visible = !pathCrumbs.visible
    }
}
