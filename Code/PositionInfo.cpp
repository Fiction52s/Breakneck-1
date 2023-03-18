#include "PositionInfo.h"
#include "Physics.h"
#include "EditorTerrain.h"
#include "EditorRail.h"
#include "Session.h"

PositionInfo::PositionInfo()
	:edgeIndex(-1), groundQuantity(-1), ground(NULL), railGround(NULL)
{

}

void PositionInfo::SetGroundHeight(double raised)
{
	offset.y = raised;
}

void PositionInfo::SetGroundOffset(double xOffset)
{
	offset.x = xOffset;
}


V2d PositionInfo::GetPosition()
{
	Edge *edge = GetEdge();
	if (edge != NULL)
	{
		return edge->GetRaisedPosition(groundQuantity + offset.x, offset.y);
	}
	else
	{
		return position;
	}
}

sf::Vector2f PositionInfo::GetPositionF()
{
	return sf::Vector2f(GetPosition());
}

void PositionInfo::SetPosition(V2d &pos)
{
	position = pos;
}

void PositionInfo::SetWithoutChangingOffset(PositionInfo &posInfo)
{
	position = posInfo.position;
	edgeIndex = posInfo.edgeIndex;
	groundQuantity = posInfo.groundQuantity;
	ground = posInfo.ground;
	railGround = posInfo.railGround;
	//doesnt change offset
}

int PositionInfo::GetEdgeIndex()
{
	return edgeIndex;
}

void PositionInfo::SetSurface(Edge *e, double q)
{
	if (e->poly != NULL)
	{
		ground = e->poly;
	}
	else if (e->rail != NULL)
	{
		railGround = e->rail;
	}
	else
	{
		assert(0);
	}

	edgeIndex = e->edgeIndex;
	groundQuantity = q;
	position = e->GetPosition(q);
}

void PositionInfo::SetGround(PolyPtr p_ground, int p_edgeIndex, double quant)
{
	ground = p_ground;
	edgeIndex = p_edgeIndex;
	groundQuantity = quant;
}

void PositionInfo::SetRail(RailPtr p_rail, int p_edgeIndex, double quant)
{
	railGround = p_rail;
	edgeIndex = p_edgeIndex;
	groundQuantity = quant;
}

void PositionInfo::AddActor(ActorPtr a)
{
	Session *sess = Session::GetSession();
	if (sess->IsSessTypeEdit())
	{
		if (ground != NULL)
		{
			auto &aList = ground->enemies[GetPoint()];
			if ( !aList.empty() && aList.back() == a)
			{
				assert(0); //just for debug
			}
			aList.push_back(a);
			//cout << "edge enemy size: " << ground->enemies[GetPoint()].size() << endl;
			ground->UpdateBounds();
		}
		else if (railGround != NULL)
		{
			railGround->enemies[GetPoint()].push_back(a);
			railGround->UpdateBounds();
		}
		else
		{
			//do nothing
			//assert(0);
		}
	}
}

void PositionInfo::RemoveActor(ActorPtr a)
{
	if (ground != NULL)
	{
		ground->enemies[GetPoint()].remove(a);
		ground->UpdateBounds();
		//cout << "removed edge actor: " << ground->enemies[GetPoint()].size() << endl;
	}
	else if (railGround != NULL)
	{
		railGround->enemies[GetPoint()].remove(a);
	}
	else
	{
		//do nothing
		//assert(0);
	}
}

Edge *PositionInfo::GetEdge()
{
	if (ground != NULL)
	{
		return ground->GetEdge(edgeIndex);
	}
	else if (railGround != NULL)
	{
		return NULL;
		//rails not complete yet
		//railGround->GetEdge(edgeIndex);
	}
	else
	{
		return NULL;
	}
}

double PositionInfo::GetGroundAngleRadians()
{
	Edge *e = GetEdge();
	if (e != NULL)
	{
		return e->GetNormalAngleRadians();
	}
	else
	{
		return 0;
	}
	//gn = startPosInfo.GetEdge()->Normal();
	//angle = atan2(gn.x, -gn.y);
}

double PositionInfo::GetGroundAngleDegrees()
{
	Edge *e = GetEdge();
	if (e != NULL)
	{
		return e->GetNormalAngleDegrees();
	}
	else
	{
		return 0;
	}
	//gn = startPosInfo.GetEdge()->Normal();
	//angle = atan2(gn.x, -gn.y);
}

void PositionInfo::SetAerial(V2d &pos)
{
	position = pos;
	ground = NULL;
	railGround = NULL;
}

void PositionInfo::SetAerial()
{
	position = GetPosition();
	ground = NULL;
	railGround = NULL;
}

double PositionInfo::GetQuant()
{
	return groundQuantity;
}

bool PositionInfo::IsAerial()
{
	return (ground == NULL && railGround == NULL);
}

TerrainPoint *PositionInfo::GetPoint()
{
	if (ground != NULL)
	{
		return ground->GetPoint(edgeIndex);
	}
	else if (railGround != NULL)
	{
		return NULL;
	}
	else
	{
		return NULL;
	}
}

TerrainPoint *PositionInfo::GetNextPoint()
{
	if (ground != NULL)
	{
		return ground->GetNextPoint(edgeIndex);
	}
	else if (railGround != NULL)
	{
		return NULL;
	}
	else
	{
		return NULL;
	}
}

void PositionInfo::PopulateFromData(const PositionInfoData &data)
{
	Session *sess = Session::GetSession();

	position = data.position;
	offset = data.offset;
	
	switch (data.edgeInfo.eiType)
	{
	case EdgeInfo::ETI_EMPTY:
		ground = NULL;
		railGround = NULL;
		break;
	case EdgeInfo::ETI_POLY:
		ground = sess->GetPolyFromID(data.edgeInfo.ownerIndex);
		break;
	case EdgeInfo::ETI_RAIL:
		railGround = sess->GetRailFromID(data.edgeInfo.ownerIndex);
		break;
	case EdgeInfo::ETI_GATE:
		ground = NULL;
		railGround = NULL;
		break;
	}
}

void PositionInfo::PopulateData(PositionInfoData &data)
{
	data.position = position;
	data.edgeInfo.SetFromEdge(GetEdge());
	data.offset = offset;
}