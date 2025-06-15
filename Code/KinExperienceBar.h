#ifndef __KIN_EXPERIENCE_BAR_H__
#define __KIN_EXPERIENCE_BAR_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ExpBarParams
{
	int maxLevel;
	int levelUpAmtBase;
	int maxLevelUpIncreaseLevel;
	int levelUpIncrease;
	int gold;
	int silver;
	int bronze;

	ExpBarParams();
	void Load();
};

struct KinExperienceBar
{
	enum Action
	{
		A_IDLE,
		A_ADDING,
		A_LEVEL_UP,
	};

	ExpBarParams params;
	Tileset *ts_bar;
	sf::Vertex barQuad[4];
	sf::Text expText;

	int gainedLevels;

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
	void AddMedal(int medal);
	void Draw(sf::RenderTarget *target);
};

//not sure what this is for
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

#endif