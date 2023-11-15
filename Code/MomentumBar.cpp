#include "Session.h"
#include "MomentumBar.h"
#include <iostream>

using namespace std;
using namespace sf;

MomentumBar::MomentumBar(TilesetManager *tm)
{
	ts_bar = tm->GetSizedTileset("HUD/momentumbar_105x105.png");
	ts_container = tm->GetSizedTileset("HUD/momentumbar_115x115.png");
	ts_num = tm->GetSizedTileset("HUD/momentumnum_48x48.png");

	levelNumSpr.setTexture(*ts_num->texture);
	levelNumSpr.setTextureRect(ts_bar->GetSubRect(0));

	teal.setTexture(*ts_bar->texture);
	teal.setTextureRect(ts_bar->GetSubRect(0));

	blue.setTexture(*ts_bar->texture);
	blue.setTextureRect(ts_bar->GetSubRect(1));

	purp.setTexture(*ts_bar->texture);
	purp.setTextureRect(ts_bar->GetSubRect(2));

	container.setTexture(*ts_container->texture);
	container.setTextureRect(ts_container->GetSubRect(0));

	levelNumSpr.setTexture(*ts_num->texture);
	levelNumSpr.setTextureRect(ts_num->GetSubRect(0));

	if (!partShader.loadFromFile("Resources/Shader/momentum.frag", sf::Shader::Fragment))
	{
		cout << "momentum bar SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}

	partShader.setUniform("barTex", sf::Shader::CurrentTexture);
}

void MomentumBar::SetTopLeft(sf::Vector2f &pos)
{
	Vector2f extra(5, 5);
	teal.setPosition(pos + extra);
	blue.setPosition(pos + extra);
	purp.setPosition(pos + extra);
	container.setPosition(pos);
	levelNumSpr.setPosition(pos + Vector2f(76, -50));
}

Vector2f MomentumBar::GetTopLeft()
{
	return container.getPosition();
}

void MomentumBar::SetMomentumInfo(int p_level, float p_part)
{
	level = p_level;
	part = p_part;
	partShader.setUniform("tile", (float)level);
	partShader.setUniform("factor", part);

	int tile;
	if (level == 0 && part == 0)
	{
		tile = 0;
	}
	else
	{
		tile = level + 1;
	}

	container.setTextureRect(ts_container->GetSubRect(level));
	levelNumSpr.setTextureRect(ts_num->GetSubRect(tile));
}

void MomentumBar::Draw(sf::RenderTarget *target)
{
	target->draw(container);

	if (level == 0)
	{
		target->draw(teal, &partShader);
	}
	else if (level == 1)
	{
		//target->draw(teal);
		target->draw(blue, &partShader);
	}
	else if (level == 2)
	{
		//target->draw(blue);
		target->draw(purp, &partShader);
	}

	target->draw(levelNumSpr);
}