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



	stateLength[KIN_KNEELING] = 60;
	stateLength[START_MEDITATE] = 60;
	stateLength[FADE_BACKGROUND] = 60;
	stateLength[EXPEL_ENERGY] = 60;
	stateLength[MASKOFF] = 120;
	stateLength[FADE_BACK] = 90;

	//ts_darkAura = owner->GetTileset("Kin/dark_aura_w1_384x384.png", 384, 384);
	//darkAuraSprite.setTexture(*ts_darkAura->texture);

	darkRect.setFillColor(Color::Black);
	darkRect.setSize(Vector2f(1920, 1080));
	darkRect.setPosition(0, 0);

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
	case KIN_KNEELING:
		if (frame == 0)
		{
			player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect( 0 ));
			player->dirtyAuraSprite.setOrigin(player->dirtyAuraSprite.getLocalBounds().width / 2,
				player->dirtyAuraSprite.getLocalBounds().height / 2);
			
			player->SeqKneel();
		}
		
		break;
	case START_MEDITATE:
		if (frame == 0)
		{
			player->SeqMeditateMaskOn();
		}
		break;
	case FADE_BACKGROUND:
		if (frame == 0)
		{
			owner->Fade(false, 60, Color::Black, true);
		}
		else if (frame == stateLength[FADE_BACKGROUND]-1)
		{
			owner->state = GameSession::SEQUENCE;
		}
		break;
	case EXPEL_ENERGY:
		if (frame == 0)
		{	
			owner->ClearFade();
		}
		

		if (frame < 10 * 3)
		{
			player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect(frame / 3 + 15));
		}
		

		break;
	case MASKOFF:
		if (frame == 0)
		{
			player->SeqMaskOffMeditate();
		}

		if (frame == stateLength[MASKOFF] - 1)
		{
			owner->state = GameSession::RUN;
			owner->Fade(true, 60, Color::Black, true);
		}
		
		break;
	case FADE_BACK:
		if (frame == 0)
		{
			
		}
		else if (frame == 60)
		{
			player->SeqGetAirdash();
		}
		
		
		break;
	}

	if (state != EXPEL_ENERGY)
	{
		player->dirtyAuraSprite.setTextureRect(player->ts_dirtyAura->GetSubRect((frame % (15 * 3) / 3)));
	}
	
	player->dirtyAuraSprite.setPosition(Vector2f(player->sprite->getPosition().x, 
		player->sprite->getPosition().y - 32));

	if (owner->state == GameSession::SEQUENCE)
	{
		owner->totalGameFrames++;
		player->UpdatePrePhysics();
		player->UpdatePostPhysics();
	}
	

	

	


	++frame;

	return true;
}
void GetAirdashPowerSeq::Draw(sf::RenderTarget *target, EffectLayer layer)
{

	if (layer == EffectLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{		
	}

	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (owner->state == GameSession::SEQUENCE)
	{
		target->setView(owner->uiView);
		/*sf::View v = target->getView();
		target->setView(owner->uiView);*/

		if (state >= EXPEL_ENERGY)
		{
			//darkRect.setPosition(owner->cam.pos);
			target->draw(darkRect);
		}

		//target->setView(v);



		target->setView(owner->view);


		owner->GetPlayer(0)->Draw(target);
	}
}
void GetAirdashPowerSeq::Reset()
{
	state = KIN_KNEELING;
	frame = 0;
}