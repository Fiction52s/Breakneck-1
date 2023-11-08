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
		SetAction(JUMP);
		frame = 1;
		velocity = waterEntranceVelocity;
	}
}

void Actor::TELEPORTACROSSTERRAIN_Update()
{
	velocity = normalize(waterEntranceVelocity) * 30.0;
}

void Actor::TELEPORTACROSSTERRAIN_UpdateSprite()
{
	SetSpriteTexture(JUMP);

	SetSpriteTile(1, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

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

const char * Actor::TELEPORTACROSSTERRAIN_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}