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

GetAirdashPowerSeq::GetAirdashPowerSeq(GameSession *p_owner)
	:owner(p_owner)
{
	//SetRectCenter(darkQuad, 1920, 1080, Vector2f(960, 540));// , Vector2f(pi->pos));
	//SetRectColor(darkQuad, Color(Color::Red));

	//state = ENTERCORE;



	stateLength[ENTERCORE] = 30;
	stateLength[DESTROYCORE] = 30;
	stateLength[FADEEXIT] = 9 * 3;
	stateLength[EXITCORE] = 180;



	Reset();
}

bool GetAirdashPowerSeq::Update()
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
		return false;
	}



	switch (state)
	{
	case ENTERCORE:
		break;
	case DESTROYCORE:
		break;
	case FADEEXIT:
		break;
		break;
	case EXITCORE:
		break;
	}

	++frame;

	return true;
}
void GetAirdashPowerSeq::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}
}
void GetAirdashPowerSeq::Reset()
{
	state = ENTERCORE;
	frame = 0;
}