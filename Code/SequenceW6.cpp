#include "SequenceW6.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "ScoreDisplay.h"

using namespace std;
using namespace sf;


EnterFortressScene::EnterFortressScene()
	:BasicBossScene(BasicBossScene::RUN)
{

}

void EnterFortressScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[COYOTECONV] = -1;
	stateLength[SPLITUP] = 60;
}

void EnterFortressScene::AddShots()
{
	AddShot("scenecam");
}

void EnterFortressScene::AddPoints()
{
	AddStartAndStopPoints();
}

void EnterFortressScene::AddGroups()
{
	AddGroup("enter", "W6/w6_coy_kin_enter");
	SetConvGroup("enter");
}

void EnterFortressScene::AddEnemies()
{
}

void EnterFortressScene::AddFlashes()
{
}

void EnterFortressScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();
}

void EnterFortressScene::UpdateState()
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
	case COYOTECONV:
		ConvUpdate();
		break;
	case SPLITUP:
		break;
	}
}




TigerAndBirdTunnelScene::TigerAndBirdTunnelScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void TigerAndBirdTunnelScene::StartRunning()
{

	//owner->state = GameSession::SEQUENCE;
	sess->FreezePlayerAndEnemies(true);
	sess->SetPlayerInputOn(false);
}

void TigerAndBirdTunnelScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[FADEOUT] = 60;
}

void TigerAndBirdTunnelScene::ReturnToGame()
{

}

void TigerAndBirdTunnelScene::AddShots()
{
	AddShot("scenecam");
}

void TigerAndBirdTunnelScene::AddPoints()
{

}

void TigerAndBirdTunnelScene::AddFlashes()
{

}

void TigerAndBirdTunnelScene::AddEnemies()
{

}

void TigerAndBirdTunnelScene::AddGroups()
{
	AddGroup("conv", "W6/w6_bird_tiger_enter");
	SetConvGroup("conv");
}

void TigerAndBirdTunnelScene::UpdateState()
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


SkeletonPreFightScene::SkeletonPreFightScene()
	:BasicBossScene(BasicBossScene::RUN)
{

}

void SkeletonPreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[SKELECONV] = -1;

	SkeletonPostFightScene *scene = new SkeletonPostFightScene;
	scene->Init();
	nextSeq = scene;
}

void SkeletonPreFightScene::AddShots()
{
	AddShot("scenecam");
	AddShot("fightcam");
}

void SkeletonPreFightScene::AddPoints()
{
	AddStartAndStopPoints();
}

void SkeletonPreFightScene::AddGroups()
{
	AddGroup("pre_fight", "W6/w6_coy_skele_fight");
	SetConvGroup("pre_fight");
}

void SkeletonPreFightScene::AddEnemies()
{
}

void SkeletonPreFightScene::AddFlashes()
{
}

void SkeletonPreFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	EaseShot("fightcam", 60);
}

void SkeletonPreFightScene::UpdateState()
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
	case SKELECONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

SkeletonPostFightScene::SkeletonPostFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void SkeletonPostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 10;
	stateLength[MOVIE] = -1;
	stateLength[TOP3TRANSFORMATIONSTART] = 60;
	stateLength[TOP3BIRDANDTIGER] = 60;
	stateLength[TOP3SKELETON] = 60;
	stateLength[SKELETONLEAVES] = -1;
	stateLength[KINMOVE] = -1;
	stateLength[COYOTEDEATH] = -1;


}

void SkeletonPostFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);
	player->EndLevelWithoutGoal();
	sess->SetPlayerInputOn(true);
	
}

void SkeletonPostFightScene::AddShots()
{
	AddShot("skeleleavescam");
	AddShot("skeletransformcam");
	AddShot("birdtransformcam");
	AddShot("kinmovecam");
	AddShot("coyotedeathcam");
}

void SkeletonPostFightScene::AddPoints()
{
	AddPoint("kinstand0");
	AddPoint("kincoyote");
}

void SkeletonPostFightScene::AddFlashes()
{

}

void SkeletonPostFightScene::AddEnemies()
{

}

void SkeletonPostFightScene::AddMovies()
{
	AddMovie("crawler_slash");
}

void SkeletonPostFightScene::AddGroups()
{
	AddGroup("coydeath", "W6/w6_coy_death");
	AddGroup("conv", "W6/w6_coy_skele_kill");
}

void SkeletonPostFightScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				sess->cam.SetManual(true);
				MainMenu *mm = sess->mainMenu;
				sess->CrossFade(10, 0, 60, Color::White);
				sess->TotalDissolveGates(Gate::BOSS);
			}
			else if (IsLastFrame())
			{
				sess->hud->Hide();
				SetPlayerStandPoint("kinstand0", true);
				
			}
		}
	case MOVIE:
	{
		if (frame == 0)
		{
			SetCurrMovie("crawler_slash", 60);
			sess->SetGameSessionState(GameSession::SEQUENCE);
		}

		UpdateMovie();
		break;
	}
	case TOP3TRANSFORMATIONSTART:
	{
		if (frame == 0)
		{

			sess->Fade(true, 60, Color::Black);
			sess->SetGameSessionState(GameSession::RUN);
			sess->ReverseDissolveGates(Gate::BOSS);
			SetCameraShot("skeletransformcam");
		}
		break;
	}
	case TOP3BIRDANDTIGER:
	{
		if (frame == 0)
		{
			SetCameraShot("birdtransformcam");
		}
		break;
	}
	case TOP3SKELETON:
	{
		if (frame == 0)
		{
			SetCameraShot("skeletransformcam");
		}
		break;
	}
	case SKELETONLEAVES:
	{
		if (frame == 0)
		{
			sess->TotalDissolveGates(Gate::BOSS);
			EaseShot("skeleleavescam", 60 );
			SetConvGroup("conv");
		}

		ConvUpdate();
		break;
	}
	case KINMOVE:
	{
		if (frame == 0)
		{
			SetCameraShot("kinmovecam");
			EaseShot("coyotedeathcam", 60);
			//EaseShot("kinmovecam", 60);
			StartEntranceRun(true, 10.0, "kinstand0", "kincoyote");
		}
		break;
	}
	case COYOTEDEATH:
	{
		if (frame == 0)
		{
			SetConvGroup("coydeath");
		}
		ConvUpdate();
		break;
	}
	}
}

bool SkeletonPostFightScene::IsAutoRunState()
{
	return state == KINMOVE;
}




TigerPreFight2Scene::TigerPreFight2Scene()
	:BasicBossScene(BasicBossScene::RUN)
{

}

void TigerPreFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[TIGERCONV] = -1;

	TigerPostFight2Scene *scene = new TigerPostFight2Scene;
	scene->Init();
	nextSeq = scene;
}

void TigerPreFight2Scene::AddShots()
{
	AddShot("scenecam");
	//AddShot("fightcam");
}

void TigerPreFight2Scene::AddPoints()
{
	AddStartAndStopPoints();
}

void TigerPreFight2Scene::AddGroups()
{
	AddGroup("pre_fight", "W6/w6_tiger_fight_pre");
	SetConvGroup("pre_fight");
}

void TigerPreFight2Scene::AddEnemies()
{
}

void TigerPreFight2Scene::AddFlashes()
{
}

void TigerPreFight2Scene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	//EaseShot("fightcam", 60);
}

void TigerPreFight2Scene::UpdateState()
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
	case TIGERCONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

TigerPostFight2Scene::TigerPostFight2Scene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void TigerPostFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV] = -1;
	stateLength[TIGERDEATH] = 60;
}

void TigerPostFight2Scene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);
	player->EndLevelWithoutGoal();
	sess->SetPlayerInputOn(true);
}

void TigerPostFight2Scene::AddShots()
{
	AddShot("tigerdeathcam");
}

void TigerPostFight2Scene::AddPoints()
{
	AddPoint("kinstand0");
}

void TigerPostFight2Scene::AddFlashes()
{

}

void TigerPostFight2Scene::AddEnemies()
{

}

void TigerPostFight2Scene::AddGroups()
{
	AddGroup("conv", "W6/w6_tiger_kill");
	SetConvGroup("conv");
}

void TigerPostFight2Scene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				sess->cam.SetManual(true);
				MainMenu *mm = sess->mainMenu;
				sess->CrossFade(10, 0, 60, Color::White);
			}
			else if (frame == 10)
			{
				sess->hud->Hide();
				SetPlayerStandPoint("kinstand0", true);
				SetCameraShot("tigerdeathcam");
			}
		}
	case WAIT:
		if (frame == 0)
		{
			sess->TotalDissolveGates(Gate::BOSS);
		}
		break;
	case CONV:
	{
		ConvUpdate();
		break;
	}
	case TIGERDEATH:
	{
		break;
	}
	}
}