#include "PlayerSkinShader.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"
#include "MovingGeo.h"

using namespace std;
using namespace sf;

PlayerSkinShader::PlayerSkinShader(const std::string &shaderStr)
{
	skinPaletteImage.loadFromFile("Resources/Kin/kin_palette_164x30.png");

	assert(Shader::isAvailable() && "help me");
	if (!pShader.loadFromFile("Resources/Shader/" + shaderStr + "_shader.frag", sf::Shader::Fragment))
	{
		cout << "PLAYER SKIN SHADER NOT LOADING CORRECTLY" << endl;
		assert(0 && "player skin shader not loaded");
	}
	pShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	if (shaderStr == "player")
	{
		SetDefaultPlayerVars();
	}

	SetSkin(0);
}

void PlayerSkinShader::SetDefaultPlayerVars()
{
	pShader.setUniform("u_invincible", 0.f);
	pShader.setUniform("u_super", 0.f);
	pShader.setUniform("u_slide", 0.f);
}

void PlayerSkinShader::SetSubRect(Tileset *ts, IntRect &ir)
{
	float width = ts->texture->getSize().x;
	float height = ts->texture->getSize().y;

	pShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
		(ir.left + ir.width) / width, (ir.top + ir.height) / height));
}

void PlayerSkinShader::SetSkin(int index)
{
	FillPaletteArray(index);
	pShader.setUniformArray("u_palette", paletteArray, NUM_PALETTE_COLORS);
	pShader.setUniform("u_auraColor", ColorGL(paletteArray[9]));
}

void PlayerSkinShader::BlendSkins(int first, int second, float progress )
{
	for (int i = 0; i < NUM_PALETTE_COLORS; ++i)
	{
		paletteArray[i] = sf::Glsl::Vec4(
			GetBlendColor(
				skinPaletteImage.getPixel(i, first), 
				skinPaletteImage.getPixel(i, second), progress));
	}
	pShader.setUniformArray("u_palette", paletteArray, NUM_PALETTE_COLORS);
	pShader.setUniform("u_auraColor", ColorGL(paletteArray[9]));
}

void PlayerSkinShader::FillPaletteArray(int skinIndex)
{
	for (int i = 0; i < NUM_PALETTE_COLORS; ++i)
	{
		paletteArray[i] = sf::Glsl::Vec4(skinPaletteImage.getPixel(i, skinIndex));
	}
}

void PlayerSkinShader::SetAuraColor(Color c)
{
	pShader.setUniform("u_auraColor", ColorGL(c));
}

void PlayerSkinShader::SetQuad(sf::Glsl::Vec4 &v)
{
	pShader.setUniform("u_quad", v);
}

void PlayerSkinShader::SetQuad(Tileset *ts, int tile)
{
	IntRect ir = ts->GetSubRect(tile);

	float width = ts->texture->getSize().x;
	float height = ts->texture->getSize().y;

	pShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
		(ir.left + ir.width) / width, (ir.top + ir.height) / height));
}