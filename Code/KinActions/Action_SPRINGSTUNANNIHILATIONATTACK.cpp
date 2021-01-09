#include "Actor.h"
#include "Enemy.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNANNIHILATIONATTACK_Start()
{

}

void Actor::SPRINGSTUNANNIHILATIONATTACK_End()
{
	velocity = springVel;
	SetAction(JUMP);
	frame = 1;
}

void Actor::SPRINGSTUNANNIHILATIONATTACK_Change()
{
	if ((!AttackButtonHeld() && frame > 10))
	{
		velocity = springVel;
		SetAction(JUMP);
		frame = 1;
	}

	BasicAirAttackAction();
}

void Actor::SPRINGSTUNANNIHILATIONATTACK_Update()
{
	SetCurrHitboxes(annihilationHitboxes, 0);
	velocity = V2d(0, 0);
}

void Actor::SPRINGSTUNANNIHILATIONATTACK_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::SPRINGSTUNANNIHILATIONATTACK_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNANNIHILATIONATTACK_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNANNIHILATIONATTACK_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNANNIHILATIONATTACK_GetActionLength()
{
	return 60;
}

Tileset * Actor::SPRINGSTUNANNIHILATIONATTACK_GetTileset()
{
	return JUMP_GetTileset();
}