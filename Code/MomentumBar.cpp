#include "Session.h"
#include "MomentumBar.h"
#include <iostream>
#include "Actor.h"
#include "KinUpgrades.h"

using namespace std;
using namespace sf;

MomentumBar::MomentumBar(TilesetManager *tm)
{
	ts_bar = tm->GetSizedTileset("HUD/momentumbar_757x59.png");

	if (!partShader.loadFromFile("Resources/Shader/momentum.frag", sf::Shader::Fragment))
	{
		cout << "momentum bar SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}

	actor = NULL;

	partShader.setUniform("barTex", *ts_bar->texture);

	ts_gems = tm->GetSizedTileset("HUD/gems_40x40.png");

	//gems_40x40_782, 1010
}

void MomentumBar::SetCenter(sf::Vector2f &pos)
{
	SetRectCenter(barQuad, ts_bar->tileWidth, ts_bar->tileHeight, pos);
	center = pos;

	Vector2f gemDelta(-178, -53);
	//Vector2f(960, 1080 - 45);

	//960 - 782 = 178
	//1010 

	for (int i = 0; i < 6; ++i)
	{
		SetRectCenter(gemQuads + i * 4, ts_gems->tileWidth, ts_gems->tileHeight, Vector2f((pos.x + gemDelta.x) + i * 80, pos.y + gemDelta.y));
	}
}

Vector2f MomentumBar::GetCenter()
{
	return center;
}

void MomentumBar::UpdateMomentumInfo()
{
	level = actor->speedLevel;
	part = actor->GetSpeedBarPart();
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

void MomentumBar::UpdateGems()
{
	bool conditions[6];
	conditions[0] = actor->HasUpgradeEffect(UE_AIR_DASH_UNLOCK);
	conditions[1] = actor->HasUpgradeEffect(UE_GRAVITY_CLING_UNLOCK);
	conditions[2] = actor->HasUpgradeEffect(UE_BOUNCE_SCORPION_UNLOCK);
	conditions[3] = actor->HasUpgradeEffect(UE_GRIND_BALL_UNLOCK);
	conditions[4] = actor->HasUpgradeEffect(UE_HOMING_RUSH_UNLOCK);
	conditions[5] = actor->HasUpgradeEffect(UE_DOUBLE_WIRES_UNLOCK);

	for (int i = 0; i < 6; ++i)
	{
		if (conditions[i])
		{
			SetRectSubRect(gemQuads + i * 4, ts_gems->GetSubRect(i));
		}
		else
		{
			SetRectSubRect(gemQuads + i * 4, ts_gems->GetSubRect(7));
		}
	}
	//UpdateMomentumInfo()
}

void MomentumBar::Draw(sf::RenderTarget *target)
{
	SetRectSubRectGL(barQuad, ts_bar->GetSubRect(level), Vector2f(ts_bar->texture->getSize()));
	target->draw(barQuad, 4, sf::Quads, &partShader);

	target->draw(gemQuads, 4 * 6, sf::Quads, ts_gems->texture);
}