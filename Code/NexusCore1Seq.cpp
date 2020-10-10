#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "VectorMath.h"
#include "Camera.h"
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
//#include "EditSession.h"
#include "Zone.h"
#include "Flow.h"
#include "Boss.h"
#include "PowerOrbs.h"
#include "SequenceW1.h"
#include "SoundManager.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "PauseMenu.h"
#include "Movement.h"
#include "ScoreDisplay.h"
#include "Enemy_CrawlerQueen.h"
#include "Nexus.h"
#include "MovingGeo.h"
#include "ParticleEffects.h"

using namespace sf;
using namespace std;

#define TIMESTEP (1.0 / 60.0)

NexusCore1Seq::NexusCore1Seq()
{
	nexus = NULL;
	SetRectCenter(darkQuad, 1920, 1080, Vector2f(960, 540));// , Vector2f(pi->pos));
	SetRectColor(darkQuad, Color(Color::Black));
	
	emitter = new ShapeEmitter(6, 300);// , PI / 2.0, 2 * PI, 1.0, 2.5);
	emitter->CreateParticles();
	emitter->SetRatePerSecond(120);

	for (int i = 0; i < 5; ++i)
	{
		geoGroup.AddGeo(new SpinningTri(0 + i * PI / 2.5), 30);
	}
	//geoGroup.AddGeo(new Laser(0));

	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::White, Color(100, 0, 0, 0), 30), 0);
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 30));
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 30), 10);
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Blue, Color(0, 0, 100, 0), 30), 20);
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::White, Color(0, 100, 0, 0), 30), 45);

	Color c = Color::White;
	c.a = 100;
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(100, 0, 0, 0), 30), 0);
	c = Color::Cyan;
	c.a = 100;
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30));

	//c = Color::Red;
	//c.a = 100;

	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30), 10);

	c = Color::Blue;
	c.a = 100;
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30), 20);

	c = Color::White;
	c.a = 100;
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 100, 0, 0), 30), 45);

	geoGroup.Init();
}

NexusCore1Seq::~NexusCore1Seq()
{
	delete emitter;
}

void NexusCore1Seq::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADETOBLACK] = 31;
	stateLength[ENTERCORE] = 30;
	stateLength[DESTROYCORE] = 1000000;
	stateLength[EXITCORE] = 30;
}

void NexusCore1Seq::ReturnToGame()
{
	sess->Fade(true, 60, sf::Color::White);
	sess->SetGameSessionState(GameSession::RUN);
	Actor *player = sess->GetPlayer(0);
	player->SetAction(Actor::GOALKILLWAIT);
	player->frame = 0;
	
	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		game->scoreDisplay->Activate();
	}
}

void NexusCore1Seq::UpdateState()
{
	switch (state)
	{
	case FADETOBLACK:
		if (frame == 0)
		{
			sess->Fade(false, 30, sf::Color::Black);
		}
		if (frame == stateLength[FADETOBLACK] - 1)
		{
			sess->SetGameSessionState(GameSession::SEQUENCE);
			
		}
		break;
	case ENTERCORE:
		
		if (frame == 0)
		{
			sess->Fade(true, 30, sf::Color::Black);
			//sess->cam.SetManual(true);
			//sess->cam.Set(Vector2f(960, 540), 1, 1);
		}

		break;
	case DESTROYCORE:
	{
		//sess->ClearFade();
		if (!geoGroup.Update())
		{
			frame = stateLength[state] - 1;
			//emitter->SetOn(false);
		}
		//sess->Fade(true, 30, sf::Color::White);
		//frame = stateLength[DESTROYCORE] - 1;
		break;
	}
	case EXITCORE:
		if (frame == 0)
		{
			sess->Fade(false, 30, sf::Color::White);
			nexus->FinishDestruction();
		}
			
		break;
	}
}
void NexusCore1Seq::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	sf::View oldView = target->getView();
	sf::View myView;
	myView.setCenter(960, 540);
	myView.setSize(1920, 1080);
	target->setView(myView);
	if (state >= ENTERCORE && layer == EffectLayer::BEHIND_ENEMIES)
	{
		target->draw(darkQuad, 4, sf::Quads);
	}

	sess->DrawEmitters(layer, target);
	if (layer == EffectLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{
		geoGroup.Draw(target);
	}

	target->setView(oldView);
}
void NexusCore1Seq::Reset()
{
	Sequence::Reset();

	state = FADETOBLACK;
	frame = 0;

	Vector2f pPos(960, 540);

	geoGroup.SetBase(pPos);
	geoGroup.Reset();
	geoGroup.Start();

	//emitter->SetPos(Vector2f(pPos));
	//emitter->Reset();
	//sess->AddEmitter(emitter, EffectLayer::BETWEEN_PLAYER_AND_ENEMIES);
}