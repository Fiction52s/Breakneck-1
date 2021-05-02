#ifndef __PLAYER_SKIN_SHADER_H__
#define __PLAYER_SKIN_SHADER_H__

#include "SFML/Graphics.hpp"
struct PlayerSkinShader
{
	PlayerSkinShader(const std::string &shaderStr);
	void SetSkin(int index);
	void FillPaletteArray(int skinIndex);
	const static int NUM_PALETTE_COLORS = 64;
	sf::Image skinPaletteImage;
	sf::Shader pShader;
	sf::Glsl::Vec4 paletteArray[NUM_PALETTE_COLORS];
};

#endif