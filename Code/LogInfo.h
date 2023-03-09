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
		return world * MAX_LOGS_PER_WORLD + localIndex; //used to say 22 but I think thats a bug
	}

	const static int MAX_LOGS_PER_WORLD = 7*5;

	//const static int MAX_LOGS = 256;
};

#endif