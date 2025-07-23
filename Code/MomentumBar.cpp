#include "Session.h"
#include "MomentumBar.h"
#include <iostream>

using namespace std;
using namespace sf;

MomentumBar::MomentumBar(TilesetManager *tm)
{
	ts_bar = tm->GetSizedTileset("HUD/momentum_bar_710x65.png");

	if (!partShader.loadFromFile("Resources/Shader/momentum.frag", sf::Shader::Fragment))
	{
		cout << "momentum bar SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}

	partShader.setUniform("barTex", *ts_bar->texture);
}

void MomentumBar::SetCenter(sf::Vector2f &pos)
{
	SetRectCenter(barQuad, ts_bar->tileWidth, ts_bar->tileHeight, pos);
	center = pos;
}

Vector2f MomentumBar::GetCenter()
{
	return center;
}

void MomentumBar::SetMomentumInfo(int p_level, float p_part)
{
	level = p_level;
	part = p_part;
	partShader.setUniform("tile", (float)level);
	partShader.setUniform("factor", part);

	Color c;
	switch (level)
	{
	case 0:
		c = Color(0x12, 0xdb, 0xff);
		break;
	case 1:
		c = Color(0x2e, 0x69, 0xff);
		break;
	case 2:
		c = Color(0xdd, 0x40, 0xff);
		break;
	}
	partShader.setUniform("barColor", sf::Glsl::Vec4(c));

	int tile;
	if (level == 0 && part == 0)
	{
		tile = 0;
	}
	else
	{
		tile = level + 1;
	}
}

void MomentumBar::Draw(sf::RenderTarget *target)
{
	SetRectSubRectGL(barQuad, ts_bar->GetSubRect(level), Vector2f(ts_bar->texture->getSize()));
	target->draw(barQuad, 4, sf::Quads, &partShader);
}