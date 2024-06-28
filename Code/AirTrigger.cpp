#include "AirTrigger.h"
#include "GameSession.h"
#include "Actor.h"
#include "KinUpgrades.h"

using namespace sf;
using namespace std;

AirTrigger::AirTrigger(GameSession *p_owner, V2d &pos, int width, int height, const std::string &typeStr)
{
	owner = p_owner;
	rect.left = pos.x - width / 2;
	rect.top = pos.y - height / 2;
	rect.width = width;
	rect.height = height;
	triggerType = GetTriggerType(typeStr);
	debugRect.setFillColor(Color(200, 200, 200, 150));
	debugRect.setSize(Vector2f(width, height));
	debugRect.setPosition(rect.left, rect.top);
	Reset();
}

void AirTrigger::HandleQuery(QuadTreeCollider * qtc)
{
	if (!triggered)
	{
		triggered = true;
		qtc->HandleEntrant(this);
	}
}

bool AirTrigger::IsTouchingBox(const sf::Rect<double> &r)
{
	
	return IsBoxTouchingBox(rect, r);
}

AirTrigger::TriggerType AirTrigger::GetTriggerType(const std::string &typeStr)
{
	if (typeStr == "autorunright")
	{
		return AUTORUNRIGHT;
	}
	else if (typeStr == "autorunrightairdash")
	{
		return AUTORUNRIGHTAIRDASH;
	}
}

void AirTrigger::Reset()
{
	//debugRect.setPosition(Vector2f(owner->GetPlayer(0)->position));
	triggered = false;

	switch (triggerType)
	{
	case AUTORUNRIGHTAIRDASH:
		if (owner->GetPlayer(0)->IsOptionOn(POWER_AIRDASH))
		{
			triggered = true;
		}
		break;
	}
}

void AirTrigger::DebugDraw(sf::RenderTarget *target)
{
	target->draw(debugRect);
}