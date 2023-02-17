#ifndef __MINIMAP_H__
#define __MINIMAP_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct MapHeader;
struct Session;

struct Minimap
{
	sf::Shader minimapShader;
	sf::Sprite minimapSprite;
	sf::Sprite kinMinimapIcon;
	sf::Sprite goalMapIcon;
	sf::RenderTexture *minimapTex;
	sf::VertexArray miniVA;
	Session *sess;
	Tileset *ts_miniIcons;
	Tileset *ts_miniCircle;
	sf::Vertex blackBorderQuadsMini[4 * 2];
	sf::Vertex topBorderQuadMini[4];
	static sf::Color terrainColor;
	const static float MINIMAP_ZOOM;

	Minimap();
	void DrawToTex();
	void SetupBorderQuads(
		bool *blackBorder, bool topBorderOn,
		MapHeader *mh);
	void Draw(sf::RenderTarget *target);
	void Update();
	void SetCenter(sf::Vector2f &center);

private:
	void DrawZones( sf::RenderTarget *target);
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