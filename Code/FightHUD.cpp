#include "HUD.h"
#include "Session.h"
#include "Minimap.h"
#include "MapHeader.h"
#include "GameMode.h"

using namespace std;
using namespace sf;

FightHUD::FightHUD()
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

	Color p0Color = Color::Green;
	float mh = fm->maxHealth;
	if (fm->data.health[0] < mh * .33)
	{
		p0Color = Color::Red;
	}
	else if (fm->data.health[0] < mh * .67)
	{
		p0Color = Color::Yellow;
	}

	Color p1Color = Color::Green;
	if (fm->data.health[1] < mh * .33)
	{
		p1Color = Color::Red;
	}
	else if (fm->data.health[1] < mh * .67)
	{
		p1Color = Color::Yellow;
	}
	
	p0HealthRect.setSize(Vector2f(700 * (fm->data.health[0] / mh), 30));
	p0HealthRect.setOrigin( p0HealthRect.getLocalBounds().width, 0);
	p0HealthRect.setFillColor(p0Color);
	p0HealthRect.setPosition(Vector2f(960 - 10, 10));
	
	p1HealthRect.setSize(Vector2f(700 * (fm->data.health[1] / mh), 30));
	p1HealthRect.setFillColor(p1Color);
	p1HealthRect.setPosition(Vector2f(960 + 10, 10));
}


void FightHUD::Reset()
{
}

void FightHUD::Draw(RenderTarget *target)
{
	mini->Draw(target);

	target->draw(p0HealthRect);
	target->draw(p1HealthRect);
}