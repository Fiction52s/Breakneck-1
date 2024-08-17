#include "CockpitSequence.h"
#include "Session.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "TutorialBox.h"
#include "GameSession.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

CockpitSequence::CockpitSequence()
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

CockpitSequence::~CockpitSequence()
{
}


void CockpitSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHOW] = 1000000;
}

void CockpitSequence::AddGroups()
{
	//	AddGroup("pre_crawler", "W1/w1_crawler_pre_fight");

}

void CockpitSequence::SetText(const std::string &str)
{
}

void CockpitSequence::SetBoxPos(sf::Vector2f &pos)
{
	//tutBox->SetCenterPos(pos);
}

void CockpitSequence::ReturnToGame()
{
	if (sess->IsSessTypeEdit())
	{
		EditSession *edit = EditSession::GetSession();
		edit->EndTestMode();
		sess->ClearFade();
	}
	else
	{
		GameSession *game = GameSession::GetSession();
		game->resType = GameSession::GameResultType::GR_WINCONTINUE;
		game->EndLevel();
	}
	//sess->cam.EaseOutOfManual(60);
	//sess->cam.StopRumble();
}

void CockpitSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);

	switch (seqData.state)
	{
	case SHOW:
	{
		if (seqData.frame == 0)
		{
			seqData.frame = stateLength[seqData.state] - 1;
			break;
		}

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

				player->SetAction(Actor::INSPECT_END);
				player->frame = 0;
				sess->cam.StopRumble();

				myData.confirmFrame = seqData.frame;

				//myInspectObject->HideInspectable();

				seqData.frame = stateLength[seqData.state] - 1;
			}

			//if (seqData.frame == myData.confirmFrame + 2)
			//{
			//	Actor *player = sess->GetPlayer(0);
			//	//sess->cam.EaseOutOfManual(60);
			//	//player->SetAirPos(player->position, player->facingRight);
			//	player->SetAction(Actor::INSPECT_END);
			//	player->frame = 0;
			//	sess->cam.StopRumble();
			//	
			//}

			//if (sess->GetGameSessionState() == GameSession::RUN)
			//{
			//	//if (!geoGroup.Update())
			//	//{
			//		seqData.frame = stateLength[seqData.state] - 1;
			//	//}
			//}


		}

		/*if (sess->GetGameSessionState() == GameSession::FROZEN)
		{
		player->UpdateAllEffects();
		}*/
		break;
	}
	}

}

void CockpitSequence::LayeredDraw(int layer, RenderTarget *target)
{
	if (layer == DrawLayer::UI_FRONT)
	{
		//target->draw(inspectQuad, 4, sf::Quads, ts_inspect->texture);
		//tutBox->Draw(target);
	}
	//Sequence::Draw(target, layer);
}

void CockpitSequence::Reset()
{
	Sequence::Reset();
	seqData.state = SHOW;
	seqData.frame = 0;
	myData.confirmFrame = 0;

	//SetRectSubRect(inspectQuad, ts_inspect->GetSubRect(0));
}