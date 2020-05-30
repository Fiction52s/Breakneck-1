#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::JUMPSQUAT_Start()
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

void Actor::JUMPSQUAT_End()
{
	frame = 0;
}

void Actor::JUMPSQUAT_Change()
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

	if (frame == actionLength[JUMPSQUAT] - 1)
	{
		SetActionExpr(JUMP);
		frame = 0;
		groundSpeed = storedGroundSpeed;
	}
}

void Actor::JUMPSQUAT_Update()
{
	if (frame == 0 && slowCounter == 1)
	{
		ActivateSound(S_JUMP);
		storedGroundSpeed = groundSpeed;
		/*if( reversed )
		storedGroundSpeed = -storedGroundSpeed;*/

		groundSpeed = 0;
	}
}

void Actor::JUMPSQUAT_UpdateSprite()
{
	SetSpriteTexture(action);
	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	double angle = GroundedAngle();

	//some special stuff for jumpsquat
	if (reversed)
	{
		if (-gn.y > -steepThresh)
		{
			angle = PI;
		}
	}
	else
	{
		if (gn.y > -steepThresh)
		{
			angle = 0;
		}
	}



	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);


	V2d pp;
	if (ground != NULL)
		pp = ground->GetPosition(edgeQuantity);
	else if (grindEdge != NULL)
		pp = grindEdge->GetPosition(grindQuantity);
	else
	{
		assert(0);
	}

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		sprite->setPosition(pp.x + offsetX, pp.y);
	else
		sprite->setPosition(pp.x, pp.y);
}