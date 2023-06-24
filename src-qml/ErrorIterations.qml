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
	title: qsTr("Error with respect to iteration")

	Component.onCompleted: {
		actions.updateErrorIter();
	}

	contentData: GroupBox {
		anchors.fill: parent
		anchors.margins: 10
		spacing: 10
		title: qsTr("Error with respect to iteration")

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
				min: 1
				max: 5
				labelFormat: "%i"
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
				function onErrorIterUpdate(errors, maxIter, maxError) {
					series.clear();
					yAxis.max = maxError * 1.05;
					xAxis.max = maxIter;
					for (var i = 0; i < errors.length; ++i) {
						series.append(i + 1, errors[i]);
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
			text: qsTr("Refresh")
			onClicked: {
				actions.updateErrorIter();
			}
		}
		Button {
			text: qsTr("Close")
			onClicked: window.close();
		}
	}
}
