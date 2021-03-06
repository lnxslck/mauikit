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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import org.kde.mauikit 1.0 as Maui
import org.kde.kirigami 2.0 as Kirigami
import "private"

MenuItem
{
	id: control	
	/* Controlc color scheming */
	ColorScheme {id: colorScheme}
	property alias colorScheme : colorScheme
	/***************************/
	
	width: parent.width
	height: control.visible ? rowHeight : 0
	spacing: space.medium
	font.pointSize: fontSizes.default
	
	icon.width: iconSizes.medium
	icon.height: iconSizes.medium
	
	contentItem: RowLayout
	{
		anchors.fill: control
		anchors.leftMargin: control.checkable ? control.indicator.width + control.spacing + space.big : control.spacing
		anchors.rightMargin: control.spacing
		
		Item
		{
			Layout.fillHeight: true
			Layout.alignment: Qt.AlignVCenter
			visible: control.icon.name.length
			width: control.icon.name.length ? control.icon.width : 0
			
			Maui.ToolButton
			{
				id: _controlIcon
				visible: parent.visible
				anchors.centerIn: parent
				iconName: control.icon.name
				size: control.icon.height
				isMask: true
				enabled: false
				iconColor: _controlLabel.color
			}
		}
		
		Item
		{
			Layout.fillHeight: true
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignVCenter
			Layout.leftMargin: _controlIcon.visible ? space.medium : 0
			
			Label
			{
				id: _controlLabel
				visible: control.text
				height: parent.height
				width: parent.width
				verticalAlignment:  Qt.AlignVCenter
				horizontalAlignment: Qt.AlignLeft
				
				text: control.action ? control.action.text : control.text
				font: control.font
				elide: Text.ElideRight
				
				color: control.hovered && !control.pressed ? colorScheme.highlightedTextColor : colorScheme.textColor
			}
		}
	}	
	
	indicator: Rectangle
	{
		x: control.mirrored ? control.width - width - control.rightPadding : control.spacing
		y: control.topPadding + (control.availableHeight - height) / 2
		
		height: iconSizes.medium
		width: height		
	
		visible: control.checkable
		
		color: control.checked ? control.colorScheme.highlightColor : control.colorScheme.viewBackgroundColor
		border.color: control.colorScheme.borderColor
		radius: radiusV
    }

	background: Item
	{
		anchors.fill: parent
		implicitWidth: Kirigami.Units.gridUnit * 8
		implicitHeight: rowHeight
		
		Rectangle 
		{
			anchors.fill: parent
			color: colorScheme.highlightColor
			opacity: control.hovered && !control.pressed ? 1 : 0
			Behavior on opacity { NumberAnimation { duration: 150 } }
		}
	}	
}
