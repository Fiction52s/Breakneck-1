#include "HUD.h"
#include "GameSession.h"

using namespace sf;
using namespace std;


RaceFightHUD::RaceFightHUD(GameSession* p_owner)
	:owner( p_owner )
{
	Tileset *scoreTS = owner->GetTileset("number_score_80x80.png", 80, 80);
	scoreRed = new ImageText(2, scoreTS);
	scoreBlue = new ImageText(2, scoreTS);
	ts_mask = owner->GetTileset("multi_mask_128x128.png", 128, 128);
	mask.setTexture(*ts_mask->texture);
	mask.setTextureRect(ts_mask->GetSubRect(0));
	mask.setOrigin(mask.getLocalBounds().width / 2, 0);// mask.getLocalBounds().height / 2);

	Vector2f maskPos(960, 20);

	scoreRed->SetNumber(22);
	scoreRed->topRight = Vector2f(maskPos.x - 200, maskPos.y);
	scoreRed->UpdateSprite();
	
	scoreBlue->SetNumber(55);
	scoreBlue->topRight = Vector2f(maskPos.x + 200, maskPos.y);
	scoreBlue->UpdateSprite();

	mask.setPosition(maskPos);
}

void RaceFightHUD::UpdateScoreRed( int newScore )
{
	scoreRed->SetNumber(newScore);
	scoreRed->UpdateSprite();
}

void RaceFightHUD::UpdateScoreBlue( int newScore )
{
	scoreBlue->SetNumber(newScore);
	scoreBlue->UpdateSprite();
}

void RaceFightHUD::Draw( RenderTarget *target )
{
	target->draw(mask);
	scoreRed->Draw(target);
	scoreBlue->Draw(target);
}