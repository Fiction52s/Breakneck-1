#include "Actor.h"
#include "GameSession.h"
#include "Nexus.h"

using namespace sf;
using namespace std;

void Actor::SEQ_FADE_INTO_NEXUS_Start()
{
}

void Actor::SEQ_FADE_INTO_NEXUS_End()
{
	frame = actionLength[SEQ_FADE_INTO_NEXUS] - 1;
	owner->SetActiveSequence(owner->nexus->insideSeq);
}

void Actor::SEQ_FADE_INTO_NEXUS_Change()
{
}

void Actor::SEQ_FADE_INTO_NEXUS_Update()
{
}

void Actor::SEQ_FADE_INTO_NEXUS_UpdateSprite()
{
	SetSpriteTexture(action);
	SetSpriteTile(frame / 10 + 3, facingRight);
	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2 + 24);
	sprite->setPosition(Vector2f(position));
	//sprite->setPosition(owner->goalNodePos.x, owner->goalNodePos.y - 24.f);//- 24.f );
	//sprite->setPosition(Vector2f(position));
	sprite->setRotation(0);
}

void Actor::SEQ_FADE_INTO_NEXUS_TransitionToAction(int a)
{

}

void Actor::SEQ_FADE_INTO_NEXUS_TimeIndFrameInc()
{

}

void Actor::SEQ_FADE_INTO_NEXUS_TimeDepFrameInc()
{

}

int Actor::SEQ_FADE_INTO_NEXUS_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_FADE_INTO_NEXUS_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}