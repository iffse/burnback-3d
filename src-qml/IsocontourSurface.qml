import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick3D 1.15

Item {
	View3D {
		id: view
		anchors.fill: parent
		camera: camera
		renderMode: View3D.Overlay

		PerspectiveCamera {
			id: camera
			position: Qt.vector3d(0, 200, 300)
			eulerRotation.x: -30
		}

		DirectionalLight {
			eulerRotation.x: -30
		}

		Model {
			//! [3dcube]
			id: cube
			visible: true
			position: Qt.vector3d(0, 0, 0)
			source: "#Cube"
			materials: [ DefaultMaterial {
				diffuseColor: "red"
			}
		]
		eulerRotation.y: 90

		SequentialAnimation on eulerRotation {
			loops: Animation.Infinite
			PropertyAnimation {
				duration: 5000
				from: Qt.vector3d(0, 0, 0)
				to: Qt.vector3d(360, 0, 360)
			}
		}
	}
}

}
