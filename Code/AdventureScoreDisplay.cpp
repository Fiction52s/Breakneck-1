#include "AdventureScoreDisplay.h"
#include "Medal.h"
#include "MedalSequence.h"

AdventureScoreDisplay::AdventureScoreDisplay(TilesetManager *p_tm, sf::Font &f)
	:ScoreDisplay( p_tm, f)
{
	Reset();

	ts_test = p_tm->GetSizedTileset( "Menu/adventurescoretest_1920x1080.png");
	ts_test->SetSpriteTexture(testSpr);

	testSpr.setPosition(0, 0);
}

void AdventureScoreDisplay::Reset()
{
	ScoreDisplay::Reset();
	action = A_IDLE;
	frame = 0;
}

void AdventureScoreDisplay::Activate()
{
	action = A_SHOW;
	frame = 0;
}

void AdventureScoreDisplay::Deactivate()
{
	action = A_IDLE;
	frame = 0;
}

void AdventureScoreDisplay::Update()
{
	if (!IsActive())
		return;

	if (action == A_SHOW & frame == 100)
	{
		action = A_WAIT;
		frame = 0;
	}

	++frame;
}

bool AdventureScoreDisplay::IsActive()
{
	return action != A_IDLE;
}

bool AdventureScoreDisplay::IsWaiting()
{
	return action == A_WAIT;
}

bool AdventureScoreDisplay::IsIncludingExtraOptions()
{
	return false;
}

void AdventureScoreDisplay::Draw(sf::RenderTarget *target)
{
	if (IsActive())
	{
		target->draw(testSpr);

		if (action == A_GIVE_GOLD || action == A_GIVE_SILVER || action == A_GIVE_BRONZE)
		{
			medalSeq->Draw(target, EffectLayer::UI_FRONT);
		}
	}
}