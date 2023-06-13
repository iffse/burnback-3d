import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

Item {
	RowLayout {
		anchors.fill: parent
		Slider {
			objectName: "isosurfaceSlider"
			id: slider
			Layout.fillWidth: true
			from: 0
			value: 0.5
			to: 1
			onPressedChanged: if (!pressed) actions.updateIsosurface(value)
			onValueChanged: label.text = value.toFixed(4)
		}
		Text {
			id: label
			text: "Drag the slider to see the surface at corresponding time"
			horizontalAlignment: Text.AlignHCenter
			clip: true
		}
	}

}
