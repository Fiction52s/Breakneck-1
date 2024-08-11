#include "LogSequence.h"
#include "GameSession.h"
#include "Enemy_LogItem.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "PlayerRecord.h"

using namespace sf;
using namespace std;

GetLogSequence::GetLogSequence()
{
	//sess = Session::GetSession();

	freezeFrame = 100;

	emitter = new ShapeEmitter(6, DrawLayer::BETWEEN_PLAYER_AND_ENEMIES);// , PI / 2.0, 2 * PI, 1.0, 2.5);
	emitter->CreateParticles();
	emitter->SetRatePerSecond(120);

	for (int i = 0; i < 5; ++i)
	{
		geoGroup.AddGeo(new SpinningTri(0 + i * PI / 2.5), 30);
	}
	//geoGroup.AddGeo(new Laser(0));

	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::White, Color(100, 0, 0, 0), 30), 0);
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 30));
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 30), 10);
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Blue, Color(0, 0, 100, 0), 30), 20);
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::White, Color(0, 100, 0, 0), 30), 45);

	Color c = Color::White;
	c.a = 100;
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(100, 0, 0, 0), 30), 0);
	c = Color::Cyan;
	c.a = 100;
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30));

	//c = Color::Red;
	//c.a = 100;

	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30), 10);

	c = Color::Blue;
	c.a = 100;
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30), 20);

	c = Color::White;
	c.a = 100;
	geoGroup.AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 100, 0, 0), 30), 45);

	geoGroup.Init();

	log = NULL;

	logPop = sess->logPop;

	myData.confirmFrame = 0;

	SetRectColor(overlayRect, Color(100, 100, 100, 100));
	SetRectCenter(overlayRect, 1920, 1080, Vector2f(960, 540));
}

void GetLogSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[GET] = 1000000;
}

void GetLogSequence::ReturnToGame()
{

	//Actor *player = sess->GetPlayer(0);
	////the shard might have changed
	////your drain rate

	//sess->cam.EaseOutOfManual(60);
	//player->SetAction(Actor::JUMP);
	//player->frame = 1;
	//sess->cam.StopRumble();
}


GetLogSequence::~GetLogSequence()
{
	delete emitter;
}

void GetLogSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (seqData.state)
	{
	case GET:
	{
		if (seqData.frame == 0)
		{
			sess->cam.SetManual(true);
			sess->cam.Ease(Vector2f(player->position), 1, 60, CubicBezier());
			sess->cam.SetRumble(10, 10, 90);
			player->UpdatePrePhysics();
			player->UpdatePostPhysics();
			sess->SetGameSessionState(GameSession::FROZEN);
		}

		if (seqData.frame < freezeFrame)
		{
			//sess->UpdateCamera();

			if (!geoGroup.Update())
			{
				//shouldn't happen
				assert(0);
				seqData.frame = stateLength[seqData.state] - 1;
			}
		}
		if (seqData.frame == freezeFrame)
		{
			logPop->SetInfoInEditor();
			emitter->SetOn(false);
		}
		else if (seqData.frame > freezeFrame)
		{
			if (PlayerPressedConfirm() && sess->GetGameSessionState() == GameSession::FROZEN)
			{
				sess->SetGameSessionState(GameSession::RUN);

				myData.confirmFrame = seqData.frame;
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
				if (!geoGroup.Update())
				{
					seqData.frame = stateLength[seqData.state] - 1;
				}
			}

		}

		if (sess->GetGameSessionState() == GameSession::FROZEN)
		{
			player->UpdateAllEffects();
		}
		/*if (sess->GetGameSessionState() == GameSession::RUN)
		{
			
		}*/
	}
	}

	if (seqData.frame >= freezeFrame && sess->GetGameSessionState() == GameSession::FROZEN)
	{
		logPop->Update();
	}
	//++frame; //i think this skips frames

	//emitter->Update();

}

void GetLogSequence::LayeredDraw(int layer, RenderTarget *target)
{
	if (target == sess->preScreenTex)
	{
		sess->DrawEmitters(layer, target);
		if (layer == DrawLayer::BETWEEN_PLAYER_AND_ENEMIES)
		{
			geoGroup.Draw(target);
		}
	}
	else
	{
		if (seqData.frame >= freezeFrame && sess->GetGameSessionState() == GameSession::FROZEN)
		{
			if (layer == DrawLayer::UI_FRONT)
			{
				if (target == sess->pauseTex)
				{
					target->draw(overlayRect, 4, sf::Quads);
					logPop->Draw(target);
				}
			}
		}
	}
}

void GetLogSequence::Reset()
{
	Sequence::Reset();
	if (log != NULL)
	{
		Vector2f pPos = Vector2f(sess->GetPlayer(0)->position);
		seqData.frame = 0;
		seqData.state = GET;
		geoGroup.SetBase(pPos);
		geoGroup.Reset();
		geoGroup.Start();

		myData.confirmFrame = 0;

		assert(log != NULL);

		if (sess->IsSessTypeGame())
		{
			logPop->logPreview = log->logPreview;
		}
		else
		{
			//logpop already has its own logpreview
		}
		

		logPop->SetCenter(Vector2f(960, 800));
		logPop->SetLog(log->logWorld, log->localIndex);
		logPop->Reset();

		emitter->SetPos(Vector2f(pPos));
		emitter->Reset();
		sess->AddEmitter(emitter);
	}

}

void GetLogSequence::SetIDs()
{
	SetIDAndAddToAllSequencesVec();

	emitter->SetIDAndAddToAllEmittersVec();
}

int GetLogSequence::GetNumStoredBytes()
{
	return sizeof(seqData) + geoGroup.GetNumStoredBytes() + sizeof( myData );
}

//emitters already get stored
void GetLogSequence::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &seqData, sizeof(seqData));
	bytes += sizeof(seqData);

	geoGroup.StoreBytes(bytes);
	bytes += geoGroup.GetNumStoredBytes();

	memcpy(bytes, &myData, sizeof(myData));
	bytes += sizeof(myData);
}

void GetLogSequence::SetFromBytes(unsigned char *bytes)
{
	memcpy(&seqData, bytes, sizeof(seqData));
	bytes += sizeof(seqData);

	geoGroup.SetFromBytes(bytes);
	bytes += geoGroup.GetNumStoredBytes();

	memcpy(&myData, bytes, sizeof(myData));
	bytes += sizeof(myData);

}