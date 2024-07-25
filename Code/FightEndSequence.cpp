#include "FightEndSequence.h"
#include "GameSession.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "GameMode.h"

using namespace sf;
using namespace std;

FightEndSequence::FightEndSequence()
{
	sess = Session::GetSession();

	ts_game = sess->GetSizedTileset("HUD/Multiplayer/GAME_384x256.png");
	ts_game->SetSpriteTexture(gameSprite);

	gameSprite.setOrigin(gameSprite.getLocalBounds().width / 2, gameSprite.getLocalBounds().height / 2);
	gameSprite.setPosition(960, 540);
}

FightEndSequence::~FightEndSequence()
{
}


void FightEndSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[FREEZE] = 120;
}

void FightEndSequence::ReturnToGame()
{
	sess->cam.EaseOutOfManual(60);
	sess->cam.StopRumble();
	sess->EndLevel();
}

void FightEndSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);

	switch (seqData.state)
	{
	case FREEZE:
	{
		if (seqData.frame == 0)
		{
			sess->cam.SetManual(true);
			sess->cam.SetRumble(10, 10, 90);

			sess->SetGameSessionState(GameSession::FROZEN);

			if (sess->gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE && !sess->IsParallelSession())
			{
				ParallelRaceMode *prm = (ParallelRaceMode*)sess->gameMode;
				
				for (int i = 0; i < 3; ++i)
				{
					if (prm->parallelGames[i] != NULL)
					{
						prm->parallelGames[i]->SetGameSessionState(GameSession::FROZEN);
					}
				}
			}
			//sess->cam.Ease(Vector2f(player->position), 1, 60, CubicBezier());
			//sess->cam.SetRumble(10, 10, 90, 4);
		}
	}
	}
	//++frame;

	//emitter->Update();
}

void FightEndSequence::Draw(RenderTarget *target, int layer)
{
	if (layer == DrawLayer::UI_FRONT)
	{
		if (sess->GetGameSessionState() == GameSession::FROZEN)
		{
			target->draw(gameSprite);
		}
	}
}

void FightEndSequence::Reset()
{
	Sequence::Reset();
}