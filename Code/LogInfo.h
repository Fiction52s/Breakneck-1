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
		return 0;
	}

	//const static int MAX_LOGS = 256;
};

#endif