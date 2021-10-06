#include "SequenceW3.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "ScoreDisplay.h"
#include "Enemy_Coyote.h"
#include "GroundedWarper.h"
#include "Enemy_SequenceCoyote.h"
#include "Enemy_SequenceSkeleton.h"

using namespace sf;
using namespace std;

CoyoteSleepScene::CoyoteSleepScene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
	seqCoyote = NULL;
}

void CoyoteSleepScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[COYOTEWAKE] = -1;
	stateLength[COYOTELEAVE] = 120;

	seqCoyote = (SequenceCoyote*)sess->GetEnemy(EnemyType::EN_SEQUENCECOYOTE);
}

void CoyoteSleepScene::AddShots()
{
	AddShot("sleepcam");
}

void CoyoteSleepScene::AddPoints()
{
	AddStartAndStopPoints();

	AddPoint("coyotedest");
}

void CoyoteSleepScene::AddGroups()
{
	AddGroup("sleep", "W3/w3_coy_sleep");
	SetConvGroup("sleep");
}

void CoyoteSleepScene::AddEnemies()
{
}

void CoyoteSleepScene::AddFlashes()
{
	
	list<Tileset*> bgTiles;
	bgTiles.push_back(sess->GetTileset("Story/test/BG_01.png", 1920, 1080));
	bgTiles.push_back(sess->GetTileset("Story/test/BG_02.png", 1920, 1080));
	bgTiles.push_back(sess->GetTileset("Story/test/BG_03.png", 1920, 1080));
	AddBG("anim1", bgTiles, 4);

	FlashGroup * group = AddFlashGroup("wakegroup");

	/*FlashedImage *testIm = AddFlashedImage("wake0", owner->GetTileset("Story/test/Pingpong_Test_01b_1920x1080.png", 1920, 1080),
		0, 30, 240, 30, Vector2f(960, 540));
	testIm->SetSplit(owner->GetTileset("Story/test/Pingpong_Test_01aa_1920x1080.png", 1920, 1080), NULL,0,
		Vector2f(960, 540));
	testIm->AddPanX(-1000, 0, 240);

	FlashedImage *testIm2 = AddFlashedImage("wake1", owner->GetTileset("Story/test/Pingpong_Test_02b_1920x1080.png", 1920, 1080),
		0, 30, 240, 30, Vector2f(960, 540));
	testIm2->SetSplit(owner->GetTileset("Story/test/Pingpong_Test_02aa_1920x1080.png", 1920, 1080), NULL, 0, 
		Vector2f(960, 540));
	testIm2->AddPanX(1000, 0, 240);

	AddSimulFlashToGroup(group, "wake0");
	AddSimulFlashToGroup(group, "wake1", 90);*/

	FlashedImage *testIm1 = AddFlashedImage("break_01", sess->GetTileset("Story/test/break_01.png", 1920, 1080),
	0, 0, 0, 0, Vector2f(960, 540));
	testIm1->SetSplit(sess->GetTileset("Story/test/break_01_split.png", 1920, 1080),
		sess->GetTileset("Story/test/break_01_line.png", 1920, 1080), 0,Vector2f(960, 540));
	testIm1->SetBG(GetBG("anim1"));


	FlashedImage *testIm2 = AddFlashedImage("break_02", sess->GetTileset("Story/test/break_02.png", 1920, 1080),
		0, 0, 0, 0, Vector2f(960, 540));
	testIm2->SetSplit(sess->GetTileset("Story/test/break_02_split.png", 1920, 1080),
		sess->GetTileset("Story/test/break_02_line.png", 1920, 1080), 0, Vector2f(960, 540));

	FlashedImage *testIm3 = AddFlashedImage("break_03", sess->GetTileset("Story/test/break_03.png", 1920, 1080),
		0, 0, 0, 0, Vector2f(960, 540));
	testIm3->SetSplit(sess->GetTileset("Story/test/break_03_split.png", 1920, 1080),
		sess->GetTileset("Story/test/break_03_line.png", 1920, 1080), 0, Vector2f(960, 540));
	

	FlashedImage *testIm4 = AddFlashedImage("break_04", sess->GetTileset("Story/test/break_04.png", 1920, 1080),
		0, 0, 0, 0, Vector2f(960, 540));
	testIm4->SetSplit(sess->GetTileset("Story/test/break_04_split.png", 1920, 1080),
		sess->GetTileset("Story/test/break_04_line.png", 1920, 1080), 0, Vector2f(960, 540));
	testIm4->SetBG(GetBG("anim1"));

	FlashedImage *testIm5 = AddFlashedImage("break_05", sess->GetTileset("Story/test/break_05.png", 1920, 1080),
		0, 0, 240, 0, Vector2f(960, 540));

	/*group->SetBG(bgs["anim1"]);

	AddSimulFlashToGroup(group, "break_01");
	AddSimulFlashToGroup(group, "break_02", 90);
	AddSimulFlashToGroup(group, "break_03", 90);
	AddSimulFlashToGroup(group, "break_04", 90);
	AddSimulFlashToGroup(group, "break_05", 90);*/



	AddFlashedImage("wake0", sess->GetTileset("Bosses/Coyote/Coy_02b.png", 1920, 1080),
		0, 30, 30, 30, Vector2f(960, 540));

	AddFlashedImage("wake1", sess->GetTileset("Bosses/Coyote/Coy_03b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));


	AddFlashedImage("wake2", sess->GetTileset("Bosses/Coyote/Coy_04b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));

	AddFlashedImage("wake3", sess->GetTileset("Bosses/Coyote/Coy_05b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));


	int togetherFrames = 15;

	AddSeqFlashToGroup(group, "wake0");
	AddSeqFlashToGroup(group, "wake1", togetherFrames);
	AddSeqFlashToGroup(group, "wake2", togetherFrames);
	AddSeqFlashToGroup(group, "wake3", togetherFrames);


	/*AddSeqFlashToGroup(group, "wake0" );
	AddSimulFlashToGroup(group, "wake1", 15);
	AddSeqFlashToGroup(group, "wake2", 0);
	AddSeqFlashToGroup(group, "wake3", togetherFrames);*/
	group->Init();
}

void CoyoteSleepScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	sess->RemoveEnemy(seqCoyote);

	BasicBossScene::ReturnToGame();
}

void CoyoteSleepScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		if (frame == 0)
		{
			seqCoyote->Reset();
			sess->AddEnemy(seqCoyote);
			seqCoyote->facingRight = false;
			seqCoyote->Sleep();
			
			//enemyMover.SetDestNode(node);
		}

		EntranceUpdate();
		break;
	case COYOTEWAKE:
		if (frame == 0)
		{
			SetFlashGroup("wakegroup");
		}
		else
		{
			if (currFlashGroup == NULL)
			{
				EndCurrState();
			}
		}
		break;
	case COYOTELEAVE:
	{
		if (frame == 0)
		{
			seqCoyote->Walk(GetPointPos("coyotedest"));
		}
		break;
	}
	}
}

void CoyoteSleepScene::SetEntranceShot()
{
	SetCameraShot("sleepcam");
}

CoyotePreFightScene::CoyotePreFightScene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
	SetEntranceIndex(0); //change this to 1 if its the 2nd one in a level again.
}

void CoyotePreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	//stateLength[WAIT] = 1;
	stateLength[COYOTECONV] = -1;
	stateLength[COYOTEFACES] = -1;

	//CoyotePostFightScene *scene = new CoyotePostFightScene;
	//scene->Init();
	//nextSeq = scene;

	//attempt to get coy. will work for prelevel scene
	coy = (Coyote*)sess->GetEnemy(EnemyType::EN_COYOTEBOSS);
	if (coy != NULL)
	{

	}
}

void CoyotePreFightScene::AddShots()
{
	AddShot("scenecam");
	//AddShot("fightcam");
}

void CoyotePreFightScene::AddPoints()
{
	AddStartAndStopPoints();
}

void CoyotePreFightScene::AddGroups()
{
	AddGroup("prefight", "W3/w3_coy_fight_pre");
	SetConvGroup("prefight");
}

void CoyotePreFightScene::AddEnemies()
{
}

void CoyotePreFightScene::AddFlashes()
{
	int togetherFrames = 5;

	AddFlashedImage("stare0", sess->GetTileset("Bosses/Coyote/Coy_09b.png", 1920, 1080),
		0, 30, 20, 30, Vector2f(960, 540));

	AddFlashedImage("stare1", sess->GetTileset("Bosses/Coyote/Coy_10b.png", 1920, 1080),
		0, 30, 30, 30, Vector2f(960, 540));

	AddFlashedImage("stare2", sess->GetTileset("Bosses/Coyote/Coy_11b.png", 1920, 1080),
		0, 30, 20, 30, Vector2f(960, 540));

	FlashGroup * group = AddFlashGroup("staregroup");
	AddSeqFlashToGroup(group, "stare0", 0);
	AddSeqFlashToGroup(group, "stare1", -togetherFrames);
	AddSeqFlashToGroup(group, "stare2", -togetherFrames);
	group->Init();
}

void CoyotePreFightScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();
}

void CoyotePreFightScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		if (frame == 0)
		{
			sess->AddEnemy(coy);
			coy->SeqWait();
		}
		EntranceUpdate();
		break;
	case COYOTECONV:
		ConvUpdate();
		break;
	case COYOTEFACES:
	{
		if (frame == 0)
		{
			SetFlashGroup("staregroup");
		}
		else
		{
			if (currFlashGroup == NULL)
			{
				EndCurrState();
			}
		}

		if (IsLastFrame())
		{
			coy->StartFight();
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
	}
}

CoyotePostFightScene::CoyotePostFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
	//coy = NULL;//set by coyote when he makes this scene
	seqCoyote = NULL;
	warper = NULL;
	//assert(warper != NULL);
}

void CoyotePostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = fadeFrames + explosionFadeFrames;
	stateLength[WAIT] = 60;
	stateLength[COYOTECONV] = -1;
	stateLength[NEXUSEXPLODE] = 60;
	stateLength[COYOTELEAVE] = 30;

	seqCoyote = (SequenceCoyote*)sess->GetEnemy(EnemyType::EN_SEQUENCECOYOTE);
	warper = sess->GetWarper("FinishedScenes/W3/coyoteandskeleton");
}

void CoyotePostFightScene::ReturnToGame()
{
	if (!warper->spawned)
	{
		sess->AddEnemy(warper);
	}
	warper->Activate();
	sess->cam.EaseOutOfManual(60);
	sess->RemoveEnemy(seqCoyote);
	//owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
	BasicBossScene::ReturnToGame();
}

void CoyotePostFightScene::AddShots()
{
	//AddShot("convcam");
	AddShot("scenecam");
}

void CoyotePostFightScene::AddPoints()
{
	AddStopPoint();
}

void CoyotePostFightScene::AddFlashes()
{

}

void CoyotePostFightScene::AddEnemies()
{

}

void CoyotePostFightScene::AddGroups()
{
	AddGroup("post_fight", "W3/w3_coy_fight_post");
	SetConvGroup("post_fight");
}

void CoyotePostFightScene::UpdateState()
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
			SetPlayerStandPoint("kinstop0", true);
			SetCameraShot("scenecam");
			
			seqCoyote->Reset();
			sess->AddEnemy(seqCoyote);
			
			//coy->SeqWait();
		}
		break;
	case WAIT:
		if (frame == 0)
		{
			sess->TotalDissolveGates(Gate::BOSS);
		}
		//EntranceUpdate();
		break;
	case COYOTECONV:
		ConvUpdate();
		break;
	case NEXUSEXPLODE:
		RumbleDuringState(3, 3);
		break;
		//case COYOTECONV1:

		//	ConvUpdate();
		//	break;
	case COYOTELEAVE:
		break;
	}
}

CoyoteAndSkeletonScene::CoyoteAndSkeletonScene()
	:BasicBossScene(BasicBossScene::STARTMAP_RUN)
{
	SetEntranceIndex(0);

	seqCoyote = NULL;
	seqSkeleton = NULL;
}

void CoyoteAndSkeletonScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 60;
	stateLength[SHOWIMAGE] = -1;
	stateLength[CONV1] = -1;
	stateLength[SKELETONLASER] = 60;
	stateLength[CONV2] = -1;
	stateLength[COYOTERETREAT] = 180;
	stateLength[SKELETONAPPROACH] = 120;
	stateLength[CONV3] = -1;
	stateLength[SKELETONEXIT] = 180;//-1;//120;

	seqCoyote = (SequenceCoyote*)sess->GetEnemy(EnemyType::EN_SEQUENCECOYOTE);
	seqSkeleton = (SequenceSkeleton*)sess->GetEnemy(EnemyType::EN_SEQUENCESKELETON);
}

void CoyoteAndSkeletonScene::AddShots()
{
	AddShot("skelecam");
}

void CoyoteAndSkeletonScene::AddPoints()
{
	AddStartAndStopPoints();

	AddPoint("skelestop");
}

void CoyoteAndSkeletonScene::AddGroups()
{
	AddGroup("conv1", "W3/w3_coy_skeleton_1");
	AddGroup("conv2", "W3/w3_coy_skeleton_2");
	AddGroup("conv3", "W3/w3_coy_skeleton_3");
	//SetConvGroup("conv1");
}

void CoyoteAndSkeletonScene::AddEnemies()
{
}

void CoyoteAndSkeletonScene::AddFlashes()
{
	float scrollAmount = 1080;
	float scrollFrames = 4 * 60;
	float scrollRate = scrollAmount / scrollFrames;

	int fadeOutFrames = 60;
	int fadeInFrames = 60;

	int holdFrames = scrollFrames + 60;

	AddFlashedImage("screen0", sess->GetTileset("Bosses/Coyote/Coy_Pan_02a.png", 1920, 1080),
		0, fadeInFrames, holdFrames, fadeOutFrames, Vector2f(960, 540))
		->AddPanY(scrollAmount, fadeInFrames, scrollFrames);

	AddFlashedImage("screen1", sess->GetTileset("Bosses/Coyote/Coy_Pan_02b.png", 1920, 1080),
		0, fadeInFrames, holdFrames, fadeOutFrames, Vector2f(960, 540 - 1080))
		->AddPanY(scrollAmount, fadeInFrames, scrollFrames);;
	
}

void CoyoteAndSkeletonScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

	BasicBossScene::ReturnToGame();

	sess->RemoveEnemy(seqCoyote);
	sess->RemoveEnemy(seqSkeleton);

	player->EndLevelWithoutGoal();
}

void CoyoteAndSkeletonScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:

		if (frame == 0)
		{
			seqCoyote->Reset();
			sess->AddEnemy(seqCoyote);

			seqSkeleton->Reset();
			sess->AddEnemy(seqSkeleton);
			seqSkeleton->facingRight = false;
		}

		EntranceUpdate();
		break;
	case SHOWIMAGE:
	{
		if (frame == 0)
		{
			Flash("screen0");
			Flash("screen1");
		}
		else if(IsFlashDone("screen0") && IsFlashDone( "screen1" ))
		{
			EndCurrState();	
		}
		break;
	}
	case CONV1:
	{
		if (frame == 0)
		{
			SetConvGroup("conv1");
		}
		ConvUpdate();
		break;
	}
	case SKELETONLASER:
	{
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
	case COYOTERETREAT:
	{
		if (frame == 0)
		{
			seqCoyote->SummonScorpion();
		}

		if (seqCoyote->action == SequenceCoyote::SCORPION_STAND)
		{
			seqCoyote->Bounce(seqCoyote->GetPosition() + V2d(-1000, 0));
		}
		break;
	}
	case SKELETONAPPROACH:
	{
		if (frame == 0)
		{
			seqSkeleton->Walk(GetPointPos("skelestop"));
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
	case SKELETONEXIT:
	{

		if (frame == 0)
		{
			seqSkeleton->WireThrow(seqSkeleton->GetPosition() + V2d(1000, -1000));
		}

		if (seqSkeleton->action == SequenceSkeleton::WIRE_IDLE)
		{
			seqSkeleton->WirePull();
		}


		break;
	}
		
	}
}

void CoyoteAndSkeletonScene::SetEntranceShot()
{
	SetCameraShot("skelecam");
}