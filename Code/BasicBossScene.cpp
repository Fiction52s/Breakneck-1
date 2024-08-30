#include "Session.h"
#include "Actor.h"
#include "GameSession.h"
#include "HUD.h"
#include "Barrier.h"

#include "Sequence.h"
#include "SequenceW1.h"
#include "SequenceW2.h"
#include "SequenceW3.h"
#include "SequenceW4.h"
#include "SequenceW5.h"
#include "SequenceW6.h"
#include "SequenceW7.h"

using namespace sf;
using namespace std;

BasicBossScene *BasicBossScene::CreateScene(const std::string &name)
{
	BasicBossScene *bScene = NULL;
	if (name == "birdscene0")
	{
		bScene = new BirdPreFightScene;
	}
	if (name == "birdscene1")
	{
		bScene = new BirdPreFight2Scene;
	}
	else if (name == "postbirdfight")
	{
		bScene = new BirdPostFightScene;
	}
	else if (name == "postbirdfight2")
	{
		bScene = new BirdPostFight2Scene;
	}
	else if (name == "postbirdfight3")
	{
		bScene = new BirdPostFight3Scene;
	}
	else if (name == "crawlerscene0")
	{
		bScene = new CrawlerAttackScene;
	}
	else if (name == "crawlerscene1")
	{
		bScene = new CrawlerPreFightScene;
	}
	else if (name == "crawlerscene2")
	{
		bScene = new CrawlerPostFightScene;
	}
	else if (name == "postcrawlerfight1")
	{
		bScene = new CrawlerPostFightScene;
	}
	else if (name == "crawlerscene3")
	{
		bScene = new CrawlerPreFight2Scene;
	}
	else if (name == "birdcrawleralliance")
	{
		bScene = new BirdCrawlerAllianceScene;
	}
	else if (name == "birdcrawleralliancetransition")
	{
		bScene = new TransitionSequence("FinishedScenes/W2/alliance");
	}
	else if (name == "birdtigeralliancetransition")
	{
		bScene = new TransitionSequence("FinishedScenes/W4/birdtigeralliance");
	}
	else if (name == "birdtigerapproach")
	{
		bScene = new BirdTigerApproachScene;
	}
	else if (name == "coyotesleep")
	{
		bScene = new CoyoteSleepScene;
	}
	else if (name == "coyotescene1")
	{
		bScene = new CoyotePreFightScene;
	}
	else if (name == "coyotescene2")
	{
		bScene = new CoyoteAndSkeletonScene;
	}
	else if (name == "postcoyotefight")
	{
		bScene = new CoyotePostFightScene;
	}
	else if (name == "coyotew6entrance")
	{
		bScene = new EnterFortressScene;
	}
	else if (name == "tigerscene0")
	{
		bScene = new TigerPreFightScene;
	}
	else if (name == "birdtigeralliancescene")
	{
		bScene = new BirdTigerAllianceScene;
	}
	else if (name == "postcrawlerfight2")
	{
		bScene = new CrawlerPostFight2Scene;
	}
	else if (name == "crawlervstiger")
	{
		bScene = new CrawlerVSTigerScene;
	}
	else if (name == "birdtigervsscene")
	{
		bScene = new BirdVSTigerScene;
	}
	else if (name == "gatorscene0")
	{
		bScene = new GatorPreFightScene;
	}
	else if (name == "postgatorfight")
	{
		bScene = new GatorPostFightScene;
	}
	else if (name == "mindcontrol")
	{
		bScene = new MindControlScene;
	}
	else if (name == "birdchase")
	{
		bScene = new BirdChaseScene;
	}
	else if (name == "birdfinalfight")
	{
		bScene = new BirdPreFight3Scene;

	}
	else if (name == "finalskeletonfight")
	{
		bScene = new FinalSkeletonPreFightScene;
	}
	else if (name == "enterfortress")
	{
		bScene = new EnterFortressScene;
	}
	else if (name == "tigerbirdtunnel")
	{
		bScene = new TigerAndBirdTunnelScene;
	}
	else if (name == "skeletonfight")
	{
		bScene = new SkeletonPreFightScene;
	}
	else if (name == "tigerfight2")
	{
		bScene = new TigerPreFight2Scene;
	}
	else if (name == "posttigerfight")
	{
		bScene = new TigerPostFightScene;
	}
	else if (name == "postskeletonfight")
	{
		bScene = new SkeletonPostFightScene;
	}
	else
	{
		assert(0);
	}

	if (bScene != NULL)
	{
		bScene->Init();
		bScene->SetIDAndAddToAllSequencesVec();
	}
		

	return bScene;
}

BasicBossScene::BasicBossScene(
	EntranceType et)
{
	entranceType = et;
	fadeFrames = 60;
	explosionFadeFrames = 10;
	entranceIndex = 0;
}

BasicBossScene::~BasicBossScene()
{
}

void BasicBossScene::StartRunning()
{
	sess->SetPlayerInputOn(false);
}

void BasicBossScene::StartEntranceRun(bool fr,
	double maxSpeed, const std::string &n0,
	const std::string &n1)
{
	PoiInfo *kinStart = points[n0];
	PoiInfo *kinStop = points[n1];
	sess->GetPlayer(0)->SetStoryRun(fr, maxSpeed, kinStart->edge, kinStart->edgeQuantity, kinStop->edge,
		kinStop->edgeQuantity);
	sess->FreezePlayer(false);
}

void BasicBossScene::StartEntranceStand(bool fr,
	const std::string &n)
{
	PoiInfo *kinStand = points[n];
	assert(kinStand->edge != NULL);
	Actor *player = sess->GetPlayer(0);
	player->facingRight = fr;
	player->SetGroundedPos(kinStand->edge, kinStand->edgeQuantity);
	player->StandInPlace();
	sess->FreezePlayer(false);
}

void BasicBossScene::SetEntranceRun()
{
	string entranceIndexStr = to_string(entranceIndex);
	string startStr = "kinstart" + entranceIndexStr;
	string endStr = "kinstop" + entranceIndexStr;

	StartEntranceRun(true, 10.0, startStr, endStr);
}

void BasicBossScene::SetEntranceIndex(int ind)
{
	entranceIndex = ind;
}

void BasicBossScene::SetEntranceStand()
{
	string standString = "kinstand" + to_string(entranceIndex);

	StartEntranceStand(true, standString);
}

void BasicBossScene::AddStartPoint()
{
	string startStr = "kinstart" + to_string(entranceIndex);
	AddPoint(startStr);
}

void BasicBossScene::AddStopPoint()
{
	string stopStr = "kinstop" + to_string(entranceIndex);
	AddPoint(stopStr);
}

void BasicBossScene::AddStandPoint()
{
	string standStr = "kinstand" + to_string(entranceIndex);
	AddPoint(standStr);
}

void BasicBossScene::AddStartAndStopPoints()
{
	AddStartPoint();
	AddStopPoint();
}

void BasicBossScene::SetEntranceShot()
{
	//if (shots.count("scenecam") == 1)
	{
		SetCameraShot("scenecam");
	}
	//else
	{
		//assert(0);
	}
}

void BasicBossScene::EntranceUpdate()
{
	Actor *player = sess->GetPlayer(0);

	//sess->FreezePlayerAndEnemies(false);
	sess->FreezePlayer(false);
	if (entranceType == RUN)
	{
		if (seqData.frame == 0)
		{
			sess->Fade(false, fadeFrames, Color::Black);
			sess->HideHUD(fadeFrames);
			player->Wait();
			sess->cam.SetManual(true);
		}
		else if (seqData.frame == fadeFrames)
		{
			//barrier->Trigger();
			sess->RemoveAllEnemies();
			sess->Fade(true, fadeFrames, Color::Black);
			SetEntranceShot();
			SetEntranceRun();
		}
	}
	else if (entranceType == STARTMAP_RUN)
	{
		if (seqData.frame == 0)
		{


			/*if (barrier != NULL)
			{
				barrier->Trigger();
			}*/

			sess->HideHUD();
			player->Wait();
			sess->cam.SetManual(true);
			//		sess->RemoveAllEnemies();
			sess->Fade(true, fadeFrames, Color::Black);
			SetEntranceShot();
			SetEntranceRun();
		}
	}
	else if (entranceType == APPEAR)
	{
		//owner->HideHUD(fadeFrames);
	}
}

void BasicBossScene::SetPlayerStandPoint(const std::string &n,
	bool fr)
{
	assert(points.count(n) == 1);

	PoiInfo *pi = points[n];
	Actor *player = sess->GetPlayer(0);
	sess->FreezePlayer(false);
	player->SetStandInPlacePos(pi->edge, pi->edgeQuantity, fr);
}

void BasicBossScene::SetPlayerStandDefaultPoint(bool fr)
{
	string standStr = "kinstand" + to_string(entranceIndex);
	SetPlayerStandPoint(standStr, fr);
}

void BasicBossScene::ReturnToGame()
{
	sess->SetPlayerInputOn(true);
	sess->ShowHUD(60);
	sess->cam.EaseOutOfManual(60);
}

bool BasicBossScene::IsAutoRunState()
{
	if (entranceType == RUN || entranceType == STARTMAP_RUN)
	{
		return seqData.state == 0;
	}
	else
	{
		return false;
	}

}

void BasicBossScene::Wait()
{
	seqData.state = 1;
	seqData.frame = 0;
}

bool BasicBossScene::Update()
{
	Actor *player = sess->GetPlayer(0);

	if (!StateIncrement())
		return false;

	if (IsAutoRunState() && !player->IsAutoRunning() && seqData.frame > 60)
	{
		seqData.state++;
		seqData.frame = 0;
		//Wait();
	}

	UpdateFlashes();

	UpdateFlashGroup();

	UpdateState();

	++seqData.frame;

	return true;
}

void BasicBossScene::LayeredDraw( int p_drawLayer, sf::RenderTarget *target )
{
	Sequence::LayeredDraw(p_drawLayer, target);
}

void BasicBossScene::StartBasicKillFade()
{
	sess->SetGameSessionState(GameSession::FROZEN);
	sess->HideHUD(explosionFadeFrames);
	sess->cam.SetManual(true);
	MainMenu *mm = sess->mainMenu;
	sess->CrossFade(explosionFadeFrames, 0, fadeFrames, Color::White);
}

void BasicBossScene::StartBasicNewMapKillFade()
{
	sess->SetGameSessionState(GameSession::SEQUENCE);
	sess->HideHUD();
	sess->cam.SetManual(true);
	MainMenu *mm = sess->mainMenu;
	sess->Fade(false, explosionFadeFrames, Color::White, false, DrawLayer::IN_FRONT);
}

void BasicBossScene::EndBasicNewMapKillFade()
{
	sess->Fade(true, 30, Color::White, false, DrawLayer::IN_FRONT);
	sess->SetGameSessionState(GameSession::RUN);
}