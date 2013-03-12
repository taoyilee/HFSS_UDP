#include "../UserDefinedPrimitiveDLLInclude.h"
#include "string.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#define PI 3.14159265
// User's own utility function prototypes
long cross(struct UDPFunctionLib* functionLib, void* callbackData,
double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,
        double,double,double,
        double,double,double,double);


static struct UDPPrimitiveTypeInfo primitiveInfo = {
	"Transformer Half",
	"Special Cross of a Transformer",
	"TYLEE @ RFVLSILAB NCTU",
	"01-10-2012",
	"1.0"
};
extern "C" DLLEXPORT
struct UDPPrimitiveTypeInfo* GetPrimitiveTypeInfo(){
	return &primitiveInfo;
}
struct UDPPrimitiveParameterDefinition primParams[] = {
	{"Xpos", "X Position of start point", kUDPLengthUnit, 0}, // 0
	{"Ypos", "Y Position of start point", kUDPLengthUnit, 0}, // 1
	{"Zpos", "Z Position of start point", kUDPLengthUnit, 6.075}, // 2
	{"W", "Line Width", kUDPLengthUnit, 5}, // 3
	{"S", "Line Spacing", kUDPLengthUnit, 1}, // 4
	{"OPEN", "Opening", kUDPLengthUnit, 20}, // 5
	{"Thickness", "Thickness/height of the spiral", kUDPLengthUnit, 1.45}, // 6
	{"maskGrid", "Minimum grid of technology", kUDPNoUnit, 0.005}, // 7
	{"viah", "VIA height", kUDPLengthUnit, 1} // 8
};
static int numOfParameters = sizeof(primParams)/sizeof(primParams[0]);
extern "C" DLLEXPORT
int GetPrimitiveParametersDefinition(struct UDPPrimitiveParameterDefinition** paramDefinition){
	*paramDefinition = primParams;
	return numOfParameters;
}
extern "C" DLLEXPORT
char* GetLengthParameterUnits(){
	return "um";
}
const int maxPathLen = 1024;
char udpDllFullPath[maxPathLen+1];
extern "C" DLLEXPORT
int SetDllFullPath(char* dllFullPath){
	if( dllFullPath)	{
		int len = strlen( dllFullPath);
		if( len > maxPathLen)
		len = maxPathLen;
		strncpy( udpDllFullPath, dllFullPath, len);
	}
	return 1;
}
// Incase of error this function should return 0
extern "C" DLLEXPORT
int AreParameterValuesValid(char ** error, double* paramValues){
	if (paramValues[3] <= 0)	{// distance should be > 0
		*error = "Distance should be more than 0.";		return 0;
	}
	if (paramValues[4] <= 0)	{// width should be > 0
		*error = "Width should be more than 0.";		return 0;
	}
	if (paramValues[5] <= 0)	{// height should be > 0
		*error = "Height/Thickness should be more than 0.";		return 0;
	}
	return 1;
}
extern "C" DLLEXPORT
long CreatePrimitive(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues)
{
	struct UDPPosition startPt = {0,0,0};

	struct UDPPosition endPt = {0,0,0};
	double xo = paramValues[0];
	double yo = paramValues[1];
	double zo = paramValues[2];
	double W = paramValues[3];
	double S = paramValues[4];
	double OPEN = paramValues[5];
	double thick = paramValues[6];
	double grid = paramValues[7];
	double H = paramValues[8];
    double size[3] = {W,W,H};

	struct UDPPosition startPt2 = {xo,yo-OPEN/2,zo+thick};
	struct UDPPosition startPt3 = {xo+S+W,yo+OPEN/2-W,zo+thick};
	long path;
	long polygon;	
    long polygon2;
		path = cross(functionLib, callbackData, paramValues, &startPt, &endPt,S, W,OPEN,grid,xo,yo,zo);
		// Is thicken polygon valid?
        polygon = functionLib->sheetThicken(path,thick,0,callbackData);
		if ( polygon == -1)		{
			functionLib->addMessage(kUDPErrorMessage, "Sheet Thickening Failed", callbackData);
			return 0;
		}	
    functionLib->createBox(&startPt2, size, callbackData);
//    functionLib->createBox(&startPt3, size, callbackData);
	return polygon;
}
// User defined extra functions
long cross(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,
double S, double W, double OPENING, double maskGrid, double xoff, double yoff, double zoff){
	double CONST_Z = 0.0;
	int noOfPoints = 8;
	int noOfSegments = noOfPoints;
	struct UDPPosition*  arrayOfPositions = new UDPPosition[noOfPoints];

    double C = round(W*tan(PI/8)/maskGrid)*maskGrid;
    double K = round((S+W+C)/2/maskGrid)*maskGrid;

// 0:-OPENING/2    W:-OPENING/2
	arrayOfPositions[0].x = 0;
	arrayOfPositions[0].y = -OPENING/2;
	arrayOfPositions[1].x = W;
	arrayOfPositions[1].y = -OPENING/2;
// W:-K            W+S+W:-K+S+W
	arrayOfPositions[2].x = W;
	arrayOfPositions[2].y = -K;
	arrayOfPositions[3].x = W+S+W;
	arrayOfPositions[3].y = -K+S+W;
// W+S+W:OPENING/2         W+S:OPENING/2                                                       
	arrayOfPositions[4].x = W+S+W;
	arrayOfPositions[4].y = OPENING/2;
	arrayOfPositions[5].x = W+S;
	arrayOfPositions[5].y = OPENING/2;
// W+S:K           0:K-S-W 
	arrayOfPositions[6].x = W+S;
	arrayOfPositions[6].y = K;
	arrayOfPositions[7].x = 0;
	arrayOfPositions[7].y = K-S-W;

	for (int i=0; i<noOfPoints; i++){
		arrayOfPositions[i].x +=  xoff;
		arrayOfPositions[i].y +=  yoff;
		arrayOfPositions[i].z = CONST_Z + zoff;
	}
	struct UDPPolylineSegmentDefinition*  arrayOfSegmentDefinition = new UDPPolylineSegmentDefinition[noOfSegments];
	for (int i=0; i<noOfSegments; i++)	{
		arrayOfSegmentDefinition[i].segmentType = kUDPLineSegment;
		arrayOfSegmentDefinition[i].segmentStartIndex = i;
	}
	struct UDPPolylineDefinition polylineDefinition;
	polylineDefinition.noOfPoints = noOfPoints;
	polylineDefinition.arrayOfPosition = arrayOfPositions;
	polylineDefinition.noOfSegments = noOfSegments;
	polylineDefinition.arrayOfSegmentDefinition = arrayOfSegmentDefinition;
	polylineDefinition.isCovered = 1;
	long path = functionLib->createPolyline(&polylineDefinition, callbackData);
	*startPt = arrayOfPositions[0];
	*endPt = arrayOfPositions[noOfPoints-1];
	delete[] arrayOfPositions;
	return path;
}
