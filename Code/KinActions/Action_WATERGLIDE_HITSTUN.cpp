#include "Actor.h"
#include "HUD.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

void Actor::WATERGLIDE_HITSTUN_Start()
{
	SetExpr(KinMask::Expr::Expr_HURT);
}

void Actor::WATERGLIDE_HITSTUN_End()
{

}

void Actor::WATERGLIDE_HITSTUN_Change()
{
	if (hitstunFrames == 0)
	{
		if (InWater(TerrainPolygon::WATER_GLIDE))
		{
			SetAction(WATERGLIDE);
		}
		else
		{
			SetAction(JUMP);
			frame = 1;
		}

		BasicAirAction();
		return;
	}

	frame = 0;
	if (hitstunFrames <= setHitstunFrames / 2)
	{
		//AirAttack();
	}
}

void Actor::WATERGLIDE_HITSTUN_Update()
{
	hitstunFrames--;
}

void Actor::WATERGLIDE_HITSTUN_UpdateSprite()
{
	if (frame == 0)
	{
		//playerHitSound.stop();
		//playerHitSound.play();
	}

	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::WATERGLIDE_HITSTUN_TransitionToAction(int a)
{
	if (kinMode == K_DESPERATION)
	{
		SetExpr(KinMask::Expr::Expr_DESP);
	}
	else
	{
		SetExpr(KinMask::Expr::Expr_NEUTRAL);
	}
}

void Actor::WATERGLIDE_HITSTUN_TimeIndFrameInc()
{

}

void Actor::WATERGLIDE_HITSTUN_TimeDepFrameInc()
{

}

int Actor::WATERGLIDE_HITSTUN_GetActionLength()
{
	return 1;
}

const char * Actor::WATERGLIDE_HITSTUN_GetTilesetName()
{
	return "hurt_64x64.png";
}