#include "HUD.h"
#include "Session.h"
#include "Minimap.h"
#include "MapHeader.h"
#include "GameMode.h"

using namespace std;
using namespace sf;

FightHUD::FightHUD()
{
	Reset();
}

FightHUD::~FightHUD()
{
}

void FightHUD::Update()
{
	mini->Update();

	assert(sess->GetGameMode() == MapHeader::T_FIGHT);

	FightMode *fm = (FightMode*)sess->gameMode;

	Color p0Color = Color::Green;
	float mh = fm->maxHealth;
	if (fm->data.p0Health < mh * .67)
	{
		p0Color = Color::Yellow;
	}
	else if (fm->data.p0Health < mh / 3)
	{
		p0Color = Color::Red;
	}

	Color p1Color = Color::Green;
	if (fm->data.p1Health < mh * .67)
	{
		p1Color = Color::Yellow;
	}
	else if (fm->data.p1Health < mh / 3)
	{
		p1Color = Color::Red;
	}
	
	p0HealthRect.setSize(Vector2f(700 * (fm->data.p0Health / mh), 30));
	p0HealthRect.setOrigin( p0HealthRect.getLocalBounds().width, 0);
	p0HealthRect.setFillColor(p0Color);
	p0HealthRect.setPosition(Vector2f(960 - 10, 10));
	
	p1HealthRect.setSize(Vector2f(700 * (fm->data.p1Health / mh), 30));
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