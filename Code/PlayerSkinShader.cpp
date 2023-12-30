#include "PlayerSkinShader.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"
#include "MovingGeo.h"

using namespace std;
using namespace sf;

PlayerSkinShader::PlayerSkinShader(ShaderType sType)
{
	skinPaletteImage.loadFromFile("Resources/Kin/kin_palette_256x30.png");

	shaderType = sType;
	string shaderStr;
	switch (sType)
	{
	case ST_DEFAULT:
		shaderStr = "player";
		break;
	case ST_BOOST:
		shaderStr = "boostplayer";
		break;
	case ST_AURA:
		shaderStr = "aura";
		break;
	}

	MainMenu::GetInstance()->LoadShader(pShader, shaderStr);
	pShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	if (shaderType == ST_DEFAULT )
	{
		SetDefaultPlayerVars();
	}

	SetSkin(0);
}

void PlayerSkinShader::SetDefaultPlayerVars()
{
	pShader.setUniform("u_invincible", 0.f);
	pShader.setUniform("u_super", 0.f);
	//pShader.setUniform("u_slide", 0.f);
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

	if (shaderType != ST_AURA)
	{
		pShader.setUniformArray("u_palette", paletteArray, NUM_PALETTE_COLORS);
	}
	
	if (shaderType == ST_DEFAULT )
	{
		pShader.setUniform("u_auraColor", ColorGL(paletteArray[9]));
	}
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

	if (shaderType != ST_AURA)
	{
		pShader.setUniformArray("u_palette", paletteArray, NUM_PALETTE_COLORS);
	}

	if (shaderType == ST_DEFAULT)
	{
		pShader.setUniform("u_auraColor", ColorGL(paletteArray[9]));
	}
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
	if (shaderType == ST_DEFAULT)
	{
		pShader.setUniform("u_auraColor", ColorGL(c));
	}
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

void PlayerSkinShader::SetTileset(Tileset *ts)
{
	pShader.setUniform("u_texture", *ts->texture);
}