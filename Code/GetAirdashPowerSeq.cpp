#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "VectorMath.h"
#include "Camera.h"
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
//#include "EditSession.h"
#include "Zone.h"
#include "Flow.h"
#include "Boss.h"
#include "PowerOrbs.h"
#include "SequenceW1.h"
#include "SoundManager.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "PauseMenu.h"
#include "Movement.h"
#include "ScoreDisplay.h"
#include "Enemy_CrawlerQueen.h"
#include "MusicSelector.h"
#include "MusicPlayer.h"
#include "HUD.h"
#include "AdventureManager.h"

using namespace sf;
using namespace std;

#define TIMESTEP (1.0 / 60.0)

GetAirdashPowerSeq::GetAirdashPowerSeq()
{
	sess = Session::GetSession();
	assert(mov.openFromFile("Resources/Movie/kin_meditate_01.ogv"));
	mov.fit(sf::FloatRect(0, 0, 1920, 1080));

	sceneMusic = sess->mainMenu->musicManager->songMap["w1_26_Edge"];
	sceneMusic->Load();

	stateLength[KIN_KNEELING] = 60;
	stateLength[START_MEDITATE] = 60;
	stateLength[FADE_BACKGROUND] = 60;
	stateLength[EXPEL_ENERGY] = 60;
	stateLength[WAITAFTEREXPEL] = 60;
	stateLength[MASKOFF] = 120;
	stateLength[PLAYMOVIE] = 1000000;
	stateLength[FADE_BACK] = 90;

	darkRect.setFillColor(Color::Black);
	darkRect.setSize(Vector2f(1920, 1080));
	darkRect.setPosition(0, 0);

	Reset();
}

bool GetAirdashPowerSeq::Update()
{
	Actor *player = sess->GetPlayer(0);

	if (seqData.frame == stateLength[seqData.state] && seqData.state != END)
	{
		int s = seqData.state;
		s++;
		seqData.state = (State)s;
		seqData.frame = 0;

		if (state == END)
		{
		}
	}

	if (state == END)
	{
		sess->mainMenu->musicPlayer->TransitionMusic(sess->originalMusic, 60);
		return false;
	}



	switch (state)
	{
	case KIN_KNEELING:
		if (seqData.frame == 0)
		{
			sess->mainMenu->musicPlayer->TransitionMusic(sceneMusic, 60);
			//owner->cam.SetManual(true);
			sess->cam.Ease(Vector2f(player->position.x, player->position.y - 68), .75, 60, CubicBezier());
			//player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect( 0 ));
			//player->dirtyAuraSprite.setOrigin(player->dirtyAuraSprite.getLocalBounds().width / 2,
			//	player->dirtyAuraSprite.getLocalBounds().height / 2);

			//player->SetDirtyAura(true);
			
			player->SeqKneel();
		}
		
		break;
	case START_MEDITATE:
		if (seqData.frame == 0)
		{
			player->SeqMeditateMaskOn();
		}
		break;
	case FADE_BACKGROUND:
		if (seqData.frame == 0)
		{
			sess->Fade(false, 60, Color::Black, true);
		}
		else if (seqData.frame == stateLength[FADE_BACKGROUND]-1)
		{
			sess->SetGameSessionState(GameSession::SEQUENCE);
		}
		break;
	case EXPEL_ENERGY:
	{
		if (seqData.frame == 0)
		{
			sess->ClearFade();
		}

			int f = 60 - 3 * 10;
			int ff = seqData.frame - f;
			if (ff >= 0 && ff < 10 * 3)
			//if( frame < 10 * 3 )
			{
			//	player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect(ff / 3 + 15));
			}
			else
			{
			//	player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect((frame % (15 * 3) / 3)));
			}


		break;
	}
	case WAITAFTEREXPEL:
	{
		if (seqData.frame == 0)
		{
		//	player->SetDirtyAura(false);
		}
		break;
	}
	case MASKOFF:
		if (seqData.frame == 0)
		{
			player->SeqMaskOffMeditate();
		}
		break;

	case PLAYMOVIE:
	{

		sfe::Status movStatus = mov.getStatus();
		if (seqData.frame == 0)
		{
			mov.setPlayingOffset(sf::Time::Zero);
			mov.play();
		}
		else
		{
			mov.update();

			//cout << "mov: " << mov.getPlayingOffset().asSeconds() << endl;
			if (movStatus == sfe::Status::End || movStatus == sfe::Status::Stopped)
			{
				seqData.frame = stateLength[PLAYMOVIE] - 1;

				sess->SetGameSessionState(GameSession::RUN);
				sess->Fade(true, 60, Color::Black, true);
				sess->ShowHUD(60);
				sess->mainMenu->adventureManager->currSaveFile
					->UnlockUpgrade(Actor::UpgradeType::UPGRADE_POWER_AIRDASH);
			}
		}

		break;
	}
	
	case FADE_BACK:
		if (seqData.frame == 0)
		{
			
		}
		else if (seqData.frame == 60)
		{
			player->SeqGetAirdash();
			
		}
		else if (seqData.frame == stateLength[FADE_BACK] - 1)
		{
			cout << "set easting out of manual" << endl;
			sess->cam.EaseOutOfManual(120);
		}
		
		
		break;
	}

	if (state != EXPEL_ENERGY)
	{
	//	player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect((frame % (15 * 3) / 3)));
	}
	
	//player->dirtyAuraSprite.setPosition(Vector2f(player->sprite->getPosition().x, 
	//	player->sprite->getPosition().y - 32));

	if (sess->GetGameSessionState() == GameSession::SEQUENCE)
	{
		sess->totalGameFrames++;
		player->UpdatePrePhysics();
		player->UpdatePostPhysics();
	}
	

	

	


	++seqData.frame;

	return true;
}
void GetAirdashPowerSeq::Draw(sf::RenderTarget *target, EffectLayer layer)
{

	if (layer == EffectLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{		
	}

	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (sess->GetGameSessionState() == GameSession::SEQUENCE)
	{
		target->setView(sess->uiView);
		/*sf::View v = target->getView();
		target->setView(owner->uiView);*/

		if (state >= EXPEL_ENERGY)
		{
			//darkRect.setPosition(owner->cam.pos);
			target->draw(darkRect);
		}

		if (state == PLAYMOVIE)
		{
			target->draw(mov);
		}
		

		//target->setView(v);



		target->setView(sess->view);


		sess->GetPlayer(0)->Draw(target);
	}
}
void GetAirdashPowerSeq::Reset()
{
	seqData.state = KIN_KNEELING;
	seqData.frame = 0;
}