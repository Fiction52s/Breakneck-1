#include "PlayerSkinSwordShader.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"
#include "MovingGeo.h"

using namespace std;
using namespace sf;

PlayerSkinSwordShader::PlayerSkinSwordShader()
{
	skinPaletteImage.loadFromFile("Resources/Kin/kin_palette_180x30.png");

	MainMenu::GetInstance()->LoadShader(pShader, "sword");
	pShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	SetSkin(0);
	SetOffset(0);
}

void PlayerSkinSwordShader::SetSkin(int index)
{
	FillPaletteArray(index);

	pShader.setUniformArray("u_palette", paletteArray, NUM_PALETTE_COLORS);
}

void PlayerSkinSwordShader::SetOffset(int off)
{
	offset = off;
	pShader.setUniform("u_offset", offset);
}

void PlayerSkinSwordShader::BlendSkins(int first, int second, float progress)
{
	for (int i = 0; i < NUM_PALETTE_COLORS; ++i)
	{
		paletteArray[i] = sf::Glsl::Vec4(
			GetBlendColor(
				skinPaletteImage.getPixel(i, first),
				skinPaletteImage.getPixel(i, second), progress));
	}

	pShader.setUniformArray("u_palette", paletteArray, NUM_PALETTE_COLORS);
}

void PlayerSkinSwordShader::FillPaletteArray(int skinIndex)
{
	for (int i = 0; i < NUM_PALETTE_COLORS; ++i)
	{
		paletteArray[i] = sf::Glsl::Vec4(skinPaletteImage.getPixel(i, skinIndex));
	}
}

void PlayerSkinSwordShader::SetTileset(Tileset *ts)
{
	pShader.setUniform("u_texture", *ts->texture);
}