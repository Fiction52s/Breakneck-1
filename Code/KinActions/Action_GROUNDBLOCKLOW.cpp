#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCKLOW_Start()
{
	if (!IsBlockAction(oldAction))
	{
		framesBlocking = 0;
	}

	ts_groundBlockLowShield->SetSpriteTexture(shieldSprite);
}

void Actor::GROUNDBLOCKLOW_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCKLOW_Change()
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


	if (!currInput.LDown())
	{
		if (currInput.LUp())
		{
			SetAction(GROUNDBLOCKHIGH);
		}
		else
		{
			SetAction(GROUNDBLOCK);
		}
		frame = 0;	
	}

	
}

void Actor::GROUNDBLOCKLOW_Update()
{
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
}

void Actor::GROUNDBLOCKLOW_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();

	shieldSprite.setOrigin(sprite->getOrigin());
	shieldSprite.setPosition(sprite->getPosition());
	ts_groundBlockLowShield->SetSubRect(shieldSprite, 0, !r, reversed);
	shieldSprite.setRotation(sprite->getRotation());
}

void Actor::GROUNDBLOCKLOW_TransitionToAction(int a)
{
}

void Actor::GROUNDBLOCKLOW_TimeIndFrameInc()
{

}

void Actor::GROUNDBLOCKLOW_TimeDepFrameInc()
{

}

int Actor::GROUNDBLOCKLOW_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDBLOCKLOW_GetTileset()
{
	return GetActionTileset("block_low_64x64.png");
}