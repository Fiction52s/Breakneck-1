#ifndef __HITBOXMANAGER_H__
#define __HITBOXMANAGER_H__
#include <map>
#include "Physics.h"
#include <string>

struct HitboxManager
{
	enum HitShapeType
	{
		SH_TRI,
		SH_RECT,
		SH_CIRCLE
	};

	std::map<std::string, std::map<int,std::list<CollisionBox>>> hitboxMap;
	std::map<int, std::list<CollisionBox>> & GetHitboxList( const std::string & str );

private:
	CollisionBox LoadHitShape(std::ifstream &is);
};

#endif