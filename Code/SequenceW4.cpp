#include "SequenceW4.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "ScoreDisplay.h"

using namespace sf;
using namespace std;

CrawlerPreFight2Scene::CrawlerPreFight2Scene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::RUN)
{
}

void CrawlerPreFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[CONV] = -1;

	CrawlerPostFight2Scene *scene = new CrawlerPostFight2Scene(owner);
	scene->Init();
	nextSeq = scene;
}

void CrawlerPreFight2Scene::AddShots()
{
	AddShot("crawlercam");
	AddShot("fightcam");
}

void CrawlerPreFight2Scene::AddPoints()
{
	AddStartAndStopPoints();
}

void CrawlerPreFight2Scene::AddGroups()
{
	AddGroup("prefight", "W4/w4_crawler_kin");
	SetConvGroup("prefight");
}

void CrawlerPreFight2Scene::AddEnemies()
{
}

void CrawlerPreFight2Scene::AddFlashes()
{
	/*int togetherFrames = 5;

	AddFlashedImage("stare0", owner->GetTileset("Bosses/Coyote/Coy_09b.png", 1920, 1080),
		0, 30, 20, 30, Vector2f(960, 540));

	AddFlashedImage("stare1", owner->GetTileset("Bosses/Coyote/Coy_10b.png", 1920, 1080),
		0, 30, 30, 30, Vector2f(960, 540));

	AddFlashedImage("stare2", owner->GetTileset("Bosses/Coyote/Coy_11b.png", 1920, 1080),
		0, 30, 20, 30, Vector2f(960, 540));

	FlashGroup * group = AddFlashGroup("staregroup");
	AddFlashToGroup(group, "stare0", togetherFrames);
	AddFlashToGroup(group, "stare1", togetherFrames);
	AddFlashToGroup(group, "stare2", togetherFrames);*/
}

void CrawlerPreFight2Scene::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	EaseShot("fightcam", 60);
}

void CrawlerPreFight2Scene::SetEntranceShot()
{
	SetCameraShot("crawlercam");
}

void CrawlerPreFight2Scene::UpdateState()
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

	case CONV:
		ConvUpdate();

		if (IsLastFrame())
		{
			owner->ReverseDissolveGates(Gate::CRAWLER_UNLOCK);
		}
		break;
	}
}



CrawlerPostFight2Scene::CrawlerPostFight2Scene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::APPEAR)
{
}

void CrawlerPostFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[CRAWLERLEAVE] = 30;
}

void CrawlerPostFight2Scene::ReturnToGame()
{
	owner->cam.EaseOutOfManual(60);
	BasicBossScene::ReturnToGame();
}

void CrawlerPostFight2Scene::AddShots()
{
	AddShot("crawlercam");
}

void CrawlerPostFight2Scene::AddPoints()
{
	AddPoint("kinstand0");
}

void CrawlerPostFight2Scene::AddFlashes()
{

}

void CrawlerPostFight2Scene::AddEnemies()
{

}

void CrawlerPostFight2Scene::AddGroups()
{
	AddGroup("postfight", "W4/w4_crawler_kin_post");
	SetConvGroup("postfight");
}

void CrawlerPostFight2Scene::UpdateState()
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
				owner->cam.SetManual(true);
				MainMenu *mm = owner->mainMenu;
				owner->CrossFade(10, 0, 60, Color::White);
			}
			else if (frame == 10)
			{
				SetPlayerStandPoint("kinstand0", true);
			}
		}
	case WAIT:
		if (frame == 0)
		{
			owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
		}
		//EntranceUpdate();
		break;
	case CONV:
		ConvUpdate();
		break;
	case CRAWLERLEAVE:
		break;
	}
}



TigerPreFightScene::TigerPreFightScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::RUN)
{
	SetEntranceIndex(1);
}

void TigerPreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[TIGERENTRANCE] = 60;
	stateLength[TIGERCRAWLERCONV] = -1;
	stateLength[KINAPPROACH] = -1;
	stateLength[TIGERKINCONV] = -1;


	TigerPostFightScene *scene = new TigerPostFightScene(owner);
	scene->Init();
	nextSeq = scene;
}

void TigerPreFightScene::AddShots()
{
	AddShot("scenecam");
	AddShot("crawlerdeathcam");
}

void TigerPreFightScene::AddPoints()
{
	AddStartAndStopPoints();
	AddPoint("kinnext");
}

void TigerPreFightScene::AddGroups()
{
	AddGroup("prefight", "W4/w4_tiger_fight_pre");
	

	AddGroup("tigerkin", "W4/w4_tiger_fight_pre_kin");
	//SetConvGroup("tigerkin");
}

void TigerPreFightScene::AddEnemies()
{
}

void TigerPreFightScene::AddFlashes()
{
	/*int togetherFrames = 5;

	AddFlashedImage("stare0", owner->GetTileset("Bosses/Coyote/Coy_09b.png", 1920, 1080),
	0, 30, 20, 30, Vector2f(960, 540));

	AddFlashedImage("stare1", owner->GetTileset("Bosses/Coyote/Coy_10b.png", 1920, 1080),
	0, 30, 30, 30, Vector2f(960, 540));

	AddFlashedImage("stare2", owner->GetTileset("Bosses/Coyote/Coy_11b.png", 1920, 1080),
	0, 30, 20, 30, Vector2f(960, 540));

	FlashGroup * group = AddFlashGroup("staregroup");
	AddFlashToGroup(group, "stare0", togetherFrames);
	AddFlashToGroup(group, "stare1", togetherFrames);
	AddFlashToGroup(group, "stare2", togetherFrames);*/
}

void TigerPreFightScene::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	//EaseShot("fightcam", 60);
}

bool TigerPreFightScene::IsAutoRunState()
{
	return state == ENTRANCE || state == KINAPPROACH;
}

void TigerPreFightScene::UpdateState()
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
	case TIGERENTRANCE:
	{
		if (frame == 0)
		{
			owner->ReverseDissolveGates(Gate::CRAWLER_UNLOCK);
			//owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
			EaseShot("crawlerdeathcam", 60);
		}
		break;
	}
	case TIGERCRAWLERCONV:
	{
		if (frame == 0)
		{
			SetConvGroup("prefight");
		}
		ConvUpdate();

		if (IsLastFrame())
		{
			owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
			EaseShot("scenecam", 60);
			//owner->ReverseDissolveGates(Gate::CRAWLER_UNLOCK);
		}
		break;
	}
	case KINAPPROACH:
	{
		if (frame == 0)
		{
			StartEntranceRun(true, 10.0, "kinstop1", "kinnext");
		}
		break;
	}
	case TIGERKINCONV:
	{
		if (frame == 0)
		{
			SetConvGroup("tigerkin");
		}
		ConvUpdate();

		if (IsLastFrame())
		{
			owner->ReverseDissolveGates(Gate::CRAWLER_UNLOCK);
		}
		break;
	}
	}
}



TigerPostFightScene::TigerPostFightScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::APPEAR)
{
}

void TigerPostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[BIRDRESCUETIGER] = 60;
}

void TigerPostFightScene::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);
	player->EndLevelWithoutGoal();
	owner->SetPlayerInputOn(true);
}

void TigerPostFightScene::AddShots()
{
	AddShot("birdrescuecam");
}

void TigerPostFightScene::AddPoints()
{
	AddPoint("kinstand1");
}

void TigerPostFightScene::AddFlashes()
{

}

void TigerPostFightScene::AddEnemies()
{

}

void TigerPostFightScene::AddGroups()
{
	AddGroup("postfight", "W4/w4_tiger_Fight_post");
	SetConvGroup("postfight");
}

void TigerPostFightScene::UpdateState()
{
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				owner->cam.SetManual(true);
				MainMenu *mm = owner->mainMenu;
				owner->CrossFade(10, 0, 60, Color::White);	
			}
			else if (frame == 10)
			{
				owner->adventureHUD->Hide();
				SetPlayerStandPoint("kinstand1", true);
				SetCameraShot("birdrescuecam");
			}
		}
	case WAIT:
		if (frame == 0)
		{
			owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
		}
		//EntranceUpdate();
		break;
	case CONV:
		ConvUpdate();
		break;
	case BIRDRESCUETIGER:
		break;
	}
}



BirdTigerAllianceScene::BirdTigerAllianceScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::APPEAR)
{
}

void BirdTigerAllianceScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[TIGERSTORY] = 60;
	stateLength[FADEOUT] = 60;
}

void BirdTigerAllianceScene::ReturnToGame()
{
}

void BirdTigerAllianceScene::AddShots()
{
	AddShot("alliancecam");
}

void BirdTigerAllianceScene::AddPoints()
{

}

void BirdTigerAllianceScene::AddFlashes()
{

}

void BirdTigerAllianceScene::AddEnemies()
{

}


void BirdTigerAllianceScene::AddGroups()
{
	AddGroup("conv", "W4/w4_bird_tiger");
	SetConvGroup("conv");
}

void BirdTigerAllianceScene::UpdateState()
{
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				owner->adventureHUD->Hide();
				owner->cam.SetManual(true);
				MainMenu *mm = owner->mainMenu;
				owner->Fade(true, 60, Color::Black);
				SetCameraShot("alliancecam");
			}
		}
	case WAIT:
		break;
	case CONV:
	{
		ConvUpdate();
		break;
	}
	case TIGERSTORY:
	{
		break;
	}
	case FADEOUT:
	{
		if (frame == 0)
		{
			owner->Fade(false, 60, Color::Black);
		}

		if (IsLastFrame())
		{
			owner->goalDestroyed = true;
		}
		break;
	}

	}
}




BirdVSTigerScene::BirdVSTigerScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::APPEAR)
{
}

void BirdVSTigerScene::StartRunning()
{

	//owner->state = GameSession::SEQUENCE;
	owner->FreezePlayerAndEnemies(true);
	owner->SetPlayerInputOn(false);
}

void BirdVSTigerScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[BIRDRETREAT] = 60;
	stateLength[TIGERCHASE] = 60;
	stateLength[FADEOUT] = 60;
}

void BirdVSTigerScene::ReturnToGame()
{
	
}

void BirdVSTigerScene::AddShots()
{
	AddShot("scenecam");
}

void BirdVSTigerScene::AddPoints()
{

}

void BirdVSTigerScene::AddFlashes()
{

}

void BirdVSTigerScene::AddEnemies()
{

}

void BirdVSTigerScene::AddGroups()
{
	AddGroup("conv", "W4/w4_bird_tiger");
	SetConvGroup("conv");
}

void BirdVSTigerScene::UpdateState()
{
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				owner->adventureHUD->Hide();
				owner->cam.SetManual(true);
				MainMenu *mm = owner->mainMenu;
				owner->Fade(true, 60, Color::Black);
				SetCameraShot("scenecam");
			}
		}
	case WAIT:
		break;
	case CONV:
	{
		ConvUpdate();
		break;
	}
	case BIRDRETREAT:
	{
		break;
	}
	case TIGERCHASE:
	{
		break;
	}
	case FADEOUT:
	{
		if (frame == 0)
		{
			owner->CrossFade( 60, 0, 60, Color::Black);
		}

		if (IsLastFrame())
		{
			owner->FreezePlayerAndEnemies(false);
			owner->cam.SetManual(false);
			owner->adventureHUD->Show();
			owner->SetPlayerInputOn(true);
		}
		break;
	}

	}
}