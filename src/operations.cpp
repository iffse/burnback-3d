#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include<src/headers/operations.h>
#include<src/headers/globals.h>

using namespace std;

namespace Vectors {
auto crossProduct(const array<double, 3> &a, const array<double, 3> &b) {
	return array<double, 3>{a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
};
auto scalarProduct(const array<double, 3> &a, const array<double, 3> &b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
};
auto magnitude(const array<double, 3> &a) {
	return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
};
auto normalization(const array<double, 3> &a) {
	auto mag = magnitude(a);
	return array<double, 3>{a[0] / mag, a[1] / mag, a[2] / mag};
};
auto summation(const array<double, 3> &a, const array<double, 3> &b) {
	return array<double, 3>{a[0] + b[0], a[1] + b[1], a[2] + b[2]};
};
auto substraction(const array<double, 3> &a, const array<double, 3> &b) {
	return array<double, 3>{a[0] - b[0], a[1] - b[1], a[2] - b[2]};
};
}
using namespace Vectors;

namespace Geometry {

void computeGeometry() {
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		auto &omega = tetrahedraGeometry.solidAngle[tetrahedra];
		auto &normal = tetrahedraGeometry.normal[tetrahedra];
		auto &area = tetrahedraGeometry.triangleArea[tetrahedra];

		for (uint vertex = 0; vertex < 4; ++vertex) {
			const auto &vertexO = mesh.tetrahedra[tetrahedra][vertex];
			const auto &vertexA = mesh.tetrahedra[tetrahedra][(vertex + 1) % 4];
			const auto &vertexB = mesh.tetrahedra[tetrahedra][(vertex + 2) % 4];
			const auto &vertexC = mesh.tetrahedra[tetrahedra][(vertex + 3) % 4];

			const auto &vertexOCoord = mesh.nodes[vertexO];
			const auto &vertexACoord = mesh.nodes[vertexA];
			const auto &vertexBCoord = mesh.nodes[vertexB];
			const auto &vertexCCoord = mesh.nodes[vertexC];

			const auto OA = substraction(vertexOCoord, vertexACoord);
			const auto OB = substraction(vertexOCoord, vertexBCoord);
			const auto OC = substraction(vertexOCoord, vertexCCoord);

			// Calculating solid angle: Oosterom and Strackee algorithm
			const auto tripleProduct = scalarProduct(OA, crossProduct(OB, OC));
			const auto magnitudeOA = magnitude(OA);
			const auto magnitudeOB = magnitude(OB);
			const auto magnitudeOC = magnitude(OC);
			const auto magnitudeOABC = magnitudeOA * magnitudeOB * magnitudeOC;
			const auto sOABC = scalarProduct(OA, OB) * magnitudeOC;
			const auto sOBCA = scalarProduct(OB, OC) * magnitudeOA;
			const auto sOCAB = scalarProduct(OC, OA) * magnitudeOB;

			omega[vertex] = 2 * atan2(tripleProduct, sOABC + sOBCA + sOCAB + magnitudeOABC);

			// Calculating normal vector
			normal[vertex] = normalization(crossProduct(OA, OB));
			// check if normal vector is pointing outwards (going away from O)
			if (scalarProduct(normal[vertex], OA) < 0)
				normal[vertex] = normalization(crossProduct(OB, OA));

			area[vertex] = magnitude(crossProduct(OA, OB)) / 2;
		}
	}
}
}

namespace Tetrahedra {
void computeMeanGradient() {
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		const auto &vertexO = mesh.tetrahedra[tetrahedra][0];
		const auto &vertexA = mesh.tetrahedra[tetrahedra][1];
		const auto &vertexB = mesh.tetrahedra[tetrahedra][2];
		const auto &vertexC = mesh.tetrahedra[tetrahedra][3];
		const auto &uOABC = array<double, 4>{
			computationData.uVertex[vertexO],
			computationData.uVertex[vertexA],
			computationData.uVertex[vertexB],
			computationData.uVertex[vertexC]
		};
		const auto &vertexOCoord = mesh.nodes[vertexO];
		const auto &vertexACoord = mesh.nodes[vertexA];
		const auto &vertexBCoord = mesh.nodes[vertexB];
		const auto &vertexCCoord = mesh.nodes[vertexC];

		const auto OA = substraction(vertexOCoord, vertexACoord);
		const auto OB = substraction(vertexOCoord, vertexBCoord);
		const auto OC = substraction(vertexOCoord, vertexCCoord);

		// FIXME: jacobian can be calculated only once per tetrahedra
		const auto jacoby = scalarProduct(crossProduct(OA, OB), OC);
		// use this volume to also calculate the minimum height
		timeStep = 0;
		for (uint index = 0; index < 4; ++index) {
			const auto &area = tetrahedraGeometry.triangleArea[tetrahedra][index];
			if (jacoby / area < timeStep)
				timeStep = jacoby / area;
		}

		auto gradient = array<double, 3>();
		auto coordinates = array<array<double, 3>, 4>{{
			{vertexOCoord[0], vertexOCoord[1], vertexOCoord[2]},
			{vertexACoord[0], vertexACoord[1], vertexACoord[2]},
			{vertexBCoord[0], vertexBCoord[1], vertexBCoord[2]},
			{vertexCCoord[0], vertexCCoord[1], vertexCCoord[2]}
		}};
		for (int index = 0; index < 3; ++index) {
			auto sCoord = coordinates;
			for (int vertex = 0; vertex < 4; ++vertex)
				sCoord[vertex][index] = uOABC[vertex];
			auto r12 = substraction(sCoord[1], sCoord[0]);
			auto r13 = substraction(sCoord[2], sCoord[0]);
			auto r14 = substraction(sCoord[3], sCoord[0]);
			gradient[index] = scalarProduct(crossProduct(r12, r13), r14) / jacoby;
		};
		computationData.duVariable[tetrahedra] = summation(computationData.duVertex[vertexO], gradient);
	}
}
void computeFluxes() {
	// uVertex, argument of the hamiltonian flux
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		const auto &duVariable = computationData.duVariable[tetrahedra];
		for (auto &node: mesh.tetrahedra[tetrahedra]) {
			const auto &solidAngle = tetrahedraGeometry.solidAngle[tetrahedra][node];
			auto &duVertex = computationData.duVertex[node];
			duVertex = {
				solidAngle * duVariable[0] + duVertex[0],
				solidAngle * duVariable[1] + duVertex[1],
				solidAngle * duVariable[2] + duVertex[2]
			};
		}
	}
	for (auto &duVertex: computationData.duVertex)
		for (auto &value: duVertex)
			value /= 4 * M_PI;

	// diffusive flux
	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		const auto &duVariable = computationData.duVariable[tetrahedra];
		uint nodeIndex = 0;
		for (auto &node: mesh.tetrahedra[tetrahedra]) {
			const auto &area = tetrahedraGeometry.triangleArea[tetrahedra][nodeIndex];
			const auto &normal = tetrahedraGeometry.normal[tetrahedra][nodeIndex];
			computationData.flux[node][1] += area * scalarProduct(duVariable, normal);
			nodeIndex++;
		}
	}

	for (auto &flux: computationData.flux)
		flux[1] /= 4 * M_PI;
}
void computeMinimumTimeStep() {
}
}

namespace Triangles {
void ApplyBoundaryConditions();
}

namespace Nodes {
void computeResults();
}

