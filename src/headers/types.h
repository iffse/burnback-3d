#pragma once

#include <array>
#include <vector>
#include <map>
#include <cstdint>
#include <string>

struct Input {
	double uInitial;
	bool resume;
	double cfl;
	uint32_t targetIter;
	double diffusiveWeight;
	uint32_t diffusiveMethod;
};

struct Mesh {
	std::vector<std::array<double, 3>> nodes;
	std::vector<std::array<int, 3>> triangles;
	std::vector<std::array<int, 4>> tetrahedra;
};

struct TetrahedraGeometry {
	std::vector<std::array<double, 4>> solidAngle;
	std::vector<std::array<double, 4>> triangleArea;
	std::vector<std::array<std::array<double, 3>, 4>> normal;
	std::vector<double> jacobiDeterminant; // equal to 6 times the volume of the tetrahedra

TetrahedraGeometry() = default;
TetrahedraGeometry(uint32_t size) {
	solidAngle = triangleArea = std::vector<std::array<double, 4>>(size);
	normal = std::vector<std::array<std::array<double, 3>, 4>>(size);
	jacobiDeterminant = std::vector<double>(size);
	}
};


struct Boundary {
	uint32_t type;
	double value;
	std::string description;
};

struct ComputationData {
	std::vector<double> uVertex;
	std::vector<double> recession;
	std::vector<std::array<double, 3>> duVariable;
	std::vector<std::array<double, 3>> duVertex;
	std::vector<std::array<double, 2>> flux;

	ComputationData() = default;
	ComputationData(uint32_t size) {
		uVertex = recession = std::vector<double>(size);
		duVariable = duVertex = std::vector<std::array<double, 3>>(size);
		flux = std::vector<std::array<double, 2>>(size);
	}
};
