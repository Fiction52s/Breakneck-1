#include "ShardSequence.h"
#include "GameSession.h"
#include "Enemy_Shard.h"
#include "ParticleEffects.h"
#include "Actor.h"
#include "PlayerRecord.h"

using namespace sf;
using namespace std;

GetShardSequence::GetShardSequence()
{
	//sess = Session::GetSession();

	emitter = new ShapeEmitter(6, 300);// , PI / 2.0, 2 * PI, 1.0, 2.5);
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

	shard = NULL;

	shardPop = sess->shardPop;

	SetRectColor(overlayRect, Color(100, 100, 100, 100));
	SetRectCenter(overlayRect, 1920, 1080, Vector2f(960, 540));
}

void GetShardSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[GET] = 1000000;
}

void GetShardSequence::ReturnToGame()
{
	
	Actor *player = sess->GetPlayer(0);
	//the shard might have changed
	//your drain rate

	sess->cam.EaseOutOfManual(60);
	player->SetAction(Actor::JUMP);
	player->frame = 1;
	sess->cam.StopRumble();
}


GetShardSequence::~GetShardSequence()
{
	delete emitter;
}

void GetShardSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);
	switch (state)
	{
	case GET:
	{
		if (frame == 0)
		{
			sess->cam.SetManual(true);
			sess->cam.Ease(Vector2f(player->position), 1, 60, CubicBezier());
			sess->cam.SetRumble(10, 10, 90);
		}

		int freezeFrame = 100;
		if (frame == freezeFrame)
		{
			sess->SetGameSessionState(GameSession::FROZEN);
			emitter->SetOn(false);
		}
		else if (frame > freezeFrame)
		{
			if (PlayerPressedConfirm())
			{
				sess->SetGameSessionState(GameSession::RUN);
			}
		}

		if (sess->GetGameSessionState() == GameSession::RUN)
		{
			if (!geoGroup.Update())
			{
				frame = stateLength[state] - 1;
			}
		}
	}
	}
//++frame; //i think this skips frames

	//emitter->Update();

}

void GetShardSequence::Draw(RenderTarget *target, EffectLayer layer)
{
	if (target == sess->preScreenTex)
	{
		sess->DrawEmitters(layer, target);
		if (layer == EffectLayer::BETWEEN_PLAYER_AND_ENEMIES)
		{
			geoGroup.Draw(target);
		}
	}
	else
	{
		if (layer == EffectLayer::UI_FRONT)
		{
			if (target == sess->pauseTex)
			{
				target->draw(overlayRect, 4, sf::Quads);
				shardPop->Draw(target);
			}
		}

	}
}

void GetShardSequence::Reset()
{
	Sequence::Reset();
	if (shard != NULL)
	{
		Vector2f pPos = Vector2f(sess->GetPlayer(0)->position);
		frame = 0;
		state = GET;
		geoGroup.SetBase(pPos);
		geoGroup.Reset();
		geoGroup.Start();

		assert(shard != NULL);

		shardPop->SetShard(shard->shardWorld, shard->localIndex);
		shardPop->SetCenter(Vector2f(960, 800));
		shardPop->Reset();

		emitter->SetPos(Vector2f(pPos));
		emitter->Reset();
		sess->AddEmitter(emitter, EffectLayer::BETWEEN_PLAYER_AND_ENEMIES);
	}

}