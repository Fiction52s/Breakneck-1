#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNTELEPORT_Start()
{
}

void Actor::SPRINGSTUNTELEPORT_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNTELEPORT_Change()
{
	if (springStunFrames == 0)
	{
		//oldTeleporter->ReceiveRecover();
		SetAction(JUMP);
		frame = 1;
		position = teleportSpringDest;
		velocity = teleportSpringVel;
	}
}

void Actor::SPRINGSTUNTELEPORT_Update()
{
	velocity = springVel + springExtra;
}

void Actor::SPRINGSTUNTELEPORT_UpdateSprite()
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

void Actor::SPRINGSTUNTELEPORT_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNTELEPORT_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNTELEPORT_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNTELEPORT_GetActionLength()
{
	return 1;
}

Tileset * Actor::SPRINGSTUNTELEPORT_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}