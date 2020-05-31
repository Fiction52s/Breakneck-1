#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DEATH_Start()
{

}

void Actor::DEATH_End()
{
	frame = 0;
	dead = true;
}

void Actor::DEATH_Change()
{

}

void Actor::DEATH_Update()
{
	velocity.x = 0;
	velocity.y = 0;
	groundSpeed = 0;
}

void Actor::DEATH_UpdateSprite()
{
}