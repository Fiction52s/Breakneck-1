#ifndef __POSITIONINFO_H__
#define __POSITIONINFO_H__

#include "VectorMath.h"
#include "ISelectable.h"


struct TerrainPoint;
struct Edge;

struct PositionInfo
{
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
	void SetRail(RailPtr p_rail, int p_edgeIndex, double quant);
	int GetEdgeIndex();
	V2d GetPosition();
	void SetWithoutChangingOffset(PositionInfo &posInfo);
	sf::Vector2f GetPositionF();
	void SetPosition(V2d &pos);
	void SetGroundHeight(double raised);
	void SetGroundOffset(double xOffset);

	V2d position;
	int edgeIndex;
	double groundQuantity;
	PolyPtr ground;
	RailPtr railGround;
	V2d offset;
};

#endif