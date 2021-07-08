#ifndef __PALETTE_SHADER_H__
#define __PALETTE_SHADER_H__

#include "SFML/Graphics.hpp"
struct Tileset;
struct PaletteShader
{
	PaletteShader(const std::string &shaderStr,
		const std::string &paletteStr );
	~PaletteShader();
	void SetPaletteIndex(int index);
	void FillPaletteArray(int pIndex);
	void SetSubRect(Tileset *ts, sf::IntRect &ir);
	int paletteSize;
	int numPaletteOptions;
	sf::Image paletteImage;
	sf::Shader pShader;
	sf::Glsl::Vec4 *paletteArray;
};

#endif