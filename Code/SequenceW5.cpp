#include "SequenceW5.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "ScoreDisplay.h"
#include "Enemy_Gator.h"

using namespace std;
using namespace sf;


BirdPreFight2Scene::BirdPreFight2Scene()
	:BasicBossScene(BasicBossScene::RUN)
{

}

void BirdPreFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[BIRDCONV] = -1;

	BirdPostFight2Scene *scene = new BirdPostFight2Scene;
	scene->Init();
	nextSeq = scene;
}

void BirdPreFight2Scene::AddShots()
{
	AddShot("scenecam");
	AddShot("fightcam");
}

void BirdPreFight2Scene::AddPoints()
{
	AddStartAndStopPoints();
}

void BirdPreFight2Scene::AddGroups()
{
	AddGroup("pre_fight", "W5/w5_bird_kin");
	SetConvGroup("pre_fight");
}

void BirdPreFight2Scene::AddEnemies()
{
}

void BirdPreFight2Scene::AddFlashes()
{
}

void BirdPreFight2Scene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	EaseShot("fightcam", 60);
}

void BirdPreFight2Scene::UpdateState()
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
	case BIRDCONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

BirdPostFight2Scene::BirdPostFight2Scene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void BirdPostFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[BIRDCONV] = 1000000;
	stateLength[BIRDLEAVE] = 30;
}

void BirdPostFight2Scene::ReturnToGame()
{
	sess->cam.EaseOutOfManual(60);
	sess->TotalDissolveGates(Gate::BOSS);
	BasicBossScene::ReturnToGame();
}

void BirdPostFight2Scene::AddShots()
{
	AddShot("scenecam");
}

void BirdPostFight2Scene::AddPoints()
{
	AddPoint("kinstop0");
}

void BirdPostFight2Scene::AddFlashes()
{

}

void BirdPostFight2Scene::AddEnemies()
{

}

void BirdPostFight2Scene::AddGroups()
{
	AddGroup("post_fight", "W5/w5_bird_fight_post");
	SetConvGroup("post_fight");
}

void BirdPostFight2Scene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				sess->hud->Hide(fadeFrames);
				sess->cam.SetManual(true);
				MainMenu *mm = sess->mainMenu;
				sess->CrossFade(10, 0, 60, Color::White);
			}
			else if (frame == 10)
			{
				SetPlayerStandPoint("kinstop0", true);
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


BirdTigerApproachScene::BirdTigerApproachScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void BirdTigerApproachScene::StartRunning()
{

	//owner->state = GameSession::SEQUENCE;
	sess->FreezePlayerAndEnemies(true);
	sess->SetPlayerInputOn(false);
}

void BirdTigerApproachScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[ENTERGATORAREA] = 60;
	stateLength[FADEOUT] = 60;
}

void BirdTigerApproachScene::ReturnToGame()
{

}

void BirdTigerApproachScene::AddShots()
{
	AddShot("scenecam");
}

void BirdTigerApproachScene::AddPoints()
{

}

void BirdTigerApproachScene::AddFlashes()
{

}

void BirdTigerApproachScene::AddEnemies()
{

}

void BirdTigerApproachScene::AddGroups()
{
	AddGroup("conv", "W5/w5_bird_tiger_approach");
	SetConvGroup("conv");
}

void BirdTigerApproachScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				sess->hud->Hide();
				sess->cam.SetManual(true);
				MainMenu *mm = sess->mainMenu;
				sess->Fade(true, 60, Color::Black);
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
	case ENTERGATORAREA:
	{
		break;
	}
	case FADEOUT:
	{
		if (frame == 0)
		{
			sess->CrossFade(60, 0, 60, Color::Black);
		}

		if (IsLastFrame())
		{
			sess->cam.SetManual(false);
			sess->hud->Show();
		}
		break;
	}

	}
}




GatorPreFightScene::GatorPreFightScene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
	gator = NULL;
}

void GatorPreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[GATORCONV] = -1;

	gator = (Gator*)sess->GetEnemy(EnemyType::EN_GATORBOSS);
}

void GatorPreFightScene::AddShots()
{
	AddShot("scenecam");
	AddShot("fightcam");
}

void GatorPreFightScene::AddPoints()
{
	AddStartAndStopPoints();
}

void GatorPreFightScene::AddGroups()
{
	AddGroup("pre_fight", "W5/w5_gator_fight_pre");
	SetConvGroup("pre_fight");
}

void GatorPreFightScene::AddEnemies()
{
}

void GatorPreFightScene::AddFlashes()
{
}

void GatorPreFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	EaseShot("fightcam", 60);
}

void GatorPreFightScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		if (frame == 0)
		{
			sess->FreezePlayerAndEnemies(false);
		}
		else if (frame == 1)
		{
			gator->Wait();
		}
		EntranceUpdate();
		break;
	case GATORCONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			gator->StartFight();
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

GatorPostFightScene::GatorPostFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	gator = NULL;
}

void GatorPostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[GATORANGRY] = -1;
	stateLength[BIRDATTACKS] = 10;
	stateLength[TIGERFALL] = 10;
	stateLength[TIGERLEAVESWITHBIRD] = 10;
}

void GatorPostFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);
	player->EndLevelWithoutGoal();
	sess->SetPlayerInputOn(true);
}

void GatorPostFightScene::AddShots()
{
	AddShot("gatordeathcam");
}

void GatorPostFightScene::AddPoints()
{
	AddPoint("kinstand0");
}

void GatorPostFightScene::AddFlashes()
{

}

void GatorPostFightScene::AddEnemies()
{

}

void GatorPostFightScene::AddGroups()
{
	AddGroup("conv", "W5/w5_gator_fight_post");
	SetConvGroup("conv");
}

void GatorPostFightScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (frame == 0)
		{
			sess->SetGameSessionState(GameSession::FROZEN);
			sess->hud->Hide(10);
			sess->cam.SetManual(true);
			MainMenu *mm = sess->mainMenu;
			sess->CrossFade(10, 0, 60, Color::White);
		}
		else if (frame == 10)
		{
			sess->SetGameSessionState(GameSession::RUN);
			//sess->hud->Hide();
			SetPlayerStandPoint("kinstand0", true);
			SetCameraShot("gatordeathcam");
			gator->Wait();
		}
	case WAIT:
		if (frame == 0)
		{
			sess->TotalDissolveGates(Gate::BOSS);
		}
		break;
	case GATORANGRY:
	{
		ConvUpdate();
		break;
	}
	case BIRDATTACKS:
	{
		break;
	}
	case TIGERFALL:
	{
		break;
	}
	case TIGERLEAVESWITHBIRD:
	{
		break;
	}
	}
}
