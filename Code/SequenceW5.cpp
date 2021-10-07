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
#include "GroundedWarper.h"
#include "Enemy_Bird.h"
#include "Enemy_SequenceBird.h"

using namespace std;
using namespace sf;


BirdPreFight2Scene::BirdPreFight2Scene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{

}

void BirdPreFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[BIRDCONV] = -1;

	

	bird = (Bird*)sess->GetEnemy(EnemyType::EN_BIRDBOSS);
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
			sess->AddEnemy(bird);
			bird->SeqWait();
		}
		EntranceUpdate();
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

BirdPostFight2Scene::BirdPostFight2Scene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqBird = NULL;
	warper = NULL;
}

void BirdPostFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = fadeFrames + explosionFadeFrames;
	stateLength[WAIT] = 60;
	stateLength[BIRDCONV] = -1;
	stateLength[BIRDLEAVE] = 120;


	seqBird = (SequenceBird*)sess->GetEnemy(EnemyType::EN_SEQUENCEBIRD);
	warper = sess->GetWarper("Bosses/greyw1");
}

void BirdPostFight2Scene::ReturnToGame()
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
	sess->RemoveEnemy(seqBird);
	sess->TotalDissolveGates(Gate::BOSS);
	BasicBossScene::ReturnToGame();
}

void BirdPostFight2Scene::AddShots()
{
	AddShot("scenecam");
}

void BirdPostFight2Scene::AddPoints()
{
	AddStopPoint();
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
		if (frame == 0)
		{
			StartBasicKillFade();
		}
		else if (frame == explosionFadeFrames)
		{
			sess->SetGameSessionState(GameSession::RUN);
			SetPlayerStandPoint("kinstop0", true);
			SetCameraShot("scenecam");
			
			seqBird->Reset();
			sess->AddEnemy(seqBird);
			seqBird->facingRight = false;
		}
		break;
	case WAIT:
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
		if (frame == 0)
		{
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
			sess->AddEnemy(gator);
			gator->SeqWait();
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
			StartBasicKillFade();
		}
		else if (frame == 10)
		{
			sess->SetGameSessionState(GameSession::RUN);
			SetPlayerStandPoint("kinstand0", true);
			SetCameraShot("gatordeathcam");
			gator->SeqWait();
		}
		break;
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
