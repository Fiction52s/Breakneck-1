#ifndef __DrawLayer_H__
#define __DrawLayer_H__

#include <string>

namespace DrawLayer
{
	enum DrawLayer
	{
		TERRAIN = -1, //just for drawing normal terrain, default value
		BG_1,
		BG_2,
		BG_3,
		BG_4,
		BG_5,
		BG_6,
		BG_7,
		BG_8,
		BG_9,
		BG_10,
		BEHIND_TERRAIN,
		BACK_TERRAIN, //used for the terrain right behind normal terrain.
		BEHIND_ENEMIES,
		BETWEEN_PLAYER_AND_ENEMIES,
		IN_FRONT,
		FG_1,
		FG_2,
		FG_3,
		FG_4,
		FG_5,
		FG_6,
		FG_7,
		FG_8,
		FG_9,
		FG_10,
		IN_FRONT_OF_UI,
		UI_FRONT,
		DrawLayer_Count
	};
}

int StringToDrawLayer(const std::string &str);

float GetDrawLayerDepthFactor(int layer);

#endif 