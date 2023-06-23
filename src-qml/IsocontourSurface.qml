import QtQuick 2.15
import QtQuick.Controls 2.12
// import QtQuick3D 1.15
import QtQuick.Dialogs 1.3
import QtQuick.Scene3D 2.0
import Qt3D.Core 2.0
import Qt3D.Render 2.5
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Item {
	Connections {
		target: actions
		function onSetCulling(method) {
			switch (method) {
				case 0:
					cullFace.mode = CullFace.NoCulling
					break
				case 1:
					cullFace.mode = CullFace.Back
					break
				case 2:
					cullFace.mode = CullFace.Front
					break
			}
		}
		function onSetCameraPosition(x, y, z) {
			camera.position = Qt.vector3d(x, z, y)
			camera.viewCenter = Qt.vector3d(0, 0, 0)
			camera.upVector = Qt.vector3d(0, 1, 0)
		}
		function onLoadMeshPreview(url) {
			sceneLoader.source = url
		}
	}
	Scene3D {
		id: mainScene3d
		anchors.fill: parent
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
				farPlane : 5000.0
				position: Qt.vector3d(10, 0, 10)
				upVector: Qt.vector3d(0, 1, 0)
				viewCenter: Qt.vector3d(0, 0, 0)
				// rotate the camera so that the positive z-axis points upwards
			}

			OrbitCameraController {
				camera: camera
			}

			components: [
				RenderSettings {
					activeFrameGraph: RenderSurfaceSelector {
						ClearBuffers {
							buffers : ClearBuffers.ColorDepthBuffer
							clearColor: "transparent"

							CameraSelector {
								camera: camera
								RenderStateSet {
									renderStates: [
										CullFace {
											id: cullFace
											mode: CullFace.NoCulling
										}
									]
								}
							}
						}
					}


					// ForwardRenderer {
					// 	clearColor: "transparent"
					// 	camera: camera
					// }
				},
				InputSettings {},
				DirectionalLight {
					id: directionalLight
					color: "#FFFFFF"
					intensity: 1
					// the direction of the light is the same as the camera
					worldDirection: camera.viewVector
				}
			]
			Entity {
				components: [
					SceneLoader {
						id: sceneLoader
					}
				]
			}
		}
	}
}
