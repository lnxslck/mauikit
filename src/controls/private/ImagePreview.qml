import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

ColumnLayout
{
	id: layout
	anchors.fill: parent
	
	Item
	{
		Layout.fillWidth: true
		Layout.fillHeight: true
		Layout.margins: 0
		Layout.maximumHeight: parent.height * 0.7
		Layout.preferredHeight: parent.height * 0.7
		Layout.minimumHeight: parent.height * 0.7
		Image
		{
			anchors.centerIn: parent
			horizontalAlignment: Qt.AlignHCenter
			verticalAlignment: Qt.AlignVCenter
			width: parent.width
			height: parent.height * 0.9
			source: "file://"+currentUrl
			fillMode: Image.PreserveAspectCrop
			asynchronous: true
			sourceSize.height: height
			sourceSize.width: width
		}
		
		
	}
	
	
	Item
	{
		Layout.fillWidth: true
		Layout.fillHeight: true
		
		ScrollView
		{
			anchors.fill: parent
			
			contentHeight: _columnInfo.implicitHeight
			
			ColumnLayout
			{
				id: _columnInfo
				width: parent.width
				spacing: space.large
				// 			spacing: rowHeight
				Label
				{				
					text: iteminfo.name
					Layout.fillWidth: true
					horizontalAlignment: Qt.AlignHCenter
					verticalAlignment: Qt.AlignVCenter
					elide: Qt.ElideRight
					wrapMode: Text.Wrap
					font.pointSize: fontSizes.big
					font.weight: Font.Bold
					font.bold: true
				}
				
				Column
				{
					Layout.fillWidth: true
					spacing: space.small
					Label
					{
						text: qsTr("Type")
						font.pointSize: fontSizes.default
						font.weight: Font.Light
						
					}
					
					Label
					{							 
						horizontalAlignment: Qt.AlignHCenter
						verticalAlignment: Qt.AlignVCenter
						elide: Qt.ElideRight
						wrapMode: Text.Wrap
						font.pointSize: fontSizes.big
						font.weight: Font.Bold
						font.bold: true
						text: iteminfo.mime
					}
				}
				
				Column
				{
					Layout.fillWidth: true
					spacing: space.small
					Label
					{
						text: qsTr("Date")
						font.pointSize: fontSizes.default
						font.weight: Font.Light
						
					}
					
					Label
					{							 
						horizontalAlignment: Qt.AlignHCenter
						verticalAlignment: Qt.AlignVCenter
						elide: Qt.ElideRight
						wrapMode: Text.Wrap
						font.pointSize: fontSizes.big
						font.weight: Font.Bold
						font.bold: true
						text: iteminfo.date
					}
				}
				
				
				Column
				{
					Layout.fillWidth: true
					spacing: space.small
					Label
					{
						text: qsTr("Modified")
						font.pointSize: fontSizes.default
						font.weight: Font.Light
						
					}
					
					Label
					{							 
						horizontalAlignment: Qt.AlignHCenter
						verticalAlignment: Qt.AlignVCenter
						elide: Qt.ElideRight
						wrapMode: Text.Wrap
						font.pointSize: fontSizes.big
						font.weight: Font.Bold
						font.bold: true
						text: iteminfo.modified
					}
				}
				
				Column
				{
					Layout.fillWidth: true
					spacing: space.small
					Label
					{
						text: qsTr("Owner")
						font.pointSize: fontSizes.default
						font.weight: Font.Light
						
					}
					
					Label
					{							 
						horizontalAlignment: Qt.AlignHCenter
						verticalAlignment: Qt.AlignVCenter
						elide: Qt.ElideRight
						wrapMode: Text.Wrap
						font.pointSize: fontSizes.big
						font.weight: Font.Bold
						font.bold: true
						text: iteminfo.owner
					}
				}
				
				Column
				{
					Layout.fillWidth: true
					spacing: space.small
					Label
					{
						text: qsTr("Tags")
						font.pointSize: fontSizes.default
						font.weight: Font.Light
						
					}
					
					Label
					{							 
						horizontalAlignment: Qt.AlignHCenter
						verticalAlignment: Qt.AlignVCenter
						elide: Qt.ElideRight
						wrapMode: Text.Wrap
						font.pointSize: fontSizes.big
						font.weight: Font.Bold
						font.bold: true
						text: iteminfo.tags
					}
				}
				
				Column
				{
					Layout.fillWidth: true
					spacing: space.small
					Label
					{
						text: qsTr("Permissions")
						font.pointSize: fontSizes.default
						font.weight: Font.Light
					}
					
					Label
					{							 
						horizontalAlignment: Qt.AlignHCenter
						verticalAlignment: Qt.AlignVCenter
						elide: Qt.ElideRight
						wrapMode: Text.Wrap
						font.pointSize: fontSizes.big
						font.weight: Font.Bold
						font.bold: true
						text: iteminfo.permissions
					}
				}
			}
			
		}
	}
	
}

