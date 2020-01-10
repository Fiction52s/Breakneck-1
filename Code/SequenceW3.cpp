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

CoyoteSleepScene::CoyoteSleepScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::RUN)
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
	AddFlashedImage("wake0", owner->GetTileset("Bosses/Coyote/Coy_02b.png", 1920, 1080),
		0, 30, 30, 30, Vector2f(960, 540));

	AddFlashedImage("wake1", owner->GetTileset("Bosses/Coyote/Coy_03b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));

	AddFlashedImage("wake2", owner->GetTileset("Bosses/Coyote/Coy_04b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));

	AddFlashedImage("wake3", owner->GetTileset("Bosses/Coyote/Coy_05b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));

	FlashGroup * group = AddFlashGroup("wakegroup");
	AddFlashToGroup(group, "wake0");
	AddFlashToGroup(group, "wake1");
	AddFlashToGroup(group, "wake2");
	AddFlashToGroup(group, "wake3");
}

void CoyoteSleepScene::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);

	BasicBossScene::ReturnToGame();
}

void CoyoteSleepScene::UpdateState()
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

CoyotePreFightScene::CoyotePreFightScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::RUN)
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

	CoyotePostFightScene *scene = new CoyotePostFightScene(owner);
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
	AddFlashedImage("stare0", owner->GetTileset("Bosses/Coyote/Coy_09b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));

	AddFlashedImage("stare1", owner->GetTileset("Bosses/Coyote/Coy_10b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));

	AddFlashedImage("stare2", owner->GetTileset("Bosses/Coyote/Coy_11b.png", 1920, 1080),
		0, 30, 10, 30, Vector2f(960, 540));

	FlashGroup * group = AddFlashGroup("staregroup");
	AddFlashToGroup(group, "stare0");
	AddFlashToGroup(group, "stare1");
	AddFlashToGroup(group, "stare2");
}

void CoyotePreFightScene::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);

	BasicBossScene::ReturnToGame();
}

void CoyotePreFightScene::UpdateState()
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
	
	case COYOTECONV:
		ConvUpdate();
		if (IsLastFrame())
		{
			owner->ReverseDissolveGates(Gate::CRAWLER_UNLOCK);
		}
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
		break;
	}
	}
}

CoyotePostFightScene::CoyotePostFightScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::APPEAR)
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
	owner->cam.EaseOutOfManual(60);
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
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case FADE:
		if (state == FADE)
		{
			if (frame == 0)
			{
				owner->adventureHUD->Hide(fadeFrames);
				//player->Wait();
				owner->cam.SetManual(true);
				MainMenu *mm = owner->mainMenu;
				owner->CrossFade(10, 0, 60, Color::White);
			}
		}
	case WAIT:
		if (frame == 0)
		{
			owner->TotalDissolveGates(Gate::CRAWLER_UNLOCK);
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

CoyoteAndSkeletonScene::CoyoteAndSkeletonScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::RUN)
{
	SetEntranceIndex(2);
}

void CoyoteAndSkeletonScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
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

	AddFlashedImage("screen0", owner->GetTileset("Bosses/Coyote/coyskeleton1.png", 1920, 1080),
		0, fadeInFrames, holdFrames, fadeOutFrames, Vector2f(960, 540));
	flashes["screen0"]->AddPanY(scrollRate, fadeInFrames, scrollFrames);

	AddFlashedImage("screen1", owner->GetTileset("Bosses/Coyote/coyskeleton2.png", 1920, 1080),
		0, fadeInFrames, holdFrames, fadeOutFrames, Vector2f(960, 540 - 1080));
	flashes["screen1"]->AddPanY(scrollRate, fadeInFrames, scrollFrames);
	//flashes["screen1"]->SetDetailedPanY(0, scrollRate, 0);
}

void CoyoteAndSkeletonScene::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);

	BasicBossScene::ReturnToGame();
}

void CoyoteAndSkeletonScene::UpdateState()
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