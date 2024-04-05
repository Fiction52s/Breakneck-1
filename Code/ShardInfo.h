#ifndef __SHARDINFO_H__
#define __SHARDINFO_H__

struct ShardInfo
{
	const static int MAX_SHARDS_PER_WORLD = 64;
	const static int MAX_SHARDS = MAX_SHARDS_PER_WORLD * 8;

	ShardInfo()
		:world(-1), localIndex(-1 )
	{

	}
	ShardInfo(int w, int li)
		:world(w), localIndex(li)
	{

	}

	void Clear()
	{
		world = -1;
		localIndex = -1;
	}
	int world;
	int localIndex;
	int GetTrueIndex() { return world * MAX_SHARDS_PER_WORLD + localIndex; }

	

	
};

#endif