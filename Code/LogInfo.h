#ifndef __LOGINFO_H__
#define __LOGINFO_H__

struct LogInfo
{
	LogInfo()
		:world(0), localIndex(0)
	{

	}
	LogInfo(int w, int li)
		:world(w), localIndex(li)
	{

	}
	int world;
	int localIndex;
	int GetTrueIndex() {
		return world * MAX_LOGS_PER_WORLD + localIndex;
	}

	const static int MAX_LOGS_PER_WORLD = 32;//7*5;

	//const static int MAX_LOGS = 256;
};

#endif