/*
 * geometric3D.cpp
 *
 *  Created on: May 11, 2026
 *      Author: Tom Teplick
 */

#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <stdexcept>
#include "geometric3D.h"

// static constant members
const std::string GeoObject::geometricobject  = "geometricobject.txt"; // 3D geometric object file containing the densities, 50x50x50
const std::string GeoObject::dataDir = "..\\data\\"; // directory for object references dimensions and masses
const std::string GeoObject::geometricrefdims = "geometricrefdim.txt"; // row/column dimensions of references
const double GeoObject::pi = 3.14159265358979;

// the geometric objects that can be created
// do not change order, synchronized with geofunc[]
const char* geometricObjects[] = {
	  "ellipsoidsurface",
	  "ellipsoidsolid",
	  "plane",
	  "paraboloid",
	  "paraboloidsolid",
	  "cube",
	  "cone",
	  "conesolid",
	  "box",
	  "hyperbolicparaboloid",
	  "cylindersurface",
	  "cylindersolid",
	  "potentialwell",
	  "cardioidrevolution",
	  "cardioidrevolutionsolid",
	  "lemniscaterevolution",
	  "lemniscaterevolutionsolid",
	  "rose4leafrevolution",
	  "rose4leafrevolutionsolid",
	  "torussolid",
};

// add noise to the geometric object's density and shift the location of the geometric object
void GeoObject::addNoiseShift()
{
	// Save geo.density to a temp file, first open for writing
	const std::string tempfile = "tempdensity.txt";
	std::fstream fstrm;
	fstrm.open((GeoObject::dataDir + tempfile).c_str(), std::fstream::out);
	if (!fstrm.is_open()) {
			std::cout << "addNoiseShift: cannot open for writing" << tempfile << std::endl;
			fstrm.close();
			throw std::runtime_error("addNoiseShift: cannot open for writing" + tempfile);
	}

	// space-separated densities written to file
	for (const auto &dim1 : density) {
		for (const auto &dim2 : dim1) {
			for (const auto &dim3  : dim2) {
				fstrm << dim3 << " ";
			}
			fstrm << std::endl;
		}
	}

	// close temp density file
	fstrm.close();

	// now open the temp density file for reading
	fstrm.open((GeoObject::dataDir + tempfile).c_str(), std::fstream::in);
	if (!fstrm.is_open()) {
			std::cout << "addNoiseShift: cannot open for reading " << tempfile << std::endl;
			fstrm.close();
			throw std::runtime_error("addNoiseShift:  cannot open for reading " + tempfile);
	}

	// Clear density
	for (auto &dim1 : density) {
		for (auto &dim2 : dim1) {
			for (auto &dim3 : dim2) {
				dim3 = 0;
			}
		}
	}

	// Read in density file and place in a shifted position with density noise
	// Read the geometric object file containing the densities
	// compute noise and shift


	int signi = 1;
	if (std::rand()%2 > 0)
		signi = -1;
	int signj = 1;
	if (std::rand()%2 > 0)
		signj = -1;
	int signk = 1;
	if (std::rand()%2 > 0)
		signk = -1;

	int ishift = signi * std::rand()%GeoObject::planeDim/6;
	int jshift = signj * std::rand()%GeoObject::planeDim/6;
	int kshift = signk * std::rand()%GeoObject::planeDim/6;

	int deltaI = 0;
	int deltaJ = 0;
	int deltaK = 0;
	int junk = 0;
	for (int i = 0; i < planeDim; i++) {
		deltaI = i + ishift;
		for (int j = 0; j < planeDim; j++) {
			deltaJ = j + jshift;
			for (int k = 0; k < planeDim; k++) {
				deltaK = k + kshift;
				if ((deltaI >= 0 && deltaI < planeDim) && (deltaJ >= 0 && deltaJ < planeDim) &&
					(deltaK >= 0 && deltaK < planeDim)) {
					fstrm >> density[deltaI][deltaJ][deltaK];
					// add noise, (+/-)noiseLevel/2
				    double noise = (double)noiseLevel * ((double)std::rand()/(double)RAND_MAX - 0.5);
				    double trial = (double)density[deltaI][deltaJ][deltaK] + noise;
					density[deltaI][deltaJ][deltaK] = char(fmin(fmax(0.0, trial), 9.0));
				} else {
					fstrm >> junk;
				}
			}
		}
	}
	fstrm.close();
}

// plane, surface
void GeoObject::createPlane()
{
	// choose center of plane (x1, y1, z1) = (25, 25, 25)
	// vary x, y, in (0, 49)
	// Normal to plane is Ai + Bj + Ck
	// dot product: A(x-x1) + B(y-y1) + C(z-z1) = 0
	// Ax + By + Cz = D, => D=A*x1+B*y1+C*z1
	int x1 = GeoObject::planeDim / 2;
	int y1 = GeoObject::planeDim / 2;
	int z1 = GeoObject::planeDim / 2;
	int A = 2;
	int B = 3;
	int C = 4;
	int a = x1 / 2;
	int b = y1 / 2;
	int c = z1 / 2;

	int D = A*x1 + B*y1 + C*z1;
	char black = 9;

	for (int x = x1 - a; x < x1+a; x++) {
		for (int y = y1 - b; y < y1+b; y++) {
			int z = (D - A*x - B*y) / C;
			// this point is inside the data space and in the plane
			if (z >= z1-c && z < z1+c) {
				density[x][y][z] = black;
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// cardioid of revolution, surface
void GeoObject::createCardioidRevolution()
{
	// use polar coordinates, (r, theta), 0<=theta<2pi
	// r=a(1-cos(theta)), rotate cardioid(r,theta) about x axis to create 3D surface
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1 / 2;
	// one degree resolution
	double del =  pi / 180.0;
	int shiftx = x1 + a - a/8;
	char black = 9;
	// loop over theta, 0<=theta<180
	double theta = 0.0;
	for (int i = 0; i < 180; i++) {
		//   calculate r
		double r = double(a) * (1.0 - std::cos(theta));
		// calculate x=r*cos(theta), for (+/-) theta
		double xminus = r * std::cos(-theta);
		double xplus = r * std::cos(theta);
		// calculate h=r*sin(theta)
		double h = r * std::sin(theta);
		double phi = 0.0;
		// loop over phi, 0<=phi<180
		for (int j = 0; j < 180; j++) {
			// z=h*sin(phi), for (+/-) phi
			double z = h * std::sin(phi);
			double y = h * std::cos(phi);
			// calculate density for (+/-) theta and phi
			// translate x to (0,planeDim) with planeDim/2+a-a/8 = shiftx
			// translate (y,z) to (0,planeDim) with planeDim/2
			density[y1+int(y)][z1+int(z)][int(xminus)+shiftx] = black;
			density[y1+int(y)][z1+int(z)][int(xplus)+shiftx] = black;
			density[y1+int(y)][z1+int(-z)][int(xminus)+shiftx] = black;
			density[y1+int(y)][z1+int(-z)][int(xplus)+shiftx] = black;
			phi += del;
		}
		theta += del;
	}
	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// cardiod of revolution, solid with varying density
void GeoObject::createCardioidRevolutionSolid()
{
	// use polar coordinates, (r, theta), 0<=theta<2pi
	// r=a(1-cos(theta)), rotate cardioid(r,theta) about x axis to create 3D surface
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1 / 2;
	double norm = double(2 * a);
	char density2;
	// one degree resolution
	double del = pi / 180.0;
	int shiftx = x1 + a - a/8 - 1;
	double black = 9.0;
	int nrsteps = 3;
	double rstep = 1.0 / double(nrsteps);
	for (int i = a; i > 0; i--) {
		// loop over theta, 0<=theta<180
		double theta = 0.0;
		for (int j = 0; j < 180; j++) {
			for (int n = 0; n < nrsteps; n++) {
				// calculate r
				double k = double(i) - double(n)*rstep;
				double r = double(k) * (1.0 - std::cos(theta));
				// calculate x=r*cos(theta), for (+/-) theta
				double xminus = r * std::cos(-theta);
				double xplus = r * std::cos(theta);
				// calculate h=r*sin(theta)
				double h = r * std::sin(theta);
				double phi = 0.0;
				// loop over phi, 0<=phi<180
				for (int m = 0; m < 180; m++) {
					// z=h*sin(phi), for (+/-) phi
					double z = h * std::sin(phi);
					double y = h * std::cos(phi);
					// calculate density for (+/-) theta and phi
					// translate x to (0,planeDim) with planeDim/2+a-a/8 = shiftx
					// translate (y,z) to (0,planeDim) with planeDim/2
					// center is the most dense, decreasing as you move away from center
					density2 = char(black * (1.0 - r/norm));
					density[y1+int(y)][z1+int(z)][int(xminus)+shiftx] = density2;
					density[y1+int(y)][z1+int(z)][int(xplus)+shiftx] = density2;
					density[y1+int(y)][z1+int(-z)][int(xminus)+shiftx] = density2;
					density[y1+int(y)][z1+int(-z)][int(xplus)+shiftx] = density2;
					phi += del;
				}
			}
			theta += del;
		}
	}
	density[y1][z1][shiftx] = char(black);

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// lemniscate of revolution, surface
void GeoObject::createLemniscateRevolution()
{
	// use polar coordinates, (r, theta), 0<=theta<2pi
	// r^2=2*a^2*cos(2*theta), rotate lemniscate(r,theta) about x-axis to create 3D surface
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1 / 2;
	// use 1/2 degree resolution
	double del = pi / 360.0;
	char black = 9;
	// loop over theta, 0<=theta<45, and use symmetry to find other values
	double theta = 0.0;
	for (int i = 0; i < 90; i++) {
		// calculate r
		double r = double(a) * sqrt(std::cos(2.0*theta));
		// calculate x=r*cos(theta), first quadrant, use symmetry for others
		double x = r * std::cos(theta);
		// calculate h=r*sin(theta)
		double h = r * std::sin(theta);
		double phi = 0.0;
		// loop over phi, 0<=phi<90
		for (int j = 0; j < 180; j++ ) {
			// z=h*sin(phi), for (+/-) phi
			// y=h*cos(phi), for (+/-) phi
			double z = h * std::sin(phi);
			double y = h * std::cos(phi);
			// calculate density for (+/-) theta and phi
			// translate (x,y,z) to (0,planeDim) with planeDim/2
			density[y1+int(y)][z1+int(z)][int(x)+x1] = black;
			density[y1+int(y)][z1+int(z)][int(-x)+x1] = black;

			density[y1+int(-y)][z1+int(z)][int(x)+x1] = black;
			density[y1+int(-y)][z1+int(z)][int(-x)+x1] = black;

			density[y1+int(y)][z1+int(-z)][int(x)+x1] = black;
			density[y1+int(y)][z1+int(-z)][int(-x)+x1] = black;

			density[y1+int(-y)][z1+int(-z)][int(x)+x1] = black;
			density[y1+int(-y)][z1+int(-z)][int(-x)+x1] = black;

			phi += del;
		}
		theta += del;
	}
	density[y1][z1][x1] = black;

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// lemniscate of revolution, solid with varying density
void GeoObject::createLemniscateRevolutionSolid()
{
	// use polar coordinates, (r, theta), 0<=theta<2pi
	// r^2=2*a^2*cos(2*theta), rotate lemniscate(r,theta) about x-axis to create 3D surface
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = 3 * x1 / 4;
	// use one degree resolution
	double del = pi / 180.0;
	double black = 9.0;
	int nrsteps = 3;
	double rstep = 1.0 / double(nrsteps);
	double norm = double(a);
	char density2;
	for (int i = a; i > 0; i--) {
		// loop over theta, 0<=theta<45, and use symmetry to find other values
		double theta = 0.0;
		for (int j = 0; j < 45; j++) {
			for (int n = 0; n < nrsteps; n++) {
				// calculate r
				double k = double(i) - double(n)*rstep;
				double r = k * sqrt(std::cos(2.0*theta));
				// calculate x=r*cos(theta), first quadrant, use symmetry for others
				double x = r * std::cos(theta);
				// calculate h=r*sin(theta)
				double h = r * std::sin(theta);
				double phi = 0.0;
				// loop over phi, 0<=phi<90
				for (int m = 0; m < 90; m++) {
					// z=h*sin(phi), for (+/-) phi
					// y=h*cos(phi), for (+/-) phi
					double z = h * std::sin(phi);
					double y = h * std::cos(phi);
					density2 = char(black * (1.0 - r/norm));
					// calculate density for (+/-) theta and phi
					// translate (x,y,z) to (0,planeDim) with planeDim/2
					density[y1+int(y)][z1+int(z)][int(x)+x1] = density2;
					density[y1+int(y)][z1+int(z)][int(-x)+x1] = density2;

					density[y1+int(-y)][z1+int(z)][int(x)+x1] = density2;
					density[y1+int(-y)][z1+int(z)][int(-x)+x1] = density2;

					density[y1+int(y)][z1+int(-z)][int(x)+x1] = density2;
					density[y1+int(y)][z1+int(-z)][int(-x)+x1] = density2;
					density[y1+int(-y)][z1+int(-z)][int(x)+x1] = density2;
					density[y1+int(-y)][z1+int(-z)][int(-x)+x1] = density2;
					phi += del;
				}
			}
			theta += del;
		}
	}
	density[y1][z1][x1] = char(black);

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// Four-leaved rose of revolution, surface
void GeoObject::createRose4LeafRevolution()
{
	// r=4*sin(2*theta), rotate Rose4Leaf(r,theta) about x-axis to create 3D surface
	// use polar coordinates, (r, theta), 0<=theta<2pi
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	// one degree resolution
	double del = pi / 180.0;
	char black = 9;
	// loop over theta, 0<=theta<90, and use symmetry to find other values
	double theta = 0.0;
	for (int i = 0; i < 90;i++) {
		// calculate r
		double r = double(3*x1/4) * std::sin(2.0*theta);
		// calculate x=r*cos(theta), first quadrant, use symmetry for others
		double x = r * std::cos(theta);
		// calculate h=r*sin(theta)
		double h = r * std::sin(theta);
		double phi = 0.0;
		// loop over phi, 0<=phi<180
		for (int j = 0; j < 180; j++) {
			// z=h*sin(phi), for (+/-) phi
			// y=h*cos(phi), for (+/-) phi
			double z = h * std::sin(phi);
			double y = h * std::cos(phi);
			// calculate density for (+/-) theta and phi
			// translate (x,y,z) to (0,planeDim) with planeDim/2
			density[y1+int(y)][z1+int(z)][int(x)+x1] = black;
			density[y1+int(y)][z1+int(-z)][int(x)+x1] = black;
			density[y1+int(y)][z1+int(z)][int(-x)+x1] = black;
			density[y1+int(y)][z1+int(-z)][int(-x)+x1] = black;
			phi += del;
		}
		theta += del;
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// Four-leaved rose of revolution, solid with varying density
void GeoObject::createRose4LeafRevolutionSolid()
{
	// r=4*sin(2*theta), rotate Rose4Leaf(r,theta) about x-axis to create 3D surface
	// use polar coordinates, (r, theta), 0<=theta<2pi
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = 3 * x1 / 4;
	// one degree resolution
	double del = pi / 180.0;
	double black = 9.0;

	int nrsteps = 3;
	double rstep = 1.0 / double(nrsteps);
	double norm = double(a);
	char density2;
	for (int i = a; i >= 0; i--) {
		// loop over theta, 0<=theta<90, and use symmetry to find other values
		double theta = 0.0;
		for (int j = 0; j < 90; j++) {
			for (int n = 0; n < nrsteps; n++) {
				double k = double(i) - double(n)*rstep;
				// calculate r
				double r = double(k) * std::sin(2.0*theta);
				// calculate x=r*cos(theta), first quadrant, use symmetry for others
				double x = r * std::cos(theta);
				// calculate h=r*sin(theta)
				double h = r * std::sin(theta);
				double phi = 0.0;
				// loop over phi, 0<=phi<180
				for (int m = 0; m < 180; m++) {
					// z=h*sin(phi), for (+/-) phi
					// y=h*cos(phi), for (+/-) phi
					double z = h * std::sin(phi);
					double y = h * std::cos(phi);
					// calculate density for (+/-) theta and phi
					// translate (x,y,z) to (0,planeDim) with planeDim/2
					density2 = char(black * (1.0 - r/norm));
					density[y1+int(y)][z1+int(z)][int(x)+x1] = density2;
					density[y1+int(y)][z1+int(-z)][int(x)+x1] = density2;
					density[y1+int(y)][z1+int(z)][int(-x)+x1] = density2;
					density[y1+int(y)][z1+int(-z)][int(-x)+x1] = density2;
					phi += del;
				}
			}
			theta += del;
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// potential well, surface, amount of work required to move from 1 to r
// in height above earth
void GeoObject::createPotentialWell()
{
	// use polar coordiates, (r,theta), r>=1, 0<=theta<2pi
	// w = k*(1-1/r), k=planeDim/2
	char black = 9;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int rmax = 3 * z1 / 4;
	int z1c = z1 + rmax/2 - 1;
	double del = 2.0 * pi / 360.0;
	// loop r from 1 to planeDim/2
	for (int r = 1; r <= rmax; r++) {
		//   loop theta from 0 to 2pi
		double theta = 0.0;
		for (int i = 0; i < 360; i++) {
			// x=r*cos(theta), y=r*sin(theta), z=k*(1-1/r)
			double x = double(r) * std::cos(theta);
			double y = double(r) * std::sin(theta);
			double z = double(r) * (1.0 - 1.0/double(r));
			//   center x,y,z in (0,50) by adding planeDim/2
			density[z1c-int(z)][x1+int(x)][y1+int(y)] = black;
			theta += del;
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// elliptic cylinder, surface
void GeoObject::createCylinder()
{
	// center (x1,y1,z1)
	// (x)^2/a^2 + (y)^2/b^2 = 1
	char black = 9;
	double eps = .1;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1/2 - 3;
	int a2 = a * a;
	int b = y1/2 + 3;
	int b2 = b * b;
	int c = z1 / 2;
	for (int x = -a; x <= a; x++) {
		for (int y = -b; y <= b; y++) {
			double diff = 1.0 - double(x*x)/double(a2) - double(y*y)/double(b2);
			if (diff > -eps && diff < eps) {
				for (int z = -c; z <= c; z++) {
					density[x+x1][y+y1][z+z1] = black;
				}
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}

}

// elliptic cylinder, solid
void GeoObject::createCylinderSolid()
{
	// center (x1,y1,z1)
	// (x)^2/a^2 + (y)^2/b^2 = 1
	double black = 9.0;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1/2 - 3;
	int b = y1/2 + 3;
	int c = z1 / 2;
	char density2;
	double norm = double(a*a + b*b + c*c);
	// Fill in the ellipsoid while any axis is greater than or equal to zero
	for (int i = a; i >= 0; i--) {
		int a2 = i * i;
		for (int j = b; j >= 0; j--) {
			int b2 = j * j;
			for (int x = -i; x <= i; x++) {
				for (int y = -j; y <= j; y++) {
					double diff = 1.0 - double(x*x)/double(a2) - double(y*y)/double(b2);
					if (diff >= 0) {
						for (int z = -c; z <= c; z++) {
							double sumsq = double(x*x + y*y + z*z);
							// center is the most dense, decreasing as you move away from center
							density2 = char(black * (1.0 - std::sqrt(sumsq/norm)));
							density[x+x1][y+y1][z+z1] = density2;
						}
					}
				}
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// hyperbolic paraboloid, surface
void GeoObject::createHyperbolicParaboloid()
{
	// y^2/b^2 - x^2/a^2 = z/c
	char black = 9;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1 / 2;
	int a2 = a * a;
	int b = y1 / 2;
	int b2 = b * b;
	int c = z1 / 2;
	for (int x = -a; x <= a; x++) {
		for (int y = -b; y <= b; y++) {
			int z = int((double(y*y)/double(b2) - double(x*x)/double(a2)) * double(c));
			if (z >= -z1 && z <= z1) {
				density[z1+z][x1+x][y1+y] = black;
			}
		}
	}

	for (int y = -b; y <= b; y++) {
		for (int z = -c; z <= c; z++) {
			double tmp = double(y*y)/double(b2) - double(z)/double(c);
			if (tmp >= 0) {
				int x = int(sqrt(tmp * double(a2)));
				density[z1+z][x1+x][y1+y] = black;
			}
		}
	}

	for (int z = -c; z <= c; z++) {
		for (int x = -a; x <= a; x++) {
			double tmp = double(z)/double(c) + double(x*x)/double(a2);
			if (tmp >= 0) {
				int y = int(sqrt(tmp * double(b2)));
				density[z1+z][x1+x][y1+y] = black;
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// cube, solid with decreasing density from center
void GeoObject::createCube()
{
	double black = 9.0;
	// choose center of cube (x1,y1,z1) = (25,25,25)
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int del = planeDim / 4;
	double norm = 3.0 * double(del*del);
	for (int x = x1 - del; x < x1+del; x++) {
		for (int y = y1 - del; y < y1+del; y++) {
			for (int z = z1 - del; z < z1+del; z++) {
				double delx = double(x - x1);
				double dely = double(y - y1);
				double delz = double(z - z1);
				double sumsq = delx*delx + dely*dely + delz*delz;
				// center is the most dense, decreasing as you move away from center
				density[x][y][z] = char(black * (1.0 - sqrt(sumsq/norm)));
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}

}

// box, surface
void GeoObject::createBox()
{
	char black = 9;
	// choose center of box (x1,y1,z1) = (25,25,25)
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int del = planeDim / 4;
	for (int x = x1 - del; x <= x1+del; x++) {
		for (int y = y1 - del; y <= y1+del; y++) {
			density[x][y][z1-del] = black;
			density[x][y][z1+del] = black;
		}
	}
	for (int y = y1 - del; y <= y1+del; y++) {
		for (int z = z1 - del; z <= z1+del; z++) {
			density[x1-del][y][z] = black;
			density[x1+del][y][z] = black;
		}
	}
	for (int x = x1 - del; x <= x1+del; x++) {
		for (int z = z1 - del; z <= z1+del; z++) {
			density[x][y1-del][z] = black;
			density[x][y1+del][z] = black;
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// ellipsoid, surface
void GeoObject::createEllipsoid()
{
	// center (x1,y1,z1)
	// (x)^2/a^2 + (y)^2/b^2 + (z)^2/c^2 = 1
	char black = 9;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1/2 - 3;
	int a2 = a * a;
	int b = y1 / 2;
	int b2 = b * b;
	int c = z1/2 + 2;
	int c2 = c * c;
	for (int x = -a; x <= a; x++) {
		for (int y = -b; y <= b; y++) {
			double tmp = 1.0 - double(x*x)/double(a2) - double(y*y)/double(b2);
			if (tmp >= 0) {
				int z = int(sqrt(tmp * double(c2)));
				density[x+x1][y+y1][z+z1] = black;
				density[x+x1][y+y1][-z+z1] = black;
			}
		}
	}

	for (int y = -b; y <= b; y++) {
		for (int z = -c; z <= c; z++) {
			double tmp = 1.0 - double(z*z)/double(c2) - double(y*y)/double(b2);
			if (tmp >= 0) {
				int x = int(sqrt(tmp * double(a2)));
				density[x+x1][y+y1][z+z1] = black;
				density[-x+x1][y+y1][z+z1] = black;
			}
		}
	}

	for (int z = -c; z <= c; z++) {
		for (int x = -a; x <= a; x++) {
			double tmp = 1.0 - double(z*z)/double(c2) - double(x*x)/double(a2);
			if (tmp >= 0) {
				int y = int(sqrt(tmp * double(b2)));
				density[x+x1][y+y1][z+z1] = black;
				density[x+x1][-y+y1][z+z1] = black;
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// ellipsoid, solid with varying density
void GeoObject::createEllipsoidSolid()
{
	// center (x1,y1,z1)
	// (x)^2/a^2 + (y)^2/b^2 + (z)^2/c^2 = 1
	double black = 9.0;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	// assign axis maximums
	int a = x1/2 - 4;
	int b = y1 / 2;
	int c = z1/2 + 4;
	double norm = double(a*a + b*b + c*c);
	char density2;
	// Fill in the ellipsoid while any axis is greater than or equal to zero
	for (int i = a; i >= 0; i--) {
		int a2 = i * i;
		for (int j = b; j >= 0; j--) {
			int b2 = j * j;
			for (int k = c; k >= 0; k--) {
				int c2 = k * k;
				for (int x = -i; x <= i; x++) {
					for (int y = -j; y <= j; y++) {
						double tmp = 1.0 - double(x*x)/double(a2) - double(y*y)/double(b2);
						if (tmp >= 0) {
							int z = int(sqrt(tmp * double(c2)));
							double sumsq = double(x*x + y*y + z*z);
							// center is the most dense, decreasing as you move away from center
							density2 = char(black * (1.0 - sqrt(sumsq/norm)));
							density[x+x1][y+y1][z1+z] = density2;
							density[x+x1][y+y1][z1-z] = density2;
						}
					}
				}
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// elliptic cone, surface
void GeoObject::createCone()
{
	// center (x1,y1,z1)
	// (x)^2/a^2 + (y)^2/b^2 = (z)^2/c^2
	char black = 9;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1 / 2;
	int a2 = a * a;
	int b = y1 / 2;
	int b2 = b * b;
	int c = z1;
	int c2 = c * c;
	int z1c = z1 + c/2;
	for (int x = -a; x <= a; x++) {
		for (int y = -b; y <= b; y++) {
			int z = int(sqrt((double(x*x)/double(a2) + double(y*y)/double(b2)) * double(c2)));
			density[z1c-z][x1+x][y1+y] = black;
		}
	}

	for (int y = -b; y <= b; y++) {
		for (int z = 0; z <= c; z++) {
			double tmp = double(z*z)/double(c2) - double(y*y)/double(b2);
			if (tmp >= 0) {
				int x = int(sqrt(tmp * double(a2)));
				density[z1c-z][x1+x][y1+y] = black;
				density[z1c-z][x1-x][y1+y] = black;
			}
		}
	}

	for (int z = 0; z <= c; z++) {
		for (int x = -a; x <= a; x++) {
			double tmp = double(z*z)/double(c2) - double(x*x)/double(a2);
			if (tmp >= 0) {
				int y = int(sqrt(tmp * double(b2)));
				density[z1c-z][x1+x][y1+y] = black;
				density[z1c-z][x1+x][y1-y] = black;
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// elliptic cone, solid
void GeoObject::createConeSolid()
{
	// center (x1,y1,z1)
	// (x)^2/a^2 + (y)^2/b^2 = (z)^2/c^2
	double black = 9.0;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1 / 2;
	int a2 = a * a;
	int b = y1 / 2;
	int b2 = b * b;
	int c = z1;
	int c2 = c * c;
	int z1c = z1 + c/2;
	double norm = double(a*a + b*b + (c/2)*(c/2));
	char density2;
	/**************************************************************************/
	// Fill in the cone while any axis is greater than or equal to zero
	for (int i = a; i > 0; i--) {
		int a2 = i * i;
		for (int j = b; j > 0; j--) {
			int b2 = j * j;
			for (int k = c; k > 0; k--) {
				int k2 = k * k;
				int k1c = z1 + k/2;
				for (int x = -i; x <= i; x++) {
					for (int y = -j; y <= j; y++) {
						int z = int(sqrt((double(x*x)/double(a2) + double(y*y)/double(b2)) * double(k2)));
						if (z >= 0 && z <= k) {
							double sumsq = double(x*x + y*y + (z-k/2)*(z-k/2));
							// center is the most dense, decreasing as you move away from center
							density2 = char(black * (1.0 - sqrt(sumsq/norm)));
							density[k1c-z][x+x1][y+y1] = density2;
						}
					}
				}
			}
		}
	}

	//Miscellaneous problems
	for (int z = -c / 2; z <= c/2; z++) {
		int x = 0;
		int y = 0;
		double sumsq = double(x*x + y*y + z*z);
		char density2 = char(black * (1.0 - sqrt(sumsq/norm)));
		density[z1+z][x+x1][y+y1] = density2;
	}

	for (int z = 0; z < c; z++) {
		for (int x = -x1; x < x1; x++) {
			for (int y = -y1; y < y1; y++) {
				double test = sqrt((double(x*x)/double(a2) + double(y*y)/double(b2)) * double(c2));
				if (test > double(z)) {
					density[z1c-z][x1+x][y1+y] = 0;
				}
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// elliptic parabaloid, surface
void GeoObject::createParaboloid()
{
	// center (x1,y1,z1)
	// x^2/a^2 + y^2/b^2 = z/c
	char black = 9;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	int a = x1 / 2;
	int a2 = a * a;
	int b = y1 / 2;
	int b2 = b * b;
	int c = z1;
	int z1c = z1 + c/2;
	for (int x = -a; x < a; x++) {
		for (int y = -b; y < b; y++) {
			int z = int((double(x*x)/double(a2) + double(y*y)/double(b2)) * double(c));
			if (z1c >= z) {
				density[z1c-z][x+x1][y+y1] = black;
			}
		}
	}

	for (int y = -b; y < b; y++) {
		for (int z = 0; z <= c; z++) {
			double tmp = double(z)/double(c) - double(y*y)/double(b2);
			if (tmp >= 0) {
				int x = int((sqrt(tmp * double(a2))));
				density[z1c-z][x1+x][y1+y] = black;
				density[z1c-z][x1-x][y1+y] = black;
			}
		}
	}

	for (int z = 0; z <= c; z++) {
		for (int x = -a; x <= a; x++) {
			double tmp = double(z)/double(c) - double(x*x)/double(a2);
			if (tmp >= 0) {
				int y = int(sqrt(tmp * double(b2)));
				density[z1c-z][x1+x][y1+y] = black;
				density[z1c-z][x1+x][y1-y] = black;
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// create a solid paraboloid with varying density
void GeoObject::createParaboloidSolid()
{
	// center (x1,y1,z1)
	// x^2/a^2 + y^2/b^2 = z/c
	double black = 9.0;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	// assign axis maximums
	int a = x1 / 2;
	int b = y1 / 2;
	int c = z1;
	//c12 := c / 2;
	int z1c = z1 + c/2;
	double norm = double(a*a + b*b + (c/2)*(c/2));
	char density1;

	/**************************************************************************/
	// Fill in the paraboloid while any axis is greater than or equal to zero
	for (int i = a; i > 0; i--) {
		int a2 = i * i;
		for (int j = b; j > 0; j--) {
			int b2 = j * j;
			for (int k = c; k > 0; k--) {
				int k1c = z1 + k/2;
				for (int x = -i; x <= i; x++) {
					for (int y = -j; y <= j; y++) {
						int z = int((double(x*x)/double(a2) + double(y*y)/double(b2)) * double(k));
						if (z >= 0 && z <= k) {
							double sumsq = double(x*x + y*y + (z-k/2)*(z-k/2));
							// center is the most dense, decreasing as you move away from center
							char density1 = char(black * (1.0 - sqrt(sumsq/norm)));
							density[k1c-z][x+x1][y+y1] = density1;
						}
					}
				}
			}
		}
	}

	// Miscellaneous problems
	for (int z = -c / 2; z <= c/2; z++) {
		int x = 0;
		int y = 0;
		double sumsq = double(x*x + y*y + z*z);
		density1 = char (black * (1.0 - sqrt(sumsq/norm)));
		density[z1+z][x+x1][y+y1] = density1;
	}

	int a2 = a * a;
	int b2 = b * b;
	for (int z = 0; z < c; z++) {
		for (int x = -x1; x < x1; x++) {
			for (int y = -y1; y < y1; y++) {
				double test = (double(x*x)/double(a2) + double(y*y)/double(b2)) * double(c);
				if (test > double(z)) {
					density[z1c-z][x1+x][y1+y] = 0;
				}
			}
		}
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// Create a solid torus (doughnut, bagel, life buoy) with aspect ratio 2:1 (R/r)
// The density is inversely proportional to the distance from the r center.
void GeoObject::createTorusSolid()
{
	/* The major radius R is the distance from the center of the tube
	 * to the center of the torus and the minor radius r is the radius
	 * of the tube.  A torus is different than a solid torus, which
	 * is formed by rotating a disk, rather than a circle, around
	 * an axis. A solid torus is a torus plus the volume inside the torus.
	 * Real-world objects that approximate a solid torus include O-rings,
	 * non-inflatable lifebuoys, ring doughnuts, and bagels.
	 */
	// center (x1,y1,z1)
	// x(theta,phi) = (R + r*cos(theta))*cos(phi)
	// y(theta,phi) = (R + r*cos(theta))*sin(phi)
	// z(theta,phi) = r*sin(theta)

	double black = 9.0;
	int x1 = planeDim / 2;
	int y1 = planeDim / 2;
	int z1 = planeDim / 2;
	// assign radii, aspect ratio 2:1: R/r = 2/1
	const int R = x1/2;
	const int r = R/2;
	const int nzsteps = 10;
	char density2;
	// one degree resolution
	double del =  pi / 180.0;
	double norm = r*r;

	// loop, 0<=phi<360 using symmetry
	double phi = 0;
	for (int i = 0; i < 180; i++) {
		// loop, 0<=theta<360 using symmetry
		double theta = 0.0;
		for (int j = 0; j < 180; j++) {
			// loop to fill the disk with densities, 0<=k<=r
			for (int k = 0; k <= r; k++) {
				double kcostheta = k*std::cos(theta);
				double x = (R + kcostheta)*std::cos(phi);
				double y = (R + kcostheta)*std::sin(phi);

				// loop to fill the orthogonal axis plane not dependent on phi
				double zstep = double(k*std::sin(theta))/nzsteps;
				double z = 0.0;
				for (int m = 0; m <= nzsteps; m++) {
					// center is the most dense, decreasing as you move away from center
					density2 = char(black * (1.0 - std::sqrt(z*z+kcostheta*kcostheta)/norm));
					density[int(z)+z1][int(x)+x1][int(y)+y1] = density2;
					density[int(z)+z1][int(x)+x1][int(-y)+y1] = density2;
					density[int(-z)+z1][int(x)+x1][int(y)+y1] = density2;
					density[int(-z)+z1][int(x)+x1][int(-y)+y1] = density2;
					z += zstep;
				}
			}
			theta += del;
		}
		phi += del;
	}

	// add noise to this geometric object and shift its location
	if (shift) {
		// find maximum shift and choose random value in that range
		addNoiseShift();
	}
}

// create geometric references consisting of plane row/column mass sums and plane dimensions
void GeoObject::createGeometricReferences()
{

	// alias for a GeoObject member void function with no arguments
    using Geo = void(GeoObject::*)();
	// typedef void (*Geo)();


	// functions that construct the geometric objects
	// do not change order, synchronized with geometricObjects[]
	Geo geofunc[] = {
	    &GeoObject::createEllipsoid,
		&GeoObject::createEllipsoidSolid,
		&GeoObject::createPlane,
		&GeoObject::createParaboloid,
		&GeoObject::createParaboloidSolid,
		&GeoObject::createCube,
		&GeoObject::createCone,
		&GeoObject::createConeSolid,
		&GeoObject::createBox,
		&GeoObject::createHyperbolicParaboloid,
		&GeoObject::createCylinder,
		&GeoObject::createCylinderSolid,
		&GeoObject::createPotentialWell,
		&GeoObject::createCardioidRevolution,
		&GeoObject::createCardioidRevolutionSolid,
		&GeoObject::createLemniscateRevolution,
		&GeoObject::createLemniscateRevolutionSolid,
		&GeoObject::createRose4LeafRevolution,
		&GeoObject::createRose4LeafRevolutionSolid,
		&GeoObject::createTorusSolid,
	};

	// create geometric object reference dimension file
	std::fstream fdim;
	fdim.open((GeoObject::dataDir + geometricrefdims).c_str(), std::fstream::out);
	if (!fdim.is_open()) {
			std::cout << "createGeometricReferences: cannot open " << GeoObject::dataDir + geometricrefdims << std::endl;
			fdim.close();
			throw std::runtime_error("createGeometricReferences:  cannot open " + GeoObject::dataDir + geometricrefdims);
	}

	// loop over classes
	int i = 0;
	for (const auto &dim1 : geometricObjects) {
		//   create class specific file for the mass sums in each axis, plane, and row/column
		std::fstream fclass;
		fclass.open((GeoObject::dataDir + dim1 + ".txt").c_str(), std::fstream::out);
		if (!fclass.is_open()) {
				std::cout << std::string("createGeometricReferences: cannot open ") + dim1 + ".txt" << std::endl;
				fclass.close();
				throw std::runtime_error(std::string("addNoiseShift:  cannot open ") + dim1 + ".txt");
		}

		// clear the previous densities
		for (auto &dim1 : density) {
			for (auto &dim2 : dim1) {
				for (auto &dim3 : dim2) {
					dim3 = 0;
				}
			}
		}

		// construct this class but don't save to disk
		(this->*geofunc[i])();


		int rowFirst = planeDim;
		int rowLast  = 0;
		int colFirst = planeDim;
		int colLast  = 0;
		int sum      = 0;

        // axes are specified i, j, k in Cartesian coordinate system

		// ---------------------- axis = i ------------------------
		// loop over planes
		// For each plane, find the extent of the density
		// in the two axes, then find the mass for the rows and columns
		// inside these bounds.
		for (int plane = 0; plane < planeDim; plane++) {
			// find the row bounds
			// loop over rows from first to last
			int rowFirst = planeDim;
			for (int row = 0; row < planeDim; row++) {
				sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum the row densities, if non-zero, save this row number, break
					sum += int(density[plane][row][col]);
				}
				if (sum > 0) {
					rowFirst = row;
					break;
				}
			}

			// check if mass in this plane, if no mass go to next plane
			if (rowFirst == planeDim) {
				int nrows = 0;
				int ncols = 0;
				// write nrows and ncols to geometricrefdim on one line with space between
				fdim << nrows << " " << ncols << std::endl;
				continue;
			}

			// loop over rows from last to first
			for (int row = planeDim - 1; row >= 0; row--) {
				int sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum the densities, if non-zero, save this row number, break
					sum += int(density[plane][row][col]);
				}
				if (sum > 0) {
					rowLast = row;
					break;
				}
			}

			// find the column bounds
			// loop over cols from first to last
			for (int col = 0; col < planeDim; col++) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					//sum the col densities, if non-zero, save this col number, break
					sum += int(density[plane][row][col]);
				}
				if (sum > 0) {
					colFirst = col;
					break;
				}
			}

			// loop over cols from last to first
			for (int col = planeDim - 1; col >= 0; col--) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					//sum the col densities, if non-zero, save this col number, break
					sum += int(density[plane][row][col]);
				}
				if (sum > 0) {
					colLast = col;
					break;
				}
			}

			int nrows = rowLast - rowFirst + 1;
			int ncols = colLast - colFirst + 1;
			// write nrows and ncols to geometricrefdim on one line with space between
			fdim << nrows << " " << ncols << std::endl;

			// loop from first non-zero row sum to last row non-zero sum
			for (int row = rowFirst; row <= rowLast; row++) {
				int sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum each row density and write to file geometricrefmass on the same line with a space between
					sum += int(density[plane][row][col]);
				}
				fclass << sum << " ";
			}
			fclass << std::endl;

			// loop from first non-zero col sum to last non-zero col sum
			for (int col = colFirst; col <= colLast; col++) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					// sum each col density and write to file geometricrefmass on the same line with a space between
					sum += int(density[plane][row][col]);
				}
				fclass << sum << " ";
			}
			fclass << std::endl;
		}

		// -------------------------- axis = j -----------------------------
		// loop over planes
		// For each plane, find the extent of the density
		// in the two axes, then find the mass for the rows and columns
		// inside these bounds.
		for (int plane = 0; plane < planeDim; plane++) {
			// find the row bounds
			// loop over rows from first to last
			rowFirst = planeDim;
			for (int row = 0; row < planeDim; row++) {
				sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum the row densities, if non-zero, save this row number, break
					sum += int(density[row][plane][col]);
				}
				if (sum > 0) {
					rowFirst = row;
					break;
				}
			}

			// check if mass in this plane, if no mass go to next plane
			if (rowFirst == planeDim) {
				int nrows = 0;
				int ncols = 0;
				// write nrows and ncols to geometricrefdim on one line with space between
				fdim << nrows << " " << ncols << std::endl;
				continue;
			}

			// loop over rows from last to first
			for (int row = planeDim - 1; row >= 0; row--) {
				sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum the densities, if non-zero, save this row number, break
					sum += int(density[row][plane][col]);
				}
				if (sum > 0) {
					rowLast = row;
					break;
				}
			}

			// find the column bounds
			// loop over cols from first to last
			for (int col = 0; col <planeDim; col++) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					//sum the col densities, if non-zero, save this col number, break
					sum += int(density[row][plane][col]);
				}
				if (sum > 0) {
					colFirst = col;
					break;
				}
			}

			// loop over cols from last to first
			for (int col = planeDim - 1; col >= 0; col--) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					//sum the col densities, if non-zero, save this col number, break
					sum += int(density[row][plane][col]);
				}
				if (sum > 0) {
					colLast = col;
					break;
				}
			}

			int nrows = rowLast - rowFirst + 1;
			int ncols = colLast - colFirst + 1;
			// write nrows and ncols to geometricrefdim on one line with space between
			fdim << nrows << " " << ncols << std::endl;

			// loop from first non-zero row sum to last row non-zero sum
			for (int row = rowFirst; row <= rowLast; row++) {
				sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum each row density and write to file geometricrefmass on the same line with a space between
					sum += int(density[row][plane][col]);
				}
				fclass << sum << " ";
			}
			fclass << std::endl;

			// loop from first non-zero col sum to last non-zeron col sum
			for (int col = colFirst; col <= colLast; col++) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					// sum each col density and write to file geometricrefmass on the same line with a space between
					sum += int(density[row][plane][col]);
				}
				fclass << sum << " ";
			}
			fclass << std::endl;
		}

		// -------------------------- axis = k --------------------------------------
		// loop over planes
		// For each plane, find the extent of the density
		// in the two axes, then find the mass for the rows and columns
		// inside these bounds.
		for (int plane = 0; plane < planeDim; plane++) {
			// find the row bounds
			// loop over rows from first to last
			rowFirst = planeDim;
			for (int row = 0; row < planeDim; row++) {
				sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum the row densities, if non-zero, save this row number, break
					sum += int(density[row][col][plane]);
				}
				if (sum > 0) {
					rowFirst = row;
					break;
				}
			}

			// check if mass in this plane, if no mass go to next plane
			if (rowFirst == planeDim) {
				int nrows = 0;
				int ncols = 0;
				// write nrows and ncols to geometricrefdim on one line with space between
				fdim << nrows << " " << ncols << std::endl;
				continue;
			}

			// loop over rows from last to first
			for (int row = planeDim - 1; row >= 0; row--) {
				sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum the densities, if non-zero, save this row number, break
					sum += int(density[row][col][plane]);
				}
				if (sum > 0) {
					rowLast = row;
					break;
				}
			}

			// find the column bounds
			// loop over cols from first to last
			for (int col = 0; col < planeDim; col++) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					//sum the col densities, if non-zero, save this col number, break
					sum += int(density[row][col][plane]);
				}
				if (sum > 0) {
					colFirst = col;
					break;
				}
			}

			// loop over cols from last to first
			for (int col = planeDim - 1; col >= 0; col--) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					//sum the col densities, if non-zero, save this col number, break
					sum += int(density[row][col][plane]);
				}
				if (sum > 0) {
					colLast = col;
					break;
				}
			}

			int nrows = rowLast - rowFirst + 1;
			int ncols = colLast - colFirst + 1;
			// write nrows and ncols to geometricrefdim on one line with space between
			fdim << nrows << " " << ncols << std::endl;

			// loop from first non-zero row sum to last row non-zero sum
			for (int row = rowFirst; row <= rowLast; row++) {
				sum = 0;
				for (int col = 0; col < planeDim; col++) {
					// sum each row density and write to file geometricrefmass on the same line with a space between
					sum += int(density[row][col][plane]);
				}
				fclass << sum << " ";
			}
			fclass << std::endl;

			// loop from first non-zero col sum to last non-zeron col sum
			for (int col = colFirst; col <= colLast; col++) {
				sum = 0;
				for (int row = 0; row < planeDim; row++) {
					// sum each col density and write to file geometricrefmass on the same line with a space between
					sum += int(density[row][col][plane]);
				}
				fclass << sum << " ";
			}
			fclass << std::endl;
		}
		fclass.flush();
		fclass.close();
		i++;
	}
	fdim.close();
}

// create a geometric 3D object using its densities
void GeoObject::CreateObject(int geometricObject, int nl, bool sh)
{
	// initialize the density to zero
	for(auto &dim1 : density) {
		for (auto &dim2 : dim1) {
			for (auto &dim3 : dim2) {
				dim3 = 0;
			}
		}
	}

	// Assign the noise level and shift to the geometric object
	noiseLevel = nl;
	shift = sh;

	// determine the geometric surface/solid
	switch (geometricObject) {
	case 0:
		createEllipsoid();
		break;
	case 1:
		createEllipsoidSolid();
		break;
	case 2:
		createPlane();
		break;
	case 3:
		createParaboloid();
		break;
	case 4:
		createParaboloidSolid();
		break;
	case 5:
		createCube();
		break;
	case 6:
		createCone();
		break;
	case 7:
		createConeSolid();
		break;
	case 8:
		createBox();
		break;
	case 9:
		createHyperbolicParaboloid();
		break;
	case 10:
		createCylinder();
		break;
	case 11:
		createCylinderSolid();
		break;
	case 12:
		createPotentialWell();
		break;
	case 13:
		createCardioidRevolution();
		break;
	case 14:
		createCardioidRevolutionSolid();
		break;
	case 15:
		createLemniscateRevolution();
		break;
	case 16:
		createLemniscateRevolutionSolid();
		break;
	case 17:
		createRose4LeafRevolution();
		break;
	case 18:
		createRose4LeafRevolutionSolid();
		break;
	case 19:
		createTorusSolid();
		break;
	case 20:
		createGeometricReferences();
		break;
	default:
		std::cout << "create geometric object unknown case" << std::endl;
		throw std::runtime_error("create geometric object unknown geometric object");
	}

	// Save geometric object
	std::fstream fgeo;
	fgeo.open((GeoObject::dataDir + GeoObject::geometricobject).c_str(), std::fstream::out);
	if (!fgeo.is_open()) {
			std::cout << "createGeometricReferences: cannot open " << GeoObject::geometricobject << std::endl;
			fgeo.close();
			throw std::runtime_error("addNoiseShift:  cannot open " + GeoObject::geometricobject);
	}

	for (const auto &dim1 : density) {
		for (const auto &dim2 : dim1) {
			for (int dim3 : dim2) {
				fgeo << dim3 << " ";
			}
			fgeo << std::endl;
		}
	}
	fgeo.close();
}


