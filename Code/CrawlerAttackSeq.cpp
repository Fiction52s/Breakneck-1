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


FlashedImage::FlashedImage(Tileset *ts,
	int tileIndex, int appearFrames,
	int holdFrames,
	int disappearFrames,
	sf::Vector2f &pos)
{
	spr.setTexture(*ts->texture);
	spr.setTextureRect(ts->GetSubRect(tileIndex));
	spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);
	spr.setPosition(pos);

	Reset();

	aFrames = appearFrames;
	hFrames = holdFrames;
	dFrames = disappearFrames;
}

void FlashedImage::Reset()
{
	frame = 0;
	flashing = false;
}

void FlashedImage::Flash()
{
	flashing = true;
	frame = 0;
}

void FlashedImage::Update()
{
	if (!flashing)
		return;

	int a = 0;
	if (frame < aFrames)
	{
		a = (frame / (float)aFrames) * 255.f;
	}
	else if (frame < aFrames + hFrames)
	{
		a = 255;
	}
	else
{	
		int fr = frame - (aFrames + hFrames);
		a = (1.f - fr / (float)dFrames) * 255.f;
	}
	spr.setColor(Color(255, 255, 255, a));

	if (frame == aFrames + hFrames + dFrames)
	{
		flashing = false;
	}

	++frame;
}

void FlashedImage::Draw(sf::RenderTarget *target)
{
	if (flashing)
	{
		target->draw(spr);
	}
	
}

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

	detailedGrab = new FlashedImage(owner->GetTileset("Story/grabdetailed_1920x1080.png", 1920, 1080),
		0, 30, 60, 30, Vector2f(0, 0));//Vector2f( 1500, 500 ));
	//ts_detailedGrab = owner->GetTileset("Bosses/Crawler/");
	//detailedGrabSpr.setTexture(*ts_detailedGrab);
	//detailedGrabSpr.setTextureRect(ts_detailedGrab->GetSubRect(0));
	//detailedGrabSpr.setOrigin(detailedGrabSpr.getLocalBounds().width / 2,
	//	detailedGrabSpr.getLocalBounds().height] / 2);
	//detailedGrabSpr.setPosition(800, 500);

	stateLength[KINSTOP] = 30;
	stateLength[ROCKSFALL] = 30;
	stateLength[CRAWLERSWOOP] = 9 * 3;
	stateLength[DIGGINGAROUND] = 180;
	stateLength[THROWOUT] = 60;
	stateLength[CONVERSATION] = 10000;
	stateLength[END] = 30;

	queen = new CrawlerQueen(owner, surface->edge, surface->edgeQuantity, false);
	//queen->Setup();
	owner->fullEnemyList.push_back(queen);
	

	Reset();
}

CrawlerAttackSeq::~CrawlerAttackSeq()
{
	delete detailedGrab;
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
		if (queen->action == CrawlerQueen::SEQ_FINISHINITIALUNBURROW)
		{
			player->SetAction(Actor::STAND);
			player->frame = 0;
			return false;
		}
		else
		{
			queen->StartFight();
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

		if (frame == 20)
		{
			detailedGrab->Flash();
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

		if (frame == 60)
		{
			frame = stateLength[CONVERSATION] - 1;
		}
		
		break;
	}

	detailedGrab->Update();

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

	View v = target->getView();
	target->setView(owner->uiView);
	detailedGrab->Draw(target);
	target->setView(v);
}
void CrawlerAttackSeq::Reset()
{
	state = KINSTOP;
	frame = 0;
	queen->Reset();
	detailedGrab->Reset();
}