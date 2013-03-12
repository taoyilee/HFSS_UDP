#include "../UserDefinedPrimitiveDLLInclude.h"
#include "string.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#define PI 3.14159265
// User's own utility function prototypes
long half(struct UDPFunctionLib* functionLib, void* callbackData,
double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt);

static struct UDPPrimitiveTypeInfo primitiveInfo = {
	"Lead with vias",
	"Lead with vias",
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
	{"W", "Outer Diameter", kUDPLengthUnit, 5}, // 3
	{"L", "Width of the spiral", kUDPLengthUnit, 20}, // 4
	{"Thickness", "Thickness/height of the spiral", kUDPLengthUnit, 1.45}, // 5
	{"VIAW", "Tunging Factor", kUDPLengthUnit, 2}, // 6
	{"VIAH", "Tunging Factor", kUDPLengthUnit, 2} // 7    	
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

	return 1;
}
extern "C" DLLEXPORT
long CreatePrimitive(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues)
{

	double xo = paramValues[0];
	double yo = paramValues[1];
	double zo = paramValues[2];
	double W = paramValues[3];
	double L = paramValues[4];
	double thick = paramValues[5];
	struct UDPPosition viaPt = {xo,yo,zo};
	double VIAW = paramValues[6];
	double VIAH = paramValues[7];    
	long polygon;	    
	double size[3] = {W,L,thick};

	polygon = functionLib->createBox(&viaPt, size, callbackData);
	viaPt.z = zo+thick;
	int countx =0;
	int county =0;
	countx =  (int) floor((fabs(W)+VIAW)/(2*VIAW));
	county =  (int) floor((fabs(L)+VIAW)/(2*VIAW));
	double enclx = (fabs(W) - countx*2*VIAW+VIAW)/2;
	double encly = (fabs(L) - county*2*VIAW+VIAW)/2;
	size[0] = (W/fabs(W))*VIAW;
	size[1] = (L/fabs(L))*VIAW;
	size[2] = VIAH;	
	for(int i=0; i< countx; i++){
		for(int j=0; j< county; j++){
		viaPt.x = xo + (W/fabs(W))*(enclx + i*2*VIAW);
		viaPt.y = yo + (L/fabs(L))*(encly + j*2*VIAW);    
		polygon = functionLib->createBox(&viaPt, size, callbackData);
		}
	}
	return polygon;
}
