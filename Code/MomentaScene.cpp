#include "Sequence.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

MomentaScene::MomentaScene()
{

}

void MomentaScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[BROADCAST] = 120;
}

void MomentaScene::ReturnToGame()
{
	sess->SetGameSessionState(GameSession::RUN);
}

void MomentaScene::AddFlashes()
{

}

void MomentaScene::UpdateState()
{
	if (seqData.frame == 0)
	{
		sess->SetGameSessionState(GameSession::FROZEN);
	}
	
}

void MomentaScene::LayeredDraw(int layer, RenderTarget *target)
{

}