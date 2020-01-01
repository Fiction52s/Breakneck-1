#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "poly2tri/poly2tri.h"
#include "VectorMath.h"
#include "Camera.h"
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
//#include "EditSession.h"
#include "Zone.h"
#include "Flow.h"
#include "Boss.h"
#include "PowerOrbs.h"
#include "SequenceW1.h"
#include "SoundManager.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "PauseMenu.h"
#include "Parallax.h"
#include "Movement.h"
#include "ScoreDisplay.h"
#include "Enemy_CrawlerQueen.h"
#include "HUD.h"
#include "ImageText.h"

using namespace sf;
using namespace std;

#define TIMESTEP 1.0 / 60.0

CrawlerAttackSeq::CrawlerAttackSeq(GameSession *p_owner)
	:BasicBossScene( p_owner )
{
	AddShot("scenecam");
	AddShot("cavecam");
	AddShot("fightcam");

	AddPoint("kinstart");
	AddPoint("kinstop");

	AddGroup("pre_coy", "W3/w3_coy_fight_pre");

	currConvGroup = groups["pre_coy"];

	SetupStates();

	AddPoint("crawlerdig1");
	AddPoint("crawlerdig2");
	AddPoint("crawlersurface");
	AddPoint("crawlerthrowkin");

	ts_queenGrab = owner->GetTileset("Bosses/Crawler/crawler_queen_grab_320x320.png", 320, 320);
	queenGrabSprite.setTexture(*ts_queenGrab->texture);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(0));

	detailedGrab = new FlashedImage(owner->GetTileset("Story/Crawler_Dig_01_860x830.png", 860, 830),
		0, 30, 60, 30, Vector2f(1160, 540));//Vector2f( 1500, 500 ));

	crawlerFace = new FlashedImage(owner->GetTileset("Story/Crawler_Dig_02_828x875.png", 828, 875),
		0, 30, 60, 30, Vector2f(1350, 325));//Vector2f(960, 540));//Vector2f( 1500, 500 ));

	kinFace = new FlashedImage(owner->GetTileset("Story/Crawler_Dig_03_510x565.png", 510, 565),
		0, 30, 60, 30, Vector2f(625, 325));//Vector2f( 1500, 500 ));
	//ts_detailedGrab = owner->GetTileset("Bosses/Crawler/");
	//detailedGrabSpr.setTexture(*ts_detailedGrab);
	//detailedGrabSpr.setTextureRect(ts_detailedGrab->GetSubRect(0));
	//detailedGrabSpr.setOrigin(detailedGrabSpr.getLocalBounds().width / 2,
	//	detailedGrabSpr.getLocalBounds().height] / 2);
	//detailedGrabSpr.setPosition(800, 500);

	PoiInfo *surface = points["crawlersurface"];

	queen = new CrawlerQueen(owner, surface->edge, surface->edgeQuantity, false);
	//queen->Setup();
	owner->fullEnemyList.push_back(queen);

	Reset();
}

void CrawlerAttackSeq::SetupStates()
{
	numStates = Count;
	stateLength = new int[numStates];

	stateLength[ENTRANCE] = -1;
	stateLength[WAIT] = 1;
	stateLength[KINSTOP] = 30;
	stateLength[ROCKSFALL] = 30;
	stateLength[CRAWLERSWOOP] = 9 * 3;
	stateLength[DIGGINGAROUND] = 180;
	stateLength[THROWOUT] = 60;
	stateLength[CRAWLERFACE] = 10000;
	stateLength[CRAWLERTALK] = 10000;
	stateLength[KINTALK] = 10000;
}

CrawlerAttackSeq::~CrawlerAttackSeq()
{
	delete detailedGrab;
	delete crawlerFace;
	delete kinFace;
}

void CrawlerAttackSeq::Init()
{

}

void CrawlerAttackSeq::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);
	//if (queen->action == CrawlerQueen::SEQ_FINISHINITIALUNBURROW)
	{
		BasicBossScene::ReturnToGame();
		queen->StartFight();

		CameraShot *shot = shots["fightcam"];
		owner->cam.Ease(Vector2f(shot->centerPos), shot->zoom, 60, CubicBezier());
		//player->StandInPlace();
		//player->SetAction(Actor::STAND);
		//player->frame = 0;
		//owner->adventureHUD->Show(60);
		//return false;
	}
//	else
	{
		
		//return true;
	}
}

void CrawlerAttackSeq::UpdateState()
{
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case ENTRANCE:
		EntranceUpdate();
		break;
	case KINSTOP:
		if (frame == 0)
		{
			owner->cam.Ease(Vector2f(player->position.x, player->position.y - 200), 1, 30, CubicBezier());

			player->desperationMode = false;
			player->SetAction(Actor::SEQ_LOOKUP);
			player->frame = 0;
		}
		break;
	case ROCKSFALL:
		if (frame == 0)
		{
			owner->cam.SetRumble(3, 3, stateLength[ROCKSFALL]);
		}
		break;
	case CRAWLERSWOOP:
	{
		if (frame == 15)
		{
			player->SetAction(Actor::SEQ_LOOKUPDISAPPEAR);
			player->frame = 0;
		}
		queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(frame / 3));
		queenGrabSprite.setOrigin(queenGrabSprite.getLocalBounds().width / 2,
			queenGrabSprite.getLocalBounds().height);

		Edge *ground = player->ground;
		V2d gPoint = ground->GetPoint(player->edgeQuantity);
		V2d gNorm = ground->Normal();

		queenGrabSprite.setPosition(Vector2f(gPoint - gNorm * 30.0));

		break;
	}
	case DIGGINGAROUND:
		if (frame == 0)
		{
			owner->cam.SetRumble(10, 10, stateLength[DIGGINGAROUND]);
			owner->cam.SetManual(true);
			//owner->cam.Set(Vector2f(camPoint0->pos), 1, 0);
			owner->cam.Ease(Vector2f(points["crawlerdig1"]->pos), 1, 60, CubicBezier());
		}
		else if (frame == 60)
		{

			//owner->cam.Set(Vector2f(camPoint1->pos), 1, 0);
			owner->cam.Ease(Vector2f(points["crawlerdig2"]->pos), 1, 60, CubicBezier());
		}
		else if (frame == 120)
		{
			//owner->cam.Set(Vector2f(roomCenter->pos), 1.75, 0);
			CameraShot *shot = shots["cavecam"];
			owner->cam.Ease(Vector2f(shot->centerPos),shot->zoom, 60, CubicBezier());
		}

		if (frame == 20)
		{
			detailedGrab->Flash();
		}
		break;
	case THROWOUT:
		if (frame == 0)
		{	
			owner->currentZone->ReformAllGates();
			player->StartSeqKinThrown(points["crawlersurface"]->pos, V2d(-10, -10));

		}
		else if (frame == 30)
		{
			owner->AddEnemy(queen);
			queen->StartInitialUnburrow();
		}
		break;
	case CRAWLERFACE:
	{
		//Conversation *conv = GetCurrentConv();
		if (frame == 0)
		{
			queen->StartAngryYelling();
			crawlerFace->Flash();
		}
		else if (crawlerFace->IsDone())
		{
			frame = stateLength[CRAWLERFACE] - 1;
		}
		break;
	}
	case CRAWLERTALK:
	{
		ConvUpdate();
		break;
	}
	case KINTALK:
	{
		if (frame == 0)
		{
			GetCurrentConv()->Hide();
			kinFace->Flash();
		}
		else if (kinFace->IsDone())
		{
			frame = stateLength[KINTALK] - 1;
		}
		break;
	}
	}

	crawlerFace->Update();
	kinFace->Update();
	detailedGrab->Update();
}

void CrawlerAttackSeq::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (state == CRAWLERSWOOP)
	{
		target->draw(queenGrabSprite);
	}

	View v = target->getView();
	target->setView(owner->uiView);
	detailedGrab->Draw(target);
	crawlerFace->Draw(target);
	kinFace->Draw(target);
	target->setView(v);

	BasicBossScene::Draw(target, layer);
}
void CrawlerAttackSeq::Reset()
{
	BasicBossScene::Reset();
	queen->Reset();
	detailedGrab->Reset();
	crawlerFace->Reset();
	kinFace->Reset();
}