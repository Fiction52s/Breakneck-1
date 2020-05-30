#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::AUTORUN_Start()
{

}

void Actor::AUTORUN_End()
{
	frame = 0;
}

void Actor::AUTORUN_Change()
{

}

void Actor::AUTORUN_Update()
{
	RunMovement();
}

void Actor::AUTORUN_UpdateSprite()
{
	UpdateRunSprite();
}