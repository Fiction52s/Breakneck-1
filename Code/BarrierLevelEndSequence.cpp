#include "BarrierLevelEndSequence.h"
#include "GameSession.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "GameMode.h"

using namespace sf;
using namespace std;

BarrierLevelEndSequence::BarrierLevelEndSequence()
{
	sess = Session::GetSession();
}

BarrierLevelEndSequence::~BarrierLevelEndSequence()
{
}


void BarrierLevelEndSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADE] = 60;
}

void BarrierLevelEndSequence::ReturnToGame()
{
	//sess->cam.EaseOutOfManual(60);
	//sess->cam.StopRumble();
	//sess->EndLevel();
}

void BarrierLevelEndSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);

	switch (seqData.state)
	{
	case FADE:
	{
		if (seqData.frame == 0)
		{

		}
	}
	}
	//++frame;

	//emitter->Update();
}

void BarrierLevelEndSequence::Draw(RenderTarget *target, EffectLayer layer)
{
	if (layer == EffectLayer::UI_FRONT)
	{
	}
}

void BarrierLevelEndSequence::Reset()
{
	Sequence::Reset();
}