#include "DeathSequence.h"
#include "GameSession.h"
#include "ParticleEffects.h"
#include "Actor.h"

using namespace sf;
using namespace std;

DeathSequence::DeathSequence(GameSession *p_owner)
	:owner(p_owner)
{
	emitter = new ShapeEmitter(6, 300);// , PI / 2.0, 2 * PI, 1.0, 2.5);
	emitter->CreateParticles();
	emitter->SetRatePerSecond(120);

	stateLength[GET] = 1000000;

	geoGroup = new MovingGeoGroup;

	for (int i = 0; i < 5; ++i)
	{
		geoGroup->AddGeo(new SpinningTri(0 + i * PI / 2.5), 30);
	}
	

	//geoGroup->AddGeo( )
	geoGroup->AddGeo(new Laser(0));
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::White, Color(100, 0, 0, 0), 30), 0);
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 30));
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 30), 10);
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Blue, Color(0, 0, 100, 0), 30), 20);
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::White, Color(0, 100, 0, 0), 30), 45);

	Color c = Color::White;
	c.a = 100;
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(100, 0, 0, 0), 30), 0);
	c = Color::Cyan;
	c.a = 100;
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30));

	//c = Color::Red;
	//c.a = 100;

	geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30), 10);

	c = Color::Blue;
	c.a = 100;
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 0, 100, 0), 30), 20);

	c = Color::White;
	c.a = 100;
	geoGroup->AddGeo(new MovingRing(32, 20, 200, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		c, Color(0, 100, 0, 0), 30), 45);

	geoGroup->Init();

	SetRectColor(overlayRect, Color(100, 100, 100, 100));
	SetRectCenter(overlayRect, 1920, 1080, Vector2f(960, 540));
}

DeathSequence::~DeathSequence()
{
	delete emitter;
	delete geoGroup;
}

bool DeathSequence::Update()
{
	Actor *player = owner->GetPlayer(0);

	if (frame == stateLength[state] && state != END)
	{
		int s = state;
		s++;
		state = (State)s;
		frame = 0;

		if (state == END)
		{
			owner->cam.EaseOutOfManual(60);
			owner->cam.StopRumble();
			return false;
		}
	}

	switch (state)
	{
	case GET:
	{
		if (frame == 0)
		{
			owner->cam.SetManual(true);
			owner->cam.Ease(Vector2f(player->position), 1, 60, CubicBezier());
			owner->cam.SetRumble(10, 10, 90);
		}

		if (frame == 60)
		{
			owner->Fade(false, 14, Color::White, true);
		}

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

		if (owner->state == GameSession::RUN)
			if (!geoGroup->Update())
			{
				frame = stateLength[state] - 1;
			}
	}
	}
	++frame;

	//emitter->Update();

	return true;
}

void DeathSequence::Draw(RenderTarget *target, EffectLayer layer)
{
	owner->DrawEmitters(layer);
	if (layer == EffectLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{
		if (state != END)
		{
			geoGroup->Draw(target);
		}
	}
	else if (layer == EffectLayer::UI_FRONT)
	{
		if (state != END && owner->state == GameSession::FROZEN)
		{
			target->draw(overlayRect, 4, sf::Quads);
		}
	}
}

void DeathSequence::Reset()
{
	Vector2f pPos = Vector2f(owner->GetPlayer(0)->position);
	frame = 0;
	state = GET;
	geoGroup->SetBase(pPos);
	geoGroup->Reset();
	geoGroup->Start();

	emitter->SetPos(Vector2f(pPos));
	emitter->Reset();
	//emitter->SetOn(false);
	//owner->AddEmitter(emitter, EffectLayer::BETWEEN_PLAYER_AND_ENEMIES);

}