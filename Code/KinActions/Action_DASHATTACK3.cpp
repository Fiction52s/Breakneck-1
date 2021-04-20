#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DASHATTACK3_Start()
{
	DASHATTACK_Start();
}

void Actor::DASHATTACK3_End()
{
	DASHATTACK_End();
}

void Actor::DASHATTACK3_Change()
{
	DASHATTACK_Change();
}

void Actor::DASHATTACK3_Update()
{
	//change this to dash attack 2 hitboxes soon
	SetCurrHitboxes(dashHitboxes3[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
	}

	AttackMovement();
}

void Actor::DASHATTACK3_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_dashAttackSword3[speedLevel],
		0, 12, 2, Vector2f(0, 0));
}

void Actor::DASHATTACK3_TransitionToAction(int a)
{

}

void Actor::DASHATTACK3_TimeIndFrameInc()
{

}

void Actor::DASHATTACK3_TimeDepFrameInc()
{

}

int Actor::DASHATTACK3_GetActionLength()
{
	return 16 * 2;
}

Tileset * Actor::DASHATTACK3_GetTileset()
{
	return GetActionTileset("dash_att_03_128x64.png");
}