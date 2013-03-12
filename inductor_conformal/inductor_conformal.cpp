#include "../UserDefinedPrimitiveDLLInclude.h"
#include "string.h"
#include <math.h>
#include <stdlib.h>
#define PI 3.14159265
// User's own utility function prototypes
long octtl(struct UDPFunctionLib* functionLib, void* callbackData,
double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,double,double,double,double,double,double);

long octtlc(struct UDPFunctionLib* functionLib, void* callbackData,
double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,double,double,double,double,double,double);

long octt(struct UDPFunctionLib* functionLib, void* callbackData, 
double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,double,double,double,double,double,double);

long create_box(struct UDPFunctionLib* functionLib, void* callbackData, 
double x, double y, double z, double xs, double ys, double zs);

static struct UDPPrimitiveTypeInfo primitiveInfo = {
	"Octagonal Conformal Spiral",
	"Create a Conformal Octagonal Spiral in XY plane",
	"TYLEE @ RFVLSILAB NCTU",
	"08-20-2011",
	"1.0"
};
extern "C" DLLEXPORT
struct UDPPrimitiveTypeInfo* GetPrimitiveTypeInfo(){
	return &primitiveInfo;
}struct UDPPrimitiveParameterDefinition primParams[] = {
	{"Xpos", "X Position of start point", kUDPLengthUnit, 0}, // 0
	{"Ypos", "Y Position of start point", kUDPLengthUnit, 0}, // 1
	{"S", "Distance between turns", kUDPLengthUnit, 1.8}, // 2
	{"NT", "Number of turns", kUDPNoUnit, 2}, // 3
	{"W", "Width of the spiral", kUDPLengthUnit, 1.8}, // 4
	{"Thickness", "Thickness/height of the spiral", kUDPLengthUnit, 1.45}, // 5
	{"maskGrid", "Minimum grid of technology", kUDPLengthUnit, 0.005}, // 6
	{"DIV", "Minimum grid of technology", kUDPNoUnit, 3.2}, // 7
	{"OD", "Distance between turns", kUDPLengthUnit, 50}, // 8
	{"Zpos", "Z Position of start point", kUDPLengthUnit, 6.075}, // 9
	{"lead", "Z Position of start point", kUDPLengthUnit, 10}, // 10
	{"SideWallThick", "Z Position of start point", kUDPLengthUnit, 0}, // 11
	{"TopWallThick", "Z Position of start point", kUDPLengthUnit, 0} // 12
};
static int numOfParameters = sizeof(primParams)/sizeof(primParams[0]);
extern "C" DLLEXPORT
int GetPrimitiveParametersDefinition(struct UDPPrimitiveParameterDefinition** paramDefinition){
	*paramDefinition = primParams;
	return numOfParameters;
}extern "C" DLLEXPORT
char* GetLengthParameterUnits(){
	return "um";
}const int maxPathLen = 1024;
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
	double noOfTurns = paramValues[3];
	if (noOfTurns < 1)	{
		*error = "Number of turns cannot be less than 1.";
		return 0;
	}
	if ((2*noOfTurns - floor(2*noOfTurns)) !=0)	{
		*error = "Number of turns must bet integer multiples of 0.5";
		return 0;
	}
	double dist = paramValues[2];	double width = paramValues[4];	double height = paramValues[5];
	if (dist <= 0)	{// distance should be > 0
		*error = "Distance should be more than 0.";		return 0;
	}
	if (width <= 0)	{// width should be > 0
		*error = "Width should be more than 0.";		return 0;
	}
	if (height <= 0)	{// height should be > 0
		*error = "Height/Thickness should be more than 0.";		return 0;
	}
	return 1;
}extern "C" DLLEXPORT
long CreatePrimitive(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues)
{	struct UDPPosition startPt = {0,0,0};
	struct UDPPosition endPt = {0,0,0};

	double xo = paramValues[0];
	double yo = paramValues[1];
	double zo = paramValues[9];
	double S = paramValues[2];
	double W = paramValues[4];
	double NT = paramValues[3];
	double OD = paramValues[8];
	double lead = paramValues[10];
	double TH = paramValues[5]+paramValues[12];
	double sw = paramValues[11];
	int turns=(int) ceil(NT);
	long path; 
	long path2;
	long polygon;
	double P = S + W;
		long lead_mdl= create_box(functionLib, callbackData, OD+S-sw,sw,zo,W+lead+2*sw,-W-2*sw,TH);
	for (int i =0; i< NT-1;i++){
		path = octt(functionLib, callbackData, paramValues, &startPt, &endPt,OD-i*2*P+2*sw,W+2*sw,S-2*sw,i*P+xo-sw,yo,zo);
		if (path == -1)		{
			functionLib->addMessage(kUDPErrorMessage, "Could not create path", callbackData);
			return 0;
		}
		// Is thicken polygon valid?
		if (functionLib->sheetThicken(path,-TH,0,callbackData) == -1)		{
			functionLib->addMessage(kUDPErrorMessage, "End poly gon cannot be thicken", callbackData);
			return 0;
		}
		
	}
		if(!(NT -floor(NT))){
		// integer turns
		functionLib->addMessage(kUDPInfoMessage, "Integer number of turns received.", callbackData);
		path = octtl(functionLib, callbackData, paramValues, &startPt, &endPt,OD-(NT-1)*2*P+2*sw,W+2*sw,S-2*sw, (NT-1)*P+xo-sw,yo,zo);
		// Is path valid?
		if (path == -1)	{
			functionLib->addMessage(kUDPErrorMessage, "Could not create path", callbackData);
			return 0;
		}
		if (functionLib->sheetThicken(path,-TH,0,callbackData) == -1){
			functionLib->addMessage(kUDPErrorMessage, "End poly gon cannot be thicken", callbackData);
			return 0;
		}
		
		create_box(functionLib, callbackData, OD+S-sw,S-sw,zo,lead+W+2*sw,W+2*sw,TH);
	}else{
		// integer +0.5 turns
		functionLib->addMessage(kUDPInfoMessage, "Half Integer number of turns received.", callbackData);
		path	= octtlc(functionLib, callbackData, paramValues, &startPt, &endPt,OD-(NT-0.5)*2*P+2*sw,W+2*sw,S-2*sw,(NT-0.5)*P+xo-sw,yo,zo);

		if (path == -1)		{
			functionLib->addMessage(kUDPErrorMessage, "Could not create path", callbackData);
			return 0;
		}
		
		if (functionLib->sheetThicken(path,-TH,0,callbackData) == -1)		{
			functionLib->addMessage(kUDPErrorMessage, "End poly gon cannot be thicken", callbackData);
			return 0;
		}
		create_box(functionLib, callbackData, -S,sw,zo,-lead-sw,-W-2*sw,TH);
	}
	return path;
}long create_box(struct UDPFunctionLib* functionLib, void* callbackData, double x, double y, double z, double xs, double ys, double zs){
	struct UDPPosition boxpos = {x,y,z}; 
	double boxsize[3] = {xs,ys,zs};
	long 	box = functionLib->createBox(&boxpos, boxsize, callbackData);
	return box;
}// User defined extra functions
long octt(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,
double OD, double W, double S, double xoff, double yoff, double zoff){
	double P=S+W;
	double DIV=paramValues[7];
	double maskGrid=paramValues[6];
	double C=W*tan(PI/8)/maskGrid;
	C=maskGrid*floor(C);
	double A=OD/DIV/maskGrid;
	A=maskGrid*floor(A);
	double B=OD-2*A;
	double BA=B/2/maskGrid;
	BA=maskGrid*floor(BA);
	double BB=B-BA;
	double CONST_Z = 0.0;
	double dist = paramValues[2];
	int noOfTurns = (int)paramValues[3];
	int noOfPoints = 20;
	int noOfSegments = noOfPoints;
	struct UDPPosition*  arrayOfPositions = new UDPPosition[noOfPoints];

	/*OD:0            OD-W:0 */
	arrayOfPositions[0].x = OD;
	arrayOfPositions[0].y = 0;
	arrayOfPositions[1].x = OD-W;
	arrayOfPositions[1].y = 0;
	/*OD-W:BA         OD-A:BA+A-W*/
	arrayOfPositions[2].x = OD-W;
	arrayOfPositions[2].y = BA;
	arrayOfPositions[3].x = OD-A;
	arrayOfPositions[3].y = BA+A-W;
	/*A:BA+A-W        W:BA*/
	arrayOfPositions[4].x = A;
	arrayOfPositions[4].y = BA+A-W;
	arrayOfPositions[5].x = W;
	arrayOfPositions[5].y = BA;
	/*W:-BB           A:-BB-A+W*/
	arrayOfPositions[6].x = W;
	arrayOfPositions[6].y = -BB;
	arrayOfPositions[7].x = A;
	arrayOfPositions[7].y = -BB-A+W;
	/*OD+P-A:-BB-A+W OD+S:-BB*/
	arrayOfPositions[8].x = OD+P-A;
	arrayOfPositions[8].y = -BB-A+W;
	arrayOfPositions[9].x = OD+S;
	arrayOfPositions[9].y = -BB;
	/*OD+S:0        OD+P:0*/
	arrayOfPositions[10].x = OD+S ;
	arrayOfPositions[10].y = 0;
	arrayOfPositions[11].x = OD+P;
	arrayOfPositions[11].y = 0;
	/*OD+P:-BB-C     OD+P-A+C:-BB-A*/
	arrayOfPositions[12].x = OD+P;
	arrayOfPositions[12].y = -BB-C  ;
	arrayOfPositions[13].x = OD+P-A+C;
	arrayOfPositions[13].y = -BB-A;
	/*A-C:-BB-A      0:-BB-C*/
	arrayOfPositions[14].x = A-C;
	arrayOfPositions[14].y = -BB-A  ;
	arrayOfPositions[15].x = 0;
	arrayOfPositions[15].y = -BB-C;
	/*0:BA+C          A-C:BA+A*/
	arrayOfPositions[16].x = 0;
	arrayOfPositions[16].y = BA+C;
	arrayOfPositions[17].x = A-C;
	arrayOfPositions[17].y = BA+A;
	/*OD-A+C:BA+A     OD:BA+C*/
	arrayOfPositions[18].x = OD-A+C;
	arrayOfPositions[18].y = BA+A;
	arrayOfPositions[19].x = OD;
	arrayOfPositions[19].y = BA+C;
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
}// User defined extra functions
long octtl(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,
double OD, double W, double S, double xoff, double yoff, double zoff){
	double P=S+W;
	double DIV=paramValues[7];
	double maskGrid=paramValues[6];
	double C=W*tan(PI/8)/maskGrid;
	C=maskGrid*floor(C);
	double A=OD/DIV/maskGrid;
	A=maskGrid*floor(A);
	double B=OD-2*A;
	double BA=B/2/maskGrid;
	BA=maskGrid*floor(BA);
	double BB=B-BA;
	double xStart = paramValues[0];
	double yStart = paramValues[1];
	double CONST_Z = 0.0;
	double dist = paramValues[2];
	int noOfTurns = (int)paramValues[3];
	int noOfPoints = 20;
	int noOfSegments = noOfPoints;
	struct UDPPosition*  arrayOfPositions = new UDPPosition[noOfPoints];
	/*OD:S            OD-W:S */
	arrayOfPositions[0].x = OD;
	arrayOfPositions[0].y = S;
	arrayOfPositions[1].x = OD-W;
	arrayOfPositions[1].y = S;
	/*OD-W:BA         OD-A:BA+A-W*/
	arrayOfPositions[2].x = OD-W;
	arrayOfPositions[2].y = BA;
	arrayOfPositions[3].x = OD-A;
	arrayOfPositions[3].y = BA+A-W;
	/*A:BA+A-W        W:BA*/
	arrayOfPositions[4].x = A;
	arrayOfPositions[4].y = BA+A-W;
	arrayOfPositions[5].x = W;
	arrayOfPositions[5].y = BA;
	/*W:-BB           A:-BB-A+W*/
	arrayOfPositions[6].x = W;
	arrayOfPositions[6].y = -BB;
	arrayOfPositions[7].x = A;
	arrayOfPositions[7].y = -BB-A+W;
	/*OD+P-A:-BB-A+W OD+S:-BB*/
	arrayOfPositions[8].x = OD+P-A;
	arrayOfPositions[8].y = -BB-A+W;
	arrayOfPositions[9].x = OD+S;
	arrayOfPositions[9].y = -BB;
	/*OD+S:0        OD+P:0*/
	arrayOfPositions[10].x = OD+S ;
	arrayOfPositions[10].y = 0;
	arrayOfPositions[11].x = OD+P;
	arrayOfPositions[11].y = 0;
	/*OD+P:-BB-C     OD+P-A+C:-BB-A*/
	arrayOfPositions[12].x = OD+P;
	arrayOfPositions[12].y = -BB-C  ;
	arrayOfPositions[13].x = OD+P-A+C;
	arrayOfPositions[13].y = -BB-A;
	/*A-C:-BB-A      0:-BB-C*/
	arrayOfPositions[14].x = A-C;
	arrayOfPositions[14].y = -BB-A  ;
	arrayOfPositions[15].x = 0;
	arrayOfPositions[15].y = -BB-C;
	/*0:BA+C          A-C:BA+A*/
	arrayOfPositions[16].x = 0;
	arrayOfPositions[16].y = BA+C;
	arrayOfPositions[17].x = A-C;
	arrayOfPositions[17].y = BA+A;
	/*OD-A+C:BA+A     OD:BA+C*/
	arrayOfPositions[18].x = OD-A+C;
	arrayOfPositions[18].y = BA+A;
	arrayOfPositions[19].x = OD;
	arrayOfPositions[19].y = BA+C;
	for (int i=0; i<noOfPoints; i++){
		arrayOfPositions[i].x += xoff;
		arrayOfPositions[i].y += yoff;
		arrayOfPositions[i].z = CONST_Z + zoff;
	}
	struct UDPPolylineSegmentDefinition*  arrayOfSegmentDefinition = new UDPPolylineSegmentDefinition[noOfSegments];

	for (int i=0; i<noOfSegments; i++)	{
		arrayOfSegmentDefinition[i].segmentType = kUDPLineSegment;
		arrayOfSegmentDefinition[i].segmentStartIndex = i;
		arrayOfPositions[i].z = CONST_Z+zoff;
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
}// User defined extra functions
long octtlc(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt,
double OD, double W, double S, double xoff, double yoff, double zoff){
	double P=S+W;
	double DIV=paramValues[7];
	double maskGrid=paramValues[6];
	double C=W*tan(PI/8)/maskGrid;
	C=maskGrid*floor(C);
	double A=OD/DIV/maskGrid;
	A=maskGrid*floor(A);
	double B=OD-2*A;
	double BA=B/2/maskGrid;
	BA=maskGrid*floor(BA);
	double BB=B-BA;
	double xStart = paramValues[0];
	double yStart = paramValues[1];
	double CONST_Z = 0.0;
	double dist = paramValues[2];
	int noOfTurns = (int)paramValues[3];
	int noOfPoints = 12;
	int noOfSegments = noOfPoints;
	struct UDPPosition*  arrayOfPositions = new UDPPosition[noOfPoints];
	/*W:-BB           A:-BB-A+W*/
	arrayOfPositions[0].x = W;
	arrayOfPositions[0].y = -BB;
	arrayOfPositions[1].x = A;
	arrayOfPositions[1].y = -BB-A+W;
	/*OD+P-A:-BB-A+W OD+S:-BB*/
	arrayOfPositions[2].x = OD+P-A;
	arrayOfPositions[2].y = -BB-A+W;
	arrayOfPositions[3].x = OD+S;
	arrayOfPositions[3].y = -BB;
	/*OD+S:0        OD+P:0*/
	arrayOfPositions[4].x = OD+S;
	arrayOfPositions[4].y = 0;
	arrayOfPositions[5].x = OD+P;
	arrayOfPositions[5].y = 0;
	/*OD+P:-BB-C     OD+P-A+C:-BB-A*/
	arrayOfPositions[6].x = OD+P;
	arrayOfPositions[6].y = -BB-C;
	arrayOfPositions[7].x =  OD+P-A+C;
	arrayOfPositions[7].y = -BB-A;
	/*A-C:-BB-A      0:-BB-C*/
	arrayOfPositions[8].x = A-C;
	arrayOfPositions[8].y = -BB-A;
	arrayOfPositions[9].x =  0;
	arrayOfPositions[9].y = -BB-C;
	/*0:0 W:0 */
	arrayOfPositions[10].x = 0;
	arrayOfPositions[10].y = 0;
	arrayOfPositions[11].x = W;
	arrayOfPositions[11].y = 0;

	for (int i=0; i<noOfPoints; i++){
		arrayOfPositions[i].x = arrayOfPositions[i].x + xoff;
		arrayOfPositions[i].y = arrayOfPositions[i].y + yoff;
		arrayOfPositions[i].z = CONST_Z + zoff;
	}
	struct UDPPolylineSegmentDefinition*  arrayOfSegmentDefinition = new UDPPolylineSegmentDefinition[noOfSegments];

	for (int i=0; i<noOfSegments; i++)
	{
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
