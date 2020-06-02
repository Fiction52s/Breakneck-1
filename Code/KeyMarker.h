#ifndef __KEYMARKER_H__
#define __KEYMARKER_H__

#include <SFML/Graphics.hpp>

struct GameSession;
struct Zone;
struct Tileset;
struct ImageText;

struct KeyMarker
{
	enum Action
	{
		IDLE,
		VIBRATING,
		Count
	};

	Action action;
	int frame;
	void VibrateNumbers();
	GameSession *owner;
	KeyMarker( GameSession *owner );
	~KeyMarker();
	void Reset();
	void SetStartKeys( int neededKeys,
		int totalKeys );
	void SetStartKeysZone(Zone *z);
	void Update();
	void UpdateKeyNumbers();
	void SetPosition(sf::Vector2f &pos);
	sf::Vector2f GetPosition();

	sf::Vector2f neededCenter;
	//void SetEnergySprite();
	void Draw( sf::RenderTarget *target );
	//Tileset *ts_keys;
	//Tileset *ts_keyEnergy;
	//sf::Sprite backSprite;
	//sf::Sprite energySprite;


	sf::Sprite keyNumberSpecialSpr;
	Tileset *ts_keyNumLight;
	Tileset *ts_keyNumDark;
	ImageText *keyNumberNeededHUD;
	ImageText *keyNumberNeededHUDBack;
	//ImageText *keyNumberTotalHUD;

	Tileset *ts_keyRing;
	sf::Sprite keyRingSpr;
	//GameSession *owner;
};

#endif