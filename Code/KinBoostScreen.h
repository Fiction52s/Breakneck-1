#ifndef __KINBOOSTSCREEN_H__
#define __KINBOOSTSCREEN_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"

struct MainMenu;
struct KinBoostScreen
{
	KinBoostScreen( MainMenu *mm);
	void Draw(sf::RenderTarget *target);
	void Reset();
	sf::Sprite bgSpr;
	sf::Sprite bgShapeSpr;
	sf::Sprite starSpr[4];
	sf::Sprite lightSpr[2];
	MainMenu *mainMenu;
	Tileset *ts_stars[4];
	Tileset *ts_light[2];
	Tileset *ts_bg;
	Tileset *ts_bgShape;
	Tileset *ts_kinBoost;
	sf::Sprite kinSpr;
	int frame;
	void Update();
	sf::Shader scrollShaderStars[4];
	sf::Shader scrollShaderLight[2];

	std::string levName;
};

#endif