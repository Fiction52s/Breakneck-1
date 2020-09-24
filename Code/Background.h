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
	~ScrollingBackground();
	void Set(sf::Vector2f &pos, float zoom = 1.f);
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
	void Update(const sf::Vector2f &camPos, 
		int updateFrames = 1 );
	void SetTileIndex(int index);
	void Draw(sf::RenderTarget *target);
	void SetLeftPos(sf::Vector2f &pos);
	sf::Vector2f extraOffset;
	float extraZoom;
};


struct Background
{
	Background( GameSession *owner, int envLevel, int envType);
	Background(TilesetManager *tm, const std::string &bgName);
	Background(MainMenu *mm);
	void Set(sf::Vector2f &pos, float zoom = 1.f);
	~Background();
	static std::string GetBGNameFromBGInfo(const std::string &fileName);
	static Background *SetupFullBG(const std::string &fName,
		TilesetManager *tm, bool deleteTilesets );

	void DestroyTilesets();
	void Update( const sf::Vector2f &camPos,
		int frames = 1 );
	void Reset();
	void Draw(sf::RenderTarget *target);
	void UpdateSky();
	void UpdateShape();
	sf::Sprite background;
	sf::Sprite shape;
	sf::Color GetSkyColor();
	sf::Color GetShapeColor();
	sf::Vertex backgroundSky[4];
	
	sf::View bgView;
	int transFrames;
	int frame;

	bool show;
	void Show();
	void Hide();
	void FlipShown();

	sf::Image palette;
	Tileset *ts_bg;
	Tileset *ts_shape;
	TilesetManager *tm;
	bool deleteTilesets;

	std::string name;


	std::list<ScrollingBackground*> scrollingBackgrounds;
};


#endif