import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

ScrollView {
	clip: true
	Column {
		spacing: 10
		width: parent.width

		GroupBox {
			title: qsTr("Diffusive flux")
			width: parent.width

			LabelInput {
				width: parent.width
				text: "Weight"
				placeholderText: "Enter a number"
				toolTipText: "."
				objName: "diffusiveWeight"
				defaultInput: "1"
				decimals: true
			}
		}

		GroupBox {
			title: qsTr("Isocontour surface")
			width: parent.width

			ComboBox {
				objectName: "cullingMethod"
				width: parent.width
				currentIndex: 0
				model: [
					"No culling",
					"Backface culling",
					"Frontface culling"
				]
				onCurrentIndexChanged: {
					actions.setCullingMethod(currentIndex)
				}
			}
		}

	}
}

