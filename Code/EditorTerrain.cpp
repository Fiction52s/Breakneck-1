#include "EditorTerrain.h"
#include "Physics.h"
#include "EditSession.h"
#include "EditorRail.h"
#include "QuadTree.h"
#include "TransformTools.h"

using namespace std;
using namespace sf;

TerrainPoint *PointMoveInfo::GetPolyPoint()
{
	return poly->GetPoint(pointIndex);
}

TerrainPoint *PointMoveInfo::GetRailPoint()
{
	return rail->GetPoint(pointIndex);
}

