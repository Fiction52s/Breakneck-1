#include "SequenceW2.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
//#include "Enemy_BirdBoss.h"
#include "Enemy_Bird.h"
#include "Enemy_SequenceCrawler.h"
#include "Enemy_SequenceBird.h"

using namespace sf;
using namespace std;

BirdPreFightScene::BirdPreFightScene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
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

	bird = (Bird*)sess->GetEnemy(EnemyType::EN_BIRDBOSS);

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
	AddStartAndStopPoints();
}

void BirdPreFightScene::AddGroups()
{
	AddGroup("pre_fight", "W2/w2_bird_fight_pre");
	SetConvGroup("pre_fight");
}

void BirdPreFightScene::AddEnemies()
{
	/*bird = new BirdBoss(owner, Vector2i(shots["fightcam"]->centerPos));
	AddEnemy("bird", bird);*/





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
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	EaseShot("fightcam", 60);
}

void BirdPreFightScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		if (frame == 0)
		{
			sess->AddEnemy(bird);
			bird->SeqWait();
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
			bird->StartFight();
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

BirdPostFightScene::BirdPostFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqBird = NULL;
	
}

void BirdPostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = explosionFadeFrames;
	stateLength[FADE_IN] = fadeFrames;
	stateLength[WAIT] = 30;
	stateLength[BIRDCONV] = -1;
	stateLength[BIRDLEAVE] = 120;

	seqBird = (SequenceBird*)sess->GetEnemy(EnemyType::EN_SEQUENCEBIRD);
}

void BirdPostFightScene::ReturnToGame()
{
	sess->cam.EaseOutOfManual(60);
	sess->RemoveEnemy(seqBird);
	//sess->TotalDissolveGates(Gate::BOSS);
	BasicBossScene::ReturnToGame();
}

void BirdPostFightScene::AddShots()
{
	AddShot("scenecam");
}

void BirdPostFightScene::AddPoints()
{
	AddStandPoint();
	//AddStopPoint();
	//AddStartAndStopPoints();

	//AddPoint("kinstand0");
}

void BirdPostFightScene::AddFlashes()
{
	AddFlashedImage("birdcut", sess->GetTileset("Story/PostCrawlerFight1/Crawler_Slash_01a.png"),
		0, 30, 60, 30, Vector2f(960, 540));
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
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (frame == 0)
		{
			StartBasicNewMapKillFade();
		}
		break;
	case CUT_IMAGE:
	{
		if (frame == 0)
		{
			Flash("birdcut");
		}

		if (IsFlashDone("birdcut"))
		{
			EndCurrState();
		}
		break;
	}
	case FADE_IN:
	{
		if (frame == 0)
		{
			EndBasicNewMapKillFade();

			
			
			SetPlayerStandPoint("kinstand0", true);
			SetCameraShot("scenecam");

			seqBird->Reset();
			sess->AddEnemy(seqBird);
			seqBird->facingRight = false;
		}
		break;
	}
	case WAIT:
		/*if (frame == 0)
		{
			sess->TotalDissolveGates(Gate::BOSS);
		}*/
		//EntranceUpdate();
		break;
	case BIRDCONV:
		ConvUpdate();
		break;
	case BIRDLEAVE:
		if (frame == 0)
		{
			seqBird->Fly(seqBird->GetPosition() + V2d(1000, -1000));
		}
		break;
	}
}

BirdCrawlerAllianceScene::BirdCrawlerAllianceScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqCrawler = NULL;
	seqBird = NULL;
}

void BirdCrawlerAllianceScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CRAWLERARRIVE] = 60;
	stateLength[CONV] = -1;
	stateLength[FADEOUT] = 60;

	seqCrawler = (SequenceCrawler*)sess->GetEnemy(EnemyType::EN_SEQUENCECRAWLER);
	seqBird = (SequenceBird*)sess->GetEnemy(EnemyType::EN_SEQUENCEBIRD);
}

void BirdCrawlerAllianceScene::ReturnToGame()
{
	//sess->SetPlayerInputOn(true);
	//sess->hud->Show();
	//sess->cam.SetManual(false);
	//sess->Fade(true, 60, Color::Black);
	sess->EndLevelNoScene();
	
	//owner->cam.EaseOutOfManual(60);
	//owner->cam.EaseOutOfManual(60);
	//owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
	//BasicBossScene::ReturnToGame();
}

void BirdCrawlerAllianceScene::AddShots()
{
	AddShot("alliancecam");
}

void BirdCrawlerAllianceScene::AddPoints()
{

}

void BirdCrawlerAllianceScene::AddFlashes()
{

}

void BirdCrawlerAllianceScene::AddEnemies()
{

}

void BirdCrawlerAllianceScene::AddGroups()
{
	AddGroup("conv", "W2/w2_bird_crawler");
	SetConvGroup("conv");
}

void BirdCrawlerAllianceScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (frame == 0)
		{
			seqBird->Reset();
			sess->AddEnemy(seqBird);
			seqBird->facingRight = false;
			seqBird->Breathe();

			sess->hud->Hide();
			sess->cam.SetManual(true);
			MainMenu *mm = sess->mainMenu;
			sess->Fade(true, 60, Color::Black);
			SetCameraShot("alliancecam");
		}
		break;
	case WAIT:
		break;
	case CRAWLERARRIVE:
	{
		if (frame == 0)
		{
			seqCrawler->Reset();
			sess->AddEnemy(seqCrawler);
			seqCrawler->Underground();
			//seqCrawler->facingRight = false;
			//seqCrawler->Init();
			seqCrawler->DigOut();
		}
		
		break;
	}
	case CONV:
	{
		ConvUpdate();
		break;
	}
	case FADEOUT:
	{
		if (frame == 0)
		{
			sess->Fade(false, 60, Color::Black);
		}

		if (IsLastFrame())
		{
			//owner->goalDestroyed = true;
		}
		break;
	}
	
	}
}