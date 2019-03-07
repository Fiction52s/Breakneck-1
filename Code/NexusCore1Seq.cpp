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

NexusCore1Seq::NexusCore1Seq(GameSession *p_owner)
	:owner(p_owner)
{
	SetRectCenter(darkQuad, 1920, 1080, Vector2f(960, 540));// , Vector2f(pi->pos));
	SetRectColor(darkQuad, Color(Color::Black));
	
	//state = ENTERCORE;

	

	stateLength[FADETOBLACK] = 31;
	stateLength[ENTERCORE] = 60;
	stateLength[DESTROYCORE] = 30;
	stateLength[FADEEXIT] = 9 * 3;
	stateLength[EXITCORE] = 30;



	Reset();
}

bool NexusCore1Seq::Update()
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
		owner->Fade(true, 30, sf::Color::White);
		owner->state = GameSession::RUN;
		player->SetAction(Actor::GOALKILLWAIT);
		player->frame = 0;
		owner->scoreDisplay->Activate();
		return false;
	}



	switch (state)
	{
	case FADETOBLACK:
		if (frame == 0)
		{
			owner->Fade(false, 30, sf::Color::Black);
			//owner->ClearFade();
		}
		if (frame == stateLength[FADETOBLACK] - 1)
		{
			owner->state = GameSession::SEQUENCE;
		}
		break;
	case ENTERCORE:
		
		if (frame == 0)
		{
			owner->ClearFade();
		}
		break;
	case DESTROYCORE:
		break;
	case FADEEXIT:
		
		break;
	case EXITCORE:
		if (frame == 0)
		{
			owner->Fade(false, 30, sf::Color::White);
		}
			
		break;
	}

	++frame;

	return true;
}
void NexusCore1Seq::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (state >= ENTERCORE )
	{
		target->draw(darkQuad, 4, sf::Quads);
	}
}
void NexusCore1Seq::Reset()
{
	state = FADETOBLACK;
	frame = 0;
}