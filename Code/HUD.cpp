#include "HUD.h"
#include "GameSession.h"
#include "PowerOrbs.h"
#include "Keymarker.h"
#include "Minimap.h"
#include "Momentumbar.h"
#include "PowerOrbs.h"
#include "SaveFile.h"
#include "Actor.h"

using namespace sf;
using namespace std;

RaceFightHUD::RaceFightHUD(GameSession::RaceFight* rf)
	:raceFight( rf )
{
	owner = rf->owner;

	Tileset *ts_scoreRed = owner->GetTileset("HUD/number_score_red_64x64.png", 64, 64);
	Tileset *ts_scoreBlue = owner->GetTileset("HUD/number_score_blue_64x64.png", 64, 64);
	scoreRed = new ImageText(2,ts_scoreRed );
	scoreBlue = new ImageText(2,ts_scoreBlue );
	ts_mask = owner->GetTileset("HUD/multi_mask_160x160.png", 160, 160);

	ts_smallMask = owner->GetTileset("HUD/multi_mask_race_80x80.png", 80, 80);

	ts_ready = owner->GetTileset("HUD/ready_320x128.png", 320, 128);

	ts_fx_gainPointBlue = owner->GetTileset("HUD/multi_mask_blue_fx_256x256.png", 256, 256);
	ts_fx_gainPointRed = owner->GetTileset("HUD/multi_mask_red_fx_256x256.png", 256, 256);

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
	miniHidePos = Vector2f(-500, miniShowPos.y);

	kinMask = owner->GetPlayer(0)->kinMask;
	



	kinMaskShowPos = kinMask->GetTopLeft();
	kinMaskHidePos = Vector2f(-500, kinMaskShowPos.y);

	keyMarker = owner->keyMarker;

	keyMarkerShowPos = keyMarker->GetPosition();
	keyMarkerHidePos = Vector2f(1920 + 200, keyMarkerShowPos.y);
	
	/*momentumShowPos = momentumBar->GetTopLeft();
	momentumHidePos = Vector2f(-200, momentumShowPos.y);*/

	Reset();
}

void AdventureHUD::Hide(int frames)
{
	if (state == SHOWN)
	{
		processFrames = frames;
		state = EXITING;
		frame = 0;
	}
}

void AdventureHUD::Show(int frames)
{
	if (state == HIDDEN)
	{
		processFrames = frames;
		state = ENTERING;
		frame = 0;
	}
}

bool AdventureHUD::IsHidden()
{
	return state == HIDDEN;
}

bool AdventureHUD::IsShown()
{
	return state == SHOWN;
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
			kinMask->SetTopLeft(kinMaskShowPos);
			keyMarker->SetPosition(keyMarkerShowPos);
			//momentumBar->SetTopLeft(momentumShowPos);
		}
		else
		{
			double prog = frame/(double)processFrames;
			float a = showBez.GetValue(prog);	
			Vector2f center = miniHidePos * (1.f - a) + a * miniShowPos;
			mini->SetCenter(center);
			Vector2f topLeft = kinMaskHidePos * (1.f - a) + a * kinMaskShowPos;
			kinMask->SetTopLeft(topLeft);
			Vector2f neededCenter = keyMarkerHidePos * (1.f - a) + a * keyMarkerShowPos;
			keyMarker->SetPosition(neededCenter);
			//momentumBar->SetTopLeft(topLeft);
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
			Vector2f topLeft = kinMaskShowPos * (1.f - a) + a * kinMaskHidePos;
			kinMask->SetTopLeft(topLeft);
			Vector2f neededCenter = keyMarkerShowPos * (1.f - a) + a * keyMarkerHidePos;
			keyMarker->SetPosition(neededCenter);
			//momentumBar->SetTopLeft(topLeft);
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
	kinMask->SetTopLeft(kinMaskShowPos);
	keyMarker->SetPosition(keyMarkerShowPos);
	//momentumBar->SetTopLeft(momentumShowPos);
}

void AdventureHUD::Draw(RenderTarget *target)
{
	if (state != HIDDEN)
	{
		//Actor *p0 = owner->GetPlayer(0);
		kinMask->Draw(target);
		
		

		owner->mini->Draw(target);
		//target->draw(owner->minimapSprite, &owner->minimapShader);

		//kinRing->Draw(target);
		//target->draw(owner->kinMinimapIcon);
		/*if (owner->powerRing != NULL)
		{
			owner->powerRing->Draw(target);
			owner->despOrb->Draw(target);
		}*/
		owner->keyMarker->Draw(target);
		
	}
}

KinMask::KinMask( Actor *a )
{
	actor = a;
	owner = actor->owner;

	ts_face = owner->GetTileset("HUD/kinportrait_320x288.png", 320, 288);
	face.setTexture(*ts_face->texture);
	face.setTextureRect(ts_face->GetSubRect(0));

	faceBG.setTexture(*ts_face->texture);
	faceBG.setTextureRect(ts_face->GetSubRect(0));

	momentumBar = new MomentumBar(owner);

	ts_newShardMarker = owner->GetTileset("HUD/shardexlcaim_48x48.png", 48, 48);
	shardMarker.setTexture(*ts_newShardMarker->texture);
	shardMarker.setTextureRect(ts_newShardMarker->GetSubRect(0));
	shardMarker.setOrigin(shardMarker.getLocalBounds().width / 2, shardMarker.getLocalBounds().height / 2);
	shardMarker.setPosition(286, 202);

	kinRing = actor->kinRing;

	SetTopLeft(Vector2f(0, 0));

	Reset();
}

KinMask::~KinMask()
{
	delete momentumBar;
	delete kinRing;
}

void KinMask::Reset()
{
	SetExpr(Expr_NEUTRAL);
	Update(0,false);
	frame = 0;

	faceBG.setTextureRect(ts_face->GetSubRect(0));

	kinRing->Reset();
}

void KinMask::Draw(RenderTarget *target)
{
	if (actor->desperationMode )
	{
		target->draw(faceBG, &(actor->despFaceShader));
	}
	else
	{
		target->draw(faceBG);
	}
	target->draw(face);

	momentumBar->SetMomentumInfo(actor->speedLevel, actor->GetSpeedBarPart());
	momentumBar->Draw(target);

	kinRing->Draw(target);

	//SaveFile *currProgress = owner->GetCurrentProgress();
	//if (currProgress != NULL)
	{
		//if (currProgress->HasNewShards())
		{
			target->draw(shardMarker);
		}
	}
}

void KinMask::SetExpr(KinMask::Expr ex)
{
	frame = 0;
	expr = ex;
}

void KinMask::Update( int speedLevel, bool desp )
{
	if (expr == Expr_DEATH )
	{
		int faceDeathAnimLength = 11;
		int an = 6;
		int f = frame / an;
		
		face.setTextureRect(ts_face->GetSubRect(11 + f));
		
		++frame;
	}
	else if (expr == Expr_DEATHYELL)
	{
		faceBG.setTextureRect(ts_face->GetSubRect(5));
		face.setTextureRect(ts_face->GetSubRect(11));
	}
	else
	{ 
		if (expr == Expr_NEUTRAL || expr == Expr_SPEED1 || expr == Expr_SPEED2 || expr == Expr_HURT)
		{
			switch (speedLevel)
			{
			case 0:
				expr = Expr_NEUTRAL;
				break;
			case 1:
				expr = Expr_SPEED1;
				break;
			case 2:
				expr = Expr_SPEED2;
				break;
			}

			if (desp)
			{
				expr = Expr_DESP;
			}

			if (actor->action == Actor::GROUNDHITSTUN || actor->action == Actor::AIRHITSTUN)
			{
				expr = Expr_HURT;
			}
		}

		face.setTextureRect(ts_face->GetSubRect(expr + 6));
		faceBG.setTextureRect(ts_face->GetSubRect(expr));
	}
	/*switch (expr)
	{
	case Expr_NEUTRAL:
		frame = 0;
		face.setTextureRect(ts_face->GetSubRect(expr + 6));
		break;
	Expr_HURT:
		frame = 0;
		break;
	Expr_SPEED1:
		frame = 0;
		break;
	Expr_SPEED2:
		frame = 0;
		break;
	Expr_DESP:
		frame = 0;
		break;
	Expr_DEATH:
		
		break;
	Expr_NONE:
		frame = 0;
		break;
	}*/

	++frame;
}

void KinMask::SetTopLeft(sf::Vector2f &pos)
{
	face.setPosition(pos);
	faceBG.setPosition(pos);
	momentumBar->SetTopLeft(pos + Vector2f(202, 117));
	kinRing->SetCenter(pos + Vector2f(80, 220));
}

sf::Vector2f KinMask::GetTopLeft()
{
	return face.getPosition();
}