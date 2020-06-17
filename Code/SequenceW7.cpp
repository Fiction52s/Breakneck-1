#include "SequenceW7.h"
#include "Actor.h"
#include "Session.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "ScoreDisplay.h"

using namespace std;
using namespace sf;

BirdChaseScene::BirdChaseScene()
	:BasicBossScene(BasicBossScene::RUN)
{

}

void BirdChaseScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[SKELETONCONV] = -1;
	stateLength[SKELETONANDBIRDLEAVE] = 60;
}

void BirdChaseScene::AddShots()
{
	AddShot("scenecam");
}

void BirdChaseScene::AddPoints()
{
	AddStartAndStopPoints();
}

void BirdChaseScene::AddGroups()
{
	AddGroup("chase", "W7/w7_chase");
	SetConvGroup("chase");
}

void BirdChaseScene::AddEnemies()
{
}

void BirdChaseScene::AddFlashes()
{
}

void BirdChaseScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	//EaseShot("fightcam", 60);
}

void BirdChaseScene::UpdateState()
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
	case SKELETONCONV:
		ConvUpdate();
		break;
	case SKELETONANDBIRDLEAVE:
		break;
	}
}


BirdPreFight3Scene::BirdPreFight3Scene()
	:BasicBossScene(BasicBossScene::RUN)
{

}

void BirdPreFight3Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[SKELETONCONV] = -1;

	BirdPostFight3Scene *scene = new BirdPostFight3Scene;
	scene->Init();
	nextSeq = scene;
}

void BirdPreFight3Scene::AddShots()
{
	AddShot("scenecam");
	AddShot("fightcam");
}

void BirdPreFight3Scene::AddPoints()
{
	AddStartAndStopPoints();
}

void BirdPreFight3Scene::AddGroups()
{
	AddGroup("pre_fight", "W7/w7_bird_fight_chase");
	SetConvGroup("pre_fight");
}

void BirdPreFight3Scene::AddEnemies()
{
}

void BirdPreFight3Scene::AddFlashes()
{
}

void BirdPreFight3Scene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	EaseShot("fightcam", 60);
}

void BirdPreFight3Scene::UpdateState()
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
	case SKELETONCONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

BirdPostFight3Scene::BirdPostFight3Scene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void BirdPostFight3Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[BIRDCONV] = -1;
	stateLength[BIRDDIES] = 30;
}

void BirdPostFight3Scene::ReturnToGame()
{
	sess->cam.EaseOutOfManual(60);
	sess->TotalDissolveGates(Gate::BOSS);
	BasicBossScene::ReturnToGame();
}

void BirdPostFight3Scene::AddShots()
{
	AddShot("scenecam");
}

void BirdPostFight3Scene::AddPoints()
{
	AddPoint("kinstand0");
}

void BirdPostFight3Scene::AddFlashes()
{

}

void BirdPostFight3Scene::AddEnemies()
{

}

void BirdPostFight3Scene::AddGroups()
{
	AddGroup("post_fight", "W7/w7_bird_kill");
	SetConvGroup("post_fight");
}

void BirdPostFight3Scene::UpdateState()
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
				sess->cam.SetManual(true);
				MainMenu *mm = sess->mainMenu;
				sess->CrossFade(10, 0, 60, Color::White);
			}
			else if (frame == 10)
			{
				SetPlayerStandPoint("kinstand0", true);
			}
		}
	case WAIT:
		//EntranceUpdate();
		break;
	case BIRDCONV:
		ConvUpdate();
		break;
	case BIRDDIES:
		break;
	}
}


FinalSkeletonPreFightScene::FinalSkeletonPreFightScene()
	:BasicBossScene(BasicBossScene::RUN)
{
	SetEntranceIndex(1);
}

void FinalSkeletonPreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[SKELETONCONV] = -1;

	FinalSkeletonPostFightScene *scene = new FinalSkeletonPostFightScene;
	scene->Init();
	nextSeq = scene;
}

void FinalSkeletonPreFightScene::SetEntranceShot()
{
	SetCameraShot("scenecam1");
}

void FinalSkeletonPreFightScene::AddShots()
{
	AddShot("scenecam1");
	//AddShot("victorycam");
	AddShot("fightcam1");
}

void FinalSkeletonPreFightScene::AddPoints()
{
	AddStartAndStopPoints();
}

void FinalSkeletonPreFightScene::AddGroups()
{
	AddGroup("pre_fight", "W7/w7_core_fight");
	SetConvGroup("pre_fight");
}

void FinalSkeletonPreFightScene::AddEnemies()
{
}

void FinalSkeletonPreFightScene::AddFlashes()
{
}

void FinalSkeletonPreFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	EaseShot("fightcam1", 60);
}

void FinalSkeletonPreFightScene::UpdateState()
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
	case SKELETONCONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

FinalSkeletonPostFightScene::FinalSkeletonPostFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void FinalSkeletonPostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[SKELETONCONV] = -1;
	stateLength[SKELETONDIES] = 30;
}

void FinalSkeletonPostFightScene::ReturnToGame()
{
	sess->TotalDissolveGates(Gate::BOSS);
	BasicBossScene::ReturnToGame();
}

void FinalSkeletonPostFightScene::AddShots()
{
	AddShot("victorycam");
}

void FinalSkeletonPostFightScene::AddPoints()
{
	AddPoint("kinstand1");
}

void FinalSkeletonPostFightScene::AddFlashes()
{

}

void FinalSkeletonPostFightScene::AddEnemies()
{

}

void FinalSkeletonPostFightScene::AddGroups()
{
	AddGroup("post_fight", "W7/w7_core_skele_kill");
	SetConvGroup("post_fight");
}

void FinalSkeletonPostFightScene::UpdateState()
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
				sess->cam.SetManual(true);
				MainMenu *mm = sess->mainMenu;
				sess->CrossFade(10, 0, 60, Color::White);
			}
			else if (frame == 10)
			{
				SetPlayerStandPoint("kinstand1", true);
				SetCameraShot("victorycam");
			}
		}
		break;
	case WAIT:
		//EntranceUpdate();
		break;
	case SKELETONCONV:
		ConvUpdate();
		break;
	case SKELETONDIES:
		break;
	}
}