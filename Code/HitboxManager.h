#ifndef __HITBOXMANAGER_H__
#define __HITBOXMANAGER_H__
#include <map>
#include "Physics.h"
#include <string>

struct HitboxCollection
{
	std::map<int, std::list<CollisionBox>> hitboxMap;
	int minFrame;
	int numFrames;
};

struct HitboxManager
{
	enum HitShapeType
	{
		SH_TRI,
		SH_RECT,
		SH_CIRCLE
	};

	HitboxManager(const std::string &f)
		:folder(f)
	{

	}
	std::string folder;
	std::map<std::string, HitboxCollection> collectionMap;
	HitboxCollection & GetHitboxCollection( const std::string & str );
	CollisionBody *CreateBody(const std::string & str, HitboxInfo *hi = NULL);

private:
	CollisionBox LoadHitShape(std::ifstream &is);
};

#endif