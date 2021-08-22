#ifndef __KEYMARKER_H__
#define __KEYMARKER_H__

#include <SFML/Graphics.hpp>

struct Session;
struct Zone;
struct Tileset;
struct ImageText;

struct KeyMarker
{
	enum MarkerType
	{
		KEY,
		ENEMY,
	};

	enum Action
	{
		IDLE,
		VIBRATING,
		Count
	};

	Action action;
	MarkerType markerType;
	int frame;
	void VibrateNumbers();
	void SetMarkerType(int k);
	Session *sess;
	float scale;
	KeyMarker();
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

	sf::Text xKeyText;
	sf::Text xEnemyText;

	sf::Sprite keyIconSpr;
	sf::Sprite enemyIconSpr;

	Tileset *ts_keyIcon;
	Tileset *ts_enemyIcon;
	sf::Sprite keyNumberSpecialSpr;
	Tileset *ts_keyNumLight;
	Tileset *ts_keyNumDark;

	Tileset *ts_enemyNumLight;
	Tileset *ts_enemyNumDark;
	ImageText *keyNumberNeededHUD;
	ImageText *keyNumberNeededHUDBack;

	ImageText *enemyNumberNeededHUD;
	ImageText *enemyNumberNeededHUDBack;
	//ImageText *keyNumberTotalHUD;

	Tileset *ts_keyRing;
	sf::Sprite keyRingSpr;
	//GameSession *owner;
};

#endif