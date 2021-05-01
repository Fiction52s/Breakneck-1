#ifndef __LOADINGBACKPACK_H__
#define __LOADINGBACKPACK_H__

#include <SFML\Graphics.hpp>

struct Tileset;
struct TilesetManager;

struct LoadingBackpack
{
	Tileset *ts_loadIcon;
	sf::Sprite loadingIconBackpack[3];

	LoadingBackpack(TilesetManager *tm);
	void SetScale(float scale);
	void SetPosition(sf::Vector2f &pos);
	void Update();
	void Draw(sf::RenderTarget *target);
};


#endif 