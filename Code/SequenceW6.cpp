#include "SequenceW6.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "ScoreDisplay.h"
#include "Enemy_Skeleton.h"
#include "Enemy_CoyoteHelper.h"
#include "Enemy_Tiger.h"
#include "Enemy_SequenceCoyote.h"
#include "MusicSelector.h"

using namespace std;
using namespace sf;


EnterFortressScene::EnterFortressScene()
	:BasicBossScene(BasicBossScene::RUN)
{
	seqCoyote = NULL;

	wind = sess->mainMenu->musicManager->songMap["w6_64_Wind"];
	specialMusic = sess->mainMenu->musicManager->songMap["w3_65_Nothing"];

	assert(wind != NULL);
	wind->Load();

	assert(specialMusic != NULL);
	specialMusic->Load();
}

void EnterFortressScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[COYOTE_ENTRANCE] = 60;
	stateLength[WAIT2] = 60;
	stateLength[FACES1] = -1;
	stateLength[COYOTECONV] = -1;
	stateLength[FACES2] = -1;
	stateLength[SPLITUP] = 120;

	seqCoyote = (SequenceCoyote*)sess->GetEnemy(EnemyType::EN_SEQUENCECOYOTE);
}

void EnterFortressScene::AddShots()
{
	AddShot("scenecam");
}

void EnterFortressScene::AddPoints()
{
	AddStartAndStopPoints();
	AddPoint("coyotedest");
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
	int togetherFrames = 5;

	FlashGroup * group = AddFlashGroup("group0");
	FlashGroup * group1 = AddFlashGroup("group1");


	AddFlashedImage("c3", sess->GetTileset("Story/EnterFortress/Breakneck_Comic_W6_1920x1080_03.png"),
		0, 30, 60, 30, Vector2f(960, 540));

	AddFlashedImage("c10", sess->GetTileset("Story/EnterFortress/Breakneck_Comic_W6_1920x1080_10.png"),
		0, 30, 60, 30, Vector2f(960, 540));

	AddFlashedImage("c4", sess->GetTileset("Story/EnterFortress/Breakneck_Comic_W6_1920x1080_04.png"),
		0, 30, 60, 30, Vector2f(960, 540));

	AddSeqFlashToGroup(group, "c3", 0);
	AddSeqFlashToGroup(group, "c10", 0);

	AddSeqFlashToGroup(group1, "c4", 0);

	group->Init();
	group1->Init();

	/*for (int i = 1; i < 10; ++i)
	{
		AddFlashedImage("c" + to_string(i), sess->GetTileset("Story/Breakneck_Comic_W6_1920x1080_0" + to_string(i) + ".png"),
			0, 30, 60, 30, Vector2f(960, 540));
		
	}*/

	/*AddFlashedImage("stare2", sess->GetTileset("Bosses/Coyote/Coy_11b.png", 1920, 1080),
		0, 30, 20, 30, Vector2f(960, 540));*/

	
	//for( int i = 1; i < 10; ++)
	//AddSeqFlashToGroup(group, "smile", 0);
	//AddSeqFlashToGroup(group, "stare1", 0);//-togetherFrames);
	//AddSeqFlashToGroup(group, "stare2", 0);//-togetherFrames);
	//group->Init();

	//
	//AddSeqFlashToGroup(group1, "stare0", 0);
	//AddSeqFlashToGroup(group1, "stare1", 0);//-togetherFrames);
	//AddSeqFlashToGroup(group1, "stare2", 0);//-togetherFrames);
	//group1->Init();
}

void EnterFortressScene::StartRunning()
{
	

	//sess->TransitionMusic(wind, 180);
	//sess->mainMenu->musicPlayer->TransitionMusic(wind, 60);//StopCurrentMusic();//FadeOutCurrentMusic(60);
	//sess->mainMenu->musicPlayer->PlayMusic(wind);
}

void EnterFortressScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	sess->TransitionMusic(prevMusic, 60);
	//sess->mainMenu->musicPlayer->TransitionMusic(prevMusic, 60);

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
		else if (frame == fadeFrames)
		{
			prevMusic = sess->mainMenu->musicPlayer->currMusic;
			sess->TransitionMusic(wind, 120);
		}
		EntranceUpdate();
		break;
	case WAIT:
	{
		if (frame == 0)
		{

		}
		break;
	}
	case COYOTE_ENTRANCE:
	{
		if (frame == 0)
		{
			seqCoyote->Reset();
			sess->AddEnemy(seqCoyote);
			seqCoyote->Walk(points["coyotedest"]->pos);
			//enemyMover.SetDestNode(node);
		}
		break;
	}
	case WAIT2:
	{
		break;
	}
	case FACES1:
	{
		if (frame == 0)
		{
			SetFlashGroup("group0");
			sess->TransitionMusic(specialMusic, 120);
		}
		else
		{
			if (currFlashGroup == NULL)
			{
				EndCurrState();
			}
		}

		/*if (IsLastFrame())
		{
			coy->StartFight();
			sess->ReverseDissolveGates(Gate::BOSS);
		}*/
		break;
	}
	case COYOTECONV:
		ConvUpdate();
		break;
	case FACES2:
	{
		if (frame == 0)
		{
			SetFlashGroup("group1");
			seqCoyote->SummonScorpion();
		}
		else
		{
			if (currFlashGroup == NULL)
			{
				EndCurrState();
			}
		}

		/*if (IsLastFrame())
		{
		coy->StartFight();
		sess->ReverseDissolveGates(Gate::BOSS);
		}*/
		break;
	}
	case SPLITUP:
		if (frame == 0)
		{
			seqCoyote->Bounce(points["coyotedest"]->pos + V2d(1000, 0));
		}
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
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
	skeleton = NULL;
	coyHelper = NULL;
}

void SkeletonPreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[SKELECONV] = -1;

	skeleton = (Skeleton*)sess->GetEnemy(EnemyType::EN_SKELETONBOSS);
	coyHelper = (CoyoteHelper*)sess->GetEnemy(EnemyType::EN_COYOTEHELPER);
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
			sess->AddEnemy(skeleton);
			skeleton->SeqWait();

			sess->AddEnemy(coyHelper);
			coyHelper->Wait();
		}
		EntranceUpdate();
		break;
	case SKELECONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			skeleton->StartFight();
			coyHelper->StartFight();
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

	stateLength[FADE] = explosionFadeFrames + fadeFrames;
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
		if (frame == 0)
		{
			StartBasicKillFade();
		}
		else if (frame == explosionFadeFrames)
		{
			SetPlayerStandPoint("kinstand0", true);
			sess->SetGameSessionState(GameSession::RUN);
			skeleton->SeqWait();
			coyHelper->Wait();
		}	
		break;
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
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
	tiger = NULL;
}

void TigerPreFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[TIGERCONV] = -1;

	tiger = (Tiger*)sess->GetEnemy(EnemyType::EN_TIGERBOSS);

	/*TigerPostFight2Scene *scene = new TigerPostFight2Scene;
	scene->Init();
	nextSeq = scene;*/
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
			sess->AddEnemy(tiger);
			tiger->SeqWait();
		}
		EntranceUpdate();
		break;
	case TIGERCONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			tiger->StartFight();
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
}

TigerPostFight2Scene::TigerPostFight2Scene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	tiger = NULL;
}

void TigerPostFight2Scene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = explosionFadeFrames + fadeFrames;
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
		if (frame == 0)
		{
			StartBasicKillFade();
		}
		else if (frame == explosionFadeFrames)
		{
			sess->SetGameSessionState(GameSession::RUN);
			SetPlayerStandPoint("kinstand0", true);
			SetCameraShot("tigerdeathcam");
			tiger->SeqWait();
		}
		break;
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