#ifndef __MINIMAP_H__
#define __MINIMAP_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct GameSession;

struct Minimap
{
	Minimap( GameSession *owner);
	void DrawToTex();
	void Draw(sf::RenderTarget *target);
	void Update();
	sf::Shader minimapShader;
	sf::Sprite minimapSprite;
	sf::Sprite kinMinimapIcon;
	sf::Sprite goalMapIcon;

	sf::RenderTexture *minimapTex;

	sf::VertexArray miniVA;

	GameSession *owner;

	void SetCenter(sf::Vector2f &center);
	

	Tileset *ts_miniIcons;
	Tileset *ts_miniCircle;

	sf::Vertex blackBorderQuadsMini[4 * 2];
	sf::Vertex topBorderQuadMini[4];

private:
	void DrawTerrain(
		sf::Rect<double> &rect,
		sf::RenderTarget *target);
	void DrawMapBorders(
		sf::RenderTarget *target);
	void DrawGates(sf::Rect<double> &rect,
		sf::RenderTarget *target);
};

#endif