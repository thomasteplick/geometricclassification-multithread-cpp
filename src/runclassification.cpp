//============================================================================
// Name        : runclassification.cpp
// Author      : Tom Teplick
// Version     :
// Copyright   : ThomasTeplick
// Description : GeometricClassification in C++, Ansi-style

// Test the display by defining macro TEST_DISPLAY in Project, Properties, C/C++ Build,
// Settings, GCC C++ Compiler, Preprocessor
// Add, Preprocessor Macro, TEST_DISPLAY.

// Developed using Eclipse C/C++ IDE 2025-06, GCC C++ compiler, mingw C++ linker.
//============================================================================

#include <iostream>
#include <string>
#include <exception>
#include <stdexcept>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <fstream>
#include <iomanip>
#include <ios>
#include <queue>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include "geometric3D.h"
#include "runclassification.h"
#include "displaygeometric.h"

// static constant members
const std::string Geometric::addr = "127.0.0.1:8080";   // http server listen address
const std::string Geometric::geometricobject = "geometricobject.txt"; // 3D geometric object file containing the densities, 50x50x50
const std::string Geometric::geometricrefdims = "geometricrefdim.txt";  // dimension of references
const std::string Geometric::dataDir = "..\\data\\";  // directory for geometric objects
const double Geometric::deg2rad = 3.14159265358979 / 180.0; // convert degrees to radians

extern const char *geometricObjects[];

// Find the reference mass plane with the least square error
double Geometric::searchPlaneReferences(int cls, int axis, int plane, int refMassPlane)
{
	/*  Find the square error between the reference mass and the sample
		There are five sections to consider:  the section with reference mass (1)
		and all others (4).  If the reference has no mass, then the squared error
		is just the square of all the mass in this sample; otherwise take the
	    difference between the reference and the sample and square it.
	*/


	struct SrchBnds {
		int rowStart;
		int rowEnd;
		int colStart;
		int colEnd;
	};

	// get the bounds (number of rows and columns) for this refMass plane
	int nrows = geoRefDims[cls][axis][refMassPlane].nrows;
	int ncols = geoRefDims[cls][axis][refMassPlane].ncols;
	int rowShifts = PlaneMass::planeDim - nrows;
	int colShifts = PlaneMass::planeDim - ncols;

	const int nsections = 4;

	double minSqErr = std::numeric_limits<double>::max();

	// shift the reference mass over the sample and find the shift having the min sq error
	// the allowable number of shifts is determined by the reference bounds
	switch (axis) {
	case 0:
		// if no reference plane mass, then error is the square of the row and col sums
		if (nrows == 0) {
			minSqErr = 0;
			for (int k = 0; k < rowShifts; k++) {
				for (int m = 0; m < colShifts; m++) {
					minSqErr += double(density[plane][k][m] * density[plane][k][m]);
				}
			}
			for (int m = 0; m < colShifts; m++) {
				for (int k = 0; k < rowShifts; k++) {
					minSqErr += double(density[plane][k][m] * density[plane][k][m]);
				}
			}
			return minSqErr;
		}

		// We have reference plane mass
		for (int i = 0; i < rowShifts;i++) {
			for (int j = 0; j < colShifts; j++) {
				SrchBnds section[nsections] = {
					{
						rowStart: 0,
						rowEnd:   i,
						colStart: 0,
						colEnd:   PlaneMass::planeDim,
					},
					{
						rowStart: i,
						rowEnd:   i + nrows,
						colStart: 0,
						colEnd:   j,
					},
					{
						rowStart: i,
						rowEnd:   i + nrows,
						colStart: j + ncols,
						colEnd:   PlaneMass::planeDim,
					},
					{
						rowStart: i + nrows,
						rowEnd:   PlaneMass::planeDim,
						colStart: 0,
						colEnd:   PlaneMass::planeDim,
					},
				};

				// These sections are outside the reference mass boundary and
				// the error only consists of the sample mass
				int sqErr = 0;
				for (int secn = 0; secn < nsections; secn++) {
					for (int row = section[secn].rowStart; row < section[secn].rowEnd; row++) {
						for (int col = section[secn].colStart; col < section[secn].colEnd; col++) {
							sqErr += int(density[plane][row][col] * density[plane][row][col]);
						}
					}
				}

				// this section contains reference mass so find the squared difference
				// between the reference mass and the sample

				// sum the rows
				for (int k = 0; k < geoRefDims[cls][axis][refMassPlane].nrows; k++) {
					int rowsum = 0;
					for (int m = 0; m < geoRefDims[cls][axis][refMassPlane].ncols; m++) {
						rowsum += int(density[plane][k+i][m+j]);
					}
					int diff = geoRefMass[axis][refMassPlane].row[k] - rowsum;
					sqErr += diff * diff;
				}
				// sum the columns and find the squared difference from reference
				for (int m = 0; m < geoRefDims[cls][axis][refMassPlane].ncols; m ++) {
					int colsum = 0;
					for (int k = 0; k < geoRefDims[cls][axis][refMassPlane].nrows; k++) {
						colsum += int(density[plane][k+i][m+j]);
					}
					int diff = geoRefMass[axis][refMassPlane].col[m] - colsum;
					sqErr += diff * diff;
				}
				if (double(sqErr) < minSqErr) {
					minSqErr = double(sqErr);
				}
			}
		}
		return double(minSqErr);
	case 1:
		// if no reference plane mass, then error is the square of the row and col sums
		if (nrows == 0) {
			minSqErr = 0;
			for (int k = 0; k < rowShifts; k++) {
				for (int m = 0; m < colShifts; m++) {
					minSqErr += double(density[k][plane][m] * density[k][plane][m]);
				}
			}
			for (int m = 0; m < colShifts; m++) {
				for (int k = 0; k < rowShifts; k++) {
					minSqErr += double(density[k][plane][m] * density[k][plane][m]);
				}
			}
			return minSqErr;
		}

		// We have reference plane mass
		for (int i = 0; i < rowShifts; i++) {
			for (int j = 0; j < colShifts; j++) {
				SrchBnds section[4] {
					{
						rowStart: 0,
						rowEnd:   i,
						colStart: 0,
						colEnd:   PlaneMass::planeDim,
					},
					{
						rowStart: i,
						rowEnd:   i + nrows,
						colStart: 0,
						colEnd:   j,
					},
					{
						rowStart: i,
						rowEnd:   i + nrows,
						colStart: j + ncols,
						colEnd:   PlaneMass::planeDim,
					},
					{
						rowStart: i + nrows,
						rowEnd:   PlaneMass::planeDim,
						colStart: 0,
						colEnd:   PlaneMass::planeDim,
					},
				};

				// These sections are outside the reference mass boundary and
				// the error only consists of the sample mass
				int sqErr = 0;
				for (int secn = 0; secn < nsections; secn++) {
					for (int row = section[secn].rowStart; row < section[secn].rowEnd; row++) {
						for (int col = section[secn].colStart; col < section[secn].colEnd; col++) {
							sqErr += int(density[row][plane][col] * density[row][plane][col]);
						}
					}
				}

				// this section contains reference mass so find the squared difference
				// between the reference mass and the sample

				// sum the rows
				for (int k = 0; k < geoRefDims[cls][axis][refMassPlane].nrows; k++) {
					int rowsum = 0;
					for (int m = 0; m < geoRefDims[cls][axis][refMassPlane].ncols; m++) {
						rowsum += int(density[k+i][plane][m+j]);
					}
					int diff = geoRefMass[axis][refMassPlane].row[k] - rowsum;
					sqErr += diff * diff;
				}
				// sum the columns and find the squared difference from reference
				for (int m = 0; m < geoRefDims[cls][axis][refMassPlane].ncols; m++) {
					int colsum = 0;
					for (int k = 0; k < geoRefDims[cls][axis][refMassPlane].nrows; k++) {
						colsum += int(density[k+i][plane][m+j]);
					}
					int diff = geoRefMass[axis][refMassPlane].col[m] - colsum;
					sqErr += diff * diff;
				}
				if (double(sqErr) < minSqErr) {
					minSqErr = double(sqErr);
				}
			}
		}
		return double(minSqErr);
	case 2:
		// if no reference plane mass, then error is the square of the row and col sums
		if (nrows == 0) {
			minSqErr = 0;
			for (int k = 0; k < rowShifts; k++) {
				for (int m = 0; m < colShifts; m++) {
					minSqErr += double(density[k][m][plane] * density[k][m][plane]);
				}
			}
			for (int m = 0; m < colShifts; m++) {
				for (int k = 0; k < rowShifts; k++) {
					minSqErr += double(density[k][m][plane] * density[k][m][plane]);
				}
			}
			return minSqErr;
		}

		// We have reference plane mass
		for (int i = 0; i < rowShifts; i++) {
			for (int j = 0; j < colShifts; j++) {
				SrchBnds section[nsections] = {
					{
						rowStart: 0,
						rowEnd:   i,
						colStart: 0,
						colEnd:   PlaneMass::planeDim,
					},
					{
						rowStart: i,
						rowEnd:   i + nrows,
						colStart: 0,
						colEnd:   j,
					},
					{
						rowStart: i,
						rowEnd:   i + nrows,
						colStart: j + ncols,
						colEnd:   PlaneMass::planeDim,
					},
					{
						rowStart: i + nrows,
						rowEnd:   PlaneMass::planeDim,
						colStart: 0,
						colEnd:   PlaneMass::planeDim,
					}
				};

				// These sections are outside the reference mass boundary and
				// the error only consists of the sample mass
				int sqErr = 0;
				for (int secn = 0; secn < nsections; secn++) {
					for (int row = section[secn].rowStart; row < section[secn].rowEnd; row++) {
						for (int col = section[secn].colStart; col < section[secn].colEnd; col++) {
							sqErr += int(density[row][col][plane] * density[row][col][plane]);
						}
					}
				}

				// this section contains reference mass so find the squared difference
				// between the reference mass and the sample

				// sum the rows
				for (int k = 0; k < geoRefDims[cls][axis][refMassPlane].nrows; k++) {
					int rowsum = 0;
					for (int m = 0; m < geoRefDims[cls][axis][refMassPlane].ncols; m++) {
						rowsum += int(density[k+i][m+j][plane]);
					}
					int diff = geoRefMass[axis][refMassPlane].row[k] - rowsum;
					sqErr += diff * diff;
				}

				// sum the columns and find the squared difference from reference
				for (int m = 0; m < geoRefDims[cls][axis][refMassPlane].ncols; m++) {
					int colsum = 0;
					for (int k = 0; k < geoRefDims[cls][axis][refMassPlane].nrows; k++) {
						colsum += int(density[k+i][m+j][plane]);
					}
					int diff = geoRefMass[axis][refMassPlane].col[m] - colsum;
					sqErr += diff * diff;
				}

				if (double(sqErr) < minSqErr) {
					minSqErr = double(sqErr);
				}
			}
		}
		return minSqErr;
	default:
		throw std::runtime_error("searchPlaneReferences: error invalid axis chosen");
    }
}

// get min sq error for this plane
void Geometric::getPlaneMassError(int cls, int axis, int plane, std::queue<double>& planeQueue, std::unique_lock<std::mutex>& planelck)
{
	/*
		Loop over all geoRefMass planes for this class, axis, and plane.  Call searchPlaneReferences()
		to find minimum square error.
	*/
	double minSqErr = std::numeric_limits<double>::max();

	for (int refMassPlane = 0; refMassPlane < PlaneMass::planeDim; refMassPlane++) {
		double sqErr = searchPlaneReferences(cls, axis, plane, refMassPlane);
		if (sqErr < minSqErr) {
			minSqErr = sqErr;
		}
	}

    // critical section (exclusive access to the planeQueue)
	planelck.lock();
	planeQueue.push(minSqErr);
    planelck.unlock();

}

// compute min square error for all planes in this axis and return
void Geometric::getAxisMassError(int cls, int axis, std::queue<double>& axisQueue, std::unique_lock<std::mutex>& axislck)
{
	constexpr int blockPlanes = 10;
	constexpr int block = PlaneMass::planeDim/blockPlanes;
	double minSqErr = 0.0;

	// Launch a thread for each plane to compute the square error and create
	// a synchronized queue to collect the square error.
	std::queue<double> planeQueue;
	std::thread planeThread[PlaneMass::planeDim];
	std::mutex mtx;
	std::unique_lock<std::mutex> planelck(mtx);
	for (int plane = 0; plane < PlaneMass::planeDim; plane++) {
		planeThread[plane] = std::thread(&Geometric::getPlaneMassError, this, cls, axis, plane, std::ref(planeQueue), std::ref(planelck));
	}

	// Wait for the threads to finish
	for (auto& th : planeThread) {
		th.join();
	}
	// loop over planes and get plane mass errors
	// sum the plane square errors
	while (!planeQueue.empty()) {
		minSqErr += planeQueue.front();
		planeQueue.pop();
	}

    // critical section (exclusive access to the axisQueue)
	axislck.lock();
    axisQueue.push(minSqErr);
    axislck.unlock();
}

// Construct a Geometric instance for classification
Geometric::Geometric()
{
	// get number of samples (10-100), noise level (0-9), and shift (true|false)

	std::cout << "Enter the number of samples, noise level, and shift, separated by space" << std::endl;
	std::cout << "50<=[samples]<= 500, 0<=[noise level]<= 9, [0=false|1=true]" << std::endl;
	std::cout << "For example, '50 3 1'" << std::endl;

	int nl;
	int ns;
	int sh;
	std::string response = "";
    std::cin >> ns >> nl >> sh;
    std::cout << "You entered: " << ns << " " << nl << " " << sh << std::endl;
    // verify values of parameters
    if ((ns < 50) || (ns > 500)) {
    	response += "number samples,";
    }
    if ((nl < 0) || (nl > 9)) {
    	response += "noise level,";
    }
    if ((sh != 0) && (sh != 1)) {
    	response += "shift";
    }
    if (response.size() > 0) {
    	throw std::runtime_error("geometric classification inputs outside of ranges: " + response);
    }
    noiseLevel = nl;
    nsamples = ns;
    shift = bool(sh);
    totalCorrect = 0;
    totalCount = 0;

	// Determine if the files containing the reference dimensions exist
	std::fstream fdim;
	fdim.open((dataDir+geometricrefdims).c_str(), std::fstream::in);

	if (!fdim.is_open()) {

	    // Create Geometric Object
	    GeoObject geobj;

		// create geometric references
		geobj.CreateObject(19, 0, false);
	}
	if (!fdim.is_open()) {
		fdim.open((dataDir+geometricrefdims).c_str(), std::fstream::in);
	}

	// read in the geometric reference dimensions in order:
	// [class][axis][plane]PlaneMass::planeDim
	// class 0, 1, ..., classes-1
	// axis 0, 1, 2
	// plane0 nrows,ncols
	// ...
	// plane49 nrows,ncols
	int nrows = 0;
	int ncols = 0;
	for (int i = 0; i < Stats::nclasses; i++) {
		for (int j = 0; j < naxes; j++) {
			for (int k = 0; k < PlaneMass::planeDim; k++) {
				fdim >> nrows >> ncols;
				geoRefDims[i][j][k] = PlaneDim{nrows: nrows, ncols: ncols};
			}
		}
	}
	fdim.close();
}

// classify the geometric objects
void Geometric::classifyGeometric()
{

    // Create Geometric Object
    GeoObject geobj;

    std::cout << "classifyGeometric, start loop over the samples\n";

    const int newLine = 50;
	// loop over the number of samples
	for (int sample = 0; sample < nsamples; sample++) {
		// newline every newLine samples
		if (sample%newLine == 0) {
			std::cout << std::endl;
		}
		std::cout << "*";
		// min sq mass error
		double minSqError = std::numeric_limits<double>::max();
		// class with min sq error
		int minClass = 0;
		// generate a random geometric object with noise level and shift using geoRefDims
		int ngeometricObj = std::rand()%(Stats::nclasses);

		geobj.CreateObject(ngeometricObj, noiseLevel, shift);

		// Open the geometric object file containing the densities
		std::fstream fgeometric;
		fgeometric.open((dataDir+geometricobject).c_str(), std::fstream::in);
		if (!fgeometric.is_open()) {
			std::cout << "cannot open file " + geometricobject << std::endl;
			throw std::runtime_error("cannot open file " + geometricobject);
		}

		// Read the geometric object file containing the densities
		for (int i = 0; i < PlaneMass::planeDim; i++) {
			for (int j = 0; j < PlaneMass::planeDim; j++) {
				for (int k = 0; k < PlaneMass::planeDim; k++) {
					fgeometric >> density[i][j][k];
				}
			}
		}
		fgeometric.close();

		// loop over geometric references and open one at a time
		for (int cls = 0; cls < Stats::nclasses; cls++) {
			// read geometric reference mass sums into memory for this class reference only
			std::fstream fgeoref;
			fgeoref.open((dataDir + geometricObjects[cls] + ".txt").c_str(), std::fstream::in);
			if (!fgeoref.is_open()) {
				std::cout << "cannot open file " << geometricObjects[cls] << ".txt" << std::endl;
				throw std::runtime_error(std::string("cannot open file ") + geometricObjects[cls] + ".txt");
			}
			for (int axes = 0; axes < naxes; axes++) {
				for (int plane = 0; plane < PlaneMass::planeDim; plane++) {
					int nrows = geoRefDims[cls][axes][plane].nrows;
					for (int m = 0; m < nrows; m++) {
						fgeoref >> geoRefMass[axes][plane].row[m];
					}

					int ncols = geoRefDims[cls][axes][plane].ncols;
					for (int n = 0; n < ncols; n++) {
						fgeoref >> geoRefMass[axes][plane].col[n];
					}
				}
			}
			// close file
			fgeoref.close();

			// find minimum mass error over rowsums and colsums for all axes and planes
			// use geoRefDims for shifting the object inside the planes
			double sqerr = 0.0;
			// Launch a thread for each plane to compute the square error and create
			// a synchronized queue to collect the square error.
			std::queue<double> axisQueue;
			std::thread axisThread[naxes];
			std::mutex mtx;
			std::unique_lock<std::mutex> axislck(mtx);
			for (int axis = 0; axis < naxes; axis++) {
				axisThread[axis] = std::thread(&Geometric::getAxisMassError, this, cls, axis, std::ref(axisQueue), std::ref(axislck));
				// wait for this axis thread to finish
				axisThread[axis].join();
				// collect the square error in the queue
				sqerr += axisQueue.front();
				axisQueue.pop();
			}

			/*
			// Wait for the threads to finish
			for (auto& th : axisThread) {
				th.join();
			}

			// collect the square error in the queue
			while (!axisQueue.empty()) {
				sqerr += axisQueue.front();
				axisQueue.pop();
			}
			*/

			if (sqerr < minSqError) {
				minSqError = sqerr;
				minClass = cls;
			}

		}

		// store geometric class count and correct classification
		statistics.classCount[ngeometricObj]++;
		if (minClass == ngeometricObj) {
			statistics.correct[ngeometricObj]++;
		}
	}
	std::cout << "\n\nleaving classifyGeometric\n";
}

// insert test results into table for display
void Geometric::tabulateTestResults()
{
	totalCount = 0;
	totalCorrect = 0;
	// tabulate TestResults
	for (int i = 0; i < Stats::nclasses; i++) {
		totalCount += statistics.classCount[i];
		totalCorrect += statistics.correct[i];
		if (statistics.classCount[i] > 0) {
			statistics.correct[i] =
				statistics.correct[i] * 100 / statistics.classCount[i];
		} else {
			statistics.correct[i] = 0;
		}
	}
}

void Geometric::displayTestResults()
{
	// Show class index, geometric name, class count,
	// and correct% for each geometric class.
	// Show totals below the classes.
	/*
	|======================================================|
	|Class  |Geometric                 |Count  |Correct (%)|
	|=======|==========================|=======|===========|
	|0      |ellipsoidsurface          |16     |100        |
	|16     |lemniscaterevolutionsolid |15     |100        |
	|------------------------------------------------------|
    |Totals |                          |100    |100        |
    |======================================================|
	*/
	const int w1 = 7;
	const int w2 = 26;
	const int w3 = 11;
	std::cout << std::endl;
	std::cout << "|======================================================|" << std::endl;
	std::cout << "|Class  |Geometric                 |Count  |Correct (%)|" << std::endl;
	std::cout << "|=======|==========================|=======|===========|" << std::endl;
	for (int cls = 0; cls < Stats::nclasses; cls++) { std::cout
				<< '|' << std::setw(w1) << std::left << cls
				<< '|' << std::setw(w2) << std::left << geometricObjects[cls]
				<< '|' << std::setw(w1) << std::left << statistics.classCount[cls]
				<< '|' << std::setw(w3) << std::left << statistics.correct[cls]  << '|' << std::endl;
	}
	std::cout << "|------------------------------------------------------|" << std::endl;
	std::cout << '|' << std::setw(w1) << std::left << "Totals" << '|'
			<< std::setw(w2) << std::left << "" << '|'
			<< std::setw(w1) << std::left << totalCount << '|'
			<< std::setw(w3) << std::left << totalCorrect * 100 / totalCount << '|' << std::endl;
	std::cout << "|======================================================|" << std::endl;
	std::cout << std::endl;
}

void handleGeometricDisplay()
{
	std::cout << "---------- Geometric Display Running ----------" << std::endl;

	// Create geometric display instance using class, axis, start plane, stop plane
	// Get choices in a loop to allow for continuous evaluation
	GeoDisplay geo;

	// Display the geometric object
	geo.displayClass();
}

void handleGeometricClassification()
{
	std::cout << "---------- Geometric Classification Running ----------" << std::endl;

	// initialize random number generator
	std::srand(std::time(0));

    // show start time
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    std::cout << std::string("Start local time and date: ") << std::string(asctime(timeinfo)) << std::endl;

	// Create geometric3D instance
	// Create geometric references if they don't exist by calling geometric3D member, create a Geometric object
	// Construct a Geometric instance for classification, pass number of samples and noise level
    // Create Geometric
    Geometric geo;

    std::cout << "Geometric geo constructed, start classifyGeometric\n";

	// classify the geometric object samples
    geo.classifyGeometric();

    std::cout << "tabulateTestResults\n";
	// tabulate the classification test results
    geo.tabulateTestResults();

    std::cout << "displayTestResults\n";
	// display the test results in tabular form
    geo.displayTestResults();

    // show finish time
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    std::cout << std::string("Finish local time and date: ") << std::string(asctime(timeinfo)) << std::endl;
}

#ifdef TEST_DISPLAY
// test the console display of geometric classification
void test_display()
{

	std::cout << "---------- Display Test Running ----------" << std::endl;

	// Create Geometric
    Geometric geo;

    double mean = double(geo.nsamples)/double(Stats::nclasses);
    int sign = 1.0;
    double k1 = .05;
    double k2 = .95;
    int count = 0;

	// fill in statistics
	for (int i = 0; i < Stats::nclasses-1; i++) {
		geo.statistics.classCount[i] = int(mean*(1.0 + sign*k1));
		geo.statistics.correct[i] = int(k2*geo.statistics.classCount[i]);
		sign *= -1.0;
		count += geo.statistics.classCount[i];
	}
	geo.statistics.classCount[Stats::nclasses-1] = geo.nsamples - count;
	geo.statistics.correct[Stats::nclasses-1] =
			int(k2*geo.statistics.classCount[Stats::nclasses-1]);

	// tabulate
	geo.tabulateTestResults();
	// display
	geo.displayTestResults();

}
#endif


int main() 
{
	// call handleGeometricClassification() in try/catch
	try {
#ifdef TEST_DISPLAY
		test_display();
#else
		int choice = 0;
		// select classify or display geometric objects
		std::cout << "Choose Classify Geometric = 1 or Display Geometric = 2:  ";
		std::cin >> choice;
		if ((choice != 1) && (choice != 2)) {
			std::cout << "choose 1 or 2, you entered " << choice << std::endl;
			throw std::runtime_error("choose 1 for Classify, choose 2 for Display");
		}
		switch (choice) {
		case 1:
			handleGeometricClassification();
			break;
		case 2:
			handleGeometricDisplay();
			break;
		default:
			std::cout << "Classify = 1, Display = 2, unknown case " << choice << std::endl;
		}
#endif
	}
	catch(std::system_error &se) {
		std::cout << "caught system_error " << se.what() << std::endl;
		auto ec = se.code();
		std::cout << "category: " << ec.category().name() << std::endl;
		std::cout << "value: " << ec.value() << std::endl;
		std::cout << "message: " << ec.message() << std::endl;
	}
	catch(std::runtime_error &re) {
		std::cout << "runtime error: " << re.what() << std::endl;
		return 1;
	}
	catch (std::exception &e)
	{
		std::cout << "standard exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cout << "unknown error" << std::endl;
		return 1;
	}
	return 0;
}

