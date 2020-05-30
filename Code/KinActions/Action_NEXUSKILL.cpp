#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::NEXUSKILL_Start()
{
	SetExpr(Expr_NEUTRAL);
	velocity = V2d(0, 0);
	groundSpeed = 0;
	grindSpeed = 0;
}

void Actor::NEXUSKILL_End()
{
	SetAction(SEQ_FLOAT_TO_NEXUS_OPENING);
	frame = 0;
}

void Actor::NEXUSKILL_Change()
{
}

void Actor::NEXUSKILL_Update()
{
}

void Actor::NEXUSKILL_UpdateSprite()
{
	GOALKILL_UpdateSprite();
}