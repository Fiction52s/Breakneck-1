#include "DrawLayer.h"
#include <algorithm>
#include <assert.h>

using namespace std;

float GetDrawLayerDepthFactor(int layer)
{
	switch (layer)
	{
	case BG_1:
		return .1;
	case BG_2:
		return .2;
	case BG_3:
		return .3;
	case BG_4:
		return .4;
	case BG_5:
		return .5;
	case BG_6:
		return .6;
	case BG_7:
		return .7;
	case BG_8:
		return .8;
	case BG_9:
		return .9;
	case BG_10:
		return .95;
	case BEHIND_TERRAIN:
		return 1;
	case BEHIND_ENEMIES:
		return 1;
	case BETWEEN_PLAYER_AND_ENEMIES:
		return 1;
	case IN_FRONT:
		return 1;
	case FG_1:
		return 1.1;
	case FG_2:
		return 1.2;
	case FG_3:
		return 1.3;
	case FG_4:
		return 1.4;
	case FG_5:
		return 1.5;
	case FG_6:
		return 1.6;
	case FG_7:
		return 1.7;
	case FG_8:
		return 1.8;
	case FG_9:
		return 1.9;
	case FG_10:
		return 2.0;
	case IN_FRONT_OF_UI:
		return 1;
	case UI_FRONT:
		return 1;
	}

	assert(0);
	return -1;
}


DrawLayer StringToDrawLayer(const std::string &str)
{
	string testStr = str;
	std::transform(testStr.begin(), testStr.end(), testStr.begin(), ::tolower);

	if (str == "behind_terrain")
	{
		return BEHIND_TERRAIN;
	}
	else if (str == "behind_enemies")
	{
		return BEHIND_ENEMIES;
	}
	else if (str == "between_player_and_enemies")
	{
		return BETWEEN_PLAYER_AND_ENEMIES;
	}
	else if (str == "in_front_of_ui")
	{
		return IN_FRONT_OF_UI;
	}
}