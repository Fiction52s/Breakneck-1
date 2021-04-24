#include "PowerSelectorHUD.h"
#include "Session.h"
#include "Actor.h"

using namespace std;
using namespace sf;

PowerSelector::PowerSelector()
{
	sess = Session::GetSession();
	ts = sess->GetSizedTileset("HUD/power_selector_96x96.png");
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect( 0 ));
	sprite.setPosition(288, 140);
}

void PowerSelector::Update( int playerPowerMode )
{
	switch (state)
	{
	case STATIC:
		{
			int tile = 0;
			switch (playerPowerMode)
			{
			case Actor::PMODE_SHIELD:
				tile = 0;
				break;
			case Actor::PMODE_BOUNCE:
				tile = 6;
				break;
			case Actor::PMODE_GRIND:
				tile = 2;
				break;
			case Actor::PMODE_TIMESLOW:
				tile = 4;
				break;
			}
			sprite.setTextureRect(ts->GetSubRect(tile));

			break;
		}
	case SWITCHING:
	{
		break;
	}
		
	}

	switch (state)
	{
	case STATIC:
		break;
	case SWITCHING:
		break;
	}
}

void PowerSelector::Draw(sf::RenderTarget *target)
{
	target->draw(sprite);
}