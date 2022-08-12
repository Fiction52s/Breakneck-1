#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

void Actor::EXITBOOST_Start()
{
}

void Actor::EXITBOOST_End()
{
}

void Actor::EXITBOOST_Change()
{
}

void Actor::EXITBOOST_Update()
{
}

void Actor::EXITBOOST_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);
	//cout << "f: " << frame / 2 << endl;
	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);//position.x, position.y );
	sprite->setRotation(0);

	int aF = frame / 2 - 55;
	if (aF < 61 && aF >= 0)
	{
		exitAuraShader.SetSkin(currSkinIndex);
		showExitAura = true;
		exitAuraSprite.setTextureRect(ts_exitAura->GetSubRect(aF));
		exitAuraSprite.setOrigin(exitAuraSprite.getLocalBounds().width / 2,
			exitAuraSprite.getLocalBounds().height / 2);
		exitAuraSprite.setPosition(sprite->getPosition());
	}

	if (frame == 59 * 2)
	{
		//wtf are independent effects LOL
		owner->mainMenu->ActivateIndEffect(
			owner->mainMenu->tilesetManager.GetTileset("Kin/FX/exitenergy_0_512x512.png", 512, 512), V2d(960, 540), false, 0, 4, 2, true, 2);
	}
	else if (frame == 63 * 2)
	{
		owner->mainMenu->ActivateIndEffect(
			owner->mainMenu->tilesetManager.GetTileset("Kin/FX/exitenergy_1_512x512.png", 512, 512), V2d(960, 540), false, 0, 6, 2, true);
	}
}

void Actor::EXITBOOST_TransitionToAction(int a)
{

}

void Actor::EXITBOOST_TimeIndFrameInc()
{

}

void Actor::EXITBOOST_TimeDepFrameInc()
{

}

int Actor::EXITBOOST_GetActionLength()
{
	return 79 * 2;
}

Tileset * Actor::EXITBOOST_GetTileset()
{
	return GetActionTileset("exitboost_96x128.png");
}