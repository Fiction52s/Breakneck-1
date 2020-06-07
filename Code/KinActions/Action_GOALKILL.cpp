#include "Actor.h"
#include "GameSession.h"
#include "ScoreDisplay.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::GOALKILL_Start()
{
	SetExpr(KinMask::Expr_NEUTRAL);
	velocity = V2d(0, 0);
	groundSpeed = 0;
	grindSpeed = 0;
}

void Actor::GOALKILL_End()
{
	facingRight = true;
	SetAction(GOALKILLWAIT);
	frame = 0;
	owner->scoreDisplay->Activate();
}

void Actor::GOALKILL_Change()
{
}

void Actor::GOALKILL_Update()
{
}

void Actor::GOALKILL_UpdateSprite()
{
	assert(slowCounter == 1);
	if (slowCounter == 1)
	{
		if (frame == 20)
		{
			ActivateSound(S_GOALKILLSLASH1);
		}
		else if (frame == 36)
		{
			ActivateSound(S_GOALKILLSLASH2);
		}
		else if (frame == 60)
		{
			ActivateSound(S_GOALKILLSLASH3);
		}
		else if (frame == 84)
		{
			ActivateSound(S_GOALKILLSLASH4);
		}
	}


	int tsIndex = (frame / 2) / 16;
	switch (tsIndex)
	{
	case 0:
		SetSpriteTexture(GOALKILL);
		break;
	case 1:
		SetSpriteTexture(GOALKILL1);
		break;
	case 2:
		SetSpriteTexture(GOALKILL2);
		break;
	case 3:
		SetSpriteTexture(GOALKILL3);
		break;
	case 4:
		SetSpriteTexture(GOALKILL4);
		break;
	default:
		assert(0);
		break;
	}
	//radius is 24. 100 pixel offset

	int realFrame = (frame / 2) % 16;
	//cout << "goalkill index: " << tsIndex << ", realFrame: " << realFrame << ", frame: " << frame << endl;

	SetSpriteTile(realFrame, facingRight);
	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2 + 24);

	if (action == GOALKILL) //move this laster ewww
	{
		CubicBezier cb(.61, .3, .4, 1);//0, 0, 1, 1 );


									   //cb.GetValue()

		V2d start = owner->goalNodePos;

		V2d end = owner->goalNodePosFinal;
		int st = 48 * 2;
		if (frame >= st)
		{
			double a = (frame - st) / (double)(GetActionLength(GOALKILL) - (st + 1));
			double t = cb.GetValue(a);
			V2d newPos = start + (end - start) * t;
			position = newPos;
		}

		//72 * 2case 

		/*float fff = 78.f / (actionLength[GOALKILL] - st);
		if (frame >= st)
		{
		position.y -= fff;
		}*/
	}

	//sprite->setPosition( owner->goalNodePos.x, owner->goalNodePos.y - 24.f );//- 24.f );
	sprite->setPosition(Vector2f(position));
	sprite->setRotation(0);
}

void Actor::GOALKILL_TransitionToAction(int a)
{

}

void Actor::GOALKILL_TimeIndFrameInc()
{

}

void Actor::GOALKILL_TimeDepFrameInc()
{

}

int Actor::GOALKILL_GetActionLength()
{
	return 72 * 2;
}

Tileset * Actor::GOALKILL_GetTileset()
{
	tileset[GOALKILL1] = GetActionTileset("goal_w01_killb_384x256.png");
	tileset[GOALKILL2] = GetActionTileset("goal_w01_killc_384x256.png");
	tileset[GOALKILL3] = GetActionTileset("goal_w01_killd_384x256.png");
	tileset[GOALKILL4] = GetActionTileset("goal_w01_kille_384x256.png");

	return GetActionTileset("goal_w01_killa_384x256.png");
}