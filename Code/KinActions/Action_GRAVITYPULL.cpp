#include "Actor.h"
#include <assert.h>
#include "KinUpgrades.h"

using namespace sf;
using namespace std;

void Actor::GRAVITY_PULL_Start()
{
	JUMP_Start();
}

void Actor::GRAVITY_PULL_End()
{
}

void Actor::GRAVITY_PULL_Change()
{
	JUMP_Change();
}

void Actor::GRAVITY_PULL_Update()
{
	if (gravityPullFrame == -1)
	{
		int f = frame;
		SetAction(JUMP);
		frame = f;
	}

	JUMP_Update();
}

void Actor::GRAVITY_PULL_UpdateSprite()
{
	sf::IntRect ir;
	int tFrame = 0;

	//tFrame = GetJumpFrame();

	if (velocity.y < -15)
	{
		tFrame = 3;
	}
	else if (velocity.y < -6)
	{
		tFrame = 2;
	}
	else if (velocity.y < 2)
	{
		tFrame = 1;
	}
	else if (velocity.y < 7)
	{
		tFrame = 0;
	}
	else
	{
		tFrame = 0;
	}

	SetSpriteTexture(GRAVITY_PULL);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(tFrame, r);

	sprite->setRotation(0);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);


	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::GRAVITY_PULL_TransitionToAction(int a)
{

}

void Actor::GRAVITY_PULL_TimeIndFrameInc()
{

}

void Actor::GRAVITY_PULL_TimeDepFrameInc()
{

}

int Actor::GRAVITY_PULL_GetActionLength()
{
	return JUMP_GetActionLength();
}

const char * Actor::GRAVITY_PULL_GetTilesetName()
{
	return "grav_pull_64x96.png";
}