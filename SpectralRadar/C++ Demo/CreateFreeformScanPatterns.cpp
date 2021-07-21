// CreateFreeformScanPatterns.cpp : This file contains demonstrations programs how to create scan points that can be used in ThorImageOCT with the Freeform Scan Pattern Feature.
// The scan points can be saved in standard .txt-files with a special format and can be modified or created from the customer easily. 
// The following samples demonstrate how to create simple geometrical figures, e.g. a circle or triangle. 

#include <iostream>
#include <vector>
#include <conio.h>
#include <SpectralRadar.h>

using namespace std;


void createCircleScanPoints()
{
	// Create "edge" points used for the scan pattern. These coordinates are in mm.
	double Pi = 3.14159;
	int NumberOfCirclePoints = 101;
	double SpacingCircle = 1 / (double(NumberOfCirclePoints) - 1);
	vector<double> PosX;
	vector<double> PosY;
	vector<int> ScanIndices;
	for (int i = 0; i < NumberOfCirclePoints; ++i)
	{
	PosX.push_back(cos(i * SpacingCircle * 2 * Pi));
	PosY.push_back(sin(i * SpacingCircle * 2 * Pi));
	ScanIndices.push_back(0);
	}
	saveScanPointsToFile(PosX.data(), PosY.data(), ScanIndices.data(), PosX.size(), "ScanPoints_Circle", ScanPoints_DataFormat_TXT);
}

void createTriangleScanPoints()
{
	// Create "edge" points used for the scan pattern. These coordinates are in mm.
	int NumberOfPoints = 3;
	vector<double> PosX = {0.5, 0, -0.5};
	vector<double> PosY = {-sqrt(3)/6, sqrt(3)/3, -sqrt(3)/6};
	vector<int> ScanIndices = {0, 0, 0};
	
	saveScanPointsToFile(PosX.data(), PosY.data(), ScanIndices.data(), PosX.size(), "ScanPoints_Triangle", ScanPoints_DataFormat_TXT);
}

int main()
{
	createCircleScanPoints();
	createTriangleScanPoints();

	return 0;
}
