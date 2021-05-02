#include "PlayerSkinShader.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"

using namespace std;
using namespace sf;

PlayerSkinShader::PlayerSkinShader(const std::string &shaderStr)
{
	skinPaletteImage.loadFromFile("Resources/Kin/kin_palette_64x10.png");

	assert(Shader::isAvailable() && "help me");
	if (!pShader.loadFromFile("Resources/Shader/" + shaderStr + "_shader.frag", sf::Shader::Fragment))
	{
		cout << "PLAYER SKIN SHADER NOT LOADING CORRECTLY" << endl;
		assert(0 && "player skin shader not loaded");
	}
	pShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	SetSkin(0);
}

void PlayerSkinShader::SetSkin(int index)
{
	FillPaletteArray(index);
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