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

using namespace sf;
using namespace std;

#define TIMESTEP 1.0 / 60.0
#define V2d sf::Vector2<double>

CrawlerFightSeq::CrawlerFightSeq( GameSession *p_owner )
	:owner( p_owner )
{
	frame = 0;
	frameCount = 6000;

	nexus1Pos = owner->poiMap["nexus1"]->pos;
	cfightCenter = owner->poiMap["cfightcenter"]->pos;

	//camMove0.AddLineMovement( V2d( 0, 0 ), V2d( 1500, 0 ),
	//	CubicBezier( 0, 0, 1, 1 ), 120 ); 
	//camMove0.AddMovement( new WaitMovement( nexus1Pos, 60 ) );
	camMove0.AddLineMovement( cfightCenter + V2d( 0, -300 ), cfightCenter, CubicBezier( 0, 0, 1, 1 ), 30 ); 
	camMove0.AddMovement( new WaitMovement( cfightCenter, 60 ) );
	camMove0.AddLineMovement( cfightCenter, nexus1Pos, CubicBezier( 0, 0, 1, 1 ), 120 ); 
	camMove0.AddMovement( new WaitMovement( nexus1Pos, 60 ) );
	camMove0.AddLineMovement( nexus1Pos, cfightCenter, CubicBezier( 0, 0, 1, 1 ), 60 ); 

	camMove0.Reset();

	//playerStartFrame = 30 + 60 + 120 + 60 + 60;

	
}

void CrawlerFightSeq::Reset()
{
	camMove0.Reset();
}

bool CrawlerFightSeq::Update()
{
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
			owner->cam.SetManual( true );
			owner->cam.SetMovementSeq( &camMove0, false );
			//owner->cam.Set( Vector2f( cfightCenter.x, cfightCenter.y - 300), 1, 0 );
			owner->Fade( true, 60, Color::Black );
			owner->Pause( 60 );
			player->action = Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL;
			player->velocity = V2d( 0, 10 );
			player->position = owner->poiMap["crawlerfighttrigger"]->pos;
			player->facingRight = true;
		}
		break;
	case 90:
		{

			//player->action = Actor::Seq
		}
		break;
	case 60:
		{
		}
		break;
	}


	int blah = 30 + 60 + 120 + 60 + 60;
	if( frame == blah )//if( owner->cam.currMove == NULL )
	{
		player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
	}
	else if( frame == blah + 30 )
	{
		player->action = Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED;
		player->groundSpeed = 0;
		owner->cam.SetRumble( 10, 10, 120 );
		owner->b_crawler->action = Boss_Crawler::SHOOT;
		owner->b_crawler->frame = 0;
	}
	else if( frame == blah + 50 )
	{
		player->action = Actor::SEQ_CRAWLERFIGHT_DODGEBACK;
		frame = 0;
	}

	if( frame == 30 )
	{
		
	}

	++frame;

	if( frame == frameCount )
		return false;
	else
		return true;
}

void CrawlerFightSeq::Draw( sf::RenderTarget *target )
{

}
