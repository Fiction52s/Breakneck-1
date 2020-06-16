#ifndef __SHOCKWAVE_H__
#define __SHOCKWAVE_H__

#include <SFML/Graphics.hpp>

struct Tileset;

struct Shockwave
{
	Shockwave();
	void Update( float p_zoom );
	sf::Sprite sprite;
	sf::Shader shockShader;
	int frame;
	Tileset *ts_shock;
	sf::Vector2f pos;
	float zoom;
	void SetPosition(sf::Vector2f &pos);
	void Draw( sf::RenderTexture *tex, sf::RenderTarget *target);
};

#endif

