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

	ts_queenGrab = owner->GetTileset("Bosses/Crawler/crawler_queen_grab_192x192.png", 192, 192);
	queenGrabSprite.setTexture(*ts_queenGrab->texture);
	queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(0));

	stateLength[KINSTOP] = 30;
	stateLength[ROCKSFALL] = 30;
	stateLength[CRAWLERSWOOP] = 9 * 3;
	stateLength[DIGGINGAROUND] = 120;
	stateLength[THROWOUT] = 30;
	stateLength[CONVERSATION] = 30;
	stateLength[END] = 30;

	queen = new CrawlerQueen(owner, surface->edge, surface->edgeQuantity, false);
	//queen->Setup();
	owner->fullEnemyList.push_back(queen);
	owner->AddEnemy(queen);

	Reset();
}

void CrawlerAttackSeq::Init()
{

}

bool CrawlerAttackSeq::Update()
{
	if (frame == stateLength[state])
	{	
		int s = state;
		s++;
		state = (State)s;
		frame = 0;

		if (state == END)
		{
			return false;
		}
	}

	switch (state)
	{
	case KINSTOP:
		break;
	case ROCKSFALL:
		break;
	case CRAWLERSWOOP:
		
		queenGrabSprite.setTextureRect(ts_queenGrab->GetSubRect(frame / 3));
		queenGrabSprite.setOrigin(queenGrabSprite.getLocalBounds().width / 2,
			queenGrabSprite.getLocalBounds().height/2);
		queenGrabSprite.setPosition(Vector2f(owner->GetPlayer(0)->position));
			//Vector2f(owner->GetPlayer(0)->ground->GetPoint(owner->GetPlayer(0)->edgeQuantity)));

		break;
	case DIGGINGAROUND:
		if (frame == 0)
		{
			owner->cam.SetManual(true);
			owner->cam.Set(Vector2f(camPoint0->pos), 1, 0);
		}
		else if (frame == 60)
		{
			owner->cam.Set(Vector2f(camPoint1->pos), 1, 0);
		}
		break;
	case THROWOUT:
		if (frame == 0)
		{
			owner->cam.Set(Vector2f(roomCenter->pos), 1.75, 0);
			queen->StartInitialUnburrow();
		}
		break;
	case CONVERSATION:
		break;
	}

	++frame;

	return true;
}
void CrawlerAttackSeq::Draw(sf::RenderTarget *target)
{
	if (state == CRAWLERSWOOP)
	{
		target->draw(queenGrabSprite);
	}
}
void CrawlerAttackSeq::Reset()
{
	state = KINSTOP;
	frame = 0;
}