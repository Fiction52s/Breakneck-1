#include "Sequence.h"
#include "GameSession.h"

using namespace sf;
using namespace std;

GetShardSequence::GetShardSequence(GameSession *p_owner)
	:owner(p_owner)
{
	stateLength[GET] = 1000000;

	for (int i = 0; i < 5; ++i)
	{
		geoGroup.AddGeo(new SpinningTri(0 + i * PI / 2.5));
	}
	geoGroup.AddGeo(new Laser(0));

	geoGroup.AddGeo(new MovingRing(32, 20, 10, 200, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 10), 30));
	geoGroup.Init();
}

bool GetShardSequence::Update()
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
			//owner->state = GameSession::RUN;;
			//owner->goalDestroyed = true;
			//owner->state = GameSession::RUN;
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
		}


		if (!geoGroup.Update())
		{
			frame = stateLength[state] - 1;
		}
	}
	}
	++frame;

	return true;
}

void GetShardSequence::Draw(RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{
		return;
	}

	if (state != END)
	{
		geoGroup.Draw(target);
	}
}

void GetShardSequence::Reset()
{
	frame = 0;
	state = GET;
	geoGroup.SetBase(Vector2f(owner->GetPlayer(0)->position));
	geoGroup.Reset();
}