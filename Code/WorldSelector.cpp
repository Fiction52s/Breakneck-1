#include "MainMenu.h"
#include "VectorMath.h"
#include "WorldMap.h"

using namespace std;
using namespace sf;

WorldSelector::WorldSelector(MainMenu *mm)
{
	ts = mm->tilesetManager.GetTileset("WorldMap/world_select_192x192.png", 192, 192);
	for (int i = 0; i < 4; ++i)
	{
		SetRectSubRect(quads + i * 4, ts->GetSubRect(i));
		angles[i] = 0.f;
	}

}

void WorldSelector::Update()
{
	SetPosition(position);
	float factor = 10;
	angles[0] += .1 / factor;
	angles[1] += .2 / factor;
	angles[2] -= .1 / factor;
	angles[3] -= .2 / factor;
}

void WorldSelector::SetPosition(sf::Vector2f &pos)
{
	position = pos;
	for (int i = 0; i < 4; ++i)
	{
		SetRectRotation(quads + i * 4, angles[i], 192, 192, position);
	}
}

void WorldSelector::SetAlpha(float alpha)
{
	if (alpha < 0)
		alpha = 0;
	for (int i = 0; i < 4 * 4; ++i)
	{
		quads[i].color.a = min(alpha * 255.f, 255.f);
	}
}

void WorldSelector::Draw(sf::RenderTarget *target)
{
	target->draw(quads, 4 * 4, sf::Quads, ts->texture);
}