import QtQuick 2.15
import QtQuick.Controls 2.12
import QtCharts 2.15
import QtQuick.Controls.Material 2.15


Item {
	Text {
		id: text
		text: "Hello World"
	}
	// ChartView {
	// 	anchors.fill: parent
	// 	antialiasing: true
	// 	id: chart
	// 	// set backgroundcolor to match the theme
	// 	theme: Material.theme === Material.Dark ? ChartView.ChartThemeDark : ChartView.ChartThemeLight
	// 	backgroundColor: Material.theme === Material.Dark ? Material.color(Material.Grey, Material.Shade900) : Material.color(Material.Grey, Material.Shade50)

	// 	legend.visible: false

	// 	ValueAxis {
	// 		id: xAxis
	// 		min: 0
	// 		max: 1
	// 	}

	// 	ValueAxis {
	// 		id: yAxis
	// 		min: 0
	// 		max: 1
	// 	}


	// 	LineSeries {
	// 		id: series
	// 		axisX: xAxis
	// 		axisY: yAxis

	// 		XYPoint { x: 0; y: 0 }

	// 	}

	// }
}
