#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <src/headers/operations.h>
#include <src/headers/globals.h>

#include <iostream>


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

namespace Geometry {//{{{

void computeGeometry() {
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		auto &solidAngle = tetrahedraGeometry.solidAngle[tetrahedra];
		auto &normal = tetrahedraGeometry.normal[tetrahedra];
		auto &area = tetrahedraGeometry.triangleArea[tetrahedra];
		auto &jacobi = tetrahedraGeometry.jacobiDeterminant[tetrahedra];

		for (uint vertex = 0; vertex < 4; ++vertex) {
			const auto nodeO = mesh.tetrahedra[tetrahedra][vertex] - 1;
			const auto nodeA = mesh.tetrahedra[tetrahedra][(vertex + 1) % 4] - 1;
			const auto nodeB = mesh.tetrahedra[tetrahedra][(vertex + 2) % 4] - 1;
			const auto nodeC = mesh.tetrahedra[tetrahedra][(vertex + 3) % 4] - 1;

			const auto &nodeOCoord = mesh.nodes[nodeO];
			const auto &nodeACoord = mesh.nodes[nodeA];
			const auto &nodeBCoord = mesh.nodes[nodeB];
			const auto &nodeCCoord = mesh.nodes[nodeC];

			const auto OA = substraction(nodeACoord, nodeOCoord);
			const auto OB = substraction(nodeBCoord, nodeOCoord);
			const auto OC = substraction(nodeCCoord, nodeOCoord);

			// Calculating solid angle: Oosterom and Strackee algorithm
			const auto tripleProduct = scalarProduct(OA, crossProduct(OB, OC));
			const auto magnitudeOA = magnitude(OA);
			const auto magnitudeOB = magnitude(OB);
			const auto magnitudeOC = magnitude(OC);
			const auto magnitudeOABC = magnitudeOA * magnitudeOB * magnitudeOC;
			const auto sOABC = scalarProduct(OA, OB) * magnitudeOC;
			const auto sOBCA = scalarProduct(OB, OC) * magnitudeOA;
			const auto sOCAB = scalarProduct(OC, OA) * magnitudeOB;

			solidAngle[vertex] = abs(2 * atan2(tripleProduct, sOABC + sOBCA + sOCAB + magnitudeOABC));
			// cpp's atan2 returns values between -pi and pi,
			// solid angle is always positive, so we need to add 2pi to negative values
			// if (solidAngle[vertex] < 0)
			// 	solidAngle[vertex] *= -1;
			angleTotal[nodeO] += solidAngle[vertex];
				// solidAngle[vertex] += 2 * M_PI;

			// Calculating area and normal vector of the intersection with a unit sphere
			const auto uOA = normalization(OA);
			const auto uOB = normalization(OB);
			const auto uOC = normalization(OC);

			const auto uAB = substraction(uOB, uOA);
			const auto uAC = substraction(uOC, uOA);

			auto _normal = crossProduct(uAB, uAC);
			normal[vertex] = normalization(_normal);
			// Check if normal vector is pointing outwards (going away from O)
			if (scalarProduct(normal[vertex], OA) < 0)
				normal[vertex] = multiplication(normal[vertex], -1);
			area[vertex] = magnitude(_normal) / 2;

			// Calculating jacobi determinant and time step
			auto oppositeTriangleArea = magnitude((crossProduct(OA, OB))) / 2;

			if (vertex == 0)
				jacobi = abs(scalarProduct(crossProduct(OA, OB), OC));

			if (tetrahedra == 0)
				timeStep = jacobi / (oppositeTriangleArea * 2);
			else if (timeStep > jacobi / (oppositeTriangleArea * 2))
				timeStep = jacobi / (oppositeTriangleArea * 2);
		}
	}
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		for (uint vertex = 0; vertex < 4; ++vertex) {
			const auto &node = mesh.tetrahedra[tetrahedra][vertex] - 1;
			tetrahedraGeometry.vertexWeight[tetrahedra][vertex] = tetrahedraGeometry.solidAngle[tetrahedra][vertex] / angleTotal[node];
		}
	}
	timeStep *= input.cfl / 6;
}
}
//}}}

namespace Tetrahedra {//{{{
void computeMeanGradient() {
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		const auto nodeO = mesh.tetrahedra[tetrahedra][0] - 1;
		const auto nodeA = mesh.tetrahedra[tetrahedra][1] - 1;
		const auto nodeB = mesh.tetrahedra[tetrahedra][2] - 1;
		const auto nodeC = mesh.tetrahedra[tetrahedra][3] - 1;
		const auto uOABC = array<double, 4>{
			computationData.uVertex[nodeO],
			computationData.uVertex[nodeA],
			computationData.uVertex[nodeB],
			computationData.uVertex[nodeC]
		};
		const auto &vertexOCoord = mesh.nodes[nodeO];
		const auto &vertexACoord = mesh.nodes[nodeA];
		const auto &vertexBCoord = mesh.nodes[nodeB];
		const auto &vertexCCoord = mesh.nodes[nodeC];
		auto coordinates = array<array<double, 3>, 4>{{
			{vertexOCoord[0], vertexOCoord[1], vertexOCoord[2]},
			{vertexACoord[0], vertexACoord[1], vertexACoord[2]},
			{vertexBCoord[0], vertexBCoord[1], vertexBCoord[2]},
			{vertexCCoord[0], vertexCCoord[1], vertexCCoord[2]}
		}};

		auto &gradient = computationData.gradient[tetrahedra];
		for (int index = 0; index < 3; ++index) {
			auto sCoord = coordinates;
			for (int vertex = 0; vertex < 4; ++vertex)
				sCoord[vertex][index] = uOABC[vertex];
			auto r12 = substraction(sCoord[1], sCoord[0]);
			auto r13 = substraction(sCoord[2], sCoord[0]);
			auto r14 = substraction(sCoord[3], sCoord[0]);
			gradient[index] = scalarProduct(crossProduct(r12, r13), r14) / tetrahedraGeometry.jacobiDeterminant[tetrahedra];
		}
	}
}
void computeFluxes() {
	computationData.hamiltonArg = vector<array<double, 3>>(mesh.nodes.size());
	computationData.flux = vector<array<double, 2>>(mesh.nodes.size());

	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		const auto &gradient = computationData.gradient[tetrahedra];
		for (uint vertex = 0; vertex < 4; ++vertex) {
			const auto node = mesh.tetrahedra[tetrahedra][vertex] - 1;
			const auto &weight = tetrahedraGeometry.vertexWeight[tetrahedra][vertex];
			auto &hamiltonArg = computationData.hamiltonArg[node];
			hamiltonArg = summation(hamiltonArg, multiplication(gradient, weight));
		}
	}

	// diffusive flux
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		const auto &gradient = computationData.gradient[tetrahedra];
		uint vertexIndex = 0;
		for (auto &_node: mesh.tetrahedra[tetrahedra]) {
			const auto node = _node - 1;
			const auto &area = tetrahedraGeometry.triangleArea[tetrahedra][vertexIndex];
			const auto &normal = tetrahedraGeometry.normal[tetrahedra][vertexIndex];
			const auto &weight = tetrahedraGeometry.vertexWeight[tetrahedra][vertexIndex];
			auto &flux = computationData.flux[node];
			flux[1] += area * scalarProduct(gradient, normal) * weight;
			vertexIndex++;
		}
	}
}
}
//}}}

namespace Triangles {//{{{
void ApplyBoundaryConditions(){
	uint nodeIndex = 0;
	for (auto &tag: boundaryConditions) {
		auto &type = boundaries[tag].type;
		auto &value = boundaries[tag].value[0];
		auto &valueSup = boundaries[tag].value[1];
		auto &flux = computationData.flux[nodeIndex];
		auto hamiltonArg = computationData.hamiltonArg[nodeIndex];

		switch (type) {
			case 0: // no condition
				flux[0] = 1 - magnitude(hamiltonArg);
				break;
			case 1: // inlet
				flux[0] = 0;
				flux[1] = 0;
				break;
			case 2: // outlet
				flux[0] = 1 - magnitude(hamiltonArg);
				break;
			case 3: { // symmetry
				auto symmetryVector = array<double, 3>{cos(valueSup) * cos(value), cos(valueSup) * sin(value), sin(valueSup)};
				hamiltonArg = multiplication(
					crossProduct(crossProduct(symmetryVector, hamiltonArg), symmetryVector),2
				);
				flux[0] = 1 - magnitude(hamiltonArg);
				break;
			}
			default:
				break;
		};
		nodeIndex++;
	}
}
}
//}}}

namespace Nodes {//{{{	
void computeResults() {
	for (uint node = 0; node < mesh.nodes.size(); ++node) {
		auto &uVertex = computationData.uVertex[node];
		auto &flux = computationData.flux[node];
		uVertex += timeStep * (flux[0] + input.diffusiveWeight * flux[1]);
		timeTotal += timeStep;
	}
}
}
//}}}
