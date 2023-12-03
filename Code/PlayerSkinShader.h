#ifndef __PLAYER_SKIN_SHADER_H__
#define __PLAYER_SKIN_SHADER_H__

#include "SFML/Graphics.hpp"
struct Tileset;
struct PlayerSkinShader
{
	enum ShaderType
	{
		ST_DEFAULT,
		ST_BOOST,
		ST_AURA,
		ST_SWORD,
	};

	PlayerSkinShader(ShaderType sType = ShaderType::ST_DEFAULT );
	void SetSkin(int index);
	void FillPaletteArray(int skinIndex);
	void SetDefaultPlayerVars();
	void BlendSkins(int first, int second, float progress);
	void SetSubRect( Tileset *ts, sf::IntRect &ir);
	void SetAuraColor(sf::Color c);
	void SetQuad(sf::Glsl::Vec4 &v);
	void SetQuad(Tileset *ts, int tile);
	void SetTileset(Tileset *ts);

	int shaderType;
	const static int NUM_PALETTE_COLORS = 180;
	sf::Image skinPaletteImage;
	sf::Shader pShader;
	sf::Glsl::Vec4 paletteArray[NUM_PALETTE_COLORS];
};

#endif