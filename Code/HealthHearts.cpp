#include "HealthHearts.h"
#include "Session.h"
#include "Actor.h"

using namespace sf;
using namespace std;

HealthHearts::HealthHearts( TilesetManager *tm )
{
	ts_heartIcon = tm->GetSizedTileset("HUD/hearticon_63x58.png");
}

void HealthHearts::SetTopLeft(sf::Vector2f pos)
{
	//232, 15
	//272, 56

	//hearticon_63x58_232, 15_272, 56
	float xSpacing = 80;
	topLeft = pos;

	Vector2f firstRow = pos;//(232, 15);
	Vector2f secondRowOffset(40, 41);
	for (int i = 0; i < FIRST_ROW_ITEMS; ++i)
	{
		SetRectTopLeft(heartQuads + i * 4, ts_heartIcon->tileWidth, ts_heartIcon->tileHeight, Vector2f(firstRow.x + xSpacing * i, firstRow.y));
	}
	for (int i = FIRST_ROW_ITEMS; i < SECOND_ROW_ITEMS + FIRST_ROW_ITEMS; ++i)
	{
		SetRectTopLeft(heartQuads + i * 4, ts_heartIcon->tileWidth, ts_heartIcon->tileHeight, Vector2f(firstRow.x + secondRowOffset.x + xSpacing * (i - FIRST_ROW_ITEMS), firstRow.y + secondRowOffset.y));
	}
}

void HealthHearts::SetSession(Session *p_sess)
{
	sess = p_sess;
	actor = sess->GetPlayer(0);

	Reset();
}

void HealthHearts::Reset()
{
	health = actor->health;
	//actor->health = BASE_MAX_HEALTH;
}

void HealthHearts::Update()
{
	health = actor->health;
	SetTopLeft(topLeft);

	for (int i = 0; i < health / 2; ++i)
	{
		SetRectSubRect(heartQuads + i * 4, ts_heartIcon->GetSubRect(0));
	}

	int clearIndex = health / 2;
	if (health % 2 == 1)
	{
		SetRectSubRect(heartQuads + (clearIndex) * 4, ts_heartIcon->GetSubRect(1));
		clearIndex+=1;
	}
	

	for (int i = clearIndex; i < MAX_HEARTS; ++i)
	{
		ClearRect(heartQuads + i * 4);
	}

}


void HealthHearts::Draw(sf::RenderTarget *target)
{
	target->draw(heartQuads, 4 * MAX_HEARTS, sf::Quads, ts_heartIcon->texture);
}