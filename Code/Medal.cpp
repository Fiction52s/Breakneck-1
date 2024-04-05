#include "Tileset.h"
#include "Medal.h"
#include "Session.h"

using namespace sf;

Medal::Medal()
{
	Session *sess = Session::GetSession();

	ts_medals = sess->GetSizedTileset("HUD/medals_128x128.png");

	actionLength[A_HIDDEN] = 1;
	actionLength[A_APPEAR] = 6;
	actionLength[A_SHINE] = 12;
	actionLength[A_HOLD] = 120;
	actionLength[A_DISAPPEAR] = 6;
	actionLength[A_WAIT] = 1;

	animFactor[A_HIDDEN] = 1;
	animFactor[A_APPEAR] = 4;
	animFactor[A_SHINE] = 4;
	animFactor[A_HOLD] = 1;
	animFactor[A_DISAPPEAR] = 4;
	animFactor[A_WAIT] = 1;
}

void Medal::Reset()
{
	SetType(MEDAL_GOLD);
	SetScale(1.f);
	SetCenter(Vector2f(0, 0));

	action = A_HIDDEN;
	frame = 0;

	UpdateSprite();
}

void Medal::Show()
{
	action = A_APPEAR;
	frame = 0;
}

void Medal::SetCenter(sf::Vector2f pos)
{
	posType = PT_CENTER;
	position = pos;

	SetRectCenter(verts, ts_medals->tileWidth * scale, ts_medals->tileHeight * scale, pos);
	SetRectCenter(verts + 4, ts_medals->tileWidth * scale, ts_medals->tileHeight * scale, pos);
}

void Medal::SetTopLeft(sf::Vector2f pos)
{
	posType = PT_TOP_LEFT;
	position = pos;

	SetRectTopLeft(verts, ts_medals->tileWidth * scale, ts_medals->tileHeight * scale, pos);
	SetRectTopLeft(verts + 4, ts_medals->tileWidth * scale, ts_medals->tileHeight * scale, pos);
}

void Medal::SetScale(float s)
{
	scale = s;
	if (posType == PT_CENTER)
	{
		SetCenter(position);
	}
	else if (posType == PT_TOP_LEFT)
	{
		SetTopLeft(position);
	}
}

bool Medal::IsWaiting()
{
	return action == A_WAIT;
}

void Medal::Update()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case A_HIDDEN:
			break;
		case A_APPEAR:
			action = A_SHINE;
			break;
		case A_SHINE:
			action = A_HOLD;
			break;
		case A_HOLD:
			action = A_DISAPPEAR;
			break;
		case A_DISAPPEAR:
			action = A_WAIT;
			break;
		case A_WAIT:
			break;
		}

		frame = 0;
	}

	UpdateSprite();

	++frame;
}

void Medal::UpdateSprite()
{
	int tile = 0;
	int deltaTile = 0;

	
	switch (action)
	{
	case A_APPEAR:
		tile = frame / animFactor[A_APPEAR];
		break;
	case A_SHINE:
		tile = 5;
		SetRectSubRect(verts + 4, ts_medals->GetSubRect(18 + frame / animFactor[A_SHINE]));
		break;
	case A_HOLD:
		tile = 5;
		break;
	case A_DISAPPEAR:
		tile = 5 - frame / animFactor[A_DISAPPEAR];
		break;
	case A_WAIT:
		tile = 5;
		break;
	}

	SetRectSubRect(verts, ts_medals->GetSubRect(tile + medalType * 6 ));
}

void Medal::SetType(int t)
{
	medalType = t;
}

void Medal::Draw(sf::RenderTarget *target)
{
	if (action == A_HIDDEN || action == A_WAIT )
	{
		return;
	}

	if (action == A_SHINE)
	{
		target->draw(verts, 8, sf::Quads, ts_medals->texture);
	}
	else
	{
		target->draw(verts, 4, sf::Quads, ts_medals->texture);
	}
	
}