#pragma once

#include <QQmlApplicationEngine>
#include <QString>
#include <array>
#include <vector>
#include <src/headers/types.h>
#include <map>

inline QObject* root;
inline QString tmpDir;

inline Input input;
inline Mesh mesh;
inline TetrahedraGeometry tetrahedraGeometry;
inline std::vector<double> angleTotal;
inline std::map<uint, Boundary> boundaries;
inline std::vector<uint> boundaryConditions;
inline ComputationData computationData;

inline double timeStep;
inline double timeTotal;
inline std::vector<double> errorIter;

inline uint currentIter = 0;
inline bool running = false;

inline uint drawCount = 0;
