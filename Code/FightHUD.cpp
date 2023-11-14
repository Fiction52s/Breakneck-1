#include "HUD.h"
#include "Session.h"
#include "Minimap.h"
#include "MapHeader.h"
#include "GameMode.h"
#include "Actor.h"

using namespace std;
using namespace sf;

FightHUD::FightHUD(TilesetManager *tm)
	:HUD(tm)
{
	hType = HUDType::FIGHT;
	Reset();
}

FightHUD::~FightHUD()
{
}

void FightHUD::Update()
{
	mini->Update();

	assert(sess->gameModeType == MatchParams::GAME_MODE_FIGHT);

	FightMode *fm = (FightMode*)sess->gameMode;

	//Color barColors[4];

	//Color p0Color = Color::Green;
	float mh = fm->maxHealth;

	Vector2f barPositions[4];

	int barHeight = 30;
	int barSpacing = 10;
	int barStart = 10;
	int barWidth = 700;

	barPositions[0] = Vector2f(960 - 10 - barWidth, barStart);
	barPositions[1] = Vector2f(960 + 10, barStart);
	barPositions[2] = Vector2f(960 - 10 - barWidth, barStart + barHeight + barSpacing);
	barPositions[3] = Vector2f(960 + 10, barStart + barHeight + barSpacing);

	Color barColor;
	for (int i = 0; i < 4; ++i)
	{
		barColor = Color::Green;
		if (fm->data.health[i] < mh * .33)
		{
			barColor = Color::Red;
		}
		else if (fm->data.health[i] < mh * .67)
		{
			barColor = Color::Yellow;
		}

		
		healthRects[i].setSize(Vector2f(barWidth * (fm->data.health[i] / mh), barHeight));
		//healthRects[i].setOrigin(0, 0);
		healthRects[i].setFillColor(barColor);
		healthRects[i].setPosition(barPositions[i]);
	}
}


void FightHUD::Reset()
{
}

void FightHUD::Draw(RenderTarget *target)
{
	mini->Draw(target);

	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = sess->GetPlayer(i);
		if (p != NULL && !p->dead)
		{
			target->draw(healthRects[i]);
		}
	}
}