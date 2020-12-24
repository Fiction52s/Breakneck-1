#include "Actor.h"
#include "Enemy.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNHOMING_Start()
{
}

void Actor::SPRINGSTUNHOMING_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNHOMING_Change()
{
	if (springStunFrames == 0)
	{
		SetAction(JUMP);
		frame = 1;
		if (velocity.x < 0)
		{
			facingRight = false;
		}
		else if (velocity.x > 0)
		{
			facingRight = true;
		}
	}

	if (HomingAction())
	{
		springStunFrames = 0;
		return;
	}
	else
	{
		
	}

	if (AttackButtonPressed() && TryHomingMovement())
	{
		SetAction(SPRINGSTUNHOMINGATTACK);
	}
}

void Actor::SPRINGSTUNHOMING_Update()
{
	/*Enemy *foundEnemy = NULL;
	int foundIndex;



	if (GetClosestEnemyPos(position, 2000, foundEnemy, foundIndex))
	{
		V2d eDir = normalize(foundEnemy->GetPosition() - position);
		springVel = eDir * length(springVel);
	}*/



	//if (currInput.LUp())
	//{
	//	if (glideTurnFactor < 0)
	//	{
	//		glideTurnFactor = 0;
	//	}
	//	glideTurnFactor += glideTurnAccel;
	//	if (glideTurnFactor > maxGlideTurnFactor)
	//	{
	//		glideTurnFactor = maxGlideTurnFactor;
	//	}
	//	//RotateCCW(springVel, glideTurnFactor);
	//}
	//else if (currInput.LDown())
	//{
	//	if (glideTurnFactor > 0)
	//	{
	//		glideTurnFactor = 0;
	//	}
	//	glideTurnFactor -= glideTurnAccel;
	//	if (glideTurnFactor < -maxGlideTurnFactor)
	//	{
	//		glideTurnFactor = -maxGlideTurnFactor;
	//	}
	//	//RotateCCW(springVel, glideTurnFactor);
	//	//grav = AddGravity(V2d(0, 0));
	//}
	//else
	//{
	//	glideTurnFactor = 0;
	//}


	//if (facingRight)
	//{
	//	RotateCCW(springVel, glideTurnFactor);
	//}
	//else
	//{
	//	RotateCCW(springVel, -glideTurnFactor);
	//}

	////springExtra = AddGravity(springExtra);
	////if (springVel.y > 0)
	//if (false)
	//{
	//	double gFac = 1.0;
	//	V2d nsv = normalize(springVel);
	//	springVel = nsv * (length(springVel) + gFac * nsv.y);//AddGravity(springVel);
	//}


	//springExtra = tempVel - springVel;
	velocity = springVel + springExtra;
}

void Actor::SPRINGSTUNHOMING_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	V2d sVel = springVel + springExtra;
	if (facingRight)
	{
		double a = GetVectorAngleCW(normalize(sVel)) * 180 / PI;
		sprite->setRotation(a);
	}
	else
	{
		double a = GetVectorAngleCCW(normalize(sVel)) * 180 / PI;
		sprite->setRotation(-a + 180);
	}

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::SPRINGSTUNHOMING_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNHOMING_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNHOMING_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNHOMING_GetActionLength()
{
	return 8;
}

Tileset * Actor::SPRINGSTUNHOMING_GetTileset()
{
	return SPRINGSTUN_GetTileset();
}