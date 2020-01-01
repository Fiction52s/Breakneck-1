#include "SequenceW2.h"
#include "Actor.h"
#include "GameSession.h"
#include "MainMenu.h"
#include "Config.h"
#include "MusicPlayer.h"
#include "ImageText.h"
#include "HUD.h"
#include "Barrier.h"

using namespace std;
using namespace sf;



BirdBossScene::BirdBossScene(GameSession *p_owner)
	:BasicBossScene(p_owner)
{
	AddShot("scenecam");
	AddPoint("kinstart");
	AddPoint("kinstop");

	AddGroup("birdfightpre", "W2/w2_bird_fight_pre");
	AddGroup("birdfightpost", "W2/w2_bird_fight_post");

	currConvGroup = groups["birdfightpre"];

	SetupStates();

	Reset();
}

void BirdBossScene::SetupStates()
{
	numStates = Count;
	stateLength = new int[numStates];

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 60;
	stateLength[TALK] = -1;
}

void BirdBossScene::UpdateState()
{
	switch (state)
	{
		case ENTRANCE:
		{
			EntranceUpdate();
			break;
		}
		case TALK:
		{
			ConvUpdate();
			break;
		}
	}
}

CoyoteBossScene::CoyoteBossScene(GameSession *p_owner)
	:BasicBossScene(p_owner)
{
	AddShot("scenecam");
	AddPoint("kinstart");
	AddPoint("kinstop");

	AddGroup( "pre_coy", "W3/w3_coy_fight_pre");

	currConvGroup = groups["pre_coy"];

	SetupStates();

	Reset();
}

void CoyoteBossScene::SetupStates()
{
	numStates = Count;
	stateLength = new int[numStates];

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 60;
	stateLength[TALK] = -1;
}

void CoyoteBossScene::UpdateState()
{
	switch (state)
	{
	case ENTRANCE:
	{
		EntranceUpdate();
		break;
	}
	case TALK:
	{
		ConvUpdate();
		break;
	}
	}
}