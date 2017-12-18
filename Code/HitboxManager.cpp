#include "HitboxManager.h"
#include <fstream>
#include <sstream>
#include <assert.h>

using namespace std;

std::map<int, list<CollisionBox>> & HitboxManager::GetHitboxList( const string & str )
{
	if (hitboxMap.count(str) == 0)
	{
		std::map<int, list<CollisionBox>> & myMap = hitboxMap[str];
		ifstream is;
		stringstream ss;
		ss << "Hitboxes/" << str << ".hit";
		is.open(ss.str());
		if (is.is_open())
		{
			string tilesetName;
			getline(is, tilesetName);

			int numPopulatedFrames;
			is >> numPopulatedFrames;

			for (int i = 0; i < numPopulatedFrames; ++i)
			{
				int frameIndex, numHitboxes;
				is >> frameIndex;
				is >> numHitboxes;
				for (int h = 0; h < numHitboxes; ++h)
				{
					myMap[frameIndex].push_back( LoadHitShape(is) );
				}
			}

			return myMap;
		}
		else
		{

			assert(0);
		}
	}
	else
	{
		return hitboxMap[str];
	}
}

CollisionBox HitboxManager::LoadHitShape(std::ifstream &is)
{
	int shapeType;
	is >> shapeType;

	HitShapeType hst = (HitShapeType)shapeType;

	switch (hst)
	{
	case SH_TRI:
	{

		break;
	}
	case SH_RECT:
	{
		break;
	}
	case SH_CIRCLE:
		{
		int centerX, centerY, radius;
		is >> centerX;
		is >> centerY;
		is >> radius;
		CollisionBox cb(CollisionBox::BoxType::Hit);
		cb.offset = V2d(centerX, centerY);
		cb.isCircle = true;
		cb.rw = radius;
		cb.rh = radius;
		return cb;
		break;
		}
	}
}