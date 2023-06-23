import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.15

ApplicationWindow {
	id: window
	width: 640
	height: 480
	visible: true
	title: qsTr("Boundary Panel")

	// Component.onCompleted: {
	// 	actions.drawBurningArea(numberArea.text);
	// }

	contentData: GroupBox {
		anchors.fill: parent
		anchors.margins: 10
		spacing: 10
		title: qsTr("Burning areas")

		ChartView{
			height: parent.height
			width: parent.width
			antialiasing: true
			theme: Material.theme === Material.Dark ? ChartView.ChartThemeDark : ChartView.ChartThemeLight
			backgroundColor: Material.theme === Material.Dark ? Material.color(Material.Grey, Material.Shade900) : Material.color(Material.Grey, Material.Shade50)

			legend.visible: false
			// title: "Burning Area with Time"

			ValueAxis {
				id: xAxis
				min: 0
				max: 1
			}

			ValueAxis {
				id: yAxis
				min: 0
				max: 1
			}


			LineSeries {
				id: series
				axisX: xAxis
				axisY: yAxis
			}

			Connections {
				target: actions
				function onGraphBurningArea(x, y, xMax, yMax) {
					series.clear();
					xAxis.max = xMax;
					yAxis.max = yMax;
					for (var i = 0; i < x.length; ++i) {
						series.append(x[i], y[i]);
					}
				}
			}
		}
	}

	footer: Row {
		width: parent.width
		layoutDirection: Qt.RightToLeft

		rightPadding: 10
		spacing: 10

		Button {
			text: qsTr("Close")
			onClicked: window.close();
		}
		Button {
			text: qsTr("Draw")
			onClicked: {
				actions.drawBurningArea(numberArea.text);
			}
		}
		TextField {
			id: numberArea
			placeholderText: qsTr("Number of areas")
			text: "50"
			validator: IntValidator {
				bottom: 0
			}
		}
		Text {
			topPadding: 5
			text: qsTr("Number of areas")
		}
	}
}
