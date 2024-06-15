#include "Sequence.h"
#include "GameSession.h"
#include "Actor.h"
#include "MusicPlayer.h"
#include "MainMenu.h"
#include "EditSession.h"
#include "GameSession.h"
#include "MedalSequence.h"
#include "Medal.h"
#include "Enemy_Shard.h"

using namespace std;
using namespace sf;

MedalSequence::MedalSequence( TilesetManager *tm )
{
	shownMedal = new Medal(tm);

	SetRectColor(overlayRect, Color(100, 100, 100, 100));
	SetRectCenter(overlayRect, 1920, 1080, Vector2f(960, 540));


	//shardPop->SetShard(shard->shardWorld, shard->localIndex);
	//shardPop->SetCenter(Vector2f(960, 800));
	//shardPop->Reset();
}

MedalSequence::~MedalSequence()
{
	delete shownMedal;
}

void MedalSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHOW_MEDAL] = -1;
	stateLength[END] = -1;
}

void MedalSequence::StartGold()
{
	shownMedal->Reset();
	shownMedal->SetType(Medal::MEDAL_GOLD);

	StartRunning();
}

void MedalSequence::StartSilver()
{
	shownMedal->Reset();
	shownMedal->SetType(Medal::MEDAL_SILVER);

	StartRunning();
}

void MedalSequence::StartBronze()
{
	shownMedal->Reset();
	shownMedal->SetType(Medal::MEDAL_BRONZE);

	StartRunning();
}

void MedalSequence::StartRunning()
{
	shownMedal->SetCenter(Vector2f(960, 300));
	shownMedal->SetScale(2.f);
	shownMedal->Show();
}

void MedalSequence::AddShots()
{
}

void MedalSequence::AddEnemies()
{
}

void MedalSequence::AddFlashes()
{
}

void MedalSequence::ReturnToGame()
{
	/*if (sess->IsSessTypeEdit())
	{
		EditSession *edit = EditSession::GetSession();
		edit->EndTestMode();
		sess->ClearFade();
	}
	else
	{
		GameSession *game = GameSession::GetSession();
		game->EndLevel();
	}*/
}

void MedalSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (seqData.state)
	{
	case SHOW_MEDAL:
	{
		if (seqData.frame == 0)
		{
		}
		shownMedal->Update();
		if (shownMedal->IsWaiting())
		{
			seqData.state = END;
			EndCurrState();
		}
		break;
	}
	}
}

void MedalSequence::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::UI_FRONT)
	{
		return;
	}

	if (seqData.state == SHOW_MEDAL)
	{
		shownMedal->Draw(target);
	}
}