#ifndef __PLAYER_VISUAL_INFO_H__
#define __PLAYER_VISUAL_INFO_H__

#include <fstream>
struct PlayerVisualInfo
{
	int skinIndex;

	PlayerVisualInfo();
	void Reset();
	void Save(std::ofstream &of);
	void Load(std::ifstream &is);
};

#endif