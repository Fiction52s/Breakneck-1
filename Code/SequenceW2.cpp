#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "poly2tri/poly2tri.h"
#include "VectorMath.h"
#include "Camera.h"
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
#include "Zone.h"
#include "PowerOrbs.h"
#include "SequenceW2.h"
#include "SoundManager.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "PauseMenu.h"
#include "Parallax.h"
#include "Movement.h"
#include "ScoreDisplay.h"
#include "HUD.h"
#include "ImageText.h"
#include "Enemy_BirdBoss.h"

using namespace sf;
using namespace std;

BirdPreFightScene::BirdPreFightScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::RUN)
{
	
}

void BirdPreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[BIRDWALK] = 120;
	stateLength[BIRDFALL] = 60;
	stateLength[BIRDCONV] = -1;

	BirdPostFightScene *scene = new BirdPostFightScene(owner);
	scene->Init();
	nextSeq = scene;


}

void BirdPreFightScene::AddShots()
{
	AddShot("birdstart");
	AddShot("birdstop");
	AddShot("birdland");
	AddShot("scenecam");
	AddShot("fightcam");
}

void BirdPreFightScene::AddPoints()
{
	AddPoint("kinstart");
	AddPoint("kinstop");
}

void BirdPreFightScene::AddGroups()
{
	AddGroup("pre_fight", "W2/w2_bird_fight_pre");
	SetConvGroup("pre_fight");
}

void BirdPreFightScene::AddEnemies()
{
	bird = new BirdBoss(owner, Vector2i(shots["fightcam"]->centerPos));
	AddEnemy("bird", bird);
	//PoiInfo *surface = points["crawlersurface"];
	//queen = new CrawlerQueen(owner, surface->edge, surface->edgeQuantity, false);
	//AddEnemy("queen", queen);
}

void BirdPreFightScene::AddFlashes()
{
	//AddFlashedImage("detailedgrab", owner->GetTileset("Story/Crawler_Dig_01_860x830.png", 860, 830),
	//	0, 30, 60, 30, Vector2f(1160, 540));

	//AddFlashedImage("crawlerface", owner->GetTileset("Story/Crawler_Dig_02_828x875.png", 828, 875),
	//	0, 30, 60, 30, Vector2f(1350, 325));

	//AddFlashedImage("kinface", owner->GetTileset("Story/Crawler_Dig_03_510x565.png", 510, 565),
	//	0, 30, 60, 30, Vector2f(625, 325));
}

void BirdPreFightScene::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	CameraShot *shot = shots["fightcam"];
	owner->cam.Ease(Vector2f(shot->centerPos), shot->zoom, 60, CubicBezier());

	//owner->SetActiveSequence(new BirdPostFightScene(owner));
}

void BirdPreFightScene::UpdateState()
{
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		if (frame == 0)
		{

		}
		EntranceUpdate();
		break;
	case BIRDWALK:
		if (frame == 0)
		{
			EaseShot("birdstart", 60);
		}
		else if (frame == 60)
		{
			EaseShot("birdstop", 60);
		}
		break;
	case BIRDFALL:
		if (frame == 0)
		{
			EaseShot("birdland", 60);
		}
		else if (IsLastFrame())
		{
			EaseShot("scenecam", 60);
		}
		break;
	case BIRDCONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			owner->ReverseDissolveGates(Gate::CRAWLER_UNLOCK);
			//owner->ReformGates(Gate::CRAWLER_UNLOCK);
			//owner->currentZone->ReformAllGates();
		}
		break;
	}
}

void BirdPreFightScene::StartRunning()
{
	int x = 56;
	//if (zone != NULL)
	//bird->zone->action = Zone::OPEN;
	//owner->OpenGates(Gate::CRAWLER_UNLOCK);
}

BirdPostFightScene::BirdPostFightScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::APPEAR)
{
}

void BirdPostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[BIRDCONV] = 1000000;
	stateLength[BIRDLEAVE] = 30;
}

void BirdPostFightScene::ReturnToGame()
{
	owner->cam.EaseOutOfManual(60);
	BasicBossScene::ReturnToGame();
}

void BirdPostFightScene::AddShots()
{
	AddShot("scenecam");
}

void BirdPostFightScene::AddPoints()
{

}

void BirdPostFightScene::AddFlashes()
{

}

void BirdPostFightScene::AddEnemies()
{

}

void BirdPostFightScene::AddGroups()
{
	AddGroup("post_fight", "W2/w2_bird_fight_post");
	SetConvGroup("post_fight");
}

void BirdPostFightScene::UpdateState()
{
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				owner->adventureHUD->Hide(fadeFrames);
				//player->Wait();
				owner->cam.SetManual(true);
				MainMenu *mm = owner->mainMenu;
				owner->CrossFade(10, 0, 60, Color::White);
			}
		}
	case WAIT:
		//EntranceUpdate();
		break;
	case BIRDCONV:
		ConvUpdate();
		break;
	case BIRDLEAVE:
		if (frame == stateLength[BIRDLEAVE] - 1)
		{
			owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
			//owner->SoftenGates(Gate::CRAWLER_UNLOCK);
		}
		
		break;
	}
}