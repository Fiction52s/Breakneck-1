#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::BOOSTERBOUNCEGROUND_Start()
{
	//groundSpeed = //-storedBounceGroundSpeed / (double)slowMultiple;
}

void Actor::BOOSTERBOUNCEGROUND_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::BOOSTERBOUNCEGROUND_Change()
{
	if (CheckSetToAerialFromNormalWater()) return;

	if (TryGroundBlock()) return;


	if (TryFloorRailDropThrough()) return;

	if (TryPressGrind()) return;

	if (TryJumpSquat()) return;

	if (TryGroundAttack()) return;

	if (TryDash()) return;
}

void Actor::BOOSTERBOUNCEGROUND_Update()
{
}

void Actor::BOOSTERBOUNCEGROUND_UpdateSprite()
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
	
	int yOffset = -20;
	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height + yOffset);// +yOffset);
	//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

	sprite->setRotation(angle / PI * 180);
	V2d pp = ground->GetPosition(edgeQuantity);

	SetGroundedSpritePos(ground, angle);

	//scorpSet = true;

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpStand->texture);

		SetSpriteTile(&scorpSprite, ts_scorpStand, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 10);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::BOOSTERBOUNCEGROUND_TransitionToAction(int a)
{

}

void Actor::BOOSTERBOUNCEGROUND_TimeIndFrameInc()
{

}

void Actor::BOOSTERBOUNCEGROUND_TimeDepFrameInc()
{

}

int Actor::BOOSTERBOUNCEGROUND_GetActionLength()
{
	return 20;
}

const char * Actor::BOOSTERBOUNCEGROUND_GetTilesetName()
{
	return "bounce_wall_96x64.png";
	//return GetActionTileset("brake_64x64.png");
}