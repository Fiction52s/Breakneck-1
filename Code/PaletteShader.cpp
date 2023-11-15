#include "PaletteShader.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"

using namespace std;
using namespace sf;

PaletteShader::PaletteShader(const std::string &shaderStr,
	const std::string &paletteStr)
{
	paletteImage.loadFromFile(paletteStr);
	paletteSize = paletteImage.getSize().x;
	numPaletteOptions = paletteImage.getSize().y;

	paletteArray = new sf::Glsl::Vec4[paletteSize];

	MainMenu::GetInstance()->LoadShader(pShader, shaderStr);

	SetPaletteIndex(0);
}

PaletteShader::~PaletteShader()
{
	delete[] paletteArray;
}

void PaletteShader::SetTileset(Tileset *ts)
{
	//pShader.setUniform("u_texture", *ts->texture);
	//pShader.setUniform("u_texture", sf::Shader::CurrentTexture);
	pShader.setUniform("u_texture", *ts->texture);
}

void PaletteShader::SetSubRect(Tileset *ts, IntRect &ir)
{
	float width = ts->texture->getSize().x;
	float height = ts->texture->getSize().y;

	pShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
		(ir.left + ir.width) / width, (ir.top + ir.height) / height));
}

void PaletteShader::SetColor(int colorIndex, sf::Color c)
{
	paletteArray[colorIndex] = ColorGL(c);//sf::Glsl::Vec4(paletteImage.getPixel(i, pIndex));
	pShader.setUniformArray("u_palette", paletteArray, paletteSize);
}

void PaletteShader::SetPaletteIndex(int index)
{
	FillPaletteArray(index);
	pShader.setUniformArray("u_palette", paletteArray, paletteSize);
}

void PaletteShader::FillPaletteArray(int pIndex)
{
	for (int i = 0; i < paletteSize; ++i)
	{
		paletteArray[i] = sf::Glsl::Vec4(paletteImage.getPixel(i, pIndex));
	}
}