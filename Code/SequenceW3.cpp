#include "SequenceW3.h"
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

CoyoteSleepScene::CoyoteSleepScene()
	:BasicBossScene(BasicBossScene::RUN)
{

}

void CoyoteSleepScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[COYOTEWAKE] = -1;
}

void CoyoteSleepScene::AddShots()
{
	AddShot("sleepcam");
}

void CoyoteSleepScene::AddPoints()
{
	AddStartAndStopPoints();
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

	group->SetBG(bgs["anim1"]);

	AddSimulFlashToGroup(group, "break_01");
	AddSimulFlashToGroup(group, "break_02", 90);
	AddSimulFlashToGroup(group, "break_03", 90);
	AddSimulFlashToGroup(group, "break_04", 90);
	AddSimulFlashToGroup(group, "break_05", 90);



	/*AddFlashedImage("wake0", owner->GetTileset("Bosses/Coyote/Coy_02b.png", 1920, 1080),
		0, 30, 30, 30, Vector2f(960, 540));

	AddFlashedImage("wake1", owner->GetTileset("Bosses/Coyote/Coy_03b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));


	AddFlashedImage("wake2", owner->GetTileset("Bosses/Coyote/Coy_04b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));

	AddFlashedImage("wake3", owner->GetTileset("Bosses/Coyote/Coy_05b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));*/


	int togetherFrames = 15;

	/*AddSeqFlashToGroup(group, "wake0");
	AddSeqFlashToGroup(group, "wake1", togetherFrames);
	AddSeqFlashToGroup(group, "wake2", togetherFrames);
	AddSeqFlashToGroup(group, "wake3", togetherFrames);*/


	/*AddSeqFlashToGroup(group, "wake0" );
	AddSimulFlashToGroup(group, "wake1", 15);
	AddSeqFlashToGroup(group, "wake2", 0);
	AddSeqFlashToGroup(group, "wake3", togetherFrames);*/
	group->Init();
}

void CoyoteSleepScene::ReturnToGame()
{
	Actor *player = sess->GetPlayer(0);

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
	}
}

void CoyoteSleepScene::SetEntranceShot()
{
	SetCameraShot("sleepcam");
}

CoyotePreFightScene::CoyotePreFightScene()
	:BasicBossScene(BasicBossScene::RUN)
{
	SetEntranceIndex(1);
}

void CoyotePreFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[COYOTECONV] = -1;
	stateLength[COYOTEFACES] = -1;

	CoyotePostFightScene *scene = new CoyotePostFightScene;
	scene->Init();
	nextSeq = scene;
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
			sess->ReverseDissolveGates(Gate::BOSS);
		}
		break;
	}
	}
}

CoyotePostFightScene::CoyotePostFightScene()
	:BasicBossScene(BasicBossScene::APPEAR)
{
}

void CoyotePostFightScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
	stateLength[WAIT] = 60;
	stateLength[COYOTECONV] = -1;
	stateLength[NEXUSEXPLODE] = 60;
	stateLength[COYOTELEAVE] = 30;
}

void CoyotePostFightScene::ReturnToGame()
{
	sess->cam.EaseOutOfManual(60);
	//owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
	BasicBossScene::ReturnToGame();
}

void CoyotePostFightScene::AddShots()
{
	AddShot("convcam");
}

void CoyotePostFightScene::AddPoints()
{

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
	:BasicBossScene(BasicBossScene::RUN)
{
	SetEntranceIndex(2);
}

void CoyoteAndSkeletonScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 60;
	stateLength[SHOWIMAGE] = -1;
	stateLength[SKELECOYCONV] = -1;
}

void CoyoteAndSkeletonScene::AddShots()
{
	AddShot("skelecam");
}

void CoyoteAndSkeletonScene::AddPoints()
{
	AddStartAndStopPoints();
}

void CoyoteAndSkeletonScene::AddGroups()
{
	AddGroup("talk", "W3/w3_coy_skeleton");
	SetConvGroup("talk");
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
}

void CoyoteAndSkeletonScene::UpdateState()
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
	case SHOWIMAGE:
	{
		if (frame == 0)
		{
			Flash("screen0");
			Flash("screen1");
		}
		else if(flashes["screen0"]->IsDone() && flashes["screen1"]->IsDone())
		{
			EndCurrState();	
		}
		break;
	}
	case SKELECOYCONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			player->EndLevelWithoutGoal();
		}
		break;
	}
}

void CoyoteAndSkeletonScene::SetEntranceShot()
{
	SetCameraShot("skelecam");
}