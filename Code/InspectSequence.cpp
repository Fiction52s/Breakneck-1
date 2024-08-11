#include "InspectSequence.h"
#include "Session.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "TutorialBox.h"
#include "GameSession.h"
#include "Enemy_InspectObject.h"

using namespace sf;
using namespace std;

InspectSequence::InspectSequence()
{
	sess = Session::GetSession();

	SetRectColor(overlayRect, Color(100, 100, 100, 100));
	SetRectCenter(overlayRect, 1920, 1080, Vector2f(960, 540));

	ts_inspect = NULL;

	myInspectObject = NULL;

	//SetRectColor(inspectQuad, Color::Red);
	//SetRectCenter(inspectQuad, 600, 600, Vector2f(960, 540));
	SetRectCenter(inspectQuad, 1109 / 2, 1060 / 2, Vector2f(960, 540));
}

InspectSequence::~InspectSequence()
{
}


void InspectSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHOW] = 1000000;
}

void InspectSequence::AddGroups()
{
	//	AddGroup("pre_crawler", "W1/w1_crawler_pre_fight");

}

void InspectSequence::SetText(const std::string &str)
{
}

void InspectSequence::SetBoxPos(sf::Vector2f &pos)
{
	//tutBox->SetCenterPos(pos);
}

void InspectSequence::ReturnToGame()
{
	//sess->cam.EaseOutOfManual(60);
	//sess->cam.StopRumble();
}

void InspectSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);

	switch (seqData.state)
	{
	case SHOW:
	{
		if (seqData.frame == 0)
		{
			//sess->cam.SetManual(true);



			//sess->cam.Ease(Vector2f(player->position), 1, 60, CubicBezier());
			//sess->cam.SetRumble(10, 10, 90);
			player->UpdatePrePhysics();
			player->UpdatePostPhysics();
			sess->SetGameSessionState(GameSession::FROZEN);
		}
		else if (seqData.frame > 30)
		{
			if (PlayerPressedConfirm() && sess->GetGameSessionState() == GameSession::FROZEN)
			{
				sess->SetGameSessionState(GameSession::RUN);

				myData.confirmFrame = seqData.frame;

				myInspectObject->HideInspectable();
			}

			if (seqData.frame == myData.confirmFrame + 2)
			{
				Actor *player = sess->GetPlayer(0);
				sess->cam.EaseOutOfManual(60);
				//player->SetAirPos(player->position, player->facingRight);
				player->SetAction(Actor::JUMP);
				player->RestoreAirOptions();
				player->frame = 1;
				sess->cam.StopRumble();
				
			}

			if (sess->GetGameSessionState() == GameSession::RUN)
			{
				//if (!geoGroup.Update())
				//{
					seqData.frame = stateLength[seqData.state] - 1;
				//}
			}

			
		}

		/*if (sess->GetGameSessionState() == GameSession::FROZEN)
		{
			player->UpdateAllEffects();
		}*/
		break;
	}
	}
	
}

void InspectSequence::LayeredDraw(int layer, RenderTarget *target)
{
	if (layer == DrawLayer::UI_FRONT)
	{
		target->draw(inspectQuad, 4, sf::Quads, ts_inspect->texture);
		//tutBox->Draw(target);
	}
	//Sequence::Draw(target, layer);
}

void InspectSequence::Reset()
{
	Sequence::Reset();
	seqData.state = SHOW;
	seqData.frame = 0;
	myData.confirmFrame = 0;

	SetRectSubRect(inspectQuad, ts_inspect->GetSubRect(0));
}