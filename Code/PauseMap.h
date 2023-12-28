#ifndef __PAUSEMAP_H__
#define __PAUSEMAP_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Input.h"

struct MapHeader;
struct GameSession;


struct PauseMap
{
	TilesetManager *tm;

	sf::RectangleShape cameraRect;
	sf::Shader mapShader;
	sf::Sprite mapSprite;
	sf::Sprite kinMapIcon;
	sf::Sprite goalMapIcon;
	sf::RenderTexture *mapTex;
	sf::VertexArray miniVA;
	GameSession *game;
	Tileset *ts_miniIcons;
	Tileset *ts_miniCircle;
	sf::Vertex blackBorderQuadsMini[4 * 2];
	sf::Vertex topBorderQuadMini[4];
	static sf::Color terrainColor;

	bool blackBorderOn[2];
	bool topBorderOn;


	sf::Vector2f mapCenter;
	float mapZoomFactor;


	PauseMap(TilesetManager *tm);
	~PauseMap();
	void DrawToTex();
	void SetGame(GameSession *game);
	void SetupBorderQuads(
		bool *blackBorder, bool topBorderOn,
		MapHeader *mh);
	void Draw(sf::RenderTarget *target);
	void Update(ControllerState &currInput,
		ControllerState &prevInput);
	void SetCenter(sf::Vector2f &center);
	void Reset();

private:
	void DrawZones(sf::RenderTarget *target);
	void DrawTerrain(
		sf::Rect<double> &rect,
		sf::RenderTarget *target);
	void DrawRails(sf::Rect<double> &rect,
		sf::RenderTarget *target);
	void DrawSpecialTerrain(sf::Rect<double> &rect,
		sf::RenderTarget *target);
	void DrawMapBorders(
		sf::RenderTarget *target);
	void DrawGates(sf::Rect<double> &rect,
		sf::RenderTarget *target);
};

#endif