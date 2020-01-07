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
#include "Zone.h"
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
	:BasicBossScene( p_owner, BasicBossScene::RUN )
{
}

void CrawlerAttackSeq::SetupStates()
{
	SetNumStates(Count);

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

void CrawlerAttackSeq::AddShots()
{
	AddShot("scenecam");
	AddShot("cavecam");
	AddShot("fightcam");
}

void CrawlerAttackSeq::AddPoints()
{
	AddPoint("kinstart");
	AddPoint("kinstop");
	AddPoint("crawlerdig1");
	AddPoint("crawlerdig2");
	AddPoint("crawlersurface");
	AddPoint("crawlerthrowkin");
}

void CrawlerAttackSeq::AddGroups()
{
	AddGroup("pre_crawler", "W1/w1_crawler_pre_fight");
	SetConvGroup("pre_crawler");
}

void CrawlerAttackSeq::AddEnemies()
{
	PoiInfo *surface = points["crawlersurface"];
	queen = new CrawlerQueen(owner, surface->edge, surface->edgeQuantity, false);
	AddEnemy("queen", queen);
}

void CrawlerAttackSeq::AddFlashes()
{
	AddFlashedImage("detailedgrab", owner->GetTileset("Story/Crawler_Dig_01_860x830.png", 860, 830),
		0, 30, 60, 30, Vector2f(1160, 540));

	AddFlashedImage("crawlerface", owner->GetTileset("Story/Crawler_Dig_02_828x875.png", 828, 875),
		0, 30, 60, 30, Vector2f(1350, 325));

	AddFlashedImage("kinface", owner->GetTileset("Story/Crawler_Dig_03_510x565.png", 510, 565),
		0, 30, 60, 30, Vector2f(625, 325));
}

void CrawlerAttackSeq::SpecialInit()
{
	ts_queenGrab = owner->GetTileset("Bosses/Crawler/crawler_queen_grab_320x320.png", 320, 320);
	queenGrabSprite.setTexture(*ts_queenGrab->texture);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(0));
}

void CrawlerAttackSeq::ReturnToGame()
{
	Actor *player = owner->GetPlayer(0);
	
	BasicBossScene::ReturnToGame();
	queen->StartFight();

	CameraShot *shot = shots["fightcam"];
	owner->cam.Ease(Vector2f(shot->centerPos), shot->zoom, 60, CubicBezier());
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
			owner->cam.Ease(Vector2f(player->position.x, player->position.y - 200), 1, 30);
			player->desperationMode = false;
			player->StartAction(Actor::SEQ_LOOKUP);
		}
		break;
	case ROCKSFALL:
		RumbleDuringState(3,3);
		break;
	case CRAWLERSWOOP:
	{
		if (frame == 15)
		{
			player->StartAction(Actor::SEQ_LOOKUPDISAPPEAR);
		}

		UpdateCrawlerSwoop();
		break;
	}
	case DIGGINGAROUND:
		RumbleDuringState(10, 10);
		if (frame == 0)
		{
			EasePoint("crawlerdig1", 1, 60);
		}
		else if (frame == 60)
		{
			EasePoint("crawlerdig2", 1, 60);
		}
		else if (frame == 120)
		{
			EaseShot("cavecam", 60);
		}

		if (frame == 20)
		{
			Flash("detailedgrab");
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
		if (frame == 0)
		{
			queen->StartAngryYelling();
		}

		BasicFlashUpdateState("crawlerface");
		break;
	}
	case CRAWLERTALK:
	{
		ConvUpdate();
		break;
	}
	case KINTALK:
	{
		BasicFlashUpdateState("kinface");
		break;
	}
	}
}

void CrawlerAttackSeq::UpdateCrawlerSwoop()
{
	Actor *player = owner->GetPlayer(0);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(frame / 3));
	queenGrabSprite.setOrigin(queenGrabSprite.getLocalBounds().width / 2,
		queenGrabSprite.getLocalBounds().height);

	Edge *ground = player->ground;
	V2d gPoint = ground->GetPoint(player->edgeQuantity);
	V2d gNorm = ground->Normal();

	queenGrabSprite.setPosition(Vector2f(gPoint - gNorm * 30.0));
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

	BasicBossScene::Draw(target, layer);
}