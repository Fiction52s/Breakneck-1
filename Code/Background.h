#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include <map>
#include <vector>


struct GameSession;
struct MainMenu;
struct BackgroundObject;
struct BackgroundObject;
struct BackgroundLayer;


struct ScrollingBackground
{
	ScrollingBackground(Tileset *ts, int index,
		int depthLevel, float p_scrollSpeedX = 0.f);
	~ScrollingBackground();
	void Set(sf::Vector2f &pos, float zoom = 1.f);
	void SetExtra(sf::Vector2f &extra);
	Tileset *ts;
	int tsIndex;
	sf::Vector2f extra;
	sf::Vector2f relPos;
	sf::Vertex quad[3 * 4];
	//sf::VertexArray va;
	float xPos;
	sf::View oldView;
	sf::View newView;
	int depthLevel;
	float depth;
	float scrollSpeedX;
	float scrollOffset;
	void Update(const sf::Vector2f &camPos, 
		int updateFrames = 1 );
	void SetTileIndex(int index);
	void LayeredDraw(int p_depthLevel, sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	void SetLeftPos(sf::Vector2f &pos);
	sf::Vector2f extraOffset;
	float extraZoom;
	//sf::Shader parallaxShader;

	//sf::Color testMeColor;

	std::string tsSource;
};


struct Background : TilesetManager
{
	Background(const std::string &bgName );
	Background();
	void Set(sf::Vector2f &pos, float zoom = 1.f);
	~Background();
	static std::string GetBGNameFromBGInfo(const std::string &fileName);
	static Background *SetupFullBG(const std::string &fName);

	void SetExtra(sf::Vector2f &extra);
	void Update( const sf::Vector2f &camPos,
		int frames = 1 );
	void Reset();
	void Draw(sf::RenderTarget *target);
	void DrawBackLayer(sf::RenderTarget *target);
	void LayeredDraw(int p_drawLayer, sf::RenderTarget *target);
	sf::Shader * GetShader(const std::string &shaderStr);
	//void UpdateSky();
	//void UpdateShape();
	
	sf::View bgView;

	int transFrames;
	int frame;

	int bgWidth;

	int envWorld;

	bool show;
	void Show();
	void Hide();
	void FlipShown();

	std::string name;
	std::list<ScrollingBackground*> scrollingBackgrounds;
	std::list<BackgroundObject*> scrollingObjects;
	std::list<BackgroundLayer*> bgLayerList;
	std::map<std::string, sf::Shader*> shaderMap;
	std::map<std::string, std::vector<Tileset*>> shaderTilesetMap;
};


#endif