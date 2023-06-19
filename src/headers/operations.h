#pragma once

#include <math.h>
namespace Vectors {//{{{
template <typename T>
T crossProduct(const T &a, const T &b) {
	T result;
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[(i + 1) % 3] * b[(i + 2) % 3] - a[(i + 2) % 3] * b[(i + 1) % 3];
	return result;
};
template <typename T>
double scalarProduct(const T &a, const T &b) {
	auto result = 0.0;
	for (uint i = 0; i < a.size(); ++i)
		result += a[i] * b[i];
	return result;
};
template <typename T>
double magnitude(const T &a) {
	double result = 0.0;
	for (uint i = 0; i < a.size(); ++i)
		result += a[i] * a[i];
	return sqrt(result);
};
template <typename T>
T normalization(const T &a) {
	T result;
	auto mag = magnitude(a);
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[i] / mag;
	return result;
};
template <typename T>
T summation(const T &a, const T &b) {
	T result;
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[i] + b[i];
	return result;
};
template <typename T>
T subtraction(const T &a, const T &b) {
	T result;
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[i] - b[i];
	return result;
};
template <typename T>
T multiplication(const T &a, const double &b) {
	T result;
	for (uint i = 0; i < a.size(); ++i)
		result[i] = a[i] * b;
	return result;
};
}
//}}}

namespace Geometry {
void computeGeometry();
}

namespace Tetrahedra {
void computeMeanGradient();
void computeFluxes();
}

namespace Triangles {
void ApplyBoundaryConditions();
}

namespace Nodes {
void computeResults();
void setBoundaryConditions();
double getError();
}
