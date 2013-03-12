#include "UserDefinedPrimitiveDLLInclude.h"
#include "string.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#define PI 3.14159265
// User's own utility function prototypes
long half(struct UDPFunctionLib* functionLib, void* callbackData,
double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,
        double,double,double,double,
        double,double,double,double,double);

static struct UDPPrimitiveTypeInfo primitiveInfo = {
	"Transformer Half",
	"Sub-Component of a Transformer",
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
	{"OD", "Outer Diameter", kUDPLengthUnit, 100}, // 3
	{"W", "Width of the spiral", kUDPLengthUnit, 5}, // 4
	{"L_OP", "Left Opening", kUDPLengthUnit, 5}, // 5
	{"R_OP", "Right Opening", kUDPLengthUnit, 5}, // 6
	{"Thickness", "Thickness/height of the spiral", kUDPLengthUnit, 1.45}, // 7
	{"maskGrid", "Minimum grid of technology", kUDPNoUnit, 0.005}, // 8
	{"DIV", "Tunging Factor", kUDPNoUnit, 3.2} // 9
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
	if (paramValues[7] <= 0)	{// height should be > 0
		*error = "Height/Thickness should be more than 0.";		return 0;
	}
	return 1;
}
extern "C" DLLEXPORT
long CreatePrimitive(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues)
{
	struct UDPPosition startPt = {0,0,0};
	struct UDPPosition endPt = {0,0,0};
	struct UDPPosition posi = {0,0,0};
    struct UDPVector vect = {-1,0,0};
	double xo = paramValues[0];
	double yo = paramValues[1];
	double zo = paramValues[2];
	double OD = paramValues[3];
	double W = paramValues[4];
	double L_OP = paramValues[5];
	double R_OP = paramValues[6];
	double thick = paramValues[7];
	double DIV = paramValues[9];
	double maskGrid = paramValues[8];
	long path;
	long polygon;	
    long polygon2;
		path = half(functionLib, callbackData, paramValues, &startPt, &endPt,OD,W,xo,yo,zo, DIV,  L_OP,  R_OP,  maskGrid);
		// Is thicken polygon valid?
        polygon = functionLib->sheetThicken(path,-thick,0,callbackData);
		if ( polygon == -1)		{
			functionLib->addMessage(kUDPErrorMessage, "Sheet Thickening Failed", callbackData);
			return 0;
		}	
    // functionLib->duplicateAndMirror(polygon,&posi, &vect, callbackData);
	return polygon;
}
// User defined extra functions
long half(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,
double OD, double W, double xoff, double yoff, double zoff, double DIV, double L_OP, double R_OP, double maskGrid){
	char str[30];
	double CONST_Z = 0.0;
	int noOfPoints = 12;
	int noOfSegments = noOfPoints;
	struct UDPPosition*  arrayOfPositions = new UDPPosition[noOfPoints];

    double C=maskGrid*round(W*tan(PI/8)/maskGrid);
    
//    sprintf(str, "W = %f", W);
//    functionLib->addMessage(kUDPInfoMessage, str, callbackData);
//    sprintf(str, "maskGrid = %f", maskGrid);
//    functionLib->addMessage(kUDPInfoMessage, str, callbackData);
//    sprintf(str, "DIV = %f", DIV);
//    functionLib->addMessage(kUDPInfoMessage, str, callbackData);
//    sprintf(str, "C = %f", C);
//    functionLib->addMessage(kUDPInfoMessage, str, callbackData);
    double A=maskGrid*round(OD/DIV/maskGrid);
//    sprintf(str, "A = %f", A);
//    functionLib->addMessage(kUDPInfoMessage, str, callbackData);
    double B=OD-2*A;
//    sprintf(str, "B = %f", B);
//    functionLib->addMessage(kUDPInfoMessage, str, callbackData);
    double BA=maskGrid*round(B/2/maskGrid);
//    sprintf(str, "BA = %f", BA);
//    functionLib->addMessage(kUDPInfoMessage, str, callbackData);
    double BB=B-BA;
//    sprintf(str, "BB = %f", BB);
//    functionLib->addMessage(kUDPInfoMessage, str, callbackData);

    // OD:L_OP          OD-W:L_OP
	arrayOfPositions[0].x = OD;
	arrayOfPositions[0].y = L_OP/2;
	arrayOfPositions[1].x = OD-W;
	arrayOfPositions[1].y = L_OP/2;
    // OD-W:BA         OD-A:BA+A-W
	arrayOfPositions[2].x = OD-W;
	arrayOfPositions[2].y = BA;
	arrayOfPositions[3].x = OD-A;
	arrayOfPositions[3].y = BA+A-W;    
    // A:BA+A-W        W:BA
	arrayOfPositions[4].x = A;
	arrayOfPositions[4].y = BA+A-W;
	arrayOfPositions[5].x = W;
	arrayOfPositions[5].y = BA;    
    // W:R_OP/2 0:R_OP/2
	arrayOfPositions[6].x = W;
	arrayOfPositions[6].y = R_OP/2;
	arrayOfPositions[7].x = 0;
	arrayOfPositions[7].y = R_OP/2;    
    // 0:BA+C          A-C:BA+A
	arrayOfPositions[8].x = 0;
	arrayOfPositions[8].y = BA+C;
	arrayOfPositions[9].x = A-C;
	arrayOfPositions[9].y = BA+A;    
    // OD-A+C:BA+A     OD:BA+C
	arrayOfPositions[10].x = OD-A+C;
	arrayOfPositions[10].y = BA+A;
	arrayOfPositions[11].x = OD;
	arrayOfPositions[11].y = BA+C;    

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
