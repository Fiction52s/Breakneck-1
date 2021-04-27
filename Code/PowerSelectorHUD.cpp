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
	//sprite.setPosition(288, 140);
	state = STATIC;

	SetRectSubRect(deactivatedQuads, ts->GetSubRect(11));
	SetRectSubRect(deactivatedQuads + 4, ts->GetSubRect(9));
	SetRectSubRect(deactivatedQuads + 8, ts->GetSubRect(10));
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

			SetPosition(Vector2f(sprite.getPosition()));

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

void PowerSelector::SetPosition(Vector2f &pos)
{
	sprite.setPosition(pos);

	Actor *player = sess->GetPlayer(0);
	if (player->HasUpgrade(Actor::UPGRADE_POWER_BOUNCE))
	{
		ClearRect(deactivatedQuads);
	}
	else
	{
		SetRectTopLeft(deactivatedQuads, ts->tileWidth, ts->tileHeight,
			Vector2f(sprite.getPosition()));
	}

	if (player->HasUpgrade(Actor::UPGRADE_POWER_GRIND))
	{
		ClearRect(deactivatedQuads + 4);
	}
	else
	{
		SetRectTopLeft(deactivatedQuads + 4, ts->tileWidth, ts->tileHeight,
			Vector2f(sprite.getPosition()));
	}

	if (player->HasUpgrade(Actor::UPGRADE_POWER_TIME))
	{
		ClearRect(deactivatedQuads + 8);
	}
	else
	{
		SetRectTopLeft(deactivatedQuads + 8, ts->tileWidth, ts->tileHeight,
			Vector2f(sprite.getPosition()));
	}
}

void PowerSelector::Draw(sf::RenderTarget *target)
{
	target->draw(sprite);

	target->draw(deactivatedQuads, 3 * 4, sf::Quads, ts->texture);
}