#ifndef __SHARDINFO_H__
#define __SHARDINFO_H__

struct ShardInfo
{
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
	int GetTrueIndex() { return world * 22 + localIndex; }

	const static int MAX_SHARDS = 128;
};

#endif