#include "HUD.h"
#include "GameSession.h"
#include "PowerOrbs.h"
#include "Keymarker.h"
#include "Minimap.h"


using namespace sf;
using namespace std;

RaceFightHUD::RaceFightHUD(GameSession::RaceFight* rf)
	:raceFight( rf )
{
	owner = rf->owner;
	//Tileset *scoreTS = owner->GetTileset("number_score_80x80.png", 80, 80);
	Tileset *ts_scoreRed = owner->GetTileset("number_score_red_64x64.png", 64, 64);
	Tileset *ts_scoreBlue = owner->GetTileset("number_score_blue_64x64.png", 64, 64);
	scoreRed = new ImageText(2,ts_scoreRed );
	scoreBlue = new ImageText(2,ts_scoreBlue );
	ts_mask = owner->GetTileset("multi_mask_160x160.png", 160, 160);

	ts_smallMask = owner->GetTileset("multi_mask_race_80x80.png", 80, 80);

	ts_ready = owner->GetTileset("ready_320x128.png", 320, 128);

	ts_fx_gainPointBlue = owner->GetTileset("multi_mask_blue_fx_256x256.png", 256, 256);
	ts_fx_gainPointRed = owner->GetTileset("multi_mask_red_fx_256x256.png", 256, 256);

	mask.setTexture(*ts_mask->texture);
	mask.setTextureRect(ts_mask->GetSubRect(0));
	mask.setOrigin(mask.getLocalBounds().width / 2, mask.getLocalBounds().height / 2);

	smallMask.setTexture(*ts_smallMask->texture);
	smallMask.setTextureRect(ts_smallMask->GetSubRect(0));
	smallMask.setOrigin(smallMask.getLocalBounds().width / 2, 0);

	Vector2f maskPos(960, 70);

	scoreRed->ShowZeroes(2);
	scoreRed->SetNumber(0);
	scoreRed->topRight = Vector2f(maskPos.x - 200 + ts_scoreRed->GetSubRect(0).width * 2, maskPos.y - 60);
	scoreRed->UpdateSprite();
	
	scoreBlue->ShowZeroes(2);
	scoreBlue->SetNumber(0);
	scoreBlue->topRight = Vector2f(maskPos.x + 200, maskPos.y - 60);
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
	switch (pc)
	{
	case BLUE:
		smallMask.setPosition(960-200, 40);
		break;
	case RED:
		smallMask.setPosition(960 + 200, 40);
		break;
	}
}

void RaceFightHUD::ScorePoint(RaceFightHUD::PlayerColor pc)
{
	Vector2<double> maskPos(mask.getPosition().x, mask.getPosition().y + 10);
	switch (pc)
	{
	case BLUE:
		owner->ActivateEffect(EffectLayer::UI_FRONT, ts_fx_gainPointBlue, maskPos,
			true, 0, 16, 1, true);
		break;
	case RED:
		owner->ActivateEffect(EffectLayer::UI_FRONT, ts_fx_gainPointRed, maskPos,
			true, 0, 16, 1, true);
		break;
	case GREEN:
		break;
	case PURPLE:
		break;
	}

}

AdventureHUD::AdventureHUD(GameSession *p_owner)
	:owner(p_owner)
{
	mini = owner->mini;

	miniShowPos = mini->minimapSprite.getPosition();
	miniHidePos = Vector2f(-300, miniShowPos.y);

	Reset();
}

void AdventureHUD::Hide(int frames)
{
	processFrames = frames;
	state = EXITING;
	frame = 0;
}

void AdventureHUD::Show(int frames)
{
	processFrames = frames;
	state = ENTERING;
	frame = 0;
}

bool AdventureHUD::IsHidden()
{
	return state == HIDDEN;
}

void AdventureHUD::Update()
{
	switch (state)
	{
	case SHOWN:
		frame = 0;
		break;
	case ENTERING:
		if (frame == processFrames)
		{
			state = SHOWN;
			frame = 0;
			mini->SetCenter(miniShowPos);
		}
		else
		{
			double prog = frame/(double)processFrames;
			float a = showBez.GetValue(prog);	
			Vector2f center = miniHidePos * (1.f - a) + a * miniShowPos;
			mini->SetCenter(center);
		}
		break;
	case EXITING:
		if (frame == processFrames)
		{
			state = HIDDEN;
			frame = 0;
			mini->SetCenter(miniHidePos);
		}
		else
		{
			double prog = frame / (double)processFrames;
			float a = showBez.GetValue(prog);
			Vector2f center = miniShowPos * (1.f - a) + a * miniHidePos;
			mini->SetCenter(center);
		}
		break;
	case HIDDEN:
		frame = 0;
		break;
	}

	++frame;
}


void AdventureHUD::Reset()
{
	show = true;
	state = SHOWN;
	frame = 0;

	mini->SetCenter(miniShowPos);
}

void AdventureHUD::Draw(RenderTarget *target)
{
	if (state != HIDDEN)
	{
		Actor *p0 = owner->GetPlayer(0);
		owner->momentumBar->SetMomentumInfo(p0->speedLevel, p0->GetSpeedBarPart());
		owner->momentumBar->Draw(target);

		owner->mini->Draw(target);
		//target->draw(owner->minimapSprite, &owner->minimapShader);

		//target->draw(owner->kinMinimapIcon);
		if (owner->powerRing != NULL)
		{
			owner->powerRing->Draw(target);
			owner->despOrb->Draw(target);
		}
		owner->keyMarker->Draw(target);
		
	}
}