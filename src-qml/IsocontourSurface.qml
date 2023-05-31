import QtQuick 2.15
import QtQuick.Controls 2.12
// import QtQuick3D 1.15
import QtQuick.Dialogs 1.3
import QtQuick.Scene3D 2.0
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Item {
	Column {
		Button {
			id: button
			text: "Load"
			onClicked: fileDialog.open()
		}
		FileDialog {
			id: fileDialog
			onAccepted: {
				sceneLoader.source = fileDialog.fileUrl
				mainScene3d.forceActiveFocus()
			}
		}
		Scene3D {
			id: mainScene3d
			width: 640
			height: 480
			focus: true
			hoverEnabled: true

			aspects: ["input", "logic","render"]
			cameraAspectRatioMode: Scene3D.AutomaticAspectRatio

			Entity {
				id: sceneRoot

				Camera {
					id: camera
					projectionType: CameraLens.PerspectiveProjection
					fieldOfView: 45
					aspectRatio: 16/9
					nearPlane : 0.1
					farPlane : 100.0
					position: Qt.vector3d( 50, 0, 50)
					upVector: Qt.vector3d( 0, 1, 0)
					viewCenter: Qt.vector3d( 0, 0, 0)
					// rotate the camera so that the positive z-axis points upwards
				}

				OrbitCameraController {
					camera: camera
				}

				components: [
					RenderSettings {
						activeFrameGraph: ForwardRenderer {
							clearColor:"#999999"
							camera: camera
						}
					},
					InputSettings {},
					DirectionalLight {
						id: directionalLight
						// light blue
						color: "#FFFFFF"
						intensity: 0.5
						// the direction of the light is the same as the camera
						worldDirection: camera.viewVector
					}
				]

				Entity {
					id: surface
					components: [
						SceneLoader {
							id: sceneLoader
						}
					]
				}
			}
		}
	}
}
