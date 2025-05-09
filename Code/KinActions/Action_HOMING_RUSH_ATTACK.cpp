#include "Actor.h"
#include "Enemy.h"

using namespace sf;
using namespace std;

void Actor::HOMING_RUSH_ATTACK_Start()
{
	if (velocity.x < 0)
	{
		facingRight = false;
	}
	else if (velocity.x > 0)
	{
		facingRight = true;
	}
}

void Actor::HOMING_RUSH_ATTACK_End()
{
	SetAction(JUMP);
	frame = 1;
	//frame = 0;
	//frame = 0;
}

void Actor::HOMING_RUSH_ATTACK_Change()
{
	if (BasicAirAttackAction())
		return;

	if (airHomingFrame == -1 || !AttackButtonHeld())
	{
		SetAction(JUMP);
		frame = 1;
	}

	//if (HomingAction())
	//{
	//}
	//else
	//{
	//	//enemy not found. rare movement in Change() but necesssary
	//	if (!TryHomingMovement())
	//	{
	//		SetAction(JUMP);
	//		frame = 1;
	//		if (velocity.x < 0)
	//		{
	//			facingRight = false;
	//		}
	//		else if (velocity.x > 0)
	//		{
	//			facingRight = true;
	//		}
	//	}
	//}
}

void Actor::HOMING_RUSH_ATTACK_Update()
{
	SetCurrHitboxes(homingRushHitboxes[0], 0);

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

void Actor::HOMING_RUSH_ATTACK_UpdateSprite()
{
	showSword = true;

	Tileset *curr_ts = ts_homingRushSword[GetSwordSpeedLevel()];
	swordSprite.setTexture(*curr_ts->texture);

	Vector2f offsetArr[3];
	offsetArr[0] = Vector2f(45, 0);
	offsetArr[1] = Vector2f(45, 0);//Vector2i( 0, 48 );
	offsetArr[2] = Vector2f(45, 0);

	Transform t;

	Vector2f offset = offsetArr[GetSwordSpeedLevel()];

	SetSpriteTexture(action);

	//SetSpriteTile(frame / 2, facingRight);
	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	V2d sVel = velocity;
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
	t.rotate(sprite->getRotation());

	if (!facingRight)
	{
		offset.x = -offset.x;
	}

	offset = t.transformPoint(offset);

	curr_ts->SetSubRect(swordSprite, 0, !facingRight);
	swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2, swordSprite.getLocalBounds().height / 2);
	swordSprite.setPosition(position.x + offset.x, position.y + offset.y);
	swordSprite.setRotation(sprite->getRotation());

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::HOMING_RUSH_ATTACK_TransitionToAction(int a)
{

}

void Actor::HOMING_RUSH_ATTACK_TimeIndFrameInc()
{

}

void Actor::HOMING_RUSH_ATTACK_TimeDepFrameInc()
{

}

int Actor::HOMING_RUSH_ATTACK_GetActionLength()
{
	return 30;//15 * 2;
}

const char * Actor::HOMING_RUSH_ATTACK_GetTilesetName()
{
	return "homing_kick_80x32.png";
}