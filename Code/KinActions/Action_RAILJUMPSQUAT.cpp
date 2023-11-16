#include "Actor.h"
#include <assert.h>
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::RAILJUMPSQUAT_Start()
{
	framesInAir = 0;
	bufferedAttack = JUMP;

	if (currInput.rightShoulder && !prevInput.rightShoulder)
	{
		if (currInput.LUp())
		{
			bufferedAttack = UAIR; //none
		}
		else if (currInput.LDown())
		{
			bufferedAttack = DAIR;
		}
		else
		{
			bufferedAttack = FAIR;
		}
	}
}

void Actor::RAILJUMPSQUAT_End()
{
	frame = 0;
}

void Actor::RAILJUMPSQUAT_Change()
{
	if (currInput.rightShoulder && !prevInput.rightShoulder && bufferedAttack == JUMP)
	{
		if (currInput.LUp())
		{
			bufferedAttack = UAIR; //none
		}
		else if (currInput.LDown())
		{
			bufferedAttack = DAIR;
		}
		else
		{
			bufferedAttack = FAIR;
		}
		//bufferedAttack = true;
	}

	if (frame == GetActionLength(RAILJUMPSQUAT) - 1)
	{
		SetAction(JUMP);
		frame = 0;
		groundSpeed = storedGroundSpeed;
	}
}

void Actor::RAILJUMPSQUAT_Update()
{
	if (frame == 0 && slowCounter == 1)
	{
		ActivateSound(PlayerSounds::S_JUMP);
		storedGroundSpeed = groundSpeed;
		/*if( reversed )
		storedGroundSpeed = -storedGroundSpeed;*/

		groundSpeed = 0;
	}
}

void Actor::RAILJUMPSQUAT_UpdateSprite()
{
	SetSpriteTexture(action);
	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);


	V2d diff = grindEdge->FullAlong();
	V2d norm = grindEdge->Normal();
	if (grindEdge->Normal().y > 0)
	{
		diff = -diff;
		norm = -norm;
	}
	//atan2(testDiff.y, testDiff.x) / PI * 180;
	double angle = atan2(diff.y, diff.x);

	//some special stuff for RAILJUMPSQUAT
	if (reversed)
	{
		if (-norm.y > -steepThresh)
		{
			angle = PI;
		}
	}
	else
	{
		if (norm.y > -steepThresh)
		{
			angle = 0;
		}
	}

	if (ground != NULL)
		SetGroundedSpriteTransform(ground, angle);
	else if (grindEdge != NULL)
	{
		SetGroundedSpriteTransform(grindEdge, angle);
	}

	else
	{
		assert(0);
	}

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSlide, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 10);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::RAILJUMPSQUAT_TransitionToAction(int a)
{

}

void Actor::RAILJUMPSQUAT_TimeIndFrameInc()
{

}

void Actor::RAILJUMPSQUAT_TimeDepFrameInc()
{

}

int Actor::RAILJUMPSQUAT_GetActionLength()
{
	return 3;
}

const char * Actor::RAILJUMPSQUAT_GetTilesetName()
{
	return "jump_64x64.png";
}