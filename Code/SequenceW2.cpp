#include "SequenceW2.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "Enemy_BirdBoss.h"

using namespace sf;
using namespace std;

BirdPreFightScene::BirdPreFightScene()
	:BasicBossScene(BasicBossScene::RUN)
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

	BirdPostFightScene *scene = new BirdPostFightScene;
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
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

BirdPostFightScene::BirdPostFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
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
	sess->cam.EaseOutOfManual(60);
	sess->TotalDissolveGates(Gate::BOSS);
	BasicBossScene::ReturnToGame();
}

void BirdPostFightScene::AddShots()
{
	AddShot("scenecam");
}

void BirdPostFightScene::AddPoints()
{
	//AddPoint("kinstand0");
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
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				sess->adventureHUD->Hide(fadeFrames);
				//player->Wait();
				sess->cam.SetManual(true);
				MainMenu *mm = sess->mainMenu;
				sess->CrossFade(10, 0, 60, Color::White);
			}
		}
	case WAIT:
		//EntranceUpdate();
		break;
	case BIRDCONV:
		ConvUpdate();
		break;
	case BIRDLEAVE:
		break;
	}
}

BirdCrawlerAllianceScene::BirdCrawlerAllianceScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void BirdCrawlerAllianceScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CRAWLERARRIVE] = 60;
	stateLength[CONV] = -1;
	stateLength[FADEOUT] = 60;
}

void BirdCrawlerAllianceScene::ReturnToGame()
{
	//owner->SetPlayerInputOn(true);
	//owner->adventureHUD->Show(60);
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
		if (state == FADE)
		{
			if (frame == 0)
			{
				sess->adventureHUD->Hide();
				sess->cam.SetManual(true);
				MainMenu *mm = sess->mainMenu;
				sess->Fade(true, 60, Color::Black);
				SetCameraShot("alliancecam");
			}
		}
	case WAIT:
		break;
	case CRAWLERARRIVE:
	{
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