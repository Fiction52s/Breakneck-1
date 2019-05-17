#ifndef __ENEMY_AIRTRIGGER_H__
#define __ENEMY_AIRTRIGGER_H__

#include "QuadTree.h"
#include <SFML/Graphics.hpp>
#include "VectorMath.h"

struct GameSession;
struct AirTrigger : QuadTreeEntrant
{
	enum TriggerType
	{
		AUTORUNRIGHT,
		AUTORUNRIGHTAIRDASH
	};

	AirTrigger(GameSession *owner, V2d &pos, int width, int height,
		const std::string &triggerType );
	void Reset();
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	void DebugDraw(sf::RenderTarget *target);
	bool triggered;
	sf::Rect<double> rect;
	TriggerType triggerType;
	GameSession *owner;
	sf::RectangleShape debugRect;
	static TriggerType GetTriggerType(const std::string &typeStr);
};


#endif