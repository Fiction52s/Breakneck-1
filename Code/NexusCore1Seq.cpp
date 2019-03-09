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
	stateLength[ENTERCORE] = 30;
	stateLength[DESTROYCORE] = 52 * 2;
	stateLength[FADEEXIT] = 60;
	stateLength[EXITCORE] = 30;

	string base = "Resources/Nexus/nexus_core_1920x1080_";
	string num;
	string end = ".png";
	for (int i = 0; i < 52; ++i)
	{
		num = to_string(i+1);
		imageNames[i] = base + num + end;
		//bool succ = coreImages[i].create(1920, 1080,Color::Red);///loadFromFile(base + num + end);
		//coreImages[i].create(1920, 1080, Color::Red);
		//assert(succ);
		
		//ts_core[i] = owner->GetTileset(base + num + end, 1920, 1080);
	}

	ts_firstCore = owner->GetTileset( "Nexus/nexus_core_1920x1080_1.png", 1920, 1080);
	/*for (int i = 0; i < 5; ++i)
	{
		coreImages[0].loadFromFile(imageNames[frame]);
		tex.loadFromImage(coreImages[0]);
		coreSprite.setTexture(tex);
	}*/

	
	//coreImages[0].loadFromFile();
	//coreSprite.setTexture(*ts_core[->texture);

	//ts_core[0] = owner->GetTileset("Nexus/nexus_core_1920x1080_1.png", 1920, 1080);
	
	Reset();
	

	loadThread = new boost::thread(LoadNextTex, this);

	/*while (!ShouldLoad())
	{

	}*/
	
	//Reset();
}

void NexusCore1Seq::LoadNextTex( NexusCore1Seq *seq )
{
	while (!seq->ThreadEnded())
	{
		if (seq->ShouldLoadNext())
		{
			//seq->mut1.lock();
			//seq->mut.lock();
			seq->shouldLoad = false;
			//seq->doneLoading = false;
			//seq->coreImages[seq->ci].loadFromFile(seq->imageNames[seq->loadIndex]);
			
			//seq->mut4.lock();
			seq->coreTex[seq->ci].loadFromFile(seq->imageNames[seq->loadIndex]);
			seq->ci = !seq->ci;
			//seq->mut4.unlock();
			//seq->coreImages[1].loadFromFile(seq->imageNames[seq->loadIndex+1]);
			seq->doneLoading = true;

			//seq->mut1.unlock();
			//seq->coreImages[1].copy(seq->coreImages[0],0,0);
		}
	}
}

NexusCore1Seq::~NexusCore1Seq()
{
	endThread = true;
	loadThread->join();
	delete loadThread;
}

bool NexusCore1Seq::ThreadEnded()
{
	mut.lock();
	bool b = endThread;
	mut.unlock();
	return b;
}

bool NexusCore1Seq::ShouldLoadNext()
{
	mut.lock();
	bool b = shouldLoad;
	mut.unlock();
	return b;
}

bool NexusCore1Seq::ShouldLoad()
{
	mut1.lock();
	bool b = doneLoading;
	mut1.unlock();
	return b;
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
		owner->Fade(true, 60, sf::Color::White);
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
			owner->Fade(true, 30, sf::Color::Black);

			//coreImages[0].loadFromFile(imageNames[0]);
			
			//coreSprite.setTexture(tex);
			//tex.update(coreImages[0]);
			//tex.loadFromImage(coreImages[0]);
			
			

			//tex.loadFromFile(imageNames[0]);
			//coreSprite.setTexture(tex);

			//owner->ClearFade();
		}

		break;
	case DESTROYCORE:
	{
		if (frame == 0)
		{
			//tex.update(coreImages[frame / 4 + 10]);
		}

		


		if (frame % 2 == 0)
		{
			while (!ShouldLoad())
			{

			}
			loadIndex = frame / 2;
			doneLoading = false;

			shouldLoad = true;

			mut4.lock();
			int c = !ci;
			mut4.unlock();
			//loadFromImage(coreImages[c]);
			coreSprite.setTexture(coreTex[c]);
		}
		

		

		

		

		//coreImages[0].loadFromFile(imageNames[frame]);
		//tex.loadFromImage(coreImages[0]);
		//tex.loadFromFile(imageNames[frame]);
		
		//coreSprite.setTexture(tex);
		//coreSprite.setTexture(//*ts_core[frame / 4]->texture);
		break;
	}
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
		if( state <= DESTROYCORE )
			target->draw(coreSprite);
	}
	
}
void NexusCore1Seq::Reset()
{
	state = FADETOBLACK;
	frame = 0;
	ci = 0;
	endThread = false;
	doneLoading = false;
	loadIndex = 0;
	shouldLoad = true;

	coreSprite.setTexture(*ts_firstCore->texture);
	//coreSprite.setTexture(coreTex[c]);
}