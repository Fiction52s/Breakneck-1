#ifndef __RUSH_UPGRADE_POPUP_H__
#define __RUSH_UPGRADE_POPUP_H__

#include "Tileset.h"
#include <SFML\Graphics.hpp>

struct TutorialBox;
struct Session;
struct RushManager;

struct RushUpgradePopup
{
	enum State
	{
		SHOW,
		Count
	};

	TutorialBox *tutBox;
	sf::Sprite upgradeSpr;

	float borderHeight;
	float powerBorder;
	float width;
	float height;
	float powerSize;
	float nameHeight;

	sf::Vertex bgQuad[4];
	sf::Vertex topBorderQuad[4];
	sf::Vertex powerBorderQuad[4];

	sf::Text nameText;

	sf::Vector2f topLeft;
	sf::Vector2f descBorder;
	sf::Vector2f powerRel;

	Tileset *ts_upgrade;

	int upgradeIndex;
	int frame;
	State state;

	RushManager *rushManager;

	RushUpgradePopup( RushManager *rm );
	~RushUpgradePopup();
	void SetTopLeft(sf::Vector2f &pos);
	void SetCenter(sf::Vector2f &pos);
	void Reset();
	void Update();
	void SetUpgrade(int index);
	void SetToMostRecentUpgrade();
	void Draw(sf::RenderTarget *target);
	void SetName(const std::string &name);
};

#endif
