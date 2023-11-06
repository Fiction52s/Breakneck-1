#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::RAILBOUNCEGROUND_Start()
{
	//groundSpeed = //-storedBounceGroundSpeed / (double)slowMultiple;
}

void Actor::RAILBOUNCEGROUND_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::RAILBOUNCEGROUND_Change()
{
	if (CheckSetToAerialFromNormalWater()) return;

	if (TryGroundBlock()) return;

	if (TryFloorRailDropThrough()) return;

	if (TryPressGrind()) return;

	if (TryJumpSquat()) return;

	if (TryGroundAttack()) return;

	if (TryDash()) return;
}

void Actor::RAILBOUNCEGROUND_Update()
{
}

void Actor::RAILBOUNCEGROUND_UpdateSprite()
{
	SetSpriteTexture(action);

	int tFrame = -1;
	if (frame < 20)
	{
		tFrame = 1;
	}
	else
	{
		tFrame = 2;
	}

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(tFrame, r);

	double angle = GroundedAngle();
	int yOffset = -10;//-20;
	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height + yOffset);// +yOffset);
																									 //sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

	sprite->setRotation(angle / PI * 180);
	V2d pp = ground->GetPosition(edgeQuantity);

	SetGroundedSpritePos(ground, angle);

	scorpSet = true;
}

void Actor::RAILBOUNCEGROUND_TransitionToAction(int a)
{

}

void Actor::RAILBOUNCEGROUND_TimeIndFrameInc()
{

}

void Actor::RAILBOUNCEGROUND_TimeDepFrameInc()
{

}

int Actor::RAILBOUNCEGROUND_GetActionLength()
{
	return 8;
}

Tileset * Actor::RAILBOUNCEGROUND_GetTileset()
{
	return GetActionTileset("bounce_wall_96x64.png");
	//return GetActionTileset("brake_64x64.png");
}