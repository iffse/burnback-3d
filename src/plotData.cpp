#include <cmath>
#include <src/headers/plotData.h>
#include <src/headers/globals.h>

using namespace std;
namespace Vectors {//{{{
template <typename T>
auto crossProduct(const T &a, const T &b) {
	T result;
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[(i + 1) % 3] * b[(i + 2) % 3] - a[(i + 2) % 3] * b[(i + 1) % 3];
	return result;
};
template <typename T>
auto scalarProduct(const T &a, const T &b) {
	auto result = 0.0;
	for (uint i = 0; i < a.size(); ++i)
		result += a[i] * b[i];
	return result;
};
template <typename T>
auto magnitude(const T &a) {
	double result = 0.0;
	for (uint i = 0; i < a.size(); ++i)
		result += a[i] * a[i];
	return sqrt(result);
};
template <typename T>
auto normalization(const T &a) {
	T result;
	auto mag = magnitude(a);
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[i] / mag;
	return result;
};
template <typename T>
auto summation(const T &a, const T &b) {
	T result;
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[i] + b[i];
	return result;
};
template <typename T>
auto substraction(const T &a, const T &b) {
	T result;
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[i] - b[i];

	return result;
};
template <typename T>
auto multiplication(const T &a, const double &b) {
	T result;
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[i] * b;
	return result;
};
}
using namespace Vectors;
//}}}

// lerp is available since c++20
// as time of writing, cpp20 has only partial support by the compilers
// therefore cpp17 is expected to be used
#if __cplusplus < 202002L
double lerp(double x1, double x2, double t) {
	return x1 + t * (x2 - x1);
}
#endif


IsosurfaceData isosurfaceData(double value) {
	IsosurfaceData data;
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		auto &nodes = mesh.tetrahedra[tetrahedra];

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

					auto x1 = node1[0];
					auto y1 = node1[1];
					auto z1 = node1[2];

					auto x2 = node2[0];
					auto y2 = node2[1];
					auto z2 = node2[2];

					auto t = (value - uVertex1) / (uVertex2 - uVertex1);

					auto x = lerp(x1, x2, t);
					auto y = lerp(y1, y2, t);
					auto z = lerp(z1, z2, t);

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
				// normals from distance of vertices
				auto OA = substraction(data.nodes[triangleNodes[1]], data.nodes[triangleNodes[0]]);
				auto OB = substraction(data.nodes[triangleNodes[2]], data.nodes[triangleNodes[0]]);
				auto normal = normalization(crossProduct(OA, OB));
				// check if the normal is pointing in the right direction
				uint valueMinVertex, valueMaxVertex = 0;
				for (int i = 1; i < 4; ++i) {
					auto &uVertex = computationData.uVertex[nodes[i]];
					auto &currentMin = computationData.uVertex[nodes[valueMinVertex]];
					auto &currentMax = computationData.uVertex[nodes[valueMaxVertex]];
					auto compareValues = [&](double value) {
						return uVertex - value;
					};
					if (compareValues(currentMin) < 0) {
						valueMinVertex = i;
					} else if (compareValues(currentMax) > 0) {
						valueMaxVertex = i;
					}
				}
				auto flowDirection = substraction(mesh.nodes[nodes[valueMaxVertex]], mesh.nodes[nodes[valueMinVertex]]);
				if (scalarProduct(normal, flowDirection) < 0) {
					normal = multiplication(normal, -1);
				}
				data.normals.push_back(normal);

				break;
			}
			case 4: {
				array<uint, 4> triangleVertices1 = {0, 1, 2};
				array<uint, 4> triangleVertices2 = {0, 2, 3};

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

				// normals from distance of vertices
				// triangle 1
				auto OA = substraction(data.nodes[triangleNodes1[1]], data.nodes[triangleNodes1[0]]);
				auto OB = substraction(data.nodes[triangleNodes1[2]], data.nodes[triangleNodes1[0]]);
				auto normal = normalization(crossProduct(OA, OB));
				// check if the normal is pointing in the right direction
				uint valueMinVertex, valueMaxVertex = computationData.uVertex[nodes[0]];
				for (int i = 1; i < 4; ++i) {
					auto &uVertex = computationData.uVertex[nodes[i]];
					auto &currentMin = computationData.uVertex[nodes[valueMinVertex]];
					auto &currentMax = computationData.uVertex[nodes[valueMaxVertex]];
					auto compareValues = [&](double value) {
						return uVertex - value;
					};
					if (compareValues(currentMin) < 0) {
						valueMinVertex = i;
					} else if (compareValues(currentMax) > 0) {
						valueMaxVertex = i;
					}
				}
				auto flowDirection = substraction(mesh.nodes[nodes[valueMaxVertex]], mesh.nodes[nodes[valueMinVertex]]);
				if (scalarProduct(normal, flowDirection) < 0) {
					normal = multiplication(normal, -1);
				}
				data.normals.push_back(normal);

				// triangle 2
				OA = substraction(data.nodes[triangleNodes2[1]], data.nodes[triangleNodes2[0]]);
				OB = substraction(data.nodes[triangleNodes2[2]], data.nodes[triangleNodes2[0]]);
				normal = normalization(crossProduct(OA, OB));
				// check if the normal is pointing in the right direction
				if (scalarProduct(normal, flowDirection) < 0) {
					normal = multiplication(normal, -1);
				}
				data.normals.push_back(normal);
				break;
				}
			default:
				throw runtime_error("Unexpected number of intersection points");
				
		}
	}
	return data;
}
