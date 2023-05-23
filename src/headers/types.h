#pragma once

#include <array>
#include <vector>
#include <map>
#include <string>

struct Input {
	double uInitial;
	bool resume;
	double cfl;
	uint targetIter;
	double diffusiveWeight;
	uint diffusiveMethod;
};

struct Mesh {
	std::vector<std::array<double, 3>> nodes;
	std::vector<std::array<uint, 3>> triangles;
	std::vector<std::array<uint, 4>> tetrahedra;
};

struct TetrahedraGeometry {
	std::vector<std::array<double, 4>> solidAngle;
	std::vector<std::array<double, 4>> triangleArea;
	std::vector<std::array<std::array<double, 3>, 4>> normal;
	std::vector<double> jacobiDeterminant; // equal to 6 times the volume of the tetrahedra

TetrahedraGeometry() = default;
TetrahedraGeometry(uint size) {
	solidAngle = triangleArea = std::vector<std::array<double, 4>>(size);
	normal = std::vector<std::array<std::array<double, 3>, 4>>(size);
	jacobiDeterminant = std::vector<double>(size);
	}
};


struct Boundary {
	uint type;
	std::array<double, 2> value;
	std::string description;
};

struct ComputationData {
	std::vector<double> uVertex;
	std::vector<double> recession;
	std::vector<std::array<double, 3>> gradient;
	std::vector<std::array<double, 3>> hamiltonArg;
	std::vector<std::array<double, 2>> flux;

	ComputationData() = default;
	ComputationData(uint32_t nodes, uint32_t tetrahedra) {
		uVertex = recession = std::vector<double>(nodes);
		gradient = std::vector<std::array<double, 3>>(nodes);
		hamiltonArg = std::vector<std::array<double, 3>>(tetrahedra);
		flux = std::vector<std::array<double, 2>>(nodes);
	}
};
