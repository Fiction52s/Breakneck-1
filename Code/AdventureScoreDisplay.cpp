#include "AdventureScoreDisplay.h"
#include "Medal.h"
#include "MedalSequence.h"
#include "KinExperienceBar.h"

AdventureScoreDisplay::AdventureScoreDisplay(TilesetManager *p_tm, KinExperienceBar *p_expBar, sf::Font &f)
	:ScoreDisplay( p_tm, f)
{
	expBar = p_expBar;

	Reset();

	ts_test = p_tm->GetSizedTileset( "Menu/AdventureScoreDisplay/adventurescoretest_1920x1080.png");
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
	action = A_EXPBAR;
	frame = 0;
}

void AdventureScoreDisplay::Confirm()
{
	if (action == A_EXPBAR)
	{
		action = A_SHOW;
		frame = 0;
	}
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

bool AdventureScoreDisplay::IsConfirmable()
{
	return action == A_EXPBAR;
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
		if (action == A_EXPBAR)
		{
			expBar->Draw(target);
		}
		else if (action == A_SHOW || action == A_WAIT )
		{
			target->draw(testSpr);
		}
		else if (action == A_GIVE_GOLD || action == A_GIVE_SILVER || action == A_GIVE_BRONZE)
		{
			medalSeq->Draw(target, EffectLayer::UI_FRONT);
		}
	}
}