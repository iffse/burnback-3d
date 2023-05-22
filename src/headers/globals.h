#pragma once

#include <QQmlApplicationEngine>
#include <array>
#include <vector>
#include <src/headers/types.h>
#include <map>

inline QObject* root;

inline Input input;
inline Mesh mesh;
inline TetrahedraGeometry tetrahedraGeometry;
inline std::map<uint, Boundary> boundaries;
inline ComputationData computationData;

inline double timeStep;
inline double timeTotal;
inline std::vector<double> errorIter;

