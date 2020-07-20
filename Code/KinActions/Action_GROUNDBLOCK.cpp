#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCK_Start()
{
	if (!IsBlockAction(oldAction) )
	{
		framesBlocking = 0;
	}
}

void Actor::GROUNDBLOCK_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCK_Change()
{
	if (!currInput.Y && blockstunFrames == 0 )
	{
		SetAction(STAND);
		frame = 0;
	}
	else
	{
		if (currInput.LDown())
		{
			SetAction(GROUNDBLOCKLOW);
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
	//BasicGroundAction(currNormal);
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
	SetSpriteTile(1, r);

	SetGroundedSpriteTransform();

	/*if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSlide, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 15);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}*/
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
	return GetActionTileset("hurt_64x64.png");
}