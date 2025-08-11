#ifndef __HEALTH_HEARTS_H__
#define __HEALTH_HEARTS_H__

#include <SFML\Graphics.hpp>

struct Tileset;
struct Session;
struct Actor;
struct TilesetManager;

struct HealthHearts
{
	const static int MAX_HEARTS = 8;
	const static int FIRST_ROW_ITEMS = MAX_HEARTS / 2;
	const static int SECOND_ROW_ITEMS = MAX_HEARTS / 2;
	const static int BASE_MAX_HEALTH = MAX_HEARTS * 2;

	//hearticon = 63, 58

	Session *sess;
	Actor *actor;
	sf::Vector2f topLeft;
	Tileset *ts_heartIcon;
	int health;

	sf::Vertex heartQuads[MAX_HEARTS * 4];

	HealthHearts( TilesetManager *tm );
	void SetTopLeft(sf::Vector2f pos );
	void SetSession(Session *s);
	void Reset();
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif