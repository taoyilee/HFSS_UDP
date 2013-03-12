#include "../UserDefinedPrimitiveDLLInclude.h"
#include "string.h"
#include <math.h>
#include <stdlib.h>
#define PI 3.14159265

static struct UDPPrimitiveTypeInfo primitiveInfo = {
	"VIA Array",
	"Create via array",
	"TYLEE @ RFVLSILAB NCTU",
	"08-20-2011",
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
	{"W", "Outer Diameter", kUDPLengthUnit, 50}, // 3
	{"S", "Distance between turns", kUDPLengthUnit, 1.8}, // 4
    {"thick", "thick", kUDPLengthUnit, 1.8}, // 5
	{"NX", "Number of turns", kUDPNoUnit, 5}, // 6
	{"NY", "Width of the spiral", kUDPNoUnit, 5} // 7
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
	// Number of turns cannot be < 1
	if (paramValues[3] < 1)	{
		*error = "Number of turns cannot be less than 1.";
		return 0;
	}
	if (paramValues[4] <= 0)	{// distance should be > 0
		*error = "Distance should be more than 0.";		return 0;
	}
	if (paramValues[5] <= 0)	{// width should be > 0
		*error = "Width should be more than 0.";		return 0;
	}
	if (paramValues[6] <= 0)	{// height should be > 0
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
	double w = paramValues[3];
	double s = paramValues[4];
	double t = paramValues[5];	
	int nx = (int) paramValues[6];
	int ny = (int) paramValues[7];
	double p = w + s;
	long polygon;
	
	struct UDPPosition boxP = {0,0,zo};
	double boxS[3] = {w,w,t};
	for(int i =0; i<nx ; i++){
		for(int j =0; j<ny ; j++){
		        boxP.x = i*p; boxP.y= j*p;
                polygon = functionLib->createBox(&boxP, boxS, callbackData);
        }
    }
	return polygon;
}
