#ifndef __PLAYER_SKIN_SHADER_H__
#define __PLAYER_SKIN_SHADER_H__

#include "SFML/Graphics.hpp"
struct Tileset;
struct PlayerSkinShader
{
	PlayerSkinShader(const std::string &shaderStr);
	void SetSkin(int index);
	void FillPaletteArray(int skinIndex);
	void SetDefaultPlayerVars();
	void BlendSkins(int first, int second, float progress);
	void SetSubRect( Tileset *ts, sf::IntRect &ir);
	void SetAuraColor(sf::Color c);
	void SetQuad(sf::Glsl::Vec4 &v);
	void SetQuad(Tileset *ts, int tile);
	const static int NUM_PALETTE_COLORS = 64;
	sf::Image skinPaletteImage;
	sf::Shader pShader;
	sf::Glsl::Vec4 paletteArray[NUM_PALETTE_COLORS];
};

#endif