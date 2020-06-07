#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::BOUNCEAIR_Start()
{

}

void Actor::BOUNCEAIR_End()
{
	frame = 0;
}

void Actor::BOUNCEAIR_Change()
{
	if (!currInput.X)
	{
		SetAction(JUMP);
		frame = 1;
		BounceFlameOff();
	}

	if (TryAirDash()) return;

	if (AirAttack())
	{
		BounceFlameOn();
		//bounceFlameOn = true;
		airBounceFrame = 13 * 3;
	}
}

void Actor::BOUNCEAIR_Update()
{
	if (framesInAir > 8) //to prevent you from clinging to walls awkwardly
	{
		//	cout << "movement" << endl;
		AirMovement();
	}
	else
	{
		//	cout << "not movement" << endl;
	}
}

void Actor::BOUNCEAIR_UpdateSprite()
{
	int bounceFrame = 0;
	if (oldBounceEdge == NULL)
	{
		bounceFrame = 6;
	}
	else if (framesSinceBounce < 10)
	{
		int xThresh = 10;
		int yThresh = 10;
		if (velocity.y > yThresh)
		{
			if (abs(velocity.x) < xThresh)//10 just for testing
			{
				bounceFrame = 5;
			}
			else
			{
				bounceFrame = 8;
			}
		}
		else if (velocity.y < -yThresh)
		{
			if (abs(velocity.x) < xThresh)//10 just for testing
			{
				bounceFrame = 1;
			}
			else
			{
				bounceFrame = 7;
			}
		}
		else
		{
			if (abs(velocity.x) > xThresh)
			{
				bounceFrame = 3;
			}
			else
			{
				bounceFrame = 6;
			}
		}



		V2d bn = oldBounceNorm;//oldBounceEdge->Normal();
							   //	if( bn.y <= 0 && bn.y > -steepThresh )
							   //	{
							   //		bounceFrame = 7;
							   //		if( facingRight )
							   //		{
							   //			//facingRight = false;

	}
	else
	{
		bounceFrame = 6;
	}
	//	}
	//	else if( bn.y >= 0 && -bn.y > -steepThresh )
	//	{
	//		
	//		bounceFrame = 8;
	//	}
	//	else if( bn.y == 0 )
	//	{
	//		bounceFrame = 3;
	//	}
	//	else if( bn.y < 0 )
	//	{
	//		bounceFrame = 1;
	//	}
	//	else if( bn.y > 0 )
	//	{
	//		bounceFrame = 5;
	//	}
	//}
	//else
	//{
	//	bounceFrame = 6;
	//}

	SetSpriteTexture(action);

	SetSpriteTile(bounceFrame, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	scorpSet = true;
}

void Actor::BOUNCEAIR_TransitionToAction(int a)
{

}

void Actor::BOUNCEAIR_TimeIndFrameInc()
{

}

void Actor::BOUNCEAIR_TimeDepFrameInc()
{

}

int Actor::BOUNCEAIR_GetActionLength()
{
	return 1;
}

Tileset * Actor::BOUNCEAIR_GetTileset()
{
	return GetActionTileset("bounce_224x224.png");
}