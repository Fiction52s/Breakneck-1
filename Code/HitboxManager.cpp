#include "HitboxManager.h"
#include <fstream>
#include <sstream>
#include <assert.h>

using namespace std;

HitboxCollection & HitboxManager::GetHitboxCollection( const string & str )
{
	if (collectionMap.count(str) == 0)
	{
		auto &myCollection = collectionMap[str];
		ifstream is;
		stringstream ss;
		ss << "Resources/" + folder + "/" << str << ".hit";
		is.open(ss.str());
		if (is.is_open())
		{
			string tilesetName;
			getline(is, tilesetName);

			is >> myCollection.minFrame;
			is >> myCollection.numFrames;

			int numPopulatedFrames;
			is >> numPopulatedFrames;

			for (int i = 0; i < numPopulatedFrames; ++i)
			{
				int frameIndex, numHitboxes;
				is >> frameIndex;
				is >> numHitboxes;
				for (int h = 0; h < numHitboxes; ++h)
				{
					CollisionBox tempCB = LoadHitShape(is);
					myCollection.hitboxMap[frameIndex].push_back( tempCB );
				}
			}

			return myCollection;
		}
		else
		{

			assert(0);
		}
	}
	else
	{
		return collectionMap[str];
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
		int centerX, centerY, width, height, angle;
		is >> centerX;
		is >> centerY;
		is >> width;
		is >> height;
		is >> angle;
		CollisionBox cb;
		cb.offset = V2d(centerX, centerY);
		cb.localAngle = angle / 180.0 * PI;
		cb.isCircle = false;
		cb.rw = width;
		cb.rh = height;
		return cb;
		break;
	}
	case SH_CIRCLE:
		{
		int centerX, centerY, radius;
		is >> centerX;
		is >> centerY;
		is >> radius;
		CollisionBox cb;
		cb.offset = V2d(centerX, centerY);
		cb.isCircle = true;
		cb.rw = radius;
		cb.rh = radius;
		return cb;
		break;
		}
	}
}

CollisionBody *HitboxManager::CreateBody(const std::string & str)
{
	auto &collection = GetHitboxCollection(str);

	CollisionBody *body = new CollisionBody(collection.numFrames, collection.hitboxMap, NULL);
	return body;
}