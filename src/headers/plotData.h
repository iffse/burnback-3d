#pragma once

#include <array>
#include <src/headers/types.h>

IsocontourData isosurfaceData(double value);
std::array<std::vector<double>, 2> burnAreaData(uint numberOfAreas);
