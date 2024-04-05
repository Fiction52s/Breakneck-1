#ifndef __MEDAL_H__
#define __MEDAL_H__

#include <SFML\Graphics.hpp>

struct Tileset;

struct Medal
{
	enum PosType
	{
		PT_CENTER,
		PT_TOP_LEFT,
	};

	enum MedalType
	{
		MEDAL_BRONZE,
		MEDAL_SILVER,
		MEDAL_GOLD,
	};

	enum Action
	{
		A_HIDDEN,
		A_APPEAR,
		A_SHINE,
		A_HOLD,
		A_DISAPPEAR,
		A_WAIT,
		A_Count
	};


	int medalType;


	Medal();
	void Reset();
	void SetCenter(sf::Vector2f pos);
	void SetTopLeft(sf::Vector2f pos);
	void Update();
	void UpdateSprite();
	void SetType(int t);
	void Draw(sf::RenderTarget *target);
	void SetScale(float s);
	bool IsWaiting();
	void Show();
private:
	int actionLength[A_Count];
	int animFactor[A_Count];
	int action;
	int frame;
	Tileset *ts_medals;
	sf::Vertex verts[4 * 2];
	sf::Vector2f position; //can be center or top left depending on how its set
	float scale;
	int posType;
};

#endif