#include "LoadingBackpack.h"
#include "Tileset.h"


LoadingBackpack::LoadingBackpack( TilesetManager *tm)
{
	ts_loadIcon = tm->GetTileset("Menu/Load/loadicon_320x320.png", 320, 320);

	for (int i = 0; i < 3; ++i)
	{
		loadingIconBackpack[i].setTexture(*ts_loadIcon->texture);
		loadingIconBackpack[i].setTextureRect(ts_loadIcon->GetSubRect(i));
		loadingIconBackpack[i].setOrigin(loadingIconBackpack[i].getLocalBounds().width / 2, 
			loadingIconBackpack[i].getLocalBounds().height / 2);
	}

	SetPosition(sf::Vector2f(1920 - 260, 1080 - 200));
}

void LoadingBackpack::SetScale(float scale)
{
	for (int i = 0; i < 3; ++i)
	{
		loadingIconBackpack[i].setScale(scale, scale);
	}
}

void LoadingBackpack::SetPosition(sf::Vector2f &pos)
{
	for (int i = 0; i < 3; ++i)
	{
		loadingIconBackpack[i].setPosition(pos);
	}
}

void LoadingBackpack::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < 3; ++i)
	{
		target->draw(loadingIconBackpack[i]);
	}
}

void LoadingBackpack::Update()
{
	loadingIconBackpack[1].rotate(-1);
	loadingIconBackpack[2].rotate(2);
}

//loadingIconBackpack[i].setPosition(1920 - 260, 1080 - 200);