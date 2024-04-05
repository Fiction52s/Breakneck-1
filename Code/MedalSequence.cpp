#include "Sequence.h"
#include "GameSession.h"
#include "Actor.h"
#include "MusicPlayer.h"
#include "MainMenu.h"
#include "EditSession.h"
#include "GameSession.h"
#include "MedalSequence.h"
#include "Medal.h"

using namespace std;
using namespace sf;

MedalSequence::MedalSequence()
{
	shownMedal = new Medal;
	medalType = -1;
}

MedalSequence::~MedalSequence()
{
	delete shownMedal;
}

void MedalSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHOW_MEDAL] = -1;
}

void MedalSequence::StartRunning()
{
	shownMedal->Reset();
	assert(medalType >= 0);
	shownMedal->SetType(medalType);
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