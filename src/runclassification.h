/*
 * GeometricClassification.h
 *
 *  Created on: May 11, 2026
 *      Author: Tom Teplick
 */

#ifndef GEOMETRICCLASSIFICATION_H_
#define GEOMETRICCLASSIFICATION_H_

#include <string>
#include <queue>
#include <mutex>

// test statistics that are tabulated in HTML
struct Results {
	int cls; // class number
	std::string geometric; //name of geometric object
	int correct; // percent correct
	int count; // number of training examples in the class
};

struct PlaneDim {
	int nrows;
	int ncols;
};

// masses for a plane in the two coordinates
struct PlaneMass
{
	static const int planeDim = 50;  // number of planes in each axis
	int row[planeDim];
	int col[planeDim];
};

// classification results
struct Stats
{
	static const int nclasses = 20;  // number of geometric classes
	int correct[nclasses]; // % correct classifcation
	int classCount[nclasses]; // #samples in each class
};

void handleGeometricClassification();

void handleGeometricDisplay();

class Geometric
{
	int nsamples;            // #samples to classify
	int noiseLevel;            // noise level in the samples
	bool shift;           // shift the geometric object
	Stats statistics;
	int totalCount;    // total test samples
	int totalCorrect; // total correct classification
	int density[PlaneMass::planeDim][PlaneMass::planeDim][PlaneMass::planeDim]; // geometric object 3D densities
	void getPlaneMassError(int cls, int axis, int plane, std::queue<double>& planeQueue, std::mutex& planemtx);
	void getAxisMassError(int cls, int axis, std::queue<double>& axisQueue, std::mutex& axismtx);
	double searchPlaneReferences(int cls, int axis, int plane, int refMassPlane);
public:
	static const int naxes = 3;     // number of axes in Cartesian coordinate system
	static const std::string addr;   // http server listen address
	static const std::string geometricobject; // 3D geometric object file containing the densities, 50x50x50
	static const std::string geometricrefdims;  // dimension of references
	static const std::string dataDir;  // directory for geometric objects
	static const int axisDim = 100;                        // number of cells in each axis in y direction
	static const double deg2rad; // convert degrees to radians

	Geometric();
	~Geometric() = default;
	Geometric(const Geometric& geo) = delete;
	Geometric& operator=(const Geometric& geo) = delete;
	Geometric(Geometric&& geo) = delete;
	Geometric& operator=(Geometric&& geo) = delete;
	void classifyGeometric();
	void tabulateTestResults();
	void displayTestResults();

#ifdef TEST_DISPLAY
	friend void test_display();
#endif

private:
	PlaneMass geoRefMass[naxes][PlaneMass::planeDim];  // [axis][plane]
	PlaneDim geoRefDims[Stats::nclasses][naxes][PlaneMass::planeDim]; // [class][axis][plane]
};

#endif /* GEOMETRICCLASSIFICATION_H_ */
