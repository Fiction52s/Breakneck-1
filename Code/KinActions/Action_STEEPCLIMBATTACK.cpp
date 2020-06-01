#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::STEEPCLIMBATTACK_Start()
{
}

void Actor::STEEPCLIMBATTACK_End()
{
	SetActionExpr(STEEPCLIMB);
	frame = 0;
}

void Actor::STEEPCLIMBATTACK_Change()
{
	if (currAttackHit)//&& frame > 0 )
	{
		if (hasPowerBounce && currInput.X && !bounceFlameOn)
		{
			BounceFlameOn();
		}
		else if (!(hasPowerBounce && currInput.X) && bounceFlameOn)
		{
			//bounceGrounded = false;
			BounceFlameOff();
		}

		if (hasPowerGrindBall && currInput.Y && !prevInput.Y)
		{
			SetActionGrind();
			return;
		}

		if (currInput.A && !prevInput.A || pauseBufferedJump)
		{
			SetActionExpr(JUMPSQUAT);
			frame = 0;
			return;
		}

		if (SteepClimbAttack())
		{
			return;
		}
	}
}

void Actor::STEEPCLIMBATTACK_Update()
{
	SetCurrHitboxes(steepClimbHitboxes[speedLevel], frame / 2);

	if (frame == 0)
	{
		currAttackHit = false;
	}


	bool boost = TryClimbBoost(currNormal);

	float factor = steepClimbGravFactor;//.7 ;
	if (currInput.LUp())
	{
		//cout << "speeding up climb!" << endl;
		factor = steepClimbFastFactor;//.5;
	}

	if (reversed)
	{
		groundSpeed += dot(V2d(0, GetGravity() * factor), normalize(ground->v1 - ground->v0)) / slowMultiple;
	}
	else
	{
		groundSpeed += dot(V2d(0, GetGravity() * factor), normalize(ground->v1 - ground->v0)) / slowMultiple;
	}
}

void Actor::STEEPCLIMBATTACK_UpdateSprite()
{
	int startFrame = 0;
	showSword = true;//frame / 2 >= startFrame && frame / 2 <= 7;
	Tileset *curr_ts = ts_steepClimbAttackSword[speedLevel];
	int animFactor = 2;

	if (showSword)
	{
		steepClimbAttackSword.setTexture(*curr_ts->texture);
	}

	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	int tFrame = frame / animFactor;
	SetSpriteTile(tFrame, r);

	Vector2f offset = climbAttackOffset[speedLevel];

	V2d trueNormal;
	double angle = GroundedAngleAttack(trueNormal);

	if (showSword)
	{
		if (r)
		{
			steepClimbAttackSword.setTextureRect(curr_ts->GetSubRect(frame / animFactor - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame / animFactor - startFrame);
			steepClimbAttackSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));

			offset.x = -offset.x;

		}

		steepClimbAttackSword.setTexture(*curr_ts->texture);
		steepClimbAttackSword.setOrigin(steepClimbAttackSword.getLocalBounds().width / 2, steepClimbAttackSword.getLocalBounds().height);
		steepClimbAttackSword.setRotation(angle / PI * 180);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		sprite->setPosition(pp.x + offsetX, pp.y);
	else
		sprite->setPosition(pp.x, pp.y);

	V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y);
	V2d truDir(-trueNormal.y, trueNormal.x);//normalize( ground->v1 - ground->v0 );

	pos += trueNormal * (double)offset.y;
	pos += truDir * (double)offset.x;

	steepClimbAttackSword.setPosition(pos.x, pos.y);

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpClimb->texture);

		SetSpriteTile(&scorpSprite, ts_scorpClimb, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::STEEPCLIMBATTACK_TransitionToAction(int a)
{

}

void Actor::STEEPCLIMBATTACK_TimeIndFrameInc()
{

}

void Actor::STEEPCLIMBATTACK_TimeDepFrameInc()
{

}

int Actor::STEEPCLIMBATTACK_GetActionLength()
{
	return 4 * 4;
}

Tileset * Actor::STEEPCLIMBATTACK_GetTileset()
{
	return GetActionTileset("climb_att_128x64.png");
}