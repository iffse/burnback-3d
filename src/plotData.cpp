#include <cmath>
#include <src/headers/plotData.h>
#include <src/headers/globals.h>
#include <src/headers/operations.h>

using namespace std;
using namespace Vectors;

// lerp is available since c++20
// as time of writing, cpp20 has only partial support by the compilers
// therefore cpp17 is expected to be used
#if __cplusplus < 202002L
double lerp(double x1, double x2, double t) {
	return x1 + t * (x2 - x1);
}
#endif


IsocontourData isosurfaceData(double value) {
	IsocontourData data;
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		auto &_nodes = mesh.tetrahedra[tetrahedra];
		array<uint, 4> nodes;
		for (uint i = 0; i < 4; ++i)
			nodes[i] = _nodes[i] - 1;

		vector<array<double, 3>> intersectionPoints;

		// 4 vertices and 2 nodes, the combination is 6
		for (int i = 0; i < 4; ++i) {
			for (int j = i + 1; j < 4; ++j) {
				auto uVertex1 = computationData.uVertex[nodes[i]];
				auto uVertex2 = computationData.uVertex[nodes[j]];

				auto value1 = uVertex1 - value;
				auto value2 = uVertex2 - value;

				if (value1 * value2 < 0) {
					auto &node1 = mesh.nodes[nodes[i]];
					auto &node2 = mesh.nodes[nodes[j]];

					auto &x1 = node1[0];
					auto &y1 = node1[1];
					auto &z1 = node1[2];

					auto &x2 = node2[0];
					auto &y2 = node2[1];
					auto &z2 = node2[2];

					double uV1, uV2, ux1, ux2, uy1, uy2, uz1, uz2;

					// sort the vertices
					// so when reversing the order, the result is the same
					if (value1 < 0) {
						uV1 = uVertex1;
						uV2 = uVertex2;

						ux1 = x1;
						ux2 = x2;

						uy1 = y1;
						uy2 = y2;

						uz1 = z1;
						uz2 = z2;
					} else {
						uV1 = uVertex2;
						uV2 = uVertex1;

						ux1 = x2;
						ux2 = x1;

						uy1 = y2;
						uy2 = y1;

						uz1 = z2;
						uz2 = z1;
					}

					auto t = (value - uV1) / (uV2 - uV1);

					auto x = lerp(ux1, ux2, t);
					auto y = lerp(uy1, uy2, t);
					auto z = lerp(uz1, uz2, t);

					intersectionPoints.push_back({ x, y, z });
				}
			}
		}

		switch (intersectionPoints.size()) {
			case 0:
				break;
			case 3: {
				array<uint, 3> triangleNodes = {};
				for (int i = 0; i < 3; ++i) {
					// check if the intersection point is already in the nodes
					auto &point = intersectionPoints[i];
					uint nodeIndex = 0;
					for (; nodeIndex < data.nodes.size(); ++nodeIndex) {
						auto &node = data.nodes[nodeIndex];
						if (node[0] == point[0] && node[1] == point[1] && node[2] == point[2]) {
							break;
						}
					}
					if (nodeIndex == data.nodes.size()) {
						data.nodes.push_back(point);
					}
					triangleNodes[i] = nodeIndex;
				}
				data.triangles.push_back(triangleNodes);
				break;
			}
			case 4: {
				// order intersection points
				array<uint, 4> orderedPoints;
				//center point
				auto center = multiplication((summation(
					summation(intersectionPoints[0], intersectionPoints[1]),
					summation(intersectionPoints[2], intersectionPoints[3]))
				) , 0.25);
				// sort by angle
				auto OA = subtraction(intersectionPoints[0], center);
				auto normal = crossProduct(OA, subtraction(intersectionPoints[1], center));
				auto mOA = magnitude(OA);
				array<double, 3> angles;
				for (int i = 1; i < 4; ++i) {
					auto OB = subtraction(intersectionPoints[i], center);
					auto mOB = magnitude(OB);
					auto ab = scalarProduct(OA, OB);
					auto xab = crossProduct(OA, OB);
					auto angleArg = ab / (mOA * mOB);
					if (angleArg > 1)
						angleArg = 1;
					auto _angle = acos(angleArg);
					if (scalarProduct(normal, xab) < 0)
						_angle = 2 * M_PI - _angle;
					angles[i - 1] = _angle;
				}
				orderedPoints[0] = 0;
				for (int i = 1; i < 4; ++i) {
					auto minAngleIndex = min_element(angles.begin(), angles.end()) - angles.begin();
					orderedPoints[i] = minAngleIndex + 1;
					angles[minAngleIndex] = INFINITY;
				}

				array<uint, 4> triangleVertices1 = {orderedPoints[0], orderedPoints[1], orderedPoints[2]};
				array<uint, 4> triangleVertices2 = {orderedPoints[0], orderedPoints[2], orderedPoints[3]};

				array<uint, 3> triangleNodes1 = {};
				array<uint, 3> triangleNodes2 = {};

				for (int i = 0; i < 3; ++i) {
					// triangle 1
					auto &point = intersectionPoints[triangleVertices1[i]];
					uint nodeIndex = 0;
					for (; nodeIndex < data.nodes.size(); ++nodeIndex) {
						auto &node = data.nodes[nodeIndex];
						if (node[0] == point[0] && node[1] == point[1] && node[2] == point[2]) {
							break;
						}
					}
					if (nodeIndex == data.nodes.size()) {
						data.nodes.push_back(point);
					}
					triangleNodes1[i] = nodeIndex;

					// triangle 2
					point = intersectionPoints[triangleVertices2[i]];
					nodeIndex = 0;
					for (; nodeIndex < data.nodes.size(); ++nodeIndex) {
						auto &node = data.nodes[nodeIndex];
						if (node[0] == point[0] && node[1] == point[1] && node[2] == point[2]) {
							break;
						}
					}
					if (nodeIndex == data.nodes.size()) {
						data.nodes.push_back(point);
					}
					triangleNodes2[i] = nodeIndex;
				}
				data.triangles.push_back(triangleNodes1);
				data.triangles.push_back(triangleNodes2);
				break;
			}
			default:
				throw runtime_error("Unexpected number of intersection points");

		}
	}
	return data;
}
