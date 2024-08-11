#include "DeathSequence.h"
#include "Session.h"
#include "ParticleEffects.h"
#include "Actor.h"

using namespace sf;
using namespace std;

DeathSequence::DeathSequence()
{
	sess = Session::GetSession();

	emitter = new ShapeEmitter(6, DrawLayer::IN_FRONT);// , PI / 2.0, 2 * PI, 1.0, 2.5);
	emitter->CreateParticles();
	emitter->SetRatePerSecond(120);

	

	geoGroup = new MovingGeoGroup;

	for (int i = 0; i < 5; ++i)
	{
		geoGroup->AddGeo(new SpinningTri(0 + i * PI / 2.5), 30);
	}
	

	//geoGroup->AddGeo( )
	geoGroup->AddGeo(new Laser( 100, 200, 150, 0, Color( 255, 255, 255, 150 )));
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


void DeathSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[DIE] = 1000000;
}

void DeathSequence::ReturnToGame()
{
	sess->cam.EaseOutOfManual(60);
	sess->cam.StopRumble();
}

void DeathSequence::UpdateState()
{
	Actor *player = sess->GetPlayer(0);

	switch (seqData.state)
	{
	case DIE:
	{
		if (seqData.frame == 0)
		{
			sess->cam.SetManual(true);
			sess->cam.Ease(Vector2f(player->position), 1, 60, CubicBezier());
			//sess->cam.SetRumble(10, 10, 90, 4);
		}

		if (seqData.frame == 60)
		{
			sess->Fade(false, 14, Color::White, true);
		}


		if (sess->GetGameSessionState() == Session::RUN)
			if (!geoGroup->Update())
			{
				seqData.frame = stateLength[seqData.state] - 1;
			}
	}
	}
	//++frame;

	//emitter->Update();
}

void DeathSequence::LayeredDraw(int layer, RenderTarget *target)
{
	//sess->DrawEmitters(layer, target); //recently turned this off because I think the session draws them itself?
	if (layer == DrawLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{
		geoGroup->Draw(target);
		/*if (state != END)
		{
			
		}*/
	}
	else if (layer == DrawLayer::UI_FRONT)
	{
		if (/*state != END && */sess->GetGameSessionState() == Session::FROZEN)
		{
			target->draw(overlayRect, 4, sf::Quads);
		}
	}
}

void DeathSequence::Reset()
{
	Sequence::Reset();
	Vector2f pPos = Vector2f(sess->GetPlayer(0)->position);
	seqData.frame = 0;
	seqData.state = DIE;
	geoGroup->SetBase(pPos);
	geoGroup->Reset();
	geoGroup->Start();

	emitter->SetPos(Vector2f(pPos));
	emitter->Reset();
	//emitter->SetOn(false);
	//owner->AddEmitter(emitter, DrawLayer::BETWEEN_PLAYER_AND_ENEMIES);

}

int DeathSequence::GetNumStoredBytes()
{
	return sizeof(seqData) + geoGroup->GetNumStoredBytes();
}

void DeathSequence::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &seqData, sizeof(seqData));
	bytes += sizeof(seqData);

	geoGroup->StoreBytes(bytes);
	//bytes += geoGroup->GetNumStoredBytes();
}

void DeathSequence::SetFromBytes(unsigned char *bytes)
{
	memcpy(&seqData, bytes, sizeof(seqData));
	bytes += sizeof(seqData);
	
	geoGroup->SetFromBytes(bytes);
	//bytes += geoGroup->GetNumStoredBytes();
}