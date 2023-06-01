import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

Item {
	GroupBox {
		id: results
		title: qsTr("Results")
		anchors.fill: parent
		ColumnLayout {
			anchors.fill: parent
			IsocontourSurface {
				id: isocontourSurface
				Layout.fillWidth: true
				Layout.fillHeight: true
			}
			LabeledSlider {
				id: isocontourSlider
				Layout.fillWidth: true
				Layout.preferredHeight: 50
			}
		}
	}
}
