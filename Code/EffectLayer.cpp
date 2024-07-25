#include "DrawLayer.h"
#include <algorithm>
#include <assert.h>

using namespace std;

float GetDrawLayerDepthFactor(int layer)
{
	switch (layer)
	{
	case DrawLayer::TERRAIN:
		return 1.0;
	case DrawLayer::BG_1:
		return .1;
	case DrawLayer::BG_2:
		return .2;
	case DrawLayer::BG_3:
		return .3;
	case DrawLayer::BG_4:
		return .4;
	case DrawLayer::BG_5:
		return .5;
	case DrawLayer::BG_6:
		return .6;
	case DrawLayer::BG_7:
		return .7;
	case DrawLayer::BG_8:
		return .8;
	case DrawLayer::BG_9:
		return .9;
	case DrawLayer::BG_10:
		return .95;
	case DrawLayer::BEHIND_TERRAIN:
		return 1;
	case DrawLayer::BEHIND_ENEMIES:
		return 1;
	case DrawLayer::BETWEEN_PLAYER_AND_ENEMIES:
		return 1;
	case DrawLayer::IN_FRONT:
		return 1;
	case DrawLayer::FG_1:
		return 1.1;
	case DrawLayer::FG_2:
		return 1.2;
	case DrawLayer::FG_3:
		return 1.3;
	case DrawLayer::FG_4:
		return 1.4;
	case DrawLayer::FG_5:
		return 1.5;
	case DrawLayer::FG_6:
		return 1.6;
	case DrawLayer::FG_7:
		return 1.7;
	case DrawLayer::FG_8:
		return 1.8;
	case DrawLayer::FG_9:
		return 1.9;
	case DrawLayer::FG_10:
		return 2.0;
	case DrawLayer::IN_FRONT_OF_UI:
		return 1;
	case DrawLayer::UI_FRONT:
		return 1;
	}

	assert(0);
	return -1;
}


int StringToDrawLayer(const std::string &str)
{
	string testStr = str;
	std::transform(testStr.begin(), testStr.end(), testStr.begin(), ::tolower);

	if (str == "behind_terrain")
	{
		return DrawLayer::BEHIND_TERRAIN;
	}
	else if (str == "behind_enemies")
	{
		return DrawLayer::BEHIND_ENEMIES;
	}
	else if (str == "between_player_and_enemies")
	{
		return DrawLayer::BETWEEN_PLAYER_AND_ENEMIES;
	}
	else if (str == "in_front_of_ui")
	{
		return DrawLayer::IN_FRONT_OF_UI;
	}
}