#include "Actor.h"
#include "GameSession.h"
#include "HUD.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

void Actor::HIDDEN_Start()
{
}

void Actor::HIDDEN_End()
{
}

void Actor::HIDDEN_Change()
{

}

void Actor::HIDDEN_Update()
{
	frame = 0;
}

void Actor::HIDDEN_UpdateSprite()
{
}

void Actor::HIDDEN_TransitionToAction(int a)
{

}

void Actor::HIDDEN_TimeIndFrameInc()
{

}

void Actor::HIDDEN_TimeDepFrameInc()
{

}

int Actor::HIDDEN_GetActionLength()
{
	return 2;
}

Tileset * Actor::HIDDEN_GetTileset()
{
	return NULL;
}