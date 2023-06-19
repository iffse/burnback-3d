#pragma once

#include <QQmlApplicationEngine>
#include <QString>
#include <array>
#include <vector>
#include <src/headers/types.h>
#include <map>

enum {
	NO_CONDITION = 0,
	INLET = 1,
	OUTLET = 2,
	SYMMETRY = 3,
	OUTLET_SYMMETRY = 23
};

inline QObject* root;
inline QString tmpDir;

inline Input input;
inline Mesh mesh;
inline TetrahedraGeometry tetrahedraGeometry;
inline std::vector<double> angleTotal;
inline std::map<uint, Boundary> boundaries;
inline std::vector<std::vector<uint>> nodeConditions;
inline std::vector<uint> boundaryConditions;
inline std::map<uint, std::array<double, 3>> symmetryConditions;
inline ComputationData computationData;

inline double timeStep;
inline double timeTotal;
inline std::vector<double> errorIter;

inline uint currentIter = 0;
inline bool running = false;

inline uint drawCount = 0;

inline std::vector<double> recession;
