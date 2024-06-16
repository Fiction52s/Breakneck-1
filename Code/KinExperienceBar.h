#ifndef __KIN_EXPERIENCE_BAR_H__
#define __KIN_EXPERIENCE_BAR_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ExperienceAdder
{
	int numExp;
	std::string reasonStr;
	sf::Text fullText;
	sf::Vector2f myPos;

	ExperienceAdder();
	void SetTopLeft(sf::Vector2f pos);
	void Draw(sf::RenderTarget *target);
};

struct KinExperienceBar
{
	enum Action
	{
		A_IDLE,
		A_ADDING,
		A_LEVEL_UP,
	};


	Tileset *ts_bar;
	sf::Vertex barQuad[4];
	sf::Text expText;

	int currLevel;
	int expToLevelUp;
	int currExp;
	int expToAdd;

	int action;
	int frame;

	KinExperienceBar( TilesetManager *tm );
	int GetExpToLevelUp();
	void Setup( int currLevel, int currExp );
	void Reset();
	void Update();
	bool IsLeveledUp();
	void AddExp(int exp);
	void Draw(sf::RenderTarget *target);
};

#endif