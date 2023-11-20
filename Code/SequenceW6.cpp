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
#include "Enemy_SequenceTiger.h"
#include "Enemy_SequenceBird.h"
#include "Enemy_SequenceSkeleton.h"

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
	stateLength[WAIT] = 120;
	stateLength[COYOTE_ENTRANCE] = -1;
	stateLength[WAIT2] = 120;
	stateLength[FACES1] = -1;
	stateLength[COYOTECONV] = -1;
	stateLength[FACES2] = -1;
	stateLength[SUMMON_SCORPION] = 90;
	stateLength[SPLITUP] = 90;

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
		0, 30, 20, 30, Vector2f(960, 540));

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

	/*AddFlashedImage("stare2", sess->GetTileset("Enemies/Bosses/Coyote/Coy_11b.png", 1920, 1080),
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
	sess->RemoveEnemy(seqCoyote);
	//sess->mainMenu->musicPlayer->TransitionMusic(prevMusic, 60);

	BasicBossScene::ReturnToGame();
}

void EnterFortressScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (seqData.state)
	{
	case ENTRANCE:
		if (seqData.frame == 0)
		{
			
		}
		else if (seqData.frame == fadeFrames)
		{
			prevMusic = sess->mainMenu->musicPlayer->currMusic;
			sess->TransitionMusic(wind, 120);
		}
		EntranceUpdate();
		break;
	case WAIT:
	{
		if (seqData.frame == 0)
		{

		}
		break;
	}
	case COYOTE_ENTRANCE:
	{
		if (seqData.frame == 0)
		{
			seqCoyote->Reset();
			sess->AddEnemy(seqCoyote);
			seqCoyote->Walk(points["coyotedest"]->pos);
			//enemyMover.SetDestNode(node);
		}

		if (seqCoyote->action == SequenceCoyote::IDLE)
		{
			EndCurrState();
		}
		break;
	}
	case WAIT2:
	{
		break;
	}
	case FACES1:
	{
		if (seqData.frame == 0)
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
		if (seqData.frame == 0)
		{
			SetFlashGroup("group1");
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
	case SUMMON_SCORPION:
	{
		if (seqData.frame == 0)
		{
			seqCoyote->SummonScorpion();
		}
		

		/*if (seqCoyote->action != SequenceCoyote::SUMMON_SCORPION)
		{
			EndCurrState();
		}*/
		break;
	}
	case SPLITUP:
		if (seqData.frame == 0)
		{
			seqCoyote->Bounce(points["coyotedest"]->pos + V2d(1000, 0));
		}
		break;
	}
}




TigerAndBirdTunnelScene::TigerAndBirdTunnelScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqTiger = NULL;
	seqBird = NULL;
}

void TigerAndBirdTunnelScene::StartRunning()
{

	//owner->state = GameSession::SEQUENCE;
	//sess->FreezePlayerAndEnemies(true);
	//sess->SetPlayerInputOn(false);
}

void TigerAndBirdTunnelScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[WALK_IN] = -1;
	stateLength[CONV] = -1;
	stateLength[WALK_OUT] = 60;
	stateLength[FADEOUT] = 60;

	seqTiger = (SequenceTiger*)sess->GetEnemy(EnemyType::EN_SEQUENCETIGER);
	seqBird = (SequenceBird*)sess->GetEnemy(EnemyType::EN_SEQUENCEBIRD);
}

void TigerAndBirdTunnelScene::ReturnToGame()
{
	//sess->EndLevelNoScene();
}

void TigerAndBirdTunnelScene::AddShots()
{
	AddShot("scenecam");
}

void TigerAndBirdTunnelScene::AddPoints()
{
	AddPoint("birdwalk1");
	AddPoint("birdwalk2");
	AddPoint("tigerwalk1");
	AddPoint("tigerwalk2");
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
	switch (seqData.state)
	{
	case FADE:
		if (seqData.frame == 0)
		{
			sess->HideHUD();
			sess->cam.SetManual(true);
			MainMenu *mm = sess->mainMenu;
			sess->Fade(true, 60, Color::Black);
			SetCameraShot("scenecam");
		}
		break;
	case WAIT:
		break;
	case WALK_IN:
		if (seqData.frame == 0)
		{
			seqBird->Reset();
			sess->AddEnemy(seqBird);
			seqBird->Walk(GetPointPos("birdwalk1"));

			seqTiger->Reset();
			sess->AddEnemy(seqTiger);
			seqTiger->Walk(GetPointPos("tigerwalk1"));
		}

		if (seqBird->action == SequenceBird::IDLE && seqTiger->action == SequenceTiger::IDLE)
		{
			EndCurrState();
		}
		break;
	case CONV:
	{
		ConvUpdate();
		break;
	}
	case WALK_OUT:
	{
		if (seqData.frame == 0)
		{
			seqBird->Walk(GetPointPos("birdwalk2"));
			seqTiger->Walk(GetPointPos("tigerwalk2"));
		}

		if (seqBird->action == SequenceBird::IDLE && seqTiger->action == SequenceTiger::IDLE)
		{
			//doesnt actually get here, because the length of it is only 60 frames,
			//not -1. works pretty well though.
			EndCurrState();
		}
		break;
	}
	case FADEOUT:
	{
		if (seqData.frame == 0)
		{
			sess->CrossFade(60, 0, 60, Color::Black);
		}

		if (IsLastFrame())
		{
			sess->cam.SetManual(false);
			sess->ShowHUD();
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
	switch (seqData.state)
	{
	case ENTRANCE:
		if (seqData.frame == 0)
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
	switch (seqData.state)
	{
	case FADE:
		if (seqData.frame == 0)
		{
			StartBasicKillFade();
		}
		else if (seqData.frame == explosionFadeFrames)
		{
			SetPlayerStandPoint("kinstand0", true);
			sess->SetGameSessionState(GameSession::RUN);
		}	
		break;
	case MOVIE:
	{
		if (seqData.frame == 0)
		{
			//EndCurrState();
			SetCurrMovie("crawler_slash", 60);
			sess->SetGameSessionState(GameSession::SEQUENCE);
		}

		UpdateMovie();
		break;
	}
	case TOP3TRANSFORMATIONSTART:
	{
		if (seqData.frame == 0)
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
		if (seqData.frame == 0)
		{
			SetCameraShot("birdtransformcam");
		}
		break;
	}
	case TOP3SKELETON:
	{
		if (seqData.frame == 0)
		{
			SetCameraShot("skeletransformcam");
		}
		break;
	}
	case SKELETONLEAVES:
	{
		if (seqData.frame == 0)
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
		if (seqData.frame == 0)
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
		if (seqData.frame == 0)
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
	return seqData.state == KINMOVE;
}

MindControlScene::MindControlScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	seqBird = NULL;
	seqTiger = NULL;
	seqSkeleton = NULL;
}

void MindControlScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[CONV1] = -1;
	stateLength[NEXUS_EXPLODE] = 120;
	stateLength[CONV2] = -1;
	stateLength[SKELETON_ENTRANCE] = 60;
	stateLength[CONV3] = -1;
	stateLength[SKELETON_FACE_PRE_MIND_CONTROL] = -1;
	stateLength[SKELETON_MIND_CONTROL] = 120;
	stateLength[CONV4] = -1;
	stateLength[MIND_CONTROL_FINISH] = 60;
	stateLength[CONV5] = -1;
	stateLength[BIRD_WALK_OVER_TO_SKELETON] = -1;
	stateLength[ENTRANCE] = -1;
	stateLength[CONV6] = -1;
	stateLength[SKELETON_JUMP_ONTO_BIRD] = -1;
	stateLength[TIGER_FACE_KIN] = 30;
	stateLength[SKELETON_BIRD_EXIT] = 120;

	seqTiger = (SequenceTiger*)sess->GetEnemy(EnemyType::EN_SEQUENCETIGER);
	seqBird = (SequenceBird*)sess->GetEnemy(EnemyType::EN_SEQUENCEBIRD);
	seqSkeleton = (SequenceSkeleton*)sess->GetEnemy(EnemyType::EN_SEQUENCESKELETON);
}

void MindControlScene::AddShots()
{
	AddShot("scenecam");
}

void MindControlScene::AddPoints()
{
	AddStartAndStopPoints();

	AddPoint("birdfly1");
	AddPoint("birdwalk1");
}

void MindControlScene::AddGroups()
{
	AddGroup("conv1", "W6/w6_skele_nexus_1");
	AddGroup("conv2", "W6/w6_skele_nexus_2");
	AddGroup("conv3", "W6/w6_skele_nexus_3");
	AddGroup("conv4", "W6/w6_skele_nexus_4");
	AddGroup("conv5", "W6/w6_skele_nexus_5");
	AddGroup("conv6", "W6/w6_skele_nexus_6");
	SetConvGroup("conv1");
}

void MindControlScene::AddEnemies()
{
}

void MindControlScene::AddFlashes()
{
	AddFlashedImage("skeleangry", sess->GetTileset("Story/CoyoteAndSkeleton/Coy_23c.png"),
		0, 30, 60, 30, Vector2f(960, 540));
}

bool MindControlScene::IsAutoRunState()
{
	if (seqData.state == ENTRANCE)
	{
		return true;
	}
	else
	{
		return false;
	}

}

void MindControlScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);


	//sess->RemoveEnemy(seqCoyote);

	BasicBossScene::ReturnToGame();
}

void MindControlScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (seqData.state)
	{
	case FADE:
		if (seqData.frame == 0)
		{
			seqBird->Reset();
			sess->AddEnemy(seqBird);

			seqTiger->Reset();
			sess->AddEnemy(seqTiger);

			seqSkeleton->Reset();
			sess->AddEnemy(seqSkeleton);
			seqSkeleton->facingRight = false;

			sess->HideHUD();
			sess->cam.SetManual(true);
			MainMenu *mm = sess->mainMenu;
			sess->Fade(true, 60, Color::Black);
			SetCameraShot("scenecam");
			//enemyMover.SetDestNode(node);
		}
		break;
	case CONV1:
	{
		if (seqData.frame == 0)
		{
			SetConvGroup("conv1");
		}
		ConvUpdate();
		break;
	}
	case NEXUS_EXPLODE:
	{
		break;
	}
	case CONV2:
	{
		if (seqData.frame == 0)
		{
			SetConvGroup("conv2");
		}
		ConvUpdate();
		break;
	}
	case SKELETON_ENTRANCE:
	{
		break;
	}
	case CONV3:
	{
		if (seqData.frame == 0)
		{
			SetConvGroup("conv3");
		}
		ConvUpdate();
		break;
	}
	case SKELETON_FACE_PRE_MIND_CONTROL:
	{
		if (seqData.frame == 0)
		{
			Flash("skeleangry");
		}
		

		if (IsFlashDone("skeleangry"))
		{
			EndCurrState();
		}
		break;
	}
	case SKELETON_MIND_CONTROL:
	{
		if (seqData.frame == 0)
		{
			seqSkeleton->MindControl();
			seqBird->HitByMindControl();
			seqTiger->HitByMindControl();
		}
		break;
	}
	case CONV4:
	{
		if (seqData.frame == 0)
		{
			SetConvGroup("conv4");
		}
		ConvUpdate();
		break;
	}
	case MIND_CONTROL_FINISH:
	{
		break;
	}
	case CONV5:
	{
		if (seqData.frame == 0)
		{
			SetConvGroup("conv5");
		}
		ConvUpdate();
		break;
	}
	case BIRD_WALK_OVER_TO_SKELETON:
	{
		if (seqData.frame == 0)
		{
			seqBird->Walk(GetPointPos("birdwalk1"));
		}

		if (seqBird->action == SequenceBird::IDLE)
		{
			EndCurrState();
		}
		break;
	}
	case ENTRANCE:
	{
		if (seqData.frame == 0)
		{
			sess->FreezePlayer(false);
			SetEntranceRun();
		}
		break;
	}
	case CONV6:
	{
		if (seqData.frame == 0)
		{
			SetConvGroup("conv6");
		}
		ConvUpdate();
		break;
	}
	case SKELETON_JUMP_ONTO_BIRD:
	{
		if (seqData.frame == 0)
		{
			seqSkeleton->Hop(seqBird->GetPosition(), 5, 20);
		}

		if (seqSkeleton->action == SequenceSkeleton::IDLE)
		{
			EndCurrState();
		}
		break;
	}
	case TIGER_FACE_KIN:
	{
		if (seqData.frame == 0)
		{
			seqTiger->facingRight = false;
		}
		break;
	}
	case SKELETON_BIRD_EXIT:
	{
		if (seqData.frame == 0)
		{
			seqBird->FlyAwayWithSkeleton(GetPointPos("birdfly1"), 20);
			seqSkeleton->RideBird(seqBird);
		}
		break;
	}
	}
}

void MindControlScene::SetEntranceShot()
{
	//SetCameraShot("scenecam");
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
	switch (seqData.state)
	{
	case ENTRANCE:
		if (seqData.frame == 0)
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
	switch (seqData.state)
	{
	case FADE:
		if (seqData.frame == 0)
		{
			StartBasicKillFade();
		}
		else if (seqData.frame == explosionFadeFrames)
		{
			sess->SetGameSessionState(GameSession::RUN);
			SetPlayerStandPoint("kinstand0", true);
			SetCameraShot("tigerdeathcam");
			tiger->SeqWait();
		}
		break;
	case WAIT:
		if (seqData.frame == 0)
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