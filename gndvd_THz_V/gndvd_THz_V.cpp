#include "../UserDefinedPrimitiveDLLInclude.h"
#include "string.h"
#include "math.h"
// User's own utility function prototypes
long CreatePath(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt);
long CreateProfile(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues);
void NameEntities(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt);
static struct UDPPrimitiveTypeInfo primitiveInfo =
{
	"Variable density GND plane",
	"GND Plane",
	"National Chiao Tung University RFVLSI LAB",
	"15-11-2011",
    "1.0"
};
extern "C" DLLEXPORT
struct UDPPrimitiveTypeInfo* GetPrimitiveTypeInfo()
{
    return &primitiveInfo;
}
struct UDPPrimitiveParameterDefinition primParams[] =
{
	{"xp", "X Position of start point", kUDPLengthUnit, 0}, 	//paramValues[0];
	{"yp", "Y Position of start point", kUDPLengthUnit, 0},  	//paramValues[1];
	{"zp", "X Position of start point", kUDPLengthUnit, 0}, 	//paramValues[2];
	{"Length", "X Position of start point", kUDPLengthUnit, 50},//paramValues[3];
	{"Width", "Y Position of start point", kUDPLengthUnit, 50}, //paramValues[4];
	{"Hole", "Number of turns", kUDPLengthUnit, 2}, 				//paramValues[5];
	{"Fill", "Distance between turns", kUDPLengthUnit, 2}, 	//paramValues[6];
	{"thickness", "Number of turns", kUDPLengthUnit, 1}				//paramValues[7];
};


static int numOfParameters = sizeof(primParams)/sizeof(primParams[0]);

extern "C" DLLEXPORT
int GetPrimitiveParametersDefinition(struct UDPPrimitiveParameterDefinition** paramDefinition)
{
    *paramDefinition = primParams;
    return numOfParameters;
}

extern "C" DLLEXPORT
char* GetLengthParameterUnits()
{
    return "um";
}


const int maxPathLen = 1024;
char udpDllFullPath[maxPathLen+1];

extern "C" DLLEXPORT
int SetDllFullPath(char* dllFullPath)
{
  if( dllFullPath)
  {
    int len = strlen( dllFullPath);
    if( len > maxPathLen)
      len = maxPathLen;
    strncpy( udpDllFullPath, dllFullPath, len);
  }
  return 1;
} 

// Incase of error this function should return 0
extern "C" DLLEXPORT
int AreParameterValuesValid(char ** error, double* paramValues)
{

    if (paramValues[3] <= 0)
    {
        *error = "Length should be more than 0.";
        return 0;
    }
    if (paramValues[4] <= 0)
    {
        *error = "Width should be more than 0.";
        return 0;
    }
	if (paramValues[5] <= 0)
    {
        *error = "Hole should be more than 0.";
        return 0;
    }
	if (paramValues[6] <= 0)
    {
        *error = "Fill should be more than 0.";
        return 0;
    }
	if (paramValues[7] <= 0)
    {
        *error = "Thickness should be more than 0.";
        return 0;
    }
  
    return 1;
}

extern "C" DLLEXPORT
long CreatePrimitive(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues)
{
 
	double xpos =  paramValues[0];
	double ypos =  paramValues[1];
	double zpos =  paramValues[2];
	struct UDPPosition startPt = {xpos,ypos,zpos};
	double len =  paramValues[3];
	double wid =  paramValues[4];
	double hole =  paramValues[5];
	double fill =  paramValues[6];
	double thick =  paramValues[7];
    double sizeh[3]={fill,wid,thick};
	double sizev[3]={len,fill,thick};
	double pitch= hole+fill;
	int n_sh=(int) round(wid/pitch);
	int n_sv=(int) round(len/pitch);
	long success = 0;

    success = functionLib->createBox(&startPt, sizeh, callbackData);
    startPt.x = xpos+len - fill;
    success = functionLib->createBox(&startPt, sizeh, callbackData);
    startPt.x = xpos;

	for(int i=0;i<n_sh;i++){
			success = functionLib->createBox(&startPt, sizev, callbackData);
			startPt.y+= pitch;
			if (success == 0)
			{
				functionLib->addMessage(kUDPErrorMessage, "Could not sweep profile along path", callbackData);
				return 0;
			}
	}
	// last col
	startPt.y = wid - fill+ypos;
    success = functionLib->createBox(&startPt, sizev, callbackData);
	if (success == 0){
                functionLib->addMessage(kUDPErrorMessage, "Could not sweep profile along path", callbackData);
                return 0;
                }
    return success;
}
