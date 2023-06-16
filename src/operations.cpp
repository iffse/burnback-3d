#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <src/headers/operations.h>
#include <src/headers/globals.h>

#include <iostream>


using namespace std;
using namespace Vectors;

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

			const auto OA = subtraction(nodeACoord, nodeOCoord);
			const auto OB = subtraction(nodeBCoord, nodeOCoord);
			const auto OC = subtraction(nodeCCoord, nodeOCoord);

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

			const auto uAB = subtraction(uOB, uOA);
			const auto uAC = subtraction(uOC, uOA);

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
			auto r12 = subtraction(sCoord[1], sCoord[0]);
			auto r13 = subtraction(sCoord[2], sCoord[0]);
			auto r14 = subtraction(sCoord[3], sCoord[0]);
			gradient[index] = scalarProduct(crossProduct(r12, r13), r14) / tetrahedraGeometry.jacobiDeterminant[tetrahedra];
		}
	}
}
void computeFluxes() {
	computationData.hamiltonArg = vector<array<double, 3>>(mesh.nodes.size());
	computationData.flux.fill(vector<double>(mesh.nodes.size()));

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
			// const auto &area = tetrahedraGeometry.triangleArea[tetrahedra][vertexIndex];
			const auto &normal = tetrahedraGeometry.normal[tetrahedra][vertexIndex];
			const auto &weight = tetrahedraGeometry.vertexWeight[tetrahedra][vertexIndex];
			auto &flux = computationData.flux[1][node];
			flux += scalarProduct(gradient, normal) * weight;
			vertexIndex++;
		}
	}
}
}
//}}}

namespace Triangles {//{{{
void ApplyBoundaryConditions(){
	uint nodeIndex = 0;
	for (auto &type: boundaryConditions) {
		auto &fluxHamiltonian = computationData.flux[0][nodeIndex];
		auto &fluxDiffusive = computationData.flux[1][nodeIndex];
		auto hamiltonArg = computationData.hamiltonArg[nodeIndex];

		switch (type) {
			case NO_CONDITION:
				fluxHamiltonian = 1 - magnitude(hamiltonArg);
				break;
			case INLET:
				fluxHamiltonian = 0;
				fluxDiffusive = 0;
				break;
			case OUTLET:
				fluxHamiltonian = 1 - magnitude(hamiltonArg);
				// fluxDiffusive = 0;
				break;
			case SYMMETRY: {
				auto &symmetryVector = symmetryConditions[nodeIndex];
				hamiltonArg = crossProduct(crossProduct(symmetryVector, hamiltonArg), symmetryVector);
				fluxHamiltonian = 1 - magnitude(hamiltonArg);
				// fluxDiffusive *= 2;
				break;
			}
			case OUTLET_SYMMETRY: {
				auto &symmetryVector = symmetryConditions[nodeIndex];
				hamiltonArg = crossProduct(crossProduct(symmetryVector, hamiltonArg), symmetryVector);
				fluxHamiltonian = 1 - magnitude(hamiltonArg);
				// fluxDiffusive *= 2;
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
		auto &flux = computationData.flux;
		uVertex += timeStep * (flux[0][node] + input.diffusiveWeight * flux[1][node]);
		timeTotal += timeStep;
	}
}

void setBoundaryConditions() {
	boundaryConditions = vector<uint>(mesh.nodes.size(), 0);
	symmetryConditions.clear();
	for (uint node = 0; node < nodeConditions.size(); ++node) {
		auto &conditions = nodeConditions[node];
		auto &current = boundaryConditions[node];
		for (auto &condition: conditions) {
			if (current == INLET)
				break;
			auto &type = boundaries[condition].type;
			switch (type) {
				case INLET:
					current = INLET;
					break;
				case OUTLET:
					if (current == SYMMETRY || current == OUTLET_SYMMETRY)
						current = OUTLET_SYMMETRY;
					else
						current = OUTLET;
					break;
				case SYMMETRY: {
					if (current == OUTLET || current == OUTLET_SYMMETRY)
						current = OUTLET_SYMMETRY;
					else
						current = SYMMETRY;
					auto angleZ = boundaries[condition].value[0];
					auto angleY = boundaries[condition].value[1];
					array<double, 3> symmetryVector = {
						cos(angleZ) * cos(angleY),
						sin(angleZ) * cos(angleY),
						- sin(angleY)
					};

					if (symmetryConditions.find(node) == symmetryConditions.end()) {
						symmetryConditions.insert(pair<uint, array<double, 3>>(node, symmetryVector));
					} else {
						auto &symmetryVectorNode = symmetryConditions[node];
						symmetryVectorNode = Vectors::normalization(Vectors::subtraction(symmetryVectorNode, symmetryVector));
					}
					break;
				}
				default:
					throw invalid_argument("Unknown boundary condition in node " + to_string(node));
			}
		}
	}
}
}
//}}}
