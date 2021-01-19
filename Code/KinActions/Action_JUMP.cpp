#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::JUMP_Start()
{
	steepJump = false;
}

void Actor::JUMP_End()
{
}

void Actor::JUMP_Change()
{
	if (bufferedAttack != JUMP)
	{
		SetAction(bufferedAttack);
		bufferedAttack = JUMP;
		frame = 0;
		return;
	}

	BasicAirAction();
}

void Actor::JUMP_Update()
{
	if (frame == 0 && slowCounter == 1)
	{
		if (ground != NULL) //this should always be true but we haven't implemented running off an edge yet
		{
			//jumpSound.play();

			if (reversed)
			{
				//so you dont jump straight up on a nearly vertical edge
				double blah = .5;

				V2d dir(0, 0);

				dir.y = .2;
				V2d along = normalize(ground->v1 - ground->v0);
				V2d trueNormal = along;
				if (groundSpeed > 0)
					trueNormal = -trueNormal;

				trueNormal = normalize(trueNormal + dir);
				velocity = abs(groundSpeed) * trueNormal;

				ground = NULL;
				frame = 1; //so it doesnt use the jump frame when just dropping
				reversed = false;
				framesInAir = 0;

			}
			else
			{
				double dSpeed = GetDashSpeed();

				double blah = .25;

				V2d dir(0, 0);

				V2d trueNormal = normalize(dir + normalize(ground->v1 - ground->v0));
				velocity = groundSpeed * trueNormal;
				if (velocity.y < 0)
				{
					velocity.y *= .7;
				}

				if (DashButtonHeld())
				{
					if (currInput.LRight() && velocity.x < dSpeed)
					{
						velocity.x = dSpeed;
					}
					else if (currInput.LLeft() && velocity.x > -dSpeed)
					{
						velocity.x = -dSpeed;
					}
					else
					{
						velocity.x = (velocity.x + groundSpeed) / 2.0;
					}
				}
				else
				{
					velocity.x = (velocity.x + groundSpeed) / 2.0;
				}

				if (velocity.y > 0)
				{
					//min jump velocity for jumping off of edges.
					if (abs(velocity.x) < dSpeed && length(velocity) >= dSpeed)
					{
						if (velocity.x > 0)
						{
							velocity.x = dSpeed;
						}
						else
						{
							velocity.x = -dSpeed;
						}
					}

					velocity.y = 0;
				}

				if (steepJump)
				{
					velocity.y -= jumpStrength * .75;
				}
				else
				{
					velocity.y -= jumpStrength;
				}

				V2d pp = ground->GetPosition(edgeQuantity);
				double ang = GroundedAngle();
				V2d fxPos;
				if ((approxEquals(ang, 0) && !reversed) || (approxEquals(ang, PI) && reversed))
					fxPos = V2d(pp.x + offsetX, pp.y);
				else
					fxPos = pp;

				fxPos += currNormal * 16.0;

				switch (speedLevel)
				{
				case 0:
					ActivateEffect(EffectLayer::IN_FRONT, ts_fx_jump[0], fxPos, false, ang, 6, 4, facingRight);
					break;
				case 1:
					ActivateEffect(EffectLayer::IN_FRONT, ts_fx_jump[1], fxPos, false, ang, 6, 4, facingRight);
					break;
				case 2:
					ActivateEffect(EffectLayer::IN_FRONT, ts_fx_jump[2], fxPos, false, ang, 6, 4, facingRight);
					break;
				}

				//ActivateEffect( EffectLayer::IN_FRONT, ts_fx_jump, fxPos , false, ang, 6, 4, facingRight );

				ground = NULL;
				holdJump = true;

				framesInAir = 0;

				if (touchedGrass[Grass::JUMP])
				{
					velocity.y -= jumpGrassExtra;
				}
				//steepJump = false;
			}

		}
		else if (grindEdge != NULL)
		{
			//assert(0);
			//V2d ev0, ev1;
			Edge tempEdge(*grindEdge);
			/*if (tempEdge.Normal().y > 0)
			{
			V2d temp = tempEdge.v0;
			tempEdge.v0 = tempEdge.v1;
			tempEdge.v1 = temp;
			}*/

			double dSpeed = GetDashSpeed();

			double blah = .25;

			V2d dir(0, 0);

			V2d trueNormal = normalize(dir + normalize(tempEdge.v1 - tempEdge.v0));
			velocity = grindSpeed * trueNormal;
			if (velocity.y < 0)
			{
				velocity.y *= .7;
			}

			/*if (currInput.B)
			{
			if (currInput.LRight())
			{
			if (velocity.x < dSpeed)
			velocity.x = dSpeed;
			}
			else if (currInput.LLeft())
			{
			if (velocity.x > -dSpeed)
			velocity.x = -dSpeed;
			}
			}*/
			if (velocity.y > 0)
			{
				//min jump velocity for jumping off of edges.
				if (abs(velocity.x) < dSpeed && length(velocity) >= dSpeed)
				{
					//		cout << "here: " << velocity.x << endl;
					if (velocity.x > 0)
					{
						velocity.x = dSpeed;
					}
					else
					{
						velocity.x = -dSpeed;
					}
				}

				velocity.y = 0;
			}

			if (steepJump)
			{
				velocity.y -= jumpStrength * .75;
			}
			else
			{
				velocity.y -= jumpStrength;
			}
			holdJump = true;

			framesInAir = 0;


			grindEdge = NULL;

		}
		else
		{
			assert(0);
		}


	}
	else if (frame > 0)
	{
		//if( bufferedAttack )
		//{
		//	bufferedAttack = false;
		//}
		//cout << "vel at beg: " << velocity.x << ", " << velocity.y << endl;
		CheckHoldJump();

		if (framesInAir > 1 || velocity.y < 0)
			AirMovement();
	}
}

void Actor::JUMP_UpdateSprite()
{
	sf::IntRect ir;
	int tFrame = GetJumpFrame();

	SetSpriteTexture(JUMP);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(tFrame, r);

	if (frame > 0)
	{
		sprite->setRotation(0);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);


	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::JUMP_TransitionToAction(int a)
{

}

void Actor::JUMP_TimeIndFrameInc()
{

}

void Actor::JUMP_TimeDepFrameInc()
{

}

int Actor::JUMP_GetActionLength()
{
	return 2;
}

Tileset * Actor::JUMP_GetTileset()
{
	return GetActionTileset("jump_64x64.png");
}