#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <src/headers/iosystem.h>
#include <src/headers/globals.h>
// #include <src/headers/operations.h>
// #include <src/headers/interface.h>

#include <nlohmann/json.hpp>
#include <fstream>

using namespace std;
using json = nlohmann::json;

void readInput() {//{{{
	input.uInitial = root->findChild<QObject*>("initialCondition")->property("text").toDouble();
	input.resume = root->findChild<QObject*>("resume")->property("checked").toBool();
	input.cfl = root->findChild<QObject*>("cfl")->property("text").toDouble();
	input.targetIter = root->findChild<QObject*>("targetIter")->property("text").toInt();
	if (input.targetIter == 0)
		input.targetIter = 300;

	input.diffusiveWeight = root->findChild<QObject*>("diffusiveWeight")->property("text").toDouble();
	input.diffusiveMethod = root->findChild<QObject*>("diffusiveMethod")->property("currentIndex").toInt();
}
//}}}

namespace Json {//{{{
void readMesh(std::string &filepath) {
	fstream file(filepath);
	json json;

	try {
		json = json::parse(file);
	} catch (...) {
		throw std::invalid_argument("Unable to parse JSON file. Invalid JSON file?");
		return;
	}

	auto &meshData = json["mesh"];
	try {
		mesh.nodes = meshData["nodes"];
		mesh.triangles = meshData["triangles"];
		mesh.tetrahedra = meshData["tetrahedra"];
	} catch (...) {
		throw std::invalid_argument("Unable to read mesh from JSON file. Missing mesh field or wrong format?");
		return;
	}

	auto &conditions = json["conditions"];
	try {
		boundaries.clear();
		for (auto &boundary : conditions["boundary"]) {
			auto &tag = boundary["tag"];
			if (tag < 1)
				throw std::invalid_argument("Boundary tag must be greater than 0");
			auto &type = boundary["type"];
			auto value = boundary.value("value", std::array<double, 2>({0, 0}));
			auto description = boundary.value("description", "");

			const vector<string> boundaryTypes = {"inlet", "outlet", "symmetry"};
			// check if a type is in boundaryTypes, and use switch to assign a value
			switch (find(boundaryTypes.begin(), boundaryTypes.end(), type) - boundaryTypes.begin()) {
				case 0:
					break;
				case 1:
					value = {0, 0};
					break;
				case 2:
					value[0] *= M_PI / 180;
					value[1] *= M_PI / 180;
					break;
				default:
					type = "inlet";
					value = {0, 0};
					break;
			}

			uint typeInt = find(boundaryTypes.begin(), boundaryTypes.end(), type) - boundaryTypes.begin() + 1;
			boundaries.insert(pair<int, Boundary>(tag, Boundary{typeInt, value, description}));
		}
		boundaries.insert(pair<int, Boundary>(0, Boundary{0, {0, 0}, ""}));
		boundaryConditions = vector<uint>(mesh.nodes.size());
		auto triangleIndex = 0;
		for (auto &condition: conditions["triangle"]) {
			auto &triangle = mesh.triangles[triangleIndex];
			for (auto &node: triangle) {
				auto &current = boundaryConditions[node];
				if (current == 0) {
					current = condition;
					continue;
				}
				auto &type = boundaries[current].type;
				auto &newType = boundaries[condition].type;
				if (newType < type)
					current = condition;
			}
		}
	} catch(...) {
		throw std::invalid_argument("Unable to read boundary conditions from JSON file. Missing boundary field or wrong format?");
		return;
	}

	tetrahedraGeometry = TetrahedraGeometry(mesh.tetrahedra.size());
}
void writeData(std::string  &filepath, std::string  &origin, bool &pretty);
void updateBoundaries(std::string  &filepath, bool &pretty);
}//}}}

