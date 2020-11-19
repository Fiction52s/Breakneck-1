#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::TELEPORTACROSSTERRAIN_Start()
{
}

void Actor::TELEPORTACROSSTERRAIN_End()
{
	frame = 0;
}

void Actor::TELEPORTACROSSTERRAIN_Change()
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

void Actor::TELEPORTACROSSTERRAIN_Update()
{
	velocity = springVel + springExtra;
}

void Actor::TELEPORTACROSSTERRAIN_UpdateSprite()
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

void Actor::TELEPORTACROSSTERRAIN_TransitionToAction(int a)
{

}

void Actor::TELEPORTACROSSTERRAIN_TimeIndFrameInc()
{

}

void Actor::TELEPORTACROSSTERRAIN_TimeDepFrameInc()
{

}

int Actor::TELEPORTACROSSTERRAIN_GetActionLength()
{
	return 8;
}

Tileset * Actor::TELEPORTACROSSTERRAIN_GetTileset()
{
	return SPRINGSTUN_GetTileset();
}