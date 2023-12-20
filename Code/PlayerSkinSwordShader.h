#ifndef __PLAYER_SKIN_SWORD_SHADER_H__
#define __PLAYER_SKIN_SWORD_SHADER_H__

#include "SFML/Graphics.hpp"
struct Tileset;
struct PlayerSkinSwordShader
{
	PlayerSkinSwordShader();
	void SetOffset(int off);
	void SetSkin(int index);
	void FillPaletteArray(int skinIndex);
	void BlendSkins(int first, int second, float progress);
	void SetTileset(Tileset *ts);

	int offset;
	const static int NUM_PALETTE_COLORS = 256;
	sf::Image skinPaletteImage;
	sf::Shader pShader;
	sf::Glsl::Vec4 paletteArray[NUM_PALETTE_COLORS];
};

#endif