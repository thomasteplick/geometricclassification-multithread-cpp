/*
 * displaygeometric.h
 *
 *  Created on: Jun 2, 2026
 *      Author: Thomas Teplick
 */

#ifndef DISPLAYGEOMETRIC_H_
#define DISPLAYGEOMETRIC_H_

// Geometric display object
class GeoDisplay {
	int cls;  // class or the geometric object
	int axis;  // i, j, k of Cartesian coordinate system
	int startPlane;
	int stopPlane;

	void displayPlanes();
public:
	static const int nclasses = 20;  // number of geometric objects
    static const int planeDim=50;  // number of planes in each axis
	static const int naxes = 3;     // number of axes in Cartesian coordinate system
    static const std::string dataDir;
	static const std::string geometricobject; // 3D geometric object file containing the densities, 50x50x50
    GeoDisplay() : cls{0}, axis{0}, startPlane{0}, stopPlane{0}{}
	~GeoDisplay() = default;
	GeoDisplay(const GeoDisplay& disp) = delete;
	GeoDisplay& operator=(const GeoDisplay& disp) = delete;

	void displayClass();
private:
	int density[planeDim][planeDim][planeDim];
};

#endif /* DISPLAYGEOMETRIC_H_ */
