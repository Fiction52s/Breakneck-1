#include "GreyWarpSequence.h"
#include "Session.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "GameSession.h"
#include "Barrier.h"
#include "HUD.h"

using namespace sf;
using namespace std;

GreyWarpSequence::GreyWarpSequence()
{
	sess = Session::GetSession();
	bonus = NULL;
}

GreyWarpSequence::~GreyWarpSequence()
{
}


void GreyWarpSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADEOUT] = 60;
}

void GreyWarpSequence::ReturnToGame()
{
	sess->cam.EaseOutOfManual(60);
	sess->cam.StopRumble();
}

void GreyWarpSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);

	switch (state)
	{
	case FADEOUT:
	{
		if (frame == 0)
		{
			sess->cam.SetManual(true);
			player->Wait();
			sess->hud->Hide(60);
			sess->Fade(false, 60, Color::Black, false);
			barrier->Trigger();
		}

		if (frame == stateLength[state] - 1)
		{
			GameSession *game = GameSession::GetSession();


			if (game != NULL)
			{
				game->SetBonus(bonus, V2d(0, 0));
				game->ClearFade();
				//sess->SetGameSessionState(GameSession::RUN);
			}
		}
	}
	}
}

void GreyWarpSequence::Draw(RenderTarget *target, EffectLayer layer)
{
	if (layer == EffectLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{
	}
	else if (layer == EffectLayer::UI_FRONT)
	{
		//if (/*state != END && */sess->GetGameSessionState() == Session::FROZEN)
		//{
		//	//target->draw(overlayRect, 4, sf::Quads);
		//}
	}
}

void GreyWarpSequence::Reset()
{
	Sequence::Reset();
	//Vector2f pPos = Vector2f(sess->GetPlayer(0)->position);
	frame = 0;
	state = FADEOUT;
}