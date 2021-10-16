#include "SequenceW4.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "ScoreDisplay.h"
#include "GroundedWarper.h"
#include "Enemy_Tiger.h"
#include "Enemy_CrawlerQueen.h"

#include "Enemy_SequenceTiger.h"
#include "Enemy_SequenceBird.h"
#include "Enemy_SequenceCrawler.h"

using namespace sf;
using namespace std;

CrawlerPreFight2Scene::CrawlerPreFight2Scene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
}

void CrawlerPreFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[CONV] = -1;

	queen = (CrawlerQueen*)sess->GetEnemy(EnemyType::EN_CRAWLERQUEEN);
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
	AddFlashToGroup(group, "stare2", togetherFrames);
	group->Init();*/
}

void CrawlerPreFight2Scene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	EaseShot("fightcam", 60);
}

void CrawlerPreFight2Scene::SetEntranceShot()
{
	SetCameraShot("crawlercam");
}

void CrawlerPreFight2Scene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		if (frame == 0)
		{
			sess->AddEnemy(queen);
			queen->SeqWait();
		}
		EntranceUpdate();
		break;

	case CONV:
		ConvUpdate();

		if (IsLastFrame())
		{
			queen->StartFight();
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}



CrawlerPostFight2Scene::CrawlerPostFight2Scene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqCrawler = NULL;
	warper = NULL;
}

void CrawlerPostFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = fadeFrames + explosionFadeFrames;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[TIGERRUMBLE] = 30;
	stateLength[CRAWLERLEAVE] = 120;

	warper = sess->GetWarper("FinishedScenes/W4/tigerfight");

	seqCrawler = (SequenceCrawler*)sess->GetEnemy(EnemyType::EN_SEQUENCECRAWLER);
}

void CrawlerPostFight2Scene::ReturnToGame()
{
	if (warper != NULL)
	{
		if (!warper->spawned)
		{
			sess->AddEnemy(warper);
		}
		warper->Activate();
	}
	
	sess->cam.EaseOutOfManual(60);
	BasicBossScene::ReturnToGame();
}

void CrawlerPostFight2Scene::AddShots()
{
	AddShot("crawlercam");
}

void CrawlerPostFight2Scene::AddPoints()
{
	AddStopPoint();
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
	AddGroup("postfight2", "W4/w4_crawler_kin_post_2");
	
}

void CrawlerPostFight2Scene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (frame == 0)
		{
			StartBasicKillFade();
		}
		else if (frame == 10)
		{
			seqCrawler->Reset();
			sess->AddEnemy(seqCrawler);
			seqCrawler->facingRight = false;


			sess->SetGameSessionState(GameSession::RUN);
			sess->FreezePlayer(false);
			SetPlayerStandPoint("kinstop0", true);
			SetCameraShot("crawlercam");
		}
		break;
	case WAIT:
		if (frame == 0)
		{
			sess->TotalDissolveGates(Gate::BOSS);
		}
		//EntranceUpdate();
		break;
	case CONV:
		if (frame == 0)
		{
			SetConvGroup("postfight");
		}
		ConvUpdate();
		break;
	case TIGERRUMBLE:
	{
		if (frame == 0)
		{
			sess->cam.SetRumble(3, 3, 30);
		}
		break;
	}
	case CONV2:
	{
		if (frame == 0)
		{
			SetConvGroup("postfight2");
		}

		ConvUpdate();
		break;
	}
	case CRAWLERLEAVE:
		if (frame == 0)
		{
			seqCrawler->DigIn();
		}
		break;
	}
}

CrawlerVSTigerScene::CrawlerVSTigerScene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
	seqTiger = NULL;
	seqCrawler = NULL;

	SetEntranceIndex(0);
}

void CrawlerVSTigerScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[WAIT] = 60;
	stateLength[TIGER_KILL_CRAWLER] = 60;
	stateLength[CRAWLER_DEATH] = 60;

	seqTiger = (SequenceTiger*)sess->GetEnemy(EnemyType::EN_SEQUENCETIGER);
	seqCrawler = (SequenceCrawler*)sess->GetEnemy(EnemyType::EN_SEQUENCECRAWLER);
}

void CrawlerVSTigerScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);
	//player->EndLevelWithoutGoal();
	sess->SetPlayerInputOn(true);
}

void CrawlerVSTigerScene::AddShots()
{
	AddShot("scenecam");
}

void CrawlerVSTigerScene::AddPoints()
{
	AddStartAndStopPoints();
	/*AddPoint("kinstand0");
	AddPoint("birdfly1");
	AddPoint("birdfly2");
	AddPoint("birdfly3");*/
}

void CrawlerVSTigerScene::AddFlashes()
{

}

void CrawlerVSTigerScene::AddEnemies()
{

}

void CrawlerVSTigerScene::AddGroups()
{
	AddGroup("conv1", "W4/w4_crawler_tiger_1");
	AddGroup("conv2", "W4/w4_crawler_tiger_2");
	AddGroup("conv3", "W4/w4_crawler_tiger_3");
}

void CrawlerVSTigerScene::SetEntranceShot()
{
	SetCameraShot("scenecam");
}

void CrawlerVSTigerScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		if (frame == 0)
		{
			seqCrawler->Reset();
			sess->AddEnemy(seqCrawler);

			seqTiger->Reset();
			sess->AddEnemy(seqTiger);
			seqTiger->facingRight = false;
		}
		EntranceUpdate();
		break;
	case WAIT:
		if (frame == 0)
		{
			sess->TotalDissolveGates(Gate::BOSS);
		}
		//EntranceUpdate();
		break;
	case CONV1:
		if (frame == 0)
		{
			SetConvGroup("conv1");
		}

		ConvUpdate();
		break;
	case CRAWLER_FLIP_SWITCH:
	{
		if (frame == 0)
		{
			seqCrawler->TriggerBombs();
		}

		if (seqCrawler->action == SequenceCrawler::IDLE)
		{
			EndCurrState();
		}
		break;
	}
	case NEXUS_EXPLODE:
	{
		if (frame == 0)
		{
			sess->cam.SetRumble(5, 5, 30);
		}
		if (frame == 60)
		{
			EndCurrState();
		}
		break;
	}
	case CONV2:
	{
		if (frame == 0)
		{
			SetConvGroup("conv2");
		}

		ConvUpdate();
		break;
	}
	case TIGER_LUNGE:
	{
		if (frame == 0)
		{
			seqTiger->Lunge(seqCrawler->GetPosition(), 50, 10);
		}

		if (seqTiger->action == SequenceTiger::PRE_CRAWLER_KILL)
		{
			EndCurrState();
		}
		break;
	}
	case TIGER_KILL_CRAWLER:
	{
		if (frame == 0)
		{
			seqTiger->KillCrawler();
			seqCrawler->HitByTiger();
		}
		break;
	}
	case CONV3:
	{
		if (frame == 0)
		{
			SetConvGroup("conv3");
		}

		ConvUpdate();
		break;
	}
	case CRAWLER_DEATH:
	{
		if (frame == 0)
		{
			seqCrawler->DieByTiger();
		}
		break;
	}
	}
}


TigerPreFightScene::TigerPreFightScene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
	SetEntranceIndex(0);
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

	tiger = (Tiger*)sess->GetEnemy(EnemyType::EN_TIGERBOSS);
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
	AddFlashToGroup(group, "stare2", togetherFrames);
	group->Init();*/
}

void TigerPreFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	//EaseShot("fightcam", 60);
}

bool TigerPreFightScene::IsAutoRunState()
{
	return state == ENTRANCE || state == KINAPPROACH;
}

void TigerPreFightScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		if (frame == 0)
		{
			sess->AddEnemy(tiger);
			tiger->SeqWait();
		}
		EntranceUpdate();
		break;
	case TIGERENTRANCE:
	{
		if (frame == 0)
		{
			sess->ReverseDissolveGates(Gate::BOSS);
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
			sess->TotalDissolveGates(Gate::BOSS);
			EaseShot("scenecam", 60);
			//owner->ReverseDissolveGates(Gate::CRAWLER_UNLOCK);
		}
		break;
	}
	case KINAPPROACH:
	{
		if (frame == 0)
		{
			StartEntranceRun(true, 10.0, "kinstop0", "kinnext");
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
			tiger->StartFight();
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
	}
}



TigerPostFightScene::TigerPostFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqTiger = NULL;
	seqBird = NULL;
}

void TigerPostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = fadeFrames + explosionFadeFrames;
	stateLength[WAIT] = 60;
	stateLength[CONV1] = -1;
	stateLength[BIRD_ENTER] = 120;
	stateLength[CONV2] = -1;
	stateLength[BIRD_SWOOP] = 120;
	stateLength[CONV3] = -1;
	stateLength[BIRD_EXIT] = 120;

	seqTiger = (SequenceTiger*)sess->GetEnemy(EnemyType::EN_SEQUENCETIGER);
	seqBird = (SequenceBird*)sess->GetEnemy(EnemyType::EN_SEQUENCEBIRD);
}

void TigerPostFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);
	player->EndLevelWithoutGoal();
	sess->SetPlayerInputOn(true);
}

void TigerPostFightScene::AddShots()
{
	AddShot("birdrescuecam");
}

void TigerPostFightScene::AddPoints()
{
	AddPoint("kinstand0");
	AddPoint("birdfly1");
	AddPoint("birdfly2");
	AddPoint("birdfly3");
}

void TigerPostFightScene::AddFlashes()
{

}

void TigerPostFightScene::AddEnemies()
{

}

void TigerPostFightScene::AddGroups()
{
	AddGroup("postfight_1", "W4/w4_tiger_fight_post_1");
	AddGroup("postfight_2", "W4/w4_tiger_fight_post_2");
	AddGroup("postfight_3", "W4/w4_tiger_fight_post_3");
}

void TigerPostFightScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (frame == 0)
		{
			
			//seqTiger->Breathe();


			StartBasicKillFade();
		}
		else if (frame == explosionFadeFrames)
		{
			seqBird->Reset();
			sess->AddEnemy(seqBird);
			PositionInfo birdPos;
			birdPos.position = GetPointPos("birdfly1");
			seqBird->SetCurrPosInfo(birdPos);

			seqTiger->Reset();
			sess->AddEnemy(seqTiger);
			seqTiger->facingRight = false;
			seqTiger->Breathe();

			sess->SetGameSessionState(GameSession::RUN);
			SetPlayerStandPoint("kinstand0", true);
			SetCameraShot("birdrescuecam");
			//tiger->SeqWait();
		}
		break;
	case WAIT:
		if (frame == 0)
		{
			sess->TotalDissolveGates(Gate::BOSS);
		}
		//EntranceUpdate();
		break;
	case CONV1:
		if (frame == 0)
		{
			SetConvGroup("postfight_1");
		}
		
		ConvUpdate();
		break;
	case BIRD_ENTER:
	{
		if (frame == 0)
		{
			seqBird->Fly(GetPointPos("birdfly2"));
			seqTiger->LookUp();
			//seqBird->Fly
		}
		break;
	}
	case CONV2:
		if (frame == 0)
		{
			SetConvGroup("postfight_2");
		}

		ConvUpdate();
		break;
	case BIRD_SWOOP:
	{
		if (frame == 0)
		{
			seqBird->Fly(seqTiger->GetPosition() + V2d(0, -50));
		}
		break;
	}
	case CONV3:
		if (frame == 0)
		{
			SetConvGroup("postfight_3");
		}

		ConvUpdate();
		break;
	case BIRD_EXIT:
	{
		if (frame == 0)
		{
			seqBird->FlyAwayWithTiger(GetPointPos("birdfly3") + V2d(0, -50));
			seqTiger->Carried(GetPointPos("birdfly3"));
		}
		break;
	}
		
	}
}



BirdTigerAllianceScene::BirdTigerAllianceScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqTiger = NULL;
	seqBird = NULL;
}

void BirdTigerAllianceScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[TIGERSTORY] = 60;
	stateLength[FADEOUT] = 60;

	seqTiger = (SequenceTiger*)sess->GetEnemy(EnemyType::EN_SEQUENCETIGER);
	seqBird = (SequenceBird*)sess->GetEnemy(EnemyType::EN_SEQUENCEBIRD);
}

void BirdTigerAllianceScene::ReturnToGame()
{
	sess->EndLevelNoScene();
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
	AddGroup("conv", "W4/w4_bird_tiger_alliance");
	SetConvGroup("conv");
}

void BirdTigerAllianceScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (frame == 0)
		{
			seqBird->Reset();
			sess->AddEnemy(seqBird);
			//seqBird->facingRight = false;
			seqBird->Breathe();

			seqTiger->Reset();
			sess->AddEnemy(seqTiger);
			seqTiger->facingRight = false;
			//seqTiger->Breathe();


			sess->hud->Hide();
			sess->cam.SetManual(true);
			MainMenu *mm = sess->mainMenu;
			sess->Fade(true, 60, Color::Black);
			SetCameraShot("alliancecam");
		}
		break;
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
			sess->Fade(false, 60, Color::Black);
		}

		/*if (IsLastFrame())
		{
			sess->goalDestroyed = true;
		}*/
		break;
	}

	}
}




BirdVSTigerScene::BirdVSTigerScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqTiger = NULL;
	seqBird = NULL;
}

void BirdVSTigerScene::StartRunning()
{

	////owner->state = GameSession::SEQUENCE;
	//owner->FreezePlayerAndEnemies(true);
	//owner->SetPlayerInputOn(false);
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

	seqTiger = (SequenceTiger*)sess->GetEnemy(EnemyType::EN_SEQUENCETIGER);
	seqBird = (SequenceBird*)sess->GetEnemy(EnemyType::EN_SEQUENCEBIRD);
}

void BirdVSTigerScene::ReturnToGame()
{
	sess->EndLevelNoScene();
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
	AddGroup("conv", "W4/w4_bird_tiger_argue");
	SetConvGroup("conv");
}

void BirdVSTigerScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (frame == 0)
		{
			seqBird->Reset();
			sess->AddEnemy(seqBird);
			//seqBird->facingRight = false;
			seqBird->Breathe();

			seqTiger->Reset();
			sess->AddEnemy(seqTiger);
			seqTiger->facingRight = false;

			sess->hud->Hide();
			sess->cam.SetManual(true);
			MainMenu *mm = sess->mainMenu;
			sess->Fade(true, 60, Color::Black);
			SetCameraShot("scenecam");
		}
		break;
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
			sess->CrossFade( 60, 0, 60, Color::Black);
		}

		break;
	}

	}
}