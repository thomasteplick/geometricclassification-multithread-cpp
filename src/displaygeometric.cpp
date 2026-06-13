/*
 * displaygeometric.cpp
 *
 *  Created on: Jun 2, 2026
 *      Author: Thomas Teplick
 */

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <wincon.h>
#include <vector>
#include "displaygeometric.h"
#include "geometric3D.h"

const std::string GeoDisplay::dataDir = "..\\data\\";  // directory for geometric objects
const std::string GeoDisplay::geometricobject = "geometricobject.txt"; // 3D geometric object file containing the densities, 50x50x50

// Display the class, axis, and planes
void GeoDisplay::displayClass()
{
	// Get the class, axis, and planes in a loop for continuous evaluation
	std::cout << "Enter class, axis, start plane, and stop plane, separated by spaces\n";
	std::cout << "0<=[class]<20, 0<=[axis]<=2, 0<=[start plane]<50, 0<=[stop plane]<50: ";
	std::string response = "";

    // Create Geometric Object with fixed noise level and no shift
    GeoObject geobj;
    int noiseLevel = 0;
    bool shift = false;

	while (std::cin >> cls >> axis >> startPlane >> stopPlane) {
		if ((cls < 0) || (cls >= nclasses)) {
			response += " number class, ";
		}

		if ((axis < 0) || (axis >= naxes)) {
			response += "axes, ";
		}

		if ((startPlane < 0) || (startPlane >= planeDim)) {
			response += "startPlane, ";
		}

		if ((stopPlane < 0) || (stopPlane >= planeDim)) {
			response += "stopPlane, ";
		}

		if (startPlane > stopPlane) {
			response += "start plane > stopPlane";
		}

	    if (response.size() > 0) {
	    	throw std::runtime_error("geometric display inputs outside of specified ranges: " + response);
	    }

	    // Create the geometric object
		geobj.CreateObject(cls, noiseLevel, shift);

		// display the requested planes for the class and axis
		displayPlanes();

		std::cout << "Enter class, axis, start plane, and stop plane, separated by spaces\n";
		std::cout << "0<=[class]<19, 0<=[axis]<=2, 0<=[start plane]<50, 0<=[stop plane]<50: ";

	}
}

// display the geometric reference planes
void GeoDisplay::displayPlanes()
{
	//COLORS LIST
	//1: Blue
	//2: Green
	//3: Cyan
	//4: Red
	//5: Purple
	//6: Brown
	//7: Default white
	//8: Gray/Grey
	//9: Bright blue
	//10: Brigth green
	//11: Bright cyan
	//12: Bright red
	//13: Pink/Magenta
	//14: Yellow
	//15: Bright white
	//Numbers after 15 include background colors

	// map geometric density to windows color attribute

	// colors not in wincon.h
	enum Color : int {
		FOREGROUND_BLACK=0,
		FOREGROUND_YELLOW=14,
		FOREGROUND_ORANGE=12,
		FOREGROUND_CYAN=3,
		FOREGROUND_BROWN=6,
		FOREGROUND_PURPLE=5,
		FOREGROUND_GRAY=8,
	};

	std::vector<int> density2colorattr = {
		FOREGROUND_YELLOW,
		FOREGROUND_GRAY,
		FOREGROUND_ORANGE,
		FOREGROUND_CYAN,
		FOREGROUND_GREEN,
		FOREGROUND_BLUE,
		FOREGROUND_RED,
		FOREGROUND_PURPLE,
		FOREGROUND_BROWN,
		FOREGROUND_BLACK,
	};

	  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	  CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	  // Save the current text colors.
	  GetConsoleScreenBufferInfo(hConsole, &csbiInfo);

	// read in the geometric object
	// Open the geometric reference file containing the densities
	std::fstream fgeometric;
	fgeometric.open((dataDir+geometricobject).c_str(), std::fstream::in);
	if (!fgeometric.is_open()) {
		std::cout << "cannot open file " + geometricobject << std::endl;
		throw std::runtime_error("cannot open file " + geometricobject);
	}

	// Read the geometric object file containing the densities
	for (int i = 0; i < planeDim; i++) {
		for (int j = 0; j < planeDim; j++) {
			for (int k = 0; k < planeDim; k++) {
				fgeometric >> density[i][j][k];
			}
		}
	}
	fgeometric.close();

	// loop over the planes and display for two seconds

	switch (axis) {
	/**************** axis i *******************/
	case 0:
		// loop over the planes
		for (int i = startPlane; i <= stopPlane; i++) {
			for (int j = 0; j < planeDim; j++) {
				for (int k = 0; k < planeDim; k++) {
					SetConsoleTextAttribute(hConsole, density2colorattr[density[i][j][k]]);
					std::cout << density[i][j][k] << " ";
				}
				std::cout << std::endl;
			}
			// Sleep for two seconds
		    std::this_thread::sleep_for(std::chrono::seconds(2));
		    std::cout << "\n";
		}
		break;
	case 1:
		/************* axis j *******************/
		// loop over the planes
		for (int j = startPlane; j <= stopPlane; j++) {
			for (int i = 0; i < planeDim; i++) {
				for (int k = 0; k < planeDim; k++) {
					SetConsoleTextAttribute(hConsole, density2colorattr[density[i][j][k]]);
					std::cout << density[i][j][k] << " ";
				}
				std::cout << std::endl;
			}
			// Sleep for two seconds
		    std::this_thread::sleep_for(std::chrono::seconds(2));
		    std::cout << "\n";
		}
		break;
	case 2:
		/******************* axis k ***********************/
		// loop over the planes
		for (int k = startPlane; k <= stopPlane; k++) {
			for (int i = 0; i < planeDim; i++) {
				for (int j = 0; j < planeDim; j++) {
					SetConsoleTextAttribute(hConsole, density2colorattr[density[i][j][k]]);
					std::cout << density[i][j][k] << " ";
				}
				std::cout << std::endl;
			}
			// Sleep for two seconds
		    std::this_thread::sleep_for(std::chrono::seconds(2));
		    std::cout << "\n";
		}
		break;
	default:
		std::cout << "GeoDisplay::displayPlanes() invalid axis: " << axis << std::endl;
		throw std::runtime_error("GeoDisplay::displayPlanes() invalid axis");
	}

	  WORD wOldColorAttrs = csbiInfo.wAttributes;
	  // Restore the original text colors.
	  SetConsoleTextAttribute(hConsole, wOldColorAttrs);
}



