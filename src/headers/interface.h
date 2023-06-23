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
	void updateProgress(uint progress, uint total);
	void finished();
	void readFinished(bool success);
	void setCameraPosition(double x, double y, double z);
	void loadMeshPreview(QString);
	void setCulling(uint method);
	void graphBurningArea(std::vector<double> depth, std::vector<double> area, double xMax, double yMax);
	void graphErrorIter(std::vector<uint> iteration, std::vector<double> error, uint xMax, double yMax);

public slots:
	void readMesh(QString path);
	void readMeshWorker(QString path);
	void afterReadMesh(bool success);
	void run();
	void stop();
	void appendOutput(QString text);
	void worker();
	void afterWorker();
	void previewIsosurface(double value);
	void updateIsosurface(double value);
	void clearCache();
	void setCullingMethod(uint method);
	void exportData(QString filepath, bool pretty);
	std::vector<QString> getBoundaries();
	void updateBoundaries(bool saveToFile, bool pretty);
	QString getRecession();
	QString getRecession(QString filepath);
	void drawBurningArea(uint areas);
};

