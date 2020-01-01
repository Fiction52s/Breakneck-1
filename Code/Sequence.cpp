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
#include "ImageText.h"
#include "HUD.h"
#include "StorySequence.h"
//#include "EditSession.h"
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
#include "ButtonHolder.h"
#include "Config.h"

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

	ts_ship = owner->GetTileset("Ship/ship_exit_864x540.png", 864, 540);
	shipSprite.setTexture( *ts_ship->texture );
	shipSprite.setTextureRect( ts_ship->GetSubRect( 0 ) );
	//shipSprite.setOrigin(560, 700);
	//shipSprite.setOrigin(960, 700);
	//shipSprite.setOrigin(960 / 2, 700);
	shipSprite.setOrigin(421, 425);

	storySeq = new StorySequence(owner);
	storySeq->Load("world1outro");
	//assert(mov.openFromFile("Resources/Movie/kin_ship.ogv"));
	//mov.fit(sf::FloatRect(0, 0, 1920, 1080));

	stateLength[SHIP_SWOOP]= 1000000;
	//stateLength[FADEOUT] = 90;

	stateLength[STORYSEQ] = 1000000;
}

ShipExitSeq::~ShipExitSeq()
{
	delete storySeq;
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
	case STORYSEQ:
	{
		if (frame == 0)
		{
			owner->ClearFade();
			owner->SetStorySeq(storySeq);
		}
		
		
		//if (!storySeq->Update(owner->GetPrevInputUnfiltered(0), owner->GetCurrInputUnfiltered(0)))
		if( owner->currStorySequence == NULL )
		{
			frame = stateLength[STORYSEQ] - 1;
			owner->goalDestroyed = true;
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
	else if (state == STORYSEQ)
	{
		//mov.update();
		//target->draw(mov);
		storySeq->Draw(target);
	}
	
}

void ShipExitSeq::Reset()
{
	frame = 0;
	state = SHIP_SWOOP;
	storySeq->Reset();
}


BasicMovieSeq::BasicMovieSeq(GameSession *p_owner,
	const std::string &movieName, int preMovieLength, int postMovieLength)
	:owner(p_owner)
{

	startHolder = new ButtonHolder( 60 );
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
	delete startHolder;
}
void BasicMovieSeq::Reset()
{
	state = PREMOVIE;
	frame = 0;
	startHolder->Reset();
}

bool BasicMovieSeq::Update()
{
	Actor *player = owner->GetPlayer(0);

	bool start = owner->GetController(0).GetState().A;

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
		startHolder->Update(start);
		if (startHolder->IsHoldComplete())
		{
			frame = stateLength[PLAYMOVIE] - 1;
			mov.stop();
		}
		sfe::Status movStatus = mov.getStatus();
		if (frame == 0)
		{
			owner->ClearFade();
			owner->state = GameSession::SEQUENCE;
			mov.setPlayingOffset(sf::Time::Zero);
			mov.setVolume(owner->mainMenu->config->GetData().musicVolume);
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

bool FlashedImage::IsDone()
{
	return (!flashing && frame > 0);
}

int FlashedImage::GetNumFrames()
{
	return aFrames + hFrames + dFrames;
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

bool FlashedImage::IsFadingIn()
{
	return (flashing && frame < aFrames);
}

bool FlashedImage::IsHolding()
{
	return (flashing && frame >= aFrames && frame < aFrames + hFrames);
}

void FlashedImage::StopHolding()
{
	frame = aFrames + hFrames;
}

void FlashedImage::Update()
{
	if (!flashing)
		return;

	int a = 0;
	if (IsFadingIn())
	{
		a = (frame / (float)aFrames) * 255.f;
	}
	else if (IsHolding())
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
