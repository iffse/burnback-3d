#pragma once

#include <QString>
#include <QObject>
#include <QVariant>

class Actions: public QObject {
	Q_OBJECT
public:
	explicit Actions(QObject *parent = nullptr);

signals:
	void newOutput(QString output);
	void graphBurningArea(QVariant points, double xMax, double yMax);
	void graphErrorIter(std::vector<double> points, double errorMax);
	void updateProgress(uint progress, uint total);
	void finished();
	void readFinished(bool success);

public slots:
	void readMesh(QString path);
	void readMeshWorker(QString path);
	void afterReadMesh(bool success);
	void run();
	void stop();
	void appendOutput(QString text);
	void worker();
	void afterWorker();
	// void exportData(QString filepath, bool pretty);
	// std::vector<QString> getBoundaries();
	// void updateBoundaries(bool saveToFile, bool pretty);
	// QString getRecession();
	// QString getRecession(QString filepath);
};
