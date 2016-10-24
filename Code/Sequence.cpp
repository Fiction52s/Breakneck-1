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
	startFightMsgFrame = -1;

	state = INIT;
	//stateLength 
}

void CrawlerFightSeq::Reset()
{
	frame = 0;
	camMove0.Reset();
	startFightMsgFrame = -1;
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
			player->frame = 1;
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
		owner->cam.SetRumble( 3, 3, 40 );
		owner->b_crawler->action = Boss_Crawler::EMERGE;
		owner->b_crawler->frame = 0;
	}
	else if( frame == blah + 50 )
	{
		player->action = Actor::SEQ_CRAWLERFIGHT_DODGEBACK;
		player->frame = 0;

	}
	else if( frame == blah + 80 )
	{
		//in reality this would be calling a function within the crawler
		owner->b_crawler->StartMeetPlayerSeq();

		//owner->b_crawler->portrait.SetSprite( ts_birdFace, 1 );
	}

	if( startFightMsgFrame >= 0 && frame == startFightMsgFrame + 30 )
	{
		player->action = Actor::STAND;
		player->frame = 0;
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

void CrawlerFightSeq::StartFightMsg()
{
	startFightMsgFrame = frame;
	owner->cam.SetManual( false );
	owner->cam.EaseOutOfManual( 30 );
	//owner->player->action = Actor::STAND;
	//frame = 0;
	//owner->cam.SetManual( true );
}

CrawlerAfterFightSeq::CrawlerAfterFightSeq( GameSession *p_owner )
	:owner( p_owner )
{
	frame = 0;
	frameCount = 6000;

	//nexus1Pos = owner->poiMap["nexus1"]->pos;
	cfightCenter = owner->poiMap["cfightcenter"]->pos;

	//camMove0.AddLineMovement( V2d( 0, 0 ), V2d( 1500, 0 ),
	//	CubicBezier( 0, 0, 1, 1 ), 120 ); 
	//camMove0.AddMovement( new WaitMovement( nexus1Pos, 60 ) );
	/*camMove0.AddLineMovement( cfightCenter + V2d( 0, -300 ), cfightCenter, CubicBezier( 0, 0, 1, 1 ), 30 ); 
	camMove0.AddMovement( new WaitMovement( cfightCenter, 60 ) );
	camMove0.AddLineMovement( cfightCenter, nexus1Pos, CubicBezier( 0, 0, 1, 1 ), 120 ); 
	camMove0.AddMovement( new WaitMovement( nexus1Pos, 60 ) );
	camMove0.AddLineMovement( nexus1Pos, cfightCenter, CubicBezier( 0, 0, 1, 1 ), 60 ); 

	camMove0.Reset();*/

	//playerStartFrame = 30 + 60 + 120 + 60 + 60;
	//startFightMsgFrame = -1;

	//state = INIT;
	//stateLength 
}

void CrawlerAfterFightSeq::Reset()
{
	frame = 0;
	//camMove0.Reset();
	//startFightMsgFrame = -1;
}

bool CrawlerAfterFightSeq::Update()
{
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
			owner->Fade( false, 60, Color::White );
			owner->Pause( 60 );
			owner->cam.SetManual( true );
			//owner->cam.SetMovementSeq( &camMove0, false );
			//owner->cam.Set( Vector2f( cfightCenter.x, cfightCenter.y - 300), 1, 0 );
			
			
			//player->velocity = V2d( 0, 10 );
			//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
			//player->facingRight = true;
		}
		break;
	case 1:
		{
			owner->ClearFX();
			owner->cam.Set( Vector2f( cfightCenter.x, cfightCenter.y ), 1, 0 );
			owner->Fade( true, 60, Color::White );
			owner->Pause( 60 );
			//owner->cam.SetManual( false );
			player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;
			player->groundSpeed = 0;
			PoiInfo *pi = owner->poiMap["cfightjumpback"];
			player->ground = pi->edge;
			player->edgeQuantity = pi->edgeQuantity;
			player->offsetX = player->b.rw;
			//player->offsetX = 0;
			player->facingRight = true;
			owner->b_crawler->StartAfterFightSeq();
			//player->action = Actor::Seq
		}
		break;
	case 60:
		{
		}
		break;
	}

	if( frame == 80 )
	{
		player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		player->frame = 0;
	}
	else if( frame == 120 )
	{
		player->action = Actor::GETPOWER_AIRDASH_MEDITATE;
		player->frame = 0;
		player->groundSpeed = 0;
	}


	//int blah = 30 + 60 + 120 + 60 + 60;
	//if( frame == blah )//if( owner->cam.currMove == NULL )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
	//}
	//else if( frame == blah + 30 )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED;
	//	player->groundSpeed = 0;
	//	owner->cam.SetRumble( 3, 3, 40 );
	//	owner->b_crawler->action = Boss_Crawler::EMERGE;
	//	owner->b_crawler->frame = 0;
	//}
	//else if( frame == blah + 50 )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_DODGEBACK;
	//	player->frame = 0;

	//}
	//else if( frame == blah + 80 )
	//{
	//	//in reality this would be calling a function within the crawler
	//	owner->b_crawler->StartMeetPlayerSeq();

	//	//owner->b_crawler->portrait.SetSprite( ts_birdFace, 1 );
	//}

	//if( startFightMsgFrame >= 0 && frame == startFightMsgFrame + 30 )
	//{
	//	player->action = Actor::STAND;
	//	player->frame = 0;
	//}

	++frame;

	if( frame == frameCount )
		return false;
	else
		return true;
}

void CrawlerAfterFightSeq::Draw( sf::RenderTarget *target )
{

}

EnterNexus1Seq::EnterNexus1Seq( GameSession *p_owner )
	:owner( p_owner )
{
	frame = 0;
	frameCount = 6000;
	nexus = NULL;

	//nexus1Pos = owner->poiMap["nexus1"]->pos;
	PoiInfo *pi = owner->poiMap["afternexus"];
	afterNexusGround = pi->edge;
	afterQuantity = pi->edgeQuantity;
	//cfightCenter = owner->poiMap["cfightcenter"]->pos;

	//camMove0.AddLineMovement( V2d( 0, 0 ), V2d( 1500, 0 ),
	//	CubicBezier( 0, 0, 1, 1 ), 120 ); 
	//camMove0.AddMovement( new WaitMovement( nexus1Pos, 60 ) );
	/*camMove0.AddLineMovement( cfightCenter + V2d( 0, -300 ), cfightCenter, CubicBezier( 0, 0, 1, 1 ), 30 ); 
	camMove0.AddMovement( new WaitMovement( cfightCenter, 60 ) );
	camMove0.AddLineMovement( cfightCenter, nexus1Pos, CubicBezier( 0, 0, 1, 1 ), 120 ); 
	camMove0.AddMovement( new WaitMovement( nexus1Pos, 60 ) );
	camMove0.AddLineMovement( nexus1Pos, cfightCenter, CubicBezier( 0, 0, 1, 1 ), 60 ); 

	camMove0.Reset();*/

	//playerStartFrame = 30 + 60 + 120 + 60 + 60;
	//startFightMsgFrame = -1;

	//state = INIT;
	//stateLength 
}

void EnterNexus1Seq::Reset()
{
	frame = 0;
	nexus = NULL;
	//camMove0.Reset();
	//startFightMsgFrame = -1;
}

bool EnterNexus1Seq::Update()
{
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
			owner->cam.SetManual( true );
			//owner->ClearFX();
			//owner->cam.SetMovementSeq( &camMove0, false );
			//owner->cam.Set( Vector2f( cfightCenter.x, cfightCenter.y - 300), 1, 0 );
			owner->Fade( false, 60, Color::Black );
			owner->Pause( 60 );

			//player->action = Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL;
			//player->velocity = V2d( 0, 0 );
			//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
			//player->facingRight = true;
		}
		break;
	case 1:
		{
			owner->cam.Set( Vector2f( player->position.x, player->position.y ), 1, 0 );
			owner->cam.SetManual( false );
			//owner->cam.EaseOutOfManual( 60 );
			player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;
			player->ground = afterNexusGround;
			player->edgeQuantity = afterQuantity;
			player->facingRight = true;
			player->groundSpeed = 0;
			player->offsetX = 0;
			assert( nexus != NULL );
			nexus->action = Nexus::EXPLODED;

			for( int i =0; i < owner->numGates; ++i )
			{
				Gate *g = owner->gates[i];
				if( g->type == Gate::NEXUS1_UNLOCK )
				{
					owner->UnlockGate( g );
					if( owner->currentZone == g->zoneA )
						owner->ActivateZone( g->zoneB );
					else
						owner->ActivateZone( g->zoneA );
					//g->gState = Gate::OPEN;
				}
			}

			owner->ClearFX();
			owner->Fade( true, 60, Color::Black );
			owner->Pause( 60 );
			/*owner->ClearFX();
			owner->cam.Set( Vector2f( cfightCenter.x, cfightCenter.y ), 1, 0 );
			owner->Fade( true, 60, Color::White );
			owner->Pause( 60 );
			player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;
			player->groundSpeed = 0;
			PoiInfo *pi = owner->poiMap["cfightjumpback"];
			player->ground = pi->edge;
			player->edgeQuantity = pi->edgeQuantity;
			player->facingRight = true;
			owner->b_crawler->StartAfterFightSeq();*/
			//player->action = Actor::Seq
		}
		break;
	case 10:
		{
			player->action = Actor::STAND;
			player->frame = 0;
			player->groundSpeed = 0;
		}
		break;
	}

	/*if( frame == 80 )
	{
		player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		player->frame = 0;
	}
	else if( frame == 120 )
	{
		player->action = Actor::GETPOWER_AIRDASH_MEDITATE;
		player->frame = 0;
		player->groundSpeed = 0;
	}*/


	//int blah = 30 + 60 + 120 + 60 + 60;
	//if( frame == blah )//if( owner->cam.currMove == NULL )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
	//}
	//else if( frame == blah + 30 )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED;
	//	player->groundSpeed = 0;
	//	owner->cam.SetRumble( 3, 3, 40 );
	//	owner->b_crawler->action = Boss_Crawler::EMERGE;
	//	owner->b_crawler->frame = 0;
	//}
	//else if( frame == blah + 50 )
	//{
	//	player->action = Actor::SEQ_CRAWLERFIGHT_DODGEBACK;
	//	player->frame = 0;

	//}
	//else if( frame == blah + 80 )
	//{
	//	//in reality this would be calling a function within the crawler
	//	owner->b_crawler->StartMeetPlayerSeq();

	//	//owner->b_crawler->portrait.SetSprite( ts_birdFace, 1 );
	//}

	//if( startFightMsgFrame >= 0 && frame == startFightMsgFrame + 30 )
	//{
	//	player->action = Actor::STAND;
	//	player->frame = 0;
	//}

	++frame;

	if( frame == frameCount )
		return false;
	else
		return true;
}

void EnterNexus1Seq::Draw( sf::RenderTarget *target )
{

}

MeetCoyoteSeq::MeetCoyoteSeq( GameSession *p_owner )
	:owner( p_owner )
{
	PoiInfo *pi0 = owner->poiMap["coyotemeetstart"];
	startGround = pi0->edge;
	startQuant = pi0->edgeQuantity;

	PoiInfo *pi1 = owner->poiMap["coyotesleep"];
	coyotePos = pi1->edge;
	coyoteQuant = pi1->edgeQuantity;
	//frameCount = -1;


}

bool MeetCoyoteSeq::Update()
{
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
		owner->cam.SetManual( true );

		V2d center = startGround->GetPoint( startQuant ) + coyotePos->GetPoint( coyoteQuant );
		center /= 2.0;
		center.y -= 100;
		//owner->cam.SetMovementSeq( &camMove0, false );
		owner->cam.Set( Vector2f( center.x, center.y ), 1, 0 );
		owner->Fade( true, 60, Color::Black );
		owner->Pause( 60 );
		player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		player->offsetX = 0;
		player->frame = 0;
		player->facingRight = true;
		
		//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
		
		break;
		}
	case 30:
		{
			player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;
			break;
		}
		
	}

	if( frame == 90 )
	{
		return false;
	}

	++frame;

	return true;
}

void MeetCoyoteSeq::Draw( RenderTarget *target )
{

}

void MeetCoyoteSeq::Reset()
{
	frame = 0;
}

CoyoteTalkSeq::CoyoteTalkSeq( GameSession *p_owner )
	:owner( p_owner )
{
	PoiInfo *pi0 = owner->poiMap["coyotemeetstart"];
	startGround = pi0->edge;
	startQuant = pi0->edgeQuantity;

	PoiInfo *pi1 = owner->poiMap["coyotesleep"];
	coyotePos = pi1->edge;
	coyoteQuant = pi1->edgeQuantity;
	//frameCount = -1;


}

bool CoyoteTalkSeq::Update()
{
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
		owner->cam.SetManual( true );

		V2d center = startGround->GetPoint( startQuant ) + coyotePos->GetPoint( coyoteQuant );
		center /= 2.0;
		center.y -= 100;
		//owner->cam.SetMovementSeq( &camMove0, false );
		owner->cam.Set( Vector2f( center.x, center.y ), 1, 0 );
		owner->Fade( true, 60, Color::Black );
		owner->Pause( 60 );
		player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		player->offsetX = 0;
		player->frame = 0;
		player->facingRight = true;
		
		//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
		
		break;
		}
	case 30:
		{
			player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;
			break;
		}
		
	}

	if( frame == 90 )
	{
		return false;
	}

	++frame;

	return true;
}

void CoyoteTalkSeq::Draw( RenderTarget *target )
{

}

void CoyoteTalkSeq::Reset()
{
	frame = 0;
}

SkeletonAttackCoyoteSeq::SkeletonAttackCoyoteSeq( GameSession *p_owner )
	:owner( p_owner )
{
	PoiInfo *pi0 = owner->poiMap["coyotemeetstart"];
	startGround = pi0->edge;
	startQuant = pi0->edgeQuantity;

	PoiInfo *pi1 = owner->poiMap["coyotesleep"];
	coyotePos = pi1->edge;
	coyoteQuant = pi1->edgeQuantity;
	//frameCount = -1;


}

bool SkeletonAttackCoyoteSeq::Update()
{
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
		owner->cam.SetManual( true );

		V2d center = startGround->GetPoint( startQuant ) + coyotePos->GetPoint( coyoteQuant );
		center /= 2.0;
		center.y -= 100;
		//owner->cam.SetMovementSeq( &camMove0, false );
		owner->cam.Set( Vector2f( center.x, center.y ), 1, 0 );
		owner->Fade( true, 60, Color::Black );
		owner->Pause( 60 );
		player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		player->offsetX = 0;
		player->frame = 0;
		player->facingRight = true;
		
		//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
		
		break;
		}
	case 30:
		{
			player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;
			break;
		}
		
	}

	if( frame == 90 )
	{
		return false;
	}

	++frame;

	return true;
}

void SkeletonAttackCoyoteSeq::Draw( RenderTarget *target )
{

}

void SkeletonAttackCoyoteSeq::Reset()
{
	frame = 0;
}