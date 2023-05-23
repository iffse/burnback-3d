#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <chrono>
#include <cmath>
#include <thread>
#include <QFile>
#include <vector>

#include <src/headers/interface.h>
#include "src/headers/iosystem.h"
#include <src/headers/globals.h>
#include <src/headers/operations.h>

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

	std::thread thread(&Actions::worker, this);
	thread.detach();
}

void Actions::stop() {
	running = false;
}

void Actions::worker() {
	#ifdef DEBUG
	std::feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
	#endif
	if (!input.resume) {
		emit newOutput("--> Creating connectivity matrix");

		currentIter = 0;
		errorIter.clear();
		Geometry::computeGeometry();
		computationData = ComputationData(mesh.nodes.size(), mesh.tetrahedra.size());
	}
	emit newOutput("--> Starting subiteration loop");
	if (currentIter < input.targetIter)
		errorIter.resize(input.targetIter);

	QString linesToPrint = "";
	auto clock = std::chrono::system_clock::now();
	timeTotal = 0;

	for (; currentIter < input.targetIter; ++currentIter) {
		Tetrahedra::computeMeanGradient();
		Tetrahedra::computeFluxes();
		Triangles::ApplyBoundaryConditions();
		Nodes::computeResults();
		

		// auto error = getError();
		// errorIter[currentIter] = error;

		if (linesToPrint != "")
			linesToPrint += "\n";
		// linesToPrint += "Iteration: " + QString::number(currentIter + 1) + " Time: " + QString::number(timeTotal) + " Error: " + QString::number(error * 100) + "%";
		linesToPrint += "Iteration: " + QString::number(currentIter + 1) + " Time: " + QString::number(timeTotal);

		// if (error > 1) {
		// 	if (linesToPrint != "") {
		// 		emit newOutput(linesToPrint);
		// 		emit updateProgress(currentIter + 1, targetIter);
		// 	}
		// 	emit newOutput("Error: Divergence detected. Stopping. Try reducing the CFL.");
		// 	emit newOutput("--> Stopped");
		// 	root->findChild<QObject*>("runButton")->setProperty("text", "Run");
		// 	setBurningArea();
		// 	afterWorker();
		// 	return;
		// }

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
		emit updateProgress(currentIter + 1, input.targetIter);
	}

	emit newOutput("--> Subiteration ended");
	afterWorker();
}

void Actions::afterWorker() {
	root->findChild<QObject*>("runButton")->setProperty("text", "Run");

	for (uint i = 0; i < mesh.nodes.size(); ++i) {
		qDebug() << computationData.uVertex[i]; 
	}
}

