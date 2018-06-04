#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include <SFML/Graphics.hpp>

struct GameSession;
struct MainMenu;



struct Background
{
	Background( GameSession *owner, int envLevel, int envType);
	Background(MainMenu *mm);
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