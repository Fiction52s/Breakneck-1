#include "Actor.h"
#include "Enemy.h"

using namespace sf;
using namespace std;

void Actor::HOMINGATTACK_Start()
{

}

void Actor::HOMINGATTACK_End()
{
	frame = 0;
}

void Actor::HOMINGATTACK_Change()
{
	if (homingFrames == 0 || !AttackButtonHeld())
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
	}
	else
	{
		//enemy not found. rare movement in Change() but necesssary
		if (!TryHomingMovement())
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
	}
}

void Actor::HOMINGATTACK_Update()
{
	

	SetCurrHitboxes(homingHitboxes, 0);

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
}

void Actor::HOMINGATTACK_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);

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


	ts_homingAttackBall->SetSubRect(homingAttackBallSprite, frame / 2, !facingRight);
	homingAttackBallSprite.setOrigin(homingAttackBallSprite.getLocalBounds().width / 2, homingAttackBallSprite.getLocalBounds().height / 2);
	homingAttackBallSprite.setPosition(position.x, position.y);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::HOMINGATTACK_TransitionToAction(int a)
{

}

void Actor::HOMINGATTACK_TimeIndFrameInc()
{

}

void Actor::HOMINGATTACK_TimeDepFrameInc()
{

}

int Actor::HOMINGATTACK_GetActionLength()
{
	return 15 * 2;
}

Tileset * Actor::HOMINGATTACK_GetTileset()
{
	return GetActionTileset("homing_att_64x64.png");
	//return SPRINGSTUN_GetTileset();
}