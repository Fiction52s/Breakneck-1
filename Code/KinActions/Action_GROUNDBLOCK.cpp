#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCK_Start()
{
	if (!IsBlockAction(oldAction) )
	{
		framesBlocking = 0;
	}
	ts_groundBlockShield->SetSpriteTexture(shieldSprite);
}

void Actor::GROUNDBLOCK_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCK_Change()
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
		if (!currInput.Y )
		{
			SetAction(STAND);
			frame = 0;
			return;
		}
		else
		{
			if (TryJumpSquat()) return;

			if (TryGroundAttack()) return;

		}
	}

	if (currInput.LDown())
	{
		SetAction(GROUNDBLOCKLOW);
		frame = 0;
	}
	else if (currInput.LUp())
	{
		SetAction(GROUNDBLOCKHIGH);
		frame = 0;
	}
}

void Actor::GROUNDBLOCK_Update()
{
	//double gAngle = GroundedAngle();
	////double fac = gravity * 2.0 / 3;
	//if (gAngle != 0 && gAngle != PI)
	//{
	//	if (reversed)
	//	{
	//		double accel = dot(V2d(0, slideGravFactor * GetGravity()), normalize(ground->v1 - ground->v0)) / slowMultiple;
	//		groundSpeed += accel;

	//	}
	//	else
	//	{
	//		double accel = dot(V2d(0, slideGravFactor * GetGravity()), normalize(ground->v1 - ground->v0)) / slowMultiple;
	//		groundSpeed += accel;
	//		//cout << "accel slide: \n" << accel;
	//	}
	//}
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
}

void Actor::GROUNDBLOCK_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();

	shieldSprite.setOrigin(sprite->getOrigin());
	shieldSprite.setPosition(sprite->getPosition());
	ts_groundBlockShield->SetSubRect(shieldSprite, 0, !r, reversed);
	shieldSprite.setRotation(sprite->getRotation());
}

void Actor::GROUNDBLOCK_TransitionToAction(int a)
{
}

void Actor::GROUNDBLOCK_TimeIndFrameInc()
{

}

void Actor::GROUNDBLOCK_TimeDepFrameInc()
{

}

int Actor::GROUNDBLOCK_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDBLOCK_GetTileset()
{
	return GetActionTileset("block_slide_64x64.png");
}