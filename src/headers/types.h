#pragma once

#include <array>
#include <vector>
#include <map>
#include <string>

// this is not required in linux
// for some reason, the compiler in windows and mac does not recognize uint
typedef unsigned int uint;

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
	std::vector<std::array<double, 4>> vertexWeight;
	std::vector<std::array<double, 4>> triangleArea;
	std::vector<std::array<std::array<double, 3>, 4>> normal;
	std::vector<double> jacobiDeterminant; // equal to 6 times the volume (signed) of the tetrahedra

TetrahedraGeometry() = default;
TetrahedraGeometry(uint tetrahedra) {
	solidAngle = triangleArea = vertexWeight = std::vector<std::array<double, 4>>(tetrahedra);
	normal = std::vector<std::array<std::array<double, 3>, 4>>(tetrahedra);
	jacobiDeterminant = std::vector<double>(tetrahedra);
	}
};


struct Boundary {
	uint type;
	std::array<double, 2> value;
	std::string description;
};

struct ComputationData {
	std::vector<double> uVertex;
	std::vector<std::array<double, 3>> gradient;
	std::vector<std::array<double, 3>> vertexGradient;
	std::array<std::vector<double>, 2> flux;

	ComputationData() = default;
	ComputationData(int nodes, int tetrahedra) {
		uVertex = std::vector<double>(nodes);
		gradient = std::vector<std::array<double, 3>>(tetrahedra);
		vertexGradient = std::vector<std::array<double, 3>>(nodes);
		flux.fill(std::vector<double>(nodes));
	}
};

struct IsocontourData {
	std::vector<std::array<double, 3>> nodes;
	std::vector<std::array<uint, 3>> triangles;
};
