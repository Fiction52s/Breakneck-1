#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DASHATTACK2_Start()
{
	DASHATTACK_Start();
}

void Actor::DASHATTACK2_End()
{
	DASHATTACK_End();
}

void Actor::DASHATTACK2_Change()
{
	DASHATTACK_Change();
}

void Actor::DASHATTACK2_Update()
{
	//change this to dash attack 2 hitboxes soon
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
		
	}

	AttackMovement();
}

void Actor::DASHATTACK2_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_dashAttackSword2[speedLevel],
		0, 13, 2, Vector2f(0, 0));
}

void Actor::DASHATTACK2_TransitionToAction(int a)
{

}

void Actor::DASHATTACK2_TimeIndFrameInc()
{

}

void Actor::DASHATTACK2_TimeDepFrameInc()
{

}

int Actor::DASHATTACK2_GetActionLength()
{
	return 14 * 2;
}

Tileset * Actor::DASHATTACK2_GetTileset()
{
	return GetActionTileset("dash_att_02_128x64.png");
}