#pragma once

#include <src/headers/types.h>
#include <array>

IsocontourData isosurfaceData(double value);
std::array<std::vector<double>, 2> burnAreaData(uint numberOfAreas);
