#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h""

struct GameSession;
struct MainMenu;
struct ScrollingBackground;


struct Background
{
	Background( GameSession *owner, int envLevel, int envType);
	Background(TilesetManager &tm, const std::string &bgName);
	Background(MainMenu *mm);
	static std::string GetBGNameFromBGInfo(const std::string &fileName);
	static bool SetupFullBG(const std::string &fName,
		TilesetManager &tm,
		Background *& bg,
		std::list<ScrollingBackground*> &sBG);
	void Update();
	void Reset();
	void Draw(sf::RenderTarget *target);
	void UpdateSky();
	void UpdateShape();
	sf::Sprite background;
	sf::Sprite shape;
	sf::Color GetSkyColor();
	sf::Color GetShapeColor();
	sf::Vertex backgroundSky[4];
	sf::Image palette;
	sf::View bgView;
	int transFrames;
	int frame;
};

#endif