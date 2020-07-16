#include "HUD.h"
#include "Session.h"
#include "Minimap.h"

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
}


void FightHUD::Reset()
{
}

void FightHUD::Draw(RenderTarget *target)
{
	mini->Draw(target);
}