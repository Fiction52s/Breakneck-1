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
#include "MusicPlayer.h"

using namespace sf;
using namespace std;

#define TIMESTEP 1.0 / 60.0


ShipExitSeq::ShipExitSeq( GameSession *p_owner )
	:owner( p_owner )
{
	enterTime = 60;
	exitTime = 60 + 60;
	center.AddLineMovement( V2d( 0, 0 ), V2d( 0, 0 ), 
		CubicBezier( 0, 0, 1, 1 ), 60 );
	//shipMovement.AddCubicMovement( 
	shipMovement.AddLineMovement( V2d( 0, 0 ), 
		V2d( 0, 0 ), CubicBezier( 0, 0, 1, 1 ), enterTime );
	shipMovement.AddLineMovement( V2d( 0, 0 ), 
		V2d( 0, 0 ), CubicBezier( 0, 0, 1, 1 ), exitTime );

	//ts_ship = owner->GetTileset( "Ship/ship_exit_1920x1080.png", 1920, 1080 );
	ts_ship = owner->GetTileset("Ship/ship_exit_864x540.png", 864, 540);
	shipSprite.setTexture( *ts_ship->texture );
	shipSprite.setTextureRect( ts_ship->GetSubRect( 0 ) );
	//shipSprite.setOrigin(560, 700);
	//shipSprite.setOrigin(960, 700);
	//shipSprite.setOrigin(960 / 2, 700);
	shipSprite.setOrigin(421, 425);

	assert(mov.openFromFile("Resources/Movie/kin_ship.ogv"));
	mov.fit(sf::FloatRect(0, 0, 1920, 1080));

	stateLength[SHIP_SWOOP]= 1000000;
	//stateLength[FADEOUT] = 90;

	stateLength[PLAYMOVIE] = 1000000;
}

bool ShipExitSeq::Update()
{
	Actor *player = owner->GetPlayer( 0 );

	if (frame == stateLength[state] && state != END)
	{
		int s = state;
		s++;
		state = (State)s;
		frame = 0;

		if (state == END)
		{
			//owner->goalDestroyed = true;
			//owner->state = GameSession::RUN;
			return false;
		}
	}

	switch (state)
	{
	case SHIP_SWOOP:
	{
		int shipOffsetY = -200;
		int pOffsetY = -170;
		int sOffsetY = pOffsetY;//shipOffsetY + pOffsetY;
		int jumpLength = 6 * 5;
		int startGrabWire = enterTime - jumpLength;

		if (frame == 0)
		{
			owner->cam.SetManual(true);
			center.movementList->start = V2d(owner->cam.pos.x, owner->cam.pos.y);
			center.movementList->end = V2d(owner->GetPlayer(0)->position.x,
				owner->GetPlayer(0)->position.y - 200);

			center.Reset();
			owner->cam.SetMovementSeq(&center, false);

			abovePlayer = V2d(player->position.x, player->position.y - 300);

			shipMovement.movementList->start = abovePlayer + V2d(-1500, -900);//player->position + V2d( -1000, sOffsetY );
			shipMovement.movementList->end = abovePlayer;//player->position + V2d( 1000, sOffsetY );
			shipMovement.Reset();

			Movement *m = shipMovement.movementList->next;

			m->start = abovePlayer;
			m->end = abovePlayer + V2d(1500, -900) + V2d( 1500, -900 );

			origPlayer = owner->GetPlayer(0)->position;
			attachPoint = abovePlayer;//V2d(player->position.x, player->position.y);//abovePlayer.y + 170 );
		}
		else  if (frame == startGrabWire)
		{
			owner->GetPlayer(0)->GrabShipWire();
		}

		for (int i = 0; i < NUM_STEPS; ++i)
		{
			shipMovement.Update();
		}

		int jumpSquat = startGrabWire + 3 * 5;
		int startJump = 4 * 5;//60 - jumpSquat;

		if (frame > enterTime)
		{
			owner->GetPlayer(0)->position = V2d(shipMovement.position.x, shipMovement.position.y + 48.0);
		}
		else if (frame >= jumpSquat && frame <= enterTime)//startJump )
		{
			double adjF = frame - jumpSquat;
			double eTime = enterTime - jumpSquat;
			double a = adjF / eTime;//(double)(frame - (60 - (startJump + 1))) / (60 - (startJump - 1));
			//double a = 
			//cout << "a: " << a << endl;
			V2d pAttachPoint = attachPoint;
			pAttachPoint.y += 48.f;
			owner->GetPlayer(0)->position = origPlayer * (1.0 - a) + pAttachPoint * a;
		}

		if (shipMovement.currMovement == NULL)
		{
			frame = stateLength[SHIP_SWOOP] - 1;
			owner->mainMenu->musicPlayer->FadeOutCurrentMusic(30);
			owner->state = GameSession::SEQUENCE;
		}

		if (frame == (enterTime + exitTime) - 60)
		{
			owner->Fade(false, 60, Color::Black);
		}

		shipSprite.setPosition(shipMovement.position.x,
			shipMovement.position.y);
		break;
	}
	//case FADEOUT:
	//{
	//	if (frame == 30)
	//	{
			//owner->Fade(false, 60, Color::Black);
	//	}
	//	break;
	//}
	case PLAYMOVIE:
	{
		owner->ClearFade();
		sfe::Status movStatus = mov.getStatus();
		if (frame == 0)
		{
			//owner->Fade(true, 60, Color::Black);
			mov.setPlayingOffset(sf::Time::Zero);
			mov.play();
		}
		else
		{
			mov.update();
			
			if (movStatus == sfe::Status::End || movStatus == sfe::Status::Stopped)
			{
				frame = stateLength[PLAYMOVIE] - 1;
				owner->goalDestroyed = true;
			}
		}
		break;
	}
	}
	++frame;

	return true;
}

void ShipExitSeq::Draw( RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (state == SHIP_SWOOP)
	{
		target->draw(shipSprite);
	}
	else if (state == PLAYMOVIE)
	{
		target->draw(mov);
	}
	
}

void ShipExitSeq::Reset()
{
	frame = 0;
	state = SHIP_SWOOP;
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
	Actor *player = owner->GetPlayer( 0 );
	switch( frame )
	{
	case 0:
		{
			//owner->powerWheel->Hide( true, 0 );
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

void CrawlerFightSeq::Draw( sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}
}

void CrawlerFightSeq::StartFightMsg()
{
	startFightMsgFrame = frame;
	//owner->powerWheel->Hide( false, 30 );
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
	Actor *player = owner->GetPlayer( 0 );
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

void CrawlerAfterFightSeq::Draw( sf::RenderTarget *target, EffectLayer layer )
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
	Actor *player = owner->GetPlayer( 0 );
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
			//coy->Start_IllTestYou();
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
	
	//owner->powerWheel->Hide( false, 60 );
}

void MeetCoyoteSeq::Draw( RenderTarget *target, EffectLayer layer)
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
	Actor *player = owner->GetPlayer( 0 );
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

void CoyoteTalkSeq::Draw( RenderTarget *target, EffectLayer layer)
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
	Actor *player = owner->GetPlayer( 0 );
	Boss_Coyote *coy = owner->b_coyote;
	switch( frame )
	{
	case 0:
		{
			owner->ActivateZone( coy->zone );
			assert( coy->zone != NULL );
			coy->zone->active = true;
			

			

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

void CoyoteFightSeq::Draw( RenderTarget *target, EffectLayer layer)
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
	Actor *player = owner->GetPlayer( 0 );
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

void SkeletonAttackCoyoteSeq::Draw( RenderTarget *target, EffectLayer layer)
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
	Actor *player = owner->GetPlayer( 0 );

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

void BirdCrawlerAllianceSeq::Draw( RenderTarget *target, EffectLayer layer)
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
	Actor *player = owner->GetPlayer( 0 );

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

void SkeletonFightSeq::Draw( RenderTarget *target, EffectLayer layer)
{

}

void SkeletonFightSeq::Reset()
{
	frame = 0;
}


CrawlerDefeatedSeq::CrawlerDefeatedSeq( GameSession *p_owner)
	:owner( p_owner )
{
	Reset();

	stateLength[PLAYMOVIE] = 1000000;

	assert(mov.openFromFile("Resources/Movie/crawler_slash.ogv"));
	mov.fit(sf::FloatRect(0, 0, 1920, 1080));
}

void CrawlerDefeatedSeq::Reset()
{
	state = PLAYMOVIE;
	frame = 0;
}

bool CrawlerDefeatedSeq::Update()
{
	Actor *player = owner->GetPlayer(0);

	if (frame == stateLength[state] && state != END)
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
		//owner->Fade(true, 60, sf::Color::White);
		owner->state = GameSession::RUN;
		owner->Fade(true, 60, Color::Black);

		Actor *player = owner->GetPlayer(0);
		player->SeqAfterCrawlerFight();
		//player->SetAction(Actor::GOALKILLWAIT);
		//player->frame = 0;
		//owner->scoreDisplay->Activate();
		return false;
	}

	if( state == PLAYMOVIE )
	{
		sfe::Status movStatus = mov.getStatus();
		if (frame == 0)
		{
			mov.setPlayingOffset(sf::Time::Zero);
			mov.play();
		}
		else
		{
			mov.update();

			//cout << "mov: " << mov.getPlayingOffset().asSeconds() << endl;
			if (movStatus == sfe::Status::End || movStatus == sfe::Status::Stopped)
			{
				frame = stateLength[PLAYMOVIE] - 1;
				if (owner->originalMusic != NULL)
				{
					MainMenu *mm = owner->mainMenu;
					mm->musicPlayer->TransitionMusic(owner->originalMusic, 60);
				}

				//owner->state = GameSession::RUN;
				//owner->Fade(true, 60, Color::Black, true);
				/*if (frame == stateLength[MASKOFF] - 1)
				{

				}*/
			}
		}
	}
	++frame;

	return true;
}


void CrawlerDefeatedSeq::Draw(sf::RenderTarget *target,
	EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	target->draw(mov);
}


BasicMovieSeq::BasicMovieSeq(GameSession *p_owner,
	const std::string &movieName, int preMovieLength, int postMovieLength)
	:owner(p_owner)
{
	Reset();

	stateLength[PREMOVIE] = preMovieLength;
	stateLength[PLAYMOVIE] = 1000000;
	stateLength[POSTMOVIE] = postMovieLength;


	string path = "Resources/Movie/";
	string ext = ".ogv";
	string movieFull = path + movieName + ext;

	assert(mov.openFromFile(movieFull));
	mov.fit(sf::FloatRect(0, 0, 1920, 1080));
}


BasicMovieSeq::~BasicMovieSeq()
{

}
void BasicMovieSeq::Reset()
{
	state = PREMOVIE;
	frame = 0;
}

bool BasicMovieSeq::Update()
{
	Actor *player = owner->GetPlayer(0);

	if (frame == stateLength[state] && state != END)
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
		owner->state = GameSession::RUN;
		owner->Fade(true, 60, Color::Black);
		return false;
	}

	if (state == PREMOVIE)
	{
		PreMovieUpdate();
	}
	else if (state == PLAYMOVIE)
	{
		
		sfe::Status movStatus = mov.getStatus();
		if (frame == 0)
		{
			owner->ClearFade();
			owner->state = GameSession::SEQUENCE;
			mov.setPlayingOffset(sf::Time::Zero);
			mov.play();
		}
		else
		{
			mov.update();

			//cout << "mov: " << mov.getPlayingOffset().asSeconds() << endl;
			if (movStatus == sfe::Status::End || movStatus == sfe::Status::Stopped)
			{
				frame = stateLength[PLAYMOVIE] - 1;
			}
		}
	}
	else if (state == POSTMOVIE)
	{
		PostMovieUpdate();
	}
	++frame;

	return true;
}


void BasicMovieSeq::Draw(sf::RenderTarget *target,
	EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	target->draw(mov);
}
