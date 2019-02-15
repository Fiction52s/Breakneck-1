#ifndef __EFFECTLAYER_H__
#define __EFFECTLAYER_H__
enum EffectLayer
{
	BEHIND_TERRAIN,
	BEHIND_ENEMIES,
	BETWEEN_PLAYER_AND_ENEMIES,
	IN_FRONT,
	IN_FRONT_1,
	IN_FRONT_OF_UI,
	UI_FRONT,
	Count
};

EffectLayer StringToEffectLayer(const std::string &str);

#endif 