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


ShipExitSeq::ShipExitSeq( GameSession *p_owner )
	:owner( p_owner )
{
	center.AddLineMovement( V2d( 0, 0 ), V2d( 0, 0 ), 
		CubicBezier( 0, 0, 1, 1 ), 60 );
	//shipMovement.AddCubicMovement( 
	shipMovement.AddLineMovement( V2d( 0, 0 ), 
		V2d( 0, 0 ), CubicBezier( 0, 0, 1, 1 ), 60 );
	shipMovement.AddLineMovement( V2d( 0, 0 ), 
		V2d( 0, 0 ), CubicBezier( 0, 0, 1, 1 ), 60 );

	ts_ship = owner->GetTileset( "Ship/ship_exit_1920x1080.png", 1920, 1080 );
	shipSprite.setTexture( *ts_ship->texture );
	shipSprite.setTextureRect( ts_ship->GetSubRect( 0 ) );
	shipSprite.setScale( .5, .5 );
	shipSprite.setOrigin( shipSprite.getLocalBounds().width / 2, 
		shipSprite.getLocalBounds().height / 2 );
	
	//shipSprite.setTexture
}

bool ShipExitSeq::Update()
{
	Actor *player = owner->player;
	int shipOffsetY = -200;
	int pOffsetY = -170;
	int sOffsetY = pOffsetY;//shipOffsetY + pOffsetY;
	
	//player->action = Actor::SPAWNWAIT;
	//player->frame = 0;

	switch( frame )
	{
	case 0:
		{
			owner->cam.SetManual( true );
			center.movementList->start = V2d( owner->cam.pos.x, owner->cam.pos.y );
			center.movementList->end = V2d( owner->player->position.x, 
				owner->player->position.y - 200 );
			
			center.Reset();
			owner->cam.SetMovementSeq( &center, false );

			abovePlayer = V2d( player->position.x , player->position.y - 500 );

			shipMovement.movementList->start = abovePlayer + V2d( -1000, -500 );//player->position + V2d( -1000, sOffsetY );
			shipMovement.movementList->end = abovePlayer;//player->position + V2d( 1000, sOffsetY );
			shipMovement.Reset();

			Movement *m = shipMovement.movementList->next;

			m->start = abovePlayer;
			m->end = abovePlayer + V2d( 1000, -500 );

			origPlayer = owner->player->position;
			attachPoint = V2d( player->position.x, abovePlayer.y + 170 );
			break;
		}
	case 1:
		{
			//owner->Fade( false, 60, Color::Black );
			//owner->Pause( 60 );
			//owner->cam.Set( Vector2f( player->position.x, player->position.y ), 1, 0 );
			//owner->cam.SetManual( false );
			break;
		}
	case (60-16):
		{
			owner->player->GrabShipWire();
		}
		break;
	}

	//cout << "sequence frame : " << frame << endl;
	if( frame >= 61 )
	{
		owner->player->position = V2d( shipMovement.position.x, shipMovement.position.y + 170 );
	}
	else if( frame >= 45 )
	{
		double a = (double)(frame-44) / 16;
		owner->player->position = origPlayer * (1.0 - a ) + attachPoint * a;
	}
	

	for( int i = 0; i < NUM_STEPS; ++i )
	{
		shipMovement.Update();
	}
	
	//cout << "shipmovement: " << shipMovement.currTime << endl;
	
	
	shipSprite.setPosition( shipMovement.position.x,
		shipMovement.position.y );

	++frame;

	return true;
}

void ShipExitSeq::Draw( RenderTarget *target )
{
	target->draw( shipSprite );
}

void ShipExitSeq::Reset()
{
	frame = 0;
}



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
			owner->powerWheel->Hide( true, 0 );
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
	owner->powerWheel->Hide( false, 30 );
	owner->cam.SetManual( false );
	owner->cam.EaseOutOfManual( 30 );
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
	coyoteGone = false;
	PoiInfo *pi0 = owner->poiMap["meetcoyotestart"];
	startGround = pi0->edge;
	startQuant = pi0->edgeQuantity;

	PoiInfo *pi1 = owner->poiMap["coyotemeet"];
	coyGround = pi1->edge;
	coyQuant = pi1->edgeQuantity;

	//PoiInfo *pi1 = owner->poiMap["coyotesleep"];
	//coyotePos = coyo//pi1->edge;
	//coyoteQuant = pi1->edgeQuantity;
	//frameCount = -1;


}

bool MeetCoyoteSeq::Update()
{
	Boss_Coyote *coy = owner->b_coyote;
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
		owner->cam.SetManual( true );

		V2d center = owner->poiMap["meetcoyotecam"]->pos;//startGround->GetPoint( startQuant ) + owner->b_coyote->startGround->GetPoint( 
			//owner->b_coyote->startQuant );
		//center /= 2.0;
		//center.y -= 160;
		
		//owner->cam.SetMovementSeq( &camMove0, false );
		owner->cam.Set( Vector2f( center.x, center.y ), 1, 0 );
		owner->Fade( true, 60, Color::Black );
		owner->Pause( 60 );
		//owner->powerWheel->Hide(false, 60 );
		player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		player->offsetX = 0;
		player->ground = startGround;
		player->groundSpeed = 0;
		player->edgeQuantity = startQuant;
		player->frame = 0;
		player->facingRight = true;

		coy->testMover->ground = coyGround;
		coy->testMover->edgeQuantity = coyQuant;
		coy->testMover->UpdateGroundPos();
		coy->facingRight = false;
		
		//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
		
		break;
		}
	case 40:
		{
			player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;
			player->groundSpeed = 0;
			break;
		}
	case 90:
		{
			coy->Start_IllTestYou();
		}
		
	}




	if( coyoteGone && frame == coyoteGoneFrame + 60 )
	{
		
		player->action = Actor::STAND;
		owner->cam.SetManual( false );
		owner->cam.EaseOutOfManual( 60 );
		return false;
	}
	//if( frame > 120 )
	{

	}

	//if( frame == 90 )
	//{
	//	return false;
	//}

	++frame;

	return true;
}

void MeetCoyoteSeq::CoyoteGone()
{
	Boss_Coyote *coy = owner->b_coyote;
	
	coyoteGone = true;
	coyoteGoneFrame = frame;
	
	owner->powerWheel->Hide( false, 60 );
}

void MeetCoyoteSeq::Draw( RenderTarget *target )
{

}

void MeetCoyoteSeq::Reset()
{
	frame = 0;
	coyoteGone = false;
}


CoyoteTalkSeq::CoyoteTalkSeq( GameSession *p_owner )
	:owner( p_owner )
{
	/*PoiInfo *pi0 = owner->poiMap["coyotemeetstart"];
	startGround = pi0->edge;
	startQuant = pi0->edgeQuantity;

	PoiInfo *pi1 = owner->poiMap["coyotesleep"];
	coyotePos = pi1->edge;
	coyoteQuant = pi1->edgeQuantity;*/
	//frameCount = -1;


}

bool CoyoteTalkSeq::Update()
{
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
		//owner->cam.SetManual( true );

		//V2d center = startGround->GetPoint( startQuant ) + coyotePos->GetPoint( coyoteQuant );
		//center /= 2.0;
		//center.y -= 100;
		////owner->cam.SetMovementSeq( &camMove0, false );
		//owner->cam.Set( Vector2f( center.x, center.y ), 1, 0 );
		//owner->Fade( true, 60, Color::Black );
		//owner->Pause( 60 );
		//player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		//player->offsetX = 0;
		//player->frame = 0;
		//player->facingRight = true;
		
		//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
		
		break;
		}
	case 30:
		{
			/*player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;*/
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

CoyoteFightSeq::CoyoteFightSeq( GameSession *p_owner )
	:owner( p_owner )
{
	V2d kinCam = owner->poiMap["kincoyfightcam"]->pos;
	V2d coyCam = owner->poiMap["coyfightcam"]->pos;

	PoiInfo *start = owner->poiMap["startcoyotefight"];

	startEdge = start->edge;
	startQuant = start->edgeQuantity;
	//PoiInfo *coyStart = owner->poiMap["coyotefight"];

	//bleft = owner->poiMap["coyfightleft"]->barrier;
	//bright = owner->poiMap["coyfightright"]->barrier;
	//btop = owner->poiMap["coyfighttop"]->barrier;
	//bbot = owner->poiMap["coyfightbot"]->barrier;

	/*bleft->triggered = true;
	bright->triggered = true;
	btop->triggered = true;
	bbot->triggered = true;*/

	//startEdge = start->edge;
	//startQuant = start->edgeQuantity;

	//coyStartEdge = coyStart->edge;
	//coyStartQuant = coyStart->edgeQuantity;

	kinCamStart = Vector2f( kinCam.x, kinCam.y );
	coyCamStart = Vector2f( coyCam.x, coyCam.y );

	camMove0.AddLineMovement( kinCam, coyCam, CubicBezier( 0, 0, 1, 1 ), 60 );
	/*PoiInfo *pi0 = owner->poiMap["coyotemeetstart"];
	startGround = pi0->edge;
	startQuant = pi0->edgeQuantity;

	PoiInfo *pi1 = owner->poiMap["coyotesleep"];
	coyotePos = pi1->edge;
	coyoteQuant = pi1->edgeQuantity;*/
	//frameCount = -1;
	//owner->barriers//poiMap["coyfighttop"]->

}

bool CoyoteFightSeq::Update()
{
	Actor *player = owner->player;
	Boss_Coyote *coy = owner->b_coyote;
	switch( frame )
	{
	case 0:
		{
			owner->ActivateZone( coy->zone );
			assert( coy->zone != NULL );

			

			

			owner->cam.SetManual( true );
			owner->cam.SetMovementSeq( &camMove0, false );
			//owner->cam.Set( kinCamStart, 1, 0 );

		//coy->testMover->ground = coyStartEdge;
		//coy->testMover->edgeQuantity = coyStartQuant;
		//coy->testMover->UpdateGroundPos();
		//coy->facingRight = false;
		

		owner->Fade( true, 60, Color::Black );
		owner->Pause( 60 );

		

		player->ground = startEdge;
		player->offsetX = player->b.rw;
		player->edgeQuantity = startQuant;
		player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
		player->frame = 0;
		player->groundSpeed = 0;
		player->facingRight = true;
		player->PhysicsResponse();

		/*bleft->triggered = false;
		bleft->SetPositive();
		bright->triggered = false;
		bright->SetPositive();
		btop->triggered = false;
		btop->SetPositive();
		bbot->triggered = false;
		bbot->SetPositive();*/


		//V2d center = startGround->GetPoint( startQuant ) + coyotePos->GetPoint( coyoteQuant );
		//center /= 2.0;
		//center.y -= 100;
		////owner->cam.SetMovementSeq( &camMove0, false );
		//owner->cam.Set( Vector2f( center.x, center.y ), 1, 0 );
		//owner->Fade( true, 60, Color::Black );
		//owner->Pause( 60 );
		//player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		//player->offsetX = 0;
		//player->frame = 0;
		//player->facingRight = true;
		
		//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
		
		break;
		}
	case 30:
		{
			/*player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;*/
			break;
		}
	case 60:
		{
			owner->cam.SetManual( false );
			owner->cam.EaseOutOfManual( 60 );
			player->action = Actor::STAND;
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

void CoyoteFightSeq::Draw( RenderTarget *target )
{

}

void CoyoteFightSeq::Reset()
{
	frame = 0;
}

SkeletonAttackCoyoteSeq::SkeletonAttackCoyoteSeq( GameSession *p_owner )
	:owner( p_owner )
{
	/*PoiInfo *pi0 = owner->poiMap["coyotemeetstart"];
	startGround = pi0->edge;
	startQuant = pi0->edgeQuantity;

	PoiInfo *pi1 = owner->poiMap["coyotesleep"];
	coyotePos = pi1->edge;
	coyoteQuant = pi1->edgeQuantity;*/
	//frameCount = -1;


}

bool SkeletonAttackCoyoteSeq::Update()
{
	Actor *player = owner->player;
	switch( frame )
	{
	case 0:
		{
		//owner->cam.SetManual( true );

		//V2d center = startGround->GetPoint( startQuant ) + coyotePos->GetPoint( coyoteQuant );
		//center /= 2.0;
		//center.y -= 100;
		////owner->cam.SetMovementSeq( &camMove0, false );
		//owner->cam.Set( Vector2f( center.x, center.y ), 1, 0 );
		//owner->Fade( true, 60, Color::Black );
		//owner->Pause( 60 );
		//player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		//player->offsetX = 0;
		//player->frame = 0;
		//player->facingRight = true;
		
		//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
		
		break;
		}
	case 30:
		{
			/*player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;*/
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

BirdCrawlerAllianceSeq::BirdCrawlerAllianceSeq( GameSession *p_owner )
	:owner( p_owner )
{
	/*PoiInfo *pi0 = owner->poiMap["coyotemeetstart"];
	startGround = pi0->edge;
	startQuant = pi0->edgeQuantity;

	PoiInfo *pi1 = owner->poiMap["coyotesleep"];
	coyotePos = pi1->edge;
	coyoteQuant = pi1->edgeQuantity;*/
	//frameCount = -1;


}

bool BirdCrawlerAllianceSeq::Update()
{
	Actor *player = owner->player;

	player->action = Actor::SPAWNWAIT;
	player->frame = 0;

	switch( frame )
	{
	case 0:
		{
			cout << "frame is 0" << endl;
			owner->Fade( true, 60, Color::Black );
			owner->Pause( 60 );
			owner->cam.SetManual( true );
			
			
		//owner->cam.SetManual( true );

		//V2d center = startGround->GetPoint( startQuant ) + coyotePos->GetPoint( coyoteQuant );
		//center /= 2.0;
		//center.y -= 100;
		////owner->cam.SetMovementSeq( &camMove0, false );
		//owner->cam.Set( Vector2f( center.x, center.y ), 1, 0 );
		//owner->Fade( true, 60, Color::Black );
		//owner->Pause( 60 );
		//player->action = Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
		//player->offsetX = 0;
		//player->frame = 0;
		//player->facingRight = true;
		
		//player->position = owner->poiMap["crawlerfighttrigger"]->pos;
		
		break;
		}
	case 90:
		{
			//cout << "frame is 90" << endl;
			owner->Fade( false, 60, Color::Black );
			owner->Pause( 60 );
			//player->action = Actor::SPAWNWAIT;
			//player->frame = 0;
			owner->cam.Set( Vector2f( player->position.x, player->position.y ), 1, 0 );
			owner->cam.SetManual( false );
			
			/*player->action = Actor::SEQ_CRAWLERFIGHT_STAND;
			player->frame = 0;*/
			break;
		}
	case 91:
		{
			owner->Fade( true, 60, Color::Black );
			owner->Pause( 60 );
			return false;
			break;
		}
		
	}

	//if( frame == 90 )
	//{
	//	return false;
	//}

	++frame;

	return true;
}

void BirdCrawlerAllianceSeq::Draw( RenderTarget *target )
{

}

void BirdCrawlerAllianceSeq::Reset()
{
	frame = 0;
}

SkeletonFightSeq::SkeletonFightSeq( GameSession *p_owner )
	:owner( p_owner )
{
}

bool SkeletonFightSeq::Update()
{
	Actor *player = owner->player;

	//player->action = Actor::SPAWNWAIT;
	//player->frame = 0;

	switch( frame )
	{
	case 0:
		{
			owner->Fade( true, 60, Color::Black );
			owner->Pause( 60 );
			//owner->cam.SetManual( true );		
			break;
		}
	case 1:
		{
			//owner->Fade( false, 60, Color::Black );
			//owner->Pause( 60 );
			//owner->cam.Set( Vector2f( player->position.x, player->position.y ), 1, 0 );
			//owner->cam.SetManual( false );
			break;
		}
	}

	++frame;

	return true;
}

void SkeletonFightSeq::Draw( RenderTarget *target )
{

}

void SkeletonFightSeq::Reset()
{
	frame = 0;
}

