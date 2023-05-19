#include <cmath>
#include<src/headers/operations.h>
#include<src/headers/globals.h>

using namespace std;

namespace Geometry {

void computeGeometry() {
	const auto &crossProduct = [](const array<double, 3> &a, const array<double, 3> &b) {
		return array<double, 3>{a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
	};
	const auto &scalarProduct = [](const array<double, 3> &a, const array<double, 3> &b) {
		return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	};
	const auto &magnitude = [](const array<double, 3> &a) {
		return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	};
	const auto &normalization = [&magnitude](const array<double, 3> &a) {
		auto mag = magnitude(a);
		return array<double, 3>{a[0] / mag, a[1] / mag, a[2] / mag};
	};

	for (uint tetrahedra = 0; tetrahedra < mesh.tetrahedra.size(); ++tetrahedra) {
		auto &omega = tetrahedraGeometry.solidAngle[tetrahedra];
		auto &normal = tetrahedraGeometry.normalVector[tetrahedra];
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

			const auto &vectorSubstraction = [](const array<double, 3> &a, const array<double, 3> &b) {
				return array<double, 3>{a[0] - b[0], a[1] - b[1], a[2] - b[2]};
			};

			const auto &OA = vectorSubstraction(vertexACoord, vertexOCoord);
			const auto &OB = vectorSubstraction(vertexBCoord, vertexOCoord);
			const auto &OC = vectorSubstraction(vertexCCoord, vertexOCoord);

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
void computeMeanGradient();
void computeFluxes();
void computeMinimumTimeStep();
}

namespace Triangles {
void ApplyBoundaryConditions();
}

namespace Nodes {
void computeResults();
}

