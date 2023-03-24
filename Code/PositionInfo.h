#ifndef __POSITIONINFO_H__
#define __POSITIONINFO_H__

#include "VectorMath.h"
#include "ISelectable.h"
#include "Physics.h"

struct TerrainPoint;
struct Edge;

struct PositionInfoData
{
	V2d position;
	EdgeInfo edgeInfo;
	V2d offset;
	double groundQuantity;
};


struct PositionInfo
{
	V2d position;
	int edgeIndex;
	double groundQuantity;
	PolyPtr ground;
	RailPtr railGround;
	V2d offset;

	PositionInfo();
	Edge *GetEdge();
	double GetGroundAngleRadians();
	double GetGroundAngleDegrees();
	double GetQuant();
	TerrainPoint *GetPoint();
	TerrainPoint *GetNextPoint();
	bool IsAerial();
	void AddActor(ActorPtr a);
	void RemoveActor(ActorPtr a);
	void SetAerial( V2d &pos );
	void SetAerial();
	void SetGround(PolyPtr p_ground, int p_edgeIndex, double quant);
	void SetSurface(Edge *e, double q);
	void SetRail(RailPtr p_rail, int p_edgeIndex, double quant);
	int GetEdgeIndex();
	V2d GetPosition();
	void SetWithoutChangingOffset(PositionInfo &posInfo);
	sf::Vector2f GetPositionF();
	void SetPosition(V2d &pos);
	void SetGroundHeight(double raised);
	void SetGroundOffset(double xOffset);
	void PopulateFromData(const PositionInfoData &data);
	void PopulateData(PositionInfoData &data);
};

#endif