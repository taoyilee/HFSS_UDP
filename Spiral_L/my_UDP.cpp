#include "UserDefinedPrimitiveDLLInclude.h"
#include "string.h"

// User's own utility function prototypes
long CreatePath(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt);
long CreateProfile(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues);
void NameEntities(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt);

static struct UDPPrimitiveTypeInfo primitiveInfo =
{
	"Rectangular Spiral",
	"Create a Rectangular Spiral in XY plane",
	"Ansoft Corporation",
	"01-01-2004",
    "1.0"
};

extern "C" DLLEXPORT
struct UDPPrimitiveTypeInfo* GetPrimitiveTypeInfo()
{
    return &primitiveInfo;
}

struct UDPPrimitiveParameterDefinition primParams[] =
{
	{"Xpos", "X Position of start point", kUDPLengthUnit, 0},
	{"Ypos", "Y Position of start point", kUDPLengthUnit, 0},
	{"Dist", "Distance between turns", kUDPLengthUnit, 5},
	{"Turns", "Number of turns", kUDPNoUnit, 2},
	{"Width", "Width of the spiral", kUDPLengthUnit, 2},
	{"Thickness", "Thickness/height of the spiral", kUDPLengthUnit, 1}
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
    return "mm";
}

char* registeredFaceNames[] = { "InnerEndFace", "OuterEndFace"};
static int numOfFaceNames = sizeof(registeredFaceNames)/sizeof(char*);

extern "C" DLLEXPORT
int GetRegisteredFaceNames(char*** faceNames)
{
    *faceNames = registeredFaceNames;
    return numOfFaceNames;
}

char* registeredEdgeNames[] = { "Inner-B", "Inner-L", "Inner-T", "Inner-R",
                                "Outer-B", "Outer-L", "Outer-T", "Outer-R"};
static int numOfEdgeNames = sizeof(registeredEdgeNames)/sizeof(char*);

extern "C" DLLEXPORT
int GetRegisteredEdgeNames(char*** edgeNames)
{
    *edgeNames = registeredEdgeNames;
    return numOfEdgeNames;
}

char* registeredVertexNames[] = { "Inner-B-L", "Inner-L-T", "Inner-T-R", "Inner-R-B",
                                  "Outer-B-L", "Outer-L-T", "Outer-T-R", "Outer-R-B"};
static int numOfVertexNames = sizeof(registeredVertexNames)/sizeof(char*);

extern "C" DLLEXPORT
int GetRegisteredVertexNames(char*** vertexNames)
{
    *vertexNames = registeredVertexNames;
    return numOfVertexNames;
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
    // Number of turns cannot be < 1
    int noOfTurns = (int)paramValues[3];
    if (noOfTurns < 1)
    {
        *error = "Number of turns cannot be less than 1.";
        return 0;
    }

    double dist = paramValues[2];
    double width = paramValues[4];
    double height = paramValues[5];

    // distance should be > 0
    if (dist <= 0)
    {
        *error = "Distance should be more than 0.";
        return 0;
    }

    // width should be > 0
    if (width <= 0)
    {
        *error = "Width should be more than 0.";
        return 0;
    }

    // height should be > 0
    if (height <= 0)
    {
        *error = "Height/Thickness should be more than 0.";
        return 0;
    }

    // distance between turns should be more than the width
    if (dist <= width)
    {
        *error = "Distance between turns should be more than the width.";
        return 0;
    }

    return 1;
}

extern "C" DLLEXPORT
long CreatePrimitive(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues)
{
    struct UDPPosition startPt = {0,0,0};
    struct UDPPosition endPt = {0,0,0};

    long path = CreatePath(functionLib, callbackData, paramValues, &startPt, &endPt);

    // Is path valid?
    if (path == -1)
    {
        functionLib->addMessage(kUDPErrorMessage, "Could not create path", callbackData);
        return 0;
    }

    // The profile
    long profile = CreateProfile(functionLib, callbackData, paramValues);

    // Is profile valid?
    if (profile == -1)
    {
        functionLib->addMessage(kUDPErrorMessage, "Could not create profile", callbackData);
        return 0;
    }

    struct UDPSweepOptions sweepOptions = {kUDPRoundDraft, 0.0, 0.0};

    long success = functionLib->sweepAlongPath(profile, path, &sweepOptions, callbackData);
    // Did the sweep operation succeed?
    if (success == 0)
    {
        functionLib->addMessage(kUDPErrorMessage, "Could not sweep profile along path", callbackData);
        return 0;
    }

    NameEntities(functionLib, callbackData, paramValues, &startPt, &endPt);

    return success;
}

// User defined extra functions
long CreatePath(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt)
{
    double xStart = paramValues[0];
    double yStart = paramValues[1];
    double CONST_Z = 0.0;

    double dist = paramValues[2];
    int noOfTurns = (int)paramValues[3];

    int noOfPoints = 2 + 4*noOfTurns;
    int noOfSegments = noOfPoints-1;

    struct UDPPosition*  arrayOfPositions = new UDPPosition[noOfPoints];

    int i=0;
    for (i=0; i<noOfPoints; i++)
        arrayOfPositions[i].z = CONST_Z;

    arrayOfPositions[0].x = xStart;
    arrayOfPositions[0].y = yStart;

    arrayOfPositions[1].y = yStart;
    arrayOfPositions[noOfPoints-1].x = xStart;


    for (i=0; i<noOfTurns; i++)
    {
        int xs = i*4+1;
        int ys = xs+1;
        double coord = dist*(i+1);

        arrayOfPositions[xs].x = arrayOfPositions[xs+1].x = xStart-coord;
        arrayOfPositions[xs+2].x = arrayOfPositions[xs+3].x = xStart+coord;

        arrayOfPositions[ys].y = arrayOfPositions[ys+1].y = yStart+coord;
        arrayOfPositions[ys+2].y = arrayOfPositions[ys+3].y = yStart-coord;

    }


    struct UDPPolylineSegmentDefinition*  arrayOfSegmentDefinition = new UDPPolylineSegmentDefinition[noOfSegments];

    for (i=0; i<noOfSegments; i++)
    {
        arrayOfSegmentDefinition[i].segmentType = kUDPLineSegment;
        arrayOfSegmentDefinition[i].segmentStartIndex = i;
    }

    struct UDPPolylineDefinition polylineDefinition;
    polylineDefinition.noOfPoints = noOfPoints;
    polylineDefinition.arrayOfPosition = arrayOfPositions;
    polylineDefinition.noOfSegments = noOfSegments;
    polylineDefinition.arrayOfSegmentDefinition = arrayOfSegmentDefinition;
    polylineDefinition.isCovered = 0;

    long path = functionLib->createPolyline(&polylineDefinition, callbackData);

    *startPt = arrayOfPositions[0];
    *endPt = arrayOfPositions[noOfPoints-1];

    delete[] arrayOfPositions;

    // Did the polyline operation failed?
    //assert(path == -1);

    return path;
}

long CreateProfile(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues)
{
    double xStart = paramValues[0];
    double yStart = paramValues[1];
    double CONST_Z = 0.0;

    double width = paramValues[4];
    double height = paramValues[5];

    int noOfPoints = 5;
    int noOfSegments = noOfPoints-1;

    struct UDPPosition*  arrayOfProfPositions = new UDPPosition[noOfPoints];

    arrayOfProfPositions[0].x = xStart;
    arrayOfProfPositions[0].y = yStart-(width/2.0);
    arrayOfProfPositions[0].z = -height/2.0;

    arrayOfProfPositions[1].x = xStart;
    arrayOfProfPositions[1].y = yStart+(width/2.0);
    arrayOfProfPositions[1].z = -height/2.0;

    arrayOfProfPositions[2].x = xStart;
    arrayOfProfPositions[2].y = yStart+(width/2.0);
    arrayOfProfPositions[2].z = height/2.0;

    arrayOfProfPositions[3].x = xStart;
    arrayOfProfPositions[3].y = yStart-(width/2.0);
    arrayOfProfPositions[3].z = height/2.0;

    arrayOfProfPositions[4].x = xStart;
    arrayOfProfPositions[4].y = yStart-(width/2.0);
    arrayOfProfPositions[4].z = -height/2.0;

    struct UDPPolylineSegmentDefinition*  arrayOfSegmentDefinition = new UDPPolylineSegmentDefinition[noOfSegments];

    for (int i=0; i<noOfSegments; i++)
    {
        arrayOfSegmentDefinition[i].segmentType = kUDPLineSegment;
        arrayOfSegmentDefinition[i].segmentStartIndex = i;
    }
    struct UDPPolylineDefinition polylineDefinition;
    polylineDefinition.noOfPoints = noOfPoints;
    polylineDefinition.arrayOfPosition = arrayOfProfPositions;
    polylineDefinition.noOfSegments = noOfSegments;
    polylineDefinition.arrayOfSegmentDefinition = arrayOfSegmentDefinition;
    polylineDefinition.isCovered = 1;

    long profile = functionLib->createPolyline(&polylineDefinition, callbackData);

    delete[] arrayOfProfPositions;

    // Did the polyline operation failed?
    //assert(profile == -1);

    return profile;
}

void NameEntities(struct UDPFunctionLib* functionLib, void* callbackData, double* paramValues, struct UDPPosition* startPt, struct UDPPosition* endPt)
{
    // Name faces
    functionLib->nameAFace(startPt, registeredFaceNames[0], callbackData);
    functionLib->nameAFace(endPt, registeredFaceNames[1], callbackData);

    double width = paramValues[4];
    double height = paramValues[5];

    struct UDPPosition posOnEdge[8]; //numOfEdgeNames=8
    // Inner face edges
    // Inner face edge - Bottom
    posOnEdge[0].x = (*startPt).x;
    posOnEdge[0].y = (*startPt).y;
    posOnEdge[0].z = (*startPt).z - height/2.0;
    // Inner face edge - Left
    posOnEdge[1].x = (*startPt).x;
    posOnEdge[1].y = (*startPt).y - width/2.0;
    posOnEdge[1].z = (*startPt).z;
    // Inner face edge - Top
    posOnEdge[2].x = (*startPt).x;
    posOnEdge[2].y = (*startPt).y;
    posOnEdge[2].z = (*startPt).z + height/2.0;
    // Inner face edge - Right
    posOnEdge[3].x = (*startPt).x;
    posOnEdge[3].y = (*startPt).y + width/2.0;
    posOnEdge[3].z = (*startPt).z;
    // Outer face edges
    // Outer face edge - Bottom
    posOnEdge[4].x = (*endPt).x;
    posOnEdge[4].y = (*endPt).y;
    posOnEdge[4].z = (*endPt).z - height/2.0;
    // Outer face edge - Left
    posOnEdge[5].x = (*endPt).x;
    posOnEdge[5].y = (*endPt).y - width/2.0;
    posOnEdge[5].z = (*endPt).z;
    // Outer face edge - Top
    posOnEdge[6].x = (*endPt).x;
    posOnEdge[6].y = (*endPt).y;
    posOnEdge[6].z = (*endPt).z + height/2.0;
    // Outer face edge - Right
    posOnEdge[7].x = (*endPt).x;
    posOnEdge[7].y = (*endPt).y + width/2.0;
    posOnEdge[7].z = (*endPt).z;

    struct UDPPosition posOnVertex[8]; //numOfVertexNames=8
    // Inner face vertexs
    // Inner face vertex - (common to Bottom & Left edge)
    posOnVertex[0].x = (*startPt).x;
    posOnVertex[0].y = (*startPt).y - width/2.0;
    posOnVertex[0].z = (*startPt).z - height/2.0;
    // Inner face vertex - (common to Left & Top edge)
    posOnVertex[1].x = (*startPt).x;
    posOnVertex[1].y = (*startPt).y - width/2.0;
    posOnVertex[1].z = (*startPt).z + height/2.0;
    // Inner face vertex - (common to Top & Right edge)
    posOnVertex[2].x = (*startPt).x;
    posOnVertex[2].y = (*startPt).y + width/2.0;
    posOnVertex[2].z = (*startPt).z + height/2.0;
    // Inner face vertex - (common to Right & Bottom edge)
    posOnVertex[3].x = (*startPt).x;
    posOnVertex[3].y = (*startPt).y + width/2.0;
    posOnVertex[3].z = (*startPt).z - height/2.0;
    // Outer face vertexs
    // Outer face vertex - (common to Bottom & Left edge)
    posOnVertex[4].x = (*endPt).x;
    posOnVertex[4].y = (*endPt).y - width/2.0;
    posOnVertex[4].z = (*endPt).z - height/2.0;
    // Outer face vertex - (common to Left & Top edge)
    posOnVertex[5].x = (*endPt).x;
    posOnVertex[5].y = (*endPt).y - width/2.0;
    posOnVertex[5].z = (*endPt).z + height/2.0;
    // Outer face vertex - (common to Top & Right edge)
    posOnVertex[6].x = (*endPt).x;
    posOnVertex[6].y = (*endPt).y + width/2.0;
    posOnVertex[6].z = (*endPt).z + height/2.0;
    // Outer face vertex - (common to Right & Bottom edge)
    posOnVertex[7].x = (*endPt).x;
    posOnVertex[7].y = (*endPt).y + width/2.0;
    posOnVertex[7].z = (*endPt).z - height/2.0;

    for (int i=0; i<8; i++)
    {
        functionLib->nameAEdge(&(posOnEdge[i]), registeredEdgeNames[i], callbackData);
        functionLib->nameAVertex(&(posOnVertex[i]), registeredVertexNames[i], callbackData);
    }
}
