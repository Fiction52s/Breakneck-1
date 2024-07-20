#include "WarpTransitionSequence.h"
#include "Session.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "GameSession.h"
#include "Barrier.h"
#include "HUD.h"

using namespace sf;
using namespace std;

WarpTransitionSequence::WarpTransitionSequence()
{
	sess = Session::GetSession();
	bonus = NULL;
	//emitter = new ShapeEmitter(6, 300);// , PI / 2.0, 2 * PI, 1.0, 2.5);
	//emitter->CreateParticles();
	//emitter->SetRatePerSecond(120);



	//geoGroup = new MovingGeoGroup;

	//for (int i = 0; i < 5; ++i)
	//{
	//	geoGroup->AddGeo(new SpinningTri(0 + i * PI / 2.5), 30);
	//}


	////geoGroup->AddGeo( )
	//geoGroup->AddGeo(new Laser(0));
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	Color::White, Color(100, 0, 0, 0), 30), 0);
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	Color::Cyan, Color(0, 0, 100, 0), 30));
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	Color::Cyan, Color(0, 0, 100, 0), 30), 10);
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	Color::Blue, Color(0, 0, 100, 0), 30), 20);
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	Color::White, Color(0, 100, 0, 0), 30), 45);

	//Color c = Color::White;
	//c.a = 100;
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	c, Color(100, 0, 0, 0), 30), 0);
	//c = Color::Cyan;
	//c.a = 100;
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	c, Color(0, 0, 100, 0), 30));

	////c = Color::Red;
	////c.a = 100;

	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	c, Color(0, 0, 100, 0), 30), 10);

	//c = Color::Blue;
	//c.a = 100;
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	c, Color(0, 0, 100, 0), 30), 20);

	//c = Color::White;
	//c.a = 100;
	//geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
	//	c, Color(0, 100, 0, 0), 30), 45);

	//geoGroup->Init();

	//SetRectColor(overlayRect, Color(100, 100, 100, 100));
	//SetRectCenter(overlayRect, 1920, 1080, Vector2f(960, 540));
}

WarpTransitionSequence::~WarpTransitionSequence()
{
	//delete emitter;
	//delete geoGroup;
}


void WarpTransitionSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[FADEOUT] = 60;//-1;//60;
}

void WarpTransitionSequence::ReturnToGame()
{
	sess->cam.EaseOutOfManual(60);
	sess->cam.StopRumble();
}

void WarpTransitionSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);

	switch (seqData.state)
	{
	case FADEOUT:
	{
		if (seqData.frame == 0)
		{
			sess->cam.SetManual(true);
			player->Wait();
			sess->HideHUD(60);
			sess->Fade(false, 60, Color::Black, false, DrawLayer::IN_FRONT);
			barrier->Trigger();


			//player->EndLevelWithoutGoal();

			


			//sess->cam.Ease(Vector2f(player->position), 1, 60, CubicBezier());
			//sess->cam.SetRumble(10, 10, 90);
		}

		if (seqData.frame == stateLength[seqData.state] - 1)
		{
			GameSession *game = GameSession::GetSession();
			

			if (game != NULL)
			{
				game->SetBonus(bonus, V2d(0, 0));
				game->ClearFade();


				//sess->SetGameSessionState(GameSession::RUN);
			}

		}

		/*if (frame == 60)
		{
			sess->Fade(false, 14, Color::White, true);
		}*/

		/*int freezeFrame = 100;
		if (frame == freezeFrame)
		{
		owner->state = GameSession::FROZEN;
		emitter->SetOn(false);
		}
		else if (frame > freezeFrame)
		{
		if (owner->GetCurrInputUnfiltered(0).A && !owner->GetPrevInputUnfiltered(0).A)
		{
		owner->state = GameSession::RUN;
		}
		}*/

		/*if (sess->GetGameSessionState() == Session::RUN)
			if (!geoGroup->Update())
			{
				frame = stateLength[state] - 1;
			}*/
	}
	}
	//++frame;

	//emitter->Update();
}

void WarpTransitionSequence::Draw(RenderTarget *target, DrawLayer layer)
{
	//sess->DrawEmitters(layer, target);
	if (layer == DrawLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{
		//geoGroup->Draw(target);
		/*if (state != END)
		{

		}*/
	}
	else if (layer == DrawLayer::UI_FRONT)
	{
		if (/*state != END && */sess->GetGameSessionState() == Session::FROZEN)
		{
			//target->draw(overlayRect, 4, sf::Quads);
		}
	}
}

void WarpTransitionSequence::Reset()
{
	Sequence::Reset();
	//Vector2f pPos = Vector2f(sess->GetPlayer(0)->position);
	seqData.frame = 0;
	seqData.state = FADEOUT;
}