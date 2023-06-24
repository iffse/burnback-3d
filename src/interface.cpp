#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <chrono>
#include <cmath>
#include <thread>
#include <QFile>
#include <vector>
#include <QDir>
#include <QStandardPaths>

#include <src/headers/interface.h>
#include "src/headers/iosystem.h"
#include <src/headers/globals.h>
#include <src/headers/operations.h>
#include <src/headers/plotData.h>

#ifdef DEBUG
#include <fenv.h>
#endif

using namespace std;

Actions::Actions(QObject *parent) : QObject(parent) {
	connect(this, &Actions::newOutput, this, &Actions::appendOutput);
	connect(this, &Actions::readFinished, this, &Actions::afterReadMesh);
}

void clearSubstring(QString &str) {
#ifdef _WIN32
	const QString substring = "file:///";
#else
	const QString substring = "file://";
#endif

	if (str.startsWith(substring)) {
		str.remove(0, substring.length());
	}
}

void Actions::appendOutput(QString text) {
	QObject *output = root->findChild<QObject*>("output");
	auto currentText = output->property("text").toString();
	auto setText = currentText + text + "\n";

	// limit the number of lines
	auto maxLines = 100;
	auto lines = setText.count("\n");
	if (lines < maxLines) {
		output->setProperty("text", setText);
		return;
	}

	auto indexRemove = 0;
	if (lines > 2 * maxLines) {
		for (int line = 0; line < maxLines; ++ line) {
			indexRemove = setText.lastIndexOf("\n", indexRemove - 2);
		}
	} else {
		for (int line = 0; line < lines - maxLines; ++ line) {
			indexRemove = setText.indexOf("\n", indexRemove + 2);
		}
	}

	setText.remove(0, indexRemove);
	output->setProperty("text", setText);
}

void Actions::readMeshWorker(QString path) {
	const QString jsonExtension = ".json";

	if (path.endsWith(jsonExtension)) {
		auto filepath = path.toStdString();
		try {
			Json::readMesh(filepath);
		} catch (std::invalid_argument &e) {
			emit newOutput("Error: " + QString(e.what()));
			emit readFinished(false);
			return;
		} catch (...) {
			emit newOutput("Error: Unknown exception when reading mesh");
			emit readFinished(false);
			return;
		}
	} else {
		emit newOutput("Error: Unknown file extension");
		emit readFinished(false);
		return;
	}
	emit readFinished(true);
}

void Actions::readMesh(QString filepath) {
	running = false;
	root->findChild<QObject*>("runButton")->setProperty("enabled", false);
	root->findChild<QObject*>("resume")->setProperty("enabled", false);
	root->findChild<QObject*>("resume")->setProperty("checked", false);
	appendOutput("--> Reading mesh");
	if (filepath.isEmpty()) {
		appendOutput("Error: No file selected");
		return;
	}

	clearSubstring(filepath);

	std::thread thread(&Actions::readMeshWorker, this, filepath);
	thread.detach();
}


void Actions::afterReadMesh(bool sucess) {
	if (!sucess) {
		appendOutput("Error: Failed to read mesh");
		return;
	}

	root->findChild<QObject*>("runButton")->setProperty("enabled", true);
	appendOutput("--> Mesh read sucessfully");
}

void Actions::run() {
	running = true;
	root->findChild<QObject*>("runButton")->setProperty("text", "Stop");
	// clear data
	appendOutput("--> Reading inputs");
	try {
		readInput();
	} catch (std::invalid_argument &e) {
		appendOutput("Error: " + QString(e.what()));
		root->findChild<QObject*>("runButton")->setProperty("text", "Run");
		return;
	} catch (...) {
		appendOutput("Error: Unknown exception");
		root->findChild<QObject*>("runButton")->setProperty("text", "Run");
		return;
	}

	root->findChild<QObject*>("resume")->setProperty("enabled", true);
	std::thread thread(&Actions::worker, this);
	thread.detach();
}

void Actions::stop() {
	running = false;
}

void Actions::worker() {
	#ifdef DEBUG
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
	#endif
	if (!input.resume) {
		currentIter = 0;
		timeTotal = 0;
		timeStep = 0;
		errorIter.clear();
		tetrahedraGeometry = TetrahedraGeometry(mesh.tetrahedra.size());
		angleTotal = vector<double>(mesh.nodes.size());
		computationData = ComputationData(mesh.nodes.size(), mesh.tetrahedra.size());
		Geometry::computeGeometry();
	}
	emit newOutput("--> Starting subiteration loop");
	if (currentIter < input.targetIter)
		errorIter.resize(input.targetIter);

	QString linesToPrint = "";
	auto clock = std::chrono::system_clock::now();

	vector<double> errors;
	for (; currentIter < input.targetIter; ++currentIter) {
		Tetrahedra::computeMeanGradient();
		Tetrahedra::computeFluxes();
		Triangles::ApplyBoundaryConditions();
		Nodes::computeResults();


		auto error = Nodes::getError();
		errorIter[currentIter] = error;

		if (linesToPrint != "")
			linesToPrint += "\n";
		linesToPrint += "Iteration: " + QString::number(currentIter + 1) + " Time: " + QString::number(timeTotal) + " Error: " + QString::number(error * 100) + "%";

		if (error > 1) {
			if (linesToPrint != "") {
				emit newOutput(linesToPrint);
				emit updateProgress(currentIter + 1, input.targetIter);
			}
			emit newOutput("Error: Divergence detected. Stopping. Try reducing the CFL.");
			emit newOutput("--> Stopped");
			root->findChild<QObject*>("runButton")->setProperty("text", "Run");
			afterWorker();
			return;
		}

		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - clock).count() > 10) {
			clock = now;
			emit newOutput(linesToPrint);
			emit updateProgress(currentIter + 1, input.targetIter);
			linesToPrint = "";
			if (!running) {
				emit newOutput("--> Stopped");
				root->findChild<QObject*>("runButton")->setProperty("text", "Run");
				afterWorker();
				return;
		}
		}
	}

	if (linesToPrint != "") {
		emit newOutput(linesToPrint);
		emit updateProgress(currentIter, input.targetIter);
	}

	emit newOutput("--> Subiteration ended");
	afterWorker();
}

void Actions::afterWorker() {
	root->findChild<QObject*>("runButton")->setProperty("text", "Run");
	auto &max_uVertex = *std::max_element(computationData.uVertex.begin(), computationData.uVertex.end());
	root->findChild<QObject*>("isosurfaceSlider")->setProperty("to", max_uVertex);

	// make sure the directory exists
	QDir dir(tmpDir);
	if (!dir.exists())
		dir.mkpath(tmpDir);


	auto nodes = vector<double>(mesh.nodes.size());
	for (uint i = 0; i < mesh.nodes.size(); ++i) {
		nodes[i] = mesh.nodes[i][2];
	}

	WriteMesh::Material();
	WriteMesh::Boundary();
	double minx = 0, miny = 0, minz = 0, maxx = 0, maxy = 0, maxz = 0;
	for (auto &node: mesh.nodes) {
		minx = min(minx, node[0]);
		miny = min(miny, node[1]);
		minz = min(minz, node[2]);
		maxx = max(maxx, node[0]);
		maxy = max(maxy, node[1]);
		maxz = max(maxz, node[2]);

	}

	auto x = (maxx - minx) * 1.5 + maxx;
	auto y = (maxy - miny) * 1.5 + maxy;
	auto z = (maxz - minz) * 1.5 + maxz;

	emit setCameraPosition(x, y, z);

	double isosurfaceValue = root->findChild<QObject*>("isosurfaceSlider")->property("value").toDouble();
	previewIsosurface(isosurfaceValue);
}

void Actions::previewIsosurface(double value) {

#ifdef _WIN32
	const QString substring = "file:///";
#else
	const QString substring = "file://";
#endif

	auto filepath = tmpDir + "surface" + QString::number(drawCount) + ".obj";
	auto url = substring + filepath;
	WriteMesh::IsocontourSurface(value, filepath.toStdString());
	emit loadMeshPreview(url);
	drawCount++;
}

void Actions::updateIsosurface(double value) {
	if (computationData.uVertex.size() > 0)
		previewIsosurface(value);
}

void Actions::clearCache() {
	if (tmpDir == "")
		return;
	QDir dir(tmpDir);
	dir.removeRecursively();
}

void Actions::setCullingMethod(uint method) {
	emit setCulling(method);
}

void Actions::exportData(QString filepath, bool pretty) {
	auto origin = root->findChild<QObject*>("fileDialog")->property("fileUrl").toString();

	clearSubstring(filepath);
	clearSubstring(origin);

	// add .json if filepath doesn't have it
	if (!filepath.endsWith(".json"))
		filepath += ".json";

	auto originPath = origin.toStdString();
	auto filePath = filepath.toStdString();

	appendOutput("Exporting data to " + filepath);
	try {
		Json::writeData(filePath, originPath, pretty);
	} catch (const std::exception &e) {
		appendOutput("Error while exporting data: " + QString(e.what()));
	} catch (...) {
		appendOutput("Error while exporting data");
	}
}

vector<QString> Actions::getBoundaries() {
	auto numBoundaries = boundaries.size() - 1;
	vector<QString> boundariesVector = vector<QString>(numBoundaries * 4);

	auto index = 0;
	for (const auto &[key, value]: boundaries) {
		if (key == 0)
			continue;
		boundariesVector[index] = QString::number(key);
		boundariesVector[index + 1] = QString::number(value.type - 1);
		try {
			switch (value.type){
				case INLET:
					boundariesVector[index + 2] = QString::number(value.value[0]);
					break;
				case OUTLET:
					boundariesVector[index + 2] = "";
					break;
				case SYMMETRY:
					boundariesVector[index + 2] = QString::number(value.value[0] * 180 / M_PI) + " "
					+ QString::number(value.value[1] * 180 / M_PI) ;
					break;
			}
		} catch (...) {
			boundariesVector[index + 2] = "";
		}
		try {
			boundariesVector[index + 3] = QString::fromStdString(value.description);
		} catch (...) {
			boundariesVector[index + 3] = "";
		}
		index += 4;
	}
	return boundariesVector;
}

void Actions::updateBoundaries(bool saveToFile, bool pretty) {
	for (auto &[key, boundary]: boundaries) {
		if (key == 0)
			continue;
		auto type = root->findChild<QObject*>("boundaryComboBox" + QString::number(key))->property("currentIndex").toInt() + 1;
		auto value = root->findChild<QObject*>("boundaryValue" + QString::number(key))->property("text").toString();
		// split string
		auto values = value.split(" ");
		auto value1 = 0.0;
		auto value2 = 0.0;
		if (type == SYMMETRY) {
			value1 = values[0].toDouble() * M_PI / 180;
			value2 = values[1].toDouble() * M_PI / 180;
		} else {
			value1 = values[0].toDouble();
		}
		auto description = root->findChild<QObject*>("boundaryDescription" + QString::number(key))->property("text").toString();

		boundaries[key] = Boundary {
			uint(type),
			std::array<double, 2>{value1, value2},
			description.toStdString()
		};
		Nodes::setBoundaryConditions();
	}

	appendOutput("Boundaries updated");

	if (saveToFile){
		auto filepath = root->findChild<QObject*>("fileDialog")->property("fileUrl").toString();
		clearSubstring(filepath);

		auto filepathString = filepath.toStdString();

		try {
			Json::updateBoundaries(filepathString, pretty);
			appendOutput("Updated to " + filepath);
		} catch (const std::exception &e) {
			appendOutput("Error while updating boundaries: " + QString(e.what()));
		} catch (...) {
			appendOutput("Error while updating boundaries");
		}
	}
}

QString Actions::getRecession() {
	QString output = "";
	for (const auto &value: recession) {
		output +=  QString::number(value) + "\n";
	}
	output.chop(1);
	return output;
}
QString Actions::getRecession(QString filepath) {
	QString output = "";
	clearSubstring(filepath);
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		appendOutput("Error while opening file " + filepath);
		return "";
	}
	QTextStream in(&file);
	while (!in.atEnd()) {
		output += in.readLine() + "\n";
	}
	file.close();
	output.chop(1);
	return output;
}

void Actions::drawBurningArea(uint areas) {
	if (areas < 2 || currentIter == 0)
		return;
	auto data = burnAreaData(areas);
	auto &burnArea = data[0];
	auto &burnDepth = data[1];
	auto maxBurnArea = *max_element(burnArea.begin(), burnArea.end()) * 1.05;
	auto maxBurnDepth = *max_element(burnDepth.begin(), burnDepth.end()) * 1.05;

	emit graphBurningArea(burnDepth, burnArea, maxBurnDepth, maxBurnArea);
}

void Actions::updateErrorIter() {
	if (currentIter == 0)
		return;
	auto maxError = *max_element(errorIter.begin(), errorIter.end()) * 1.05;
	emit errorIterUpdate(errorIter, errorIter.size(), maxError);
}
