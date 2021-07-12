#ifndef __SHARDINFO_H__
#define __SHARDINFO_H__

struct ShardInfo
{
	const static int MAX_SHARDS_PER_WORLD = 22;
	const static int MAX_SHARDS = 128;

	ShardInfo()
		:world(0), localIndex(0 )
	{

	}
	ShardInfo(int w, int li)
		:world(w), localIndex(li)
	{

	}
	int world;
	int localIndex;
	int GetTrueIndex() { return world * MAX_SHARDS_PER_WORLD + localIndex; }

	

	
};

#endif