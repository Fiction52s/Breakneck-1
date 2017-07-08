#include "HUD.h"
#include "GameSession.h"

using namespace sf;
using namespace std;

RaceFightHUD::RaceFightHUD(GameSession::RaceFight* rf)
	:raceFight( rf )
{
	owner = rf->owner;
	//Tileset *scoreTS = owner->GetTileset("number_score_80x80.png", 80, 80);
	scoreRed = new ImageText(2, owner->GetTileset( "number_score_red_64x64.png", 64, 64 ));
	scoreBlue = new ImageText(2, owner->GetTileset("number_score_blue_64x64.png", 64, 64));
	ts_mask = owner->GetTileset("multi_mask_160x160.png", 160, 160);

	ts_smallMask = owner->GetTileset("multi_mask_race_80x80.png", 80, 80);

	ts_ready = owner->GetTileset("ready_320x128.png", 320, 128);

	ts_fx_gainPointBlue = owner->GetTileset("multi_mask_blue_fx_256x256.png", 256, 256);
	ts_fx_gainPointRed = owner->GetTileset("multi_mask_red_fx_256x256.png", 256, 256);

	mask.setTexture(*ts_mask->texture);
	mask.setTextureRect(ts_mask->GetSubRect(0));
	mask.setOrigin(mask.getLocalBounds().width / 2, 0);// mask.getLocalBounds().height / 2);

	smallMask.setTexture(*ts_smallMask->texture);
	smallMask.setTextureRect(ts_smallMask->GetSubRect(0));
	smallMask.setOrigin(smallMask.getLocalBounds().width / 2, 0);

	Vector2f maskPos(960, 20);

	scoreRed->SetNumber(22);
	scoreRed->topRight = Vector2f(maskPos.x - 200, maskPos.y);
	scoreRed->UpdateSprite();
	
	scoreBlue->SetNumber(55);
	scoreBlue->topRight = Vector2f(maskPos.x + 200, maskPos.y);
	scoreBlue->UpdateSprite();

	mask.setPosition(maskPos);
}

void RaceFightHUD::UpdateScore(RaceFightHUD::PlayerColor pc, int newScore)
{
	switch( pc )
	{
	case BLUE:
		scoreBlue->SetNumber(newScore);
		scoreBlue->UpdateSprite();
		break;
	case RED:
		scoreRed->SetNumber(newScore);
		scoreRed->UpdateSprite();
		break;
	case GREEN:
		break;
	case PURPLE:
		break;
	}
}

void RaceFightHUD::UpdateScore()
{
	scoreBlue->SetNumber(raceFight->playerScore);
	scoreBlue->UpdateSprite();

	scoreRed->SetNumber(raceFight->player2Score);
	scoreRed->UpdateSprite();
}

void RaceFightHUD::Draw( RenderTarget *target )
{
	if (raceFight->raceWinnerIndex == -1)
	{
		target->draw(mask);
	}
	else
	{
		target->draw(smallMask);
	}
	
	scoreRed->Draw(target);
	scoreBlue->Draw(target);
}

void RaceFightHUD::SetRaceWinner(RaceFightHUD::PlayerColor pc)
{
	smallMask.setTextureRect(ts_smallMask->GetSubRect( (int)pc) );
}

void RaceFightHUD::ScorePoint(RaceFightHUD::PlayerColor pc)
{
	switch (pc)
	{
	case BLUE:
		owner->ActivateEffect(EffectLayer::UI_FRONT, ts_fx_gainPointBlue, Vector2<double>(960, 100),
			true, 0, 16, 1, true);
		break;
	case RED:
		owner->ActivateEffect(EffectLayer::UI_FRONT, ts_fx_gainPointRed, Vector2<double>(960, 100),
			true, 0, 16, 1, true);
		break;
	case GREEN:
		break;
	case PURPLE:
		break;
	}

}