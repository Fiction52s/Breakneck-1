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
#include "EditSession.h"
#include "Zone.h"
#include "Flow.h"
#include "Boss.h"
#include "PowerOrbs.h"
#include "Sequence.h"
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

using namespace sf;
using namespace std;

#define TIMESTEP 1.0 / 60.0

CrawlerAttackSeq::CrawlerAttackSeq(GameSession *p_owner)
	:owner(p_owner)
{
	//SetRectCenter(darkQuad, 1920, 1080, Vector2f(960, 540));// , Vector2f(pi->pos));
	//SetRectColor(darkQuad, Color(Color::Red));
	camPoint0 = owner->poiMap["crawlerdig1"];
	camPoint1 = owner->poiMap["crawlerdig2"];
	roomCenter = owner->poiMap["crawlercam"];
	surface = owner->poiMap["crawlersurface"];
	throwkin = owner->poiMap["crawlerthrowkin"];
	//state = ENTERCORE;

	ts_queenGrab = owner->GetTileset("Bosses/Crawler/crawler_queen_grab_320x320.png", 320, 320);
	queenGrabSprite.setTexture(*ts_queenGrab->texture);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(0));

	stateLength[KINSTOP] = 30;
	stateLength[ROCKSFALL] = 30;
	stateLength[CRAWLERSWOOP] = 9 * 3;
	stateLength[DIGGINGAROUND] = 180;
	stateLength[THROWOUT] = 60;
	stateLength[CONVERSATION] = 30;
	stateLength[END] = 30;

	queen = new CrawlerQueen(owner, surface->edge, surface->edgeQuantity, false);
	//queen->Setup();
	owner->fullEnemyList.push_back(queen);
	

	Reset();
}

void CrawlerAttackSeq::Init()
{

}

bool CrawlerAttackSeq::Update()
{
	Actor *player = owner->GetPlayer(0);

	if (frame == stateLength[state] && state != END )
	{	
		int s = state;
		s++;
		state = (State)s;
		frame = 0;

		if (state == END)
		{
		}
	}

	if (state == END)
	{
		if (queen->action == CrawlerQueen::DECIDE)
		{
			player->SetAction(Actor::STAND);
			player->frame = 0;
			return false;
		}
		else
		{
			return true;
		}
	}
	
	

	switch (state)
	{
	case KINSTOP:
		if( frame == 0 )
			owner->cam.Ease(Vector2f(player->position.x, player->position.y - 200), 1, 30, CubicBezier());
		break;
	case ROCKSFALL:
		if (frame == 0)
		{
			owner->cam.SetRumble(2, 2, stateLength[ROCKSFALL]);
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
			owner->cam.SetRumble(5, 5, stateLength[DIGGINGAROUND]);
			owner->cam.SetManual(true);
			//owner->cam.Set(Vector2f(camPoint0->pos), 1, 0);
			owner->cam.Ease(Vector2f(camPoint0->pos), 1, 60, CubicBezier());
		}
		else if (frame == 60)
		{
			//owner->cam.Set(Vector2f(camPoint1->pos), 1, 0);
			owner->cam.Ease(Vector2f(camPoint1->pos), 1, 60, CubicBezier());
		}
		else if (frame == 120)
		{
			//owner->cam.Set(Vector2f(roomCenter->pos), 1.75, 0);
			owner->cam.Ease(Vector2f(roomCenter->pos), 1.75, 60, CubicBezier());
		}
		break;
	case THROWOUT:
		if (frame == 0)
		{
			//owner->currentZone->LockAllGates();
			//owner->currentZone = queen->zone;
			owner->currentZone->ReformAllGates();
			player->StartSeqKinThrown(V2d(surface->pos), V2d(-10, -10));
			
		}
		else if (frame == 30)
		{
			owner->AddEnemy(queen);
			queen->StartInitialUnburrow();
		}
		break;
	case CONVERSATION:
		if (frame == 0)
		{
			queen->StartAngryYelling();
		}
		
		break;
	}

	++frame;

	return true;
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
}
void CrawlerAttackSeq::Reset()
{
	state = KINSTOP;
	frame = 0;
	queen->Reset();
}