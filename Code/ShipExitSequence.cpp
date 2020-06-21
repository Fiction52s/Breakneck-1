#include "Sequence.h"
#include "GameSession.h"
#include "Actor.h"
#include "MusicPlayer.h"
#include "MainMenu.h"
#include "EditSession.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

ShipExitScene::ShipExitScene()
{
	enterTime = 60;
	exitTime = 60 + 60;
	center.AddLineMovement(V2d(0, 0), V2d(0, 0),
		CubicBezier(0, 0, 1, 1), 60);
	shipMovement.AddLineMovement(V2d(0, 0),
		V2d(0, 0), CubicBezier(0, 0, 1, 1), enterTime);
	shipMovement.AddLineMovement(V2d(0, 0),
		V2d(0, 0), CubicBezier(0, 0, 1, 1), exitTime);

	ts_ship = sess->GetTileset("Ship/ship_exit_864x540.png", 864, 540);
	shipSprite.setTexture(*ts_ship->texture);
	shipSprite.setTextureRect(ts_ship->GetSubRect(0));
	shipSprite.setOrigin(421, 425);
}

void ShipExitScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHIP_SWOOP] = -1;
}

void ShipExitScene::AddShots()
{
}

void ShipExitScene::AddEnemies()
{
}

void ShipExitScene::AddFlashes()
{
}

void ShipExitScene::ReturnToGame()
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
		game->EndLevel();
	}
}

void ShipExitScene::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case SHIP_SWOOP:
	{
		int shipOffsetY = -200;
		int pOffsetY = -170;
		int sOffsetY = pOffsetY;//shipOffsetY + pOffsetY;
		int jumpLength = 6 * 5;
		int startGrabWire = enterTime - jumpLength;

		if (frame == 0)
		{
			sess->cam.SetManual(true);
			center.movementList->start = V2d(sess->cam.pos.x, sess->cam.pos.y);
			center.movementList->end = V2d(sess->GetPlayer(0)->position.x,
				sess->GetPlayer(0)->position.y - 200);

			center.Reset();
			sess->cam.SetMovementSeq(&center, false);

			abovePlayer = V2d(player->position.x, player->position.y - 300);

			shipMovement.movementList->start = abovePlayer + V2d(-1500, -900);//player->position + V2d( -1000, sOffsetY );
			shipMovement.movementList->end = abovePlayer;//player->position + V2d( 1000, sOffsetY );
			shipMovement.Reset();

			Movement *m = shipMovement.movementList->next;

			m->start = abovePlayer;
			m->end = abovePlayer + V2d(1500, -900) + V2d(1500, -900);

			origPlayer = sess->GetPlayer(0)->position;
			attachPoint = abovePlayer;//V2d(player->position.x, player->position.y);//abovePlayer.y + 170 );
		}
		else  if (frame == startGrabWire)
		{
			sess->GetPlayer(0)->GrabShipWire();
		}

		for (int i = 0; i < NUM_MAX_STEPS; ++i)
		{
			shipMovement.Update();
		}

		int jumpSquat = startGrabWire + 3 * 5;
		int startJump = 4 * 5;//60 - jumpSquat;

		if (frame > enterTime)
		{
			sess->GetPlayer(0)->position = V2d(shipMovement.position.x, shipMovement.position.y + 48.0);
		}
		else if (frame >= jumpSquat && frame <= enterTime)//startJump )
		{
			double adjF = frame - jumpSquat;
			double eTime = enterTime - jumpSquat;
			double a = adjF / eTime;//(double)(frame - (60 - (startJump + 1))) / (60 - (startJump - 1));
									//double a = 
									//cout << "a: " << a << endl;
			V2d pAttachPoint = attachPoint;
			pAttachPoint.y += 48.f;
			sess->GetPlayer(0)->position = origPlayer * (1.0 - a) + pAttachPoint * a;
		}

		if (shipMovement.currMovement == NULL)
		{
			frame = stateLength[SHIP_SWOOP] - 1;
			sess->mainMenu->musicPlayer->FadeOutCurrentMusic(30);
		}

		if (frame == (enterTime + exitTime) - 60)
		{
			sess->Fade(false, 60, Color::Black);
		}

		shipSprite.setPosition(shipMovement.position.x,
			shipMovement.position.y);
		break;
	}
	}
}

void ShipExitScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (state == SHIP_SWOOP)
	{
		target->draw(shipSprite);
	}
}