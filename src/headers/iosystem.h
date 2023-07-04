#pragma once

#include <string>

void readInput();

namespace Json {
void readMesh(std::string &filepath);
void writeData(std::string &filepath, std::string &origin, bool &pretty);
void updateBoundaries(std::string &filepath, bool &pretty);
void updateRecessions(std::string &filepath, bool &pretty);
}

namespace WriteMesh {
void IsocontourSurface(double value, std::string filepath);
void Boundary();
void Material();
}
