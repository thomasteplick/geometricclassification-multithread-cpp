/*
 * geometric3D.h
 *
 *  Created on: May 11, 2026
 *      Author: Thomas Teplick
 */

#ifndef GEOMETRIC3D_H_
#define GEOMETRIC3D_H_

#include <string>
#include <vector>

// 3D geometric object
class GeoObject
{
	int noiseLevel;
	bool shift;
    static const double pi;

	// Geometric object creators
	void addNoiseShift();
	void createPlane();
	void createCardioidRevolution();
	void createCardioidRevolutionSolid();
	void createLemniscateRevolution();
	void createLemniscateRevolutionSolid();
	void createRose4LeafRevolution();
	void createRose4LeafRevolutionSolid();
	void createPotentialWell();
	void createCylinder();
	void createCylinderSolid();
	void createHyperbolicParaboloid();
	void createCube();
	void createBox();
	void createEllipsoid();
	void createEllipsoidSolid();
	void createCone();
	void createConeSolid();
	void createParaboloid();
	void createParaboloidSolid();
	void createGeometricReferences();
	void createTorusSolid();
public:
    static const int planeDim=50;
    static const std::string geometricobject;
    static const std::string dataDir;
    static const std::string geometricrefdims;

    GeoObject() : noiseLevel{0}, shift{false}{}
    ~GeoObject() = default;
    GeoObject(const GeoObject& geoObj) = delete;
    GeoObject& operator=(const GeoObject& geoObj) = delete;
    GeoObject(GeoObject&& geoObj) = delete;
    GeoObject& operator=(GeoObject&& geoObject) = delete;

    void CreateObject(int geometricObject, int noiseLevel, bool shift);

private:
	int density[planeDim][planeDim][planeDim];

};

#endif /* GEOMETRIC3D_H_ */
