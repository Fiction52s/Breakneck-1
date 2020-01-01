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

BasicBossScene::BasicBossScene(GameSession *p_owner)
	:owner(p_owner)
{
	barrier = NULL;
	currConvGroup = NULL;
	fadeFrames = 60;
	state = 0;
	numStates = 0;
	stateLength = NULL;
}

void BasicBossScene::Reset()
{
	state = 0;
	frame = 0;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		(*it).second->Reset();
	}
	cIndex = 0;
}

BasicBossScene *BasicBossScene::CreateScene( GameSession *owner, const std::string &name)
{
	if (name == "birdscene0")
	{
		return new BirdBossScene(owner);
	}
	else
	{
		assert(0);
	}
}

void BasicBossScene::AddGroup(const std::string &groupName, const std::string &fileName )
{
	ConversationGroup *cg = new ConversationGroup(owner);
	cg->Load(fileName);

	assert(groups.count(groupName) == 0);

	groups[groupName] = cg;
}

void BasicBossScene::AddShot(const std::string &shotName)
{
	shots[shotName] = owner->cameraShotMap[shotName];
}

void BasicBossScene::AddPoint(const std::string &poiName)
{
	points[poiName] = owner->poiMap[poiName];
}

void BasicBossScene::ConvUpdate()
{
	Conversation *conv = currConvGroup->GetConv(cIndex);
	if (frame == 0)
	{
		conv->Show();
	}

	if (owner->GetCurrInput(0).A && !owner->GetPrevInput(0).A)
	{
		conv->NextSection();
	}
	if (owner->GetCurrInput(0).B)
	{
		conv->SetRate(1, 5);
	}
	else
	{
		conv->SetRate(1, 1);
	}

	if (!conv->Update())
	{
		if (cIndex < currConvGroup->numConvs - 1)
		{
			++cIndex;

			Conversation *newconv = currConvGroup->GetConv(cIndex);

			newconv->Show();
		}
		else
		{
			frame = stateLength[state] - 1;
		}
	}
}

BasicBossScene::~BasicBossScene()
{
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete (*it).second;
	}

	if (stateLength != NULL)
		delete[] stateLength;
}

void BasicBossScene::Init()
{

}

void BasicBossScene::StartEntranceRun(bool fr,
	double maxSpeed, const std::string &n0,
	const std::string &n1)
{
	PoiInfo *kinStart = points[n0];
	PoiInfo *kinStop = points[n1];
	owner->GetPlayer(0)->SetStoryRun(fr, maxSpeed, kinStart->edge, kinStart->edgeQuantity, kinStop->edge,
		kinStop->edgeQuantity);
}

void BasicBossScene::SetCameraShot(const std::string &n)
{
	CameraShot *shot = shots[n];
	owner->cam.Set(shot->centerPos, shot->zoom, owner->cam.zoomLevel);
}

void BasicBossScene::SetEntranceRun()
{
	StartEntranceRun(true, 10.0, "kinstart", "kinstop");
}

void BasicBossScene::SetEntranceShot()
{
	SetCameraShot("scenecam");
}

void BasicBossScene::EntranceUpdate()
{
	Actor *player = owner->GetPlayer(0);
	if (frame == 0)
	{
		owner->Fade(false, fadeFrames, Color::Black);
		owner->adventureHUD->Hide(fadeFrames);
		player->Wait();
		owner->cam.SetManual(true);
	}
	else if (frame == fadeFrames)
	{
		barrier->Trigger();
		owner->RemoveAllEnemies();
		owner->Fade(true, fadeFrames, Color::Black);
		SetEntranceShot();
		SetEntranceRun();
	}

	//if (!barrier->triggered && barrier->GetPlayerDist() < -100)
	//{
	//	player->Wait();
	//}
}

void BasicBossScene::ReturnToGame()
{
	owner->cutPlayerInput = false;
	owner->adventureHUD->Show(60);
	owner->cam.EaseOutOfManual(60);
}

bool BasicBossScene::IsEntering()
{
	return state == 0;
}

void BasicBossScene::Wait()
{
	state = 1;
	frame = 0;
}

bool BasicBossScene::Update()
{
	Actor *player = owner->GetPlayer(0);

	if (frame == stateLength[state] && state != numStates)
	{
		++state;
		frame = 0;
	}

	if (state == numStates)
	{
		ReturnToGame();
		return false;
	}

	if (IsEntering() && !player->IsAutoRunning() && frame > 60 )
	{
		Wait();
	}

	UpdateState();

	++frame;

	return true;
}

void BasicBossScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	View v = target->getView();
	target->setView(owner->uiView);

	Conversation *conv = currConvGroup->GetConv(cIndex);

	conv->Draw(target);
	target->setView(v);
}

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