#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h""

struct GameSession;
struct MainMenu;


struct ScrollingBackground
{
	ScrollingBackground(Tileset *ts, int index,
		int depthLevel, float p_scrollSpeedX = 0.f);
	Tileset *ts;
	int tsIndex;
	sf::Vector2f relPos;
	sf::VertexArray va;
	float xPos;
	int depthLevel;
	sf::View oldView;
	sf::View newView;
	float depth;
	float scrollSpeedX;
	float scrollOffset;
	void Update(sf::Vector2f &camPos);
	void SetTileIndex(int index);
	void Draw(sf::RenderTarget *target);
	void SetLeftPos(sf::Vector2f &pos);
};


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