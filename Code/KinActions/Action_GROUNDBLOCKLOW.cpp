#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCKLOW_Start()
{
	if (!IsBlockAction(oldAction))
	{
		framesBlocking = 0;
	}
}

void Actor::GROUNDBLOCKLOW_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCKLOW_Change()
{
	if (!currInput.Y && blockstunFrames == 0)
	{
		SetAction(STAND);
		frame = 0;
	}
	else
	{
		if (!currInput.LDown())
		{
			if (currInput.LUp())
			{
				//high block
			}

			SetAction(GROUNDBLOCK);
			frame = 0;
		}
	}

	if (currInput.LLeft())
	{
		facingRight = false;
	}
	else if (currInput.LRight())
	{
		facingRight = true;
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