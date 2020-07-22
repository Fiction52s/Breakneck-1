#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCKHIGH_Start()
{
	if (!IsBlockAction(oldAction))
	{
		framesBlocking = 0;
	}

	ts_groundBlockHighShield->SetSpriteTexture(shieldSprite);
}

void Actor::GROUNDBLOCKHIGH_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCKHIGH_Change()
{
	if (currInput.LLeft())
	{
		facingRight = false;
	}
	else if (currInput.LRight())
	{
		facingRight = true;
	}

	if (blockstunFrames == 0)
	{
		if (!currInput.Y)
		{
			SetAction(SLIDE);
			frame = 0;
			return;
		}
		else
		{

			if (TryJumpSquat()) return;

			if (TryGroundAttack()) return;
		}
	}
	
	if (!currInput.LUp())
	{
		if (currInput.LDown())
		{
			SetAction(GROUNDBLOCKLOW);
		}
		else
		{
			SetAction(GROUNDBLOCK);
		}
			
		frame = 0;
	}
}

void Actor::GROUNDBLOCKHIGH_Update()
{
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
}

void Actor::GROUNDBLOCKHIGH_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();

	shieldSprite.setOrigin(sprite->getOrigin());
	shieldSprite.setPosition(sprite->getPosition());
	ts_groundBlockHighShield->SetSubRect(shieldSprite, 0, !r, reversed);
	shieldSprite.setRotation(sprite->getRotation());
}

void Actor::GROUNDBLOCKHIGH_TransitionToAction(int a)
{
}

void Actor::GROUNDBLOCKHIGH_TimeIndFrameInc()
{

}

void Actor::GROUNDBLOCKHIGH_TimeDepFrameInc()
{

}

int Actor::GROUNDBLOCKHIGH_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDBLOCKHIGH_GetTileset()
{
	return GetActionTileset("block_high_64x64.png");
}