#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DASHATTACK_Start()
{

}

void Actor::DASHATTACK_End()
{

}

void Actor::DASHATTACK_Change()
{

}

void Actor::DASHATTACK_Update()
{

}

void Actor::DASHATTACK_UpdateSprite()
{
}

void Actor::DASHATTACK_TransitionToAction(int a)
{

}

void Actor::DASHATTACK_TimeIndFrameInc()
{

}

void Actor::DASHATTACK_TimeDepFrameInc()
{

}

int Actor::DASHATTACK_GetActionLength()
{
	return 1;
}

Tileset * Actor::DASHATTACK_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}