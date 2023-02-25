#ifndef __CREDITS_MENU_SCREEN_H__
#define __CREDITS_MENU_SCREEN_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "GUI.h"

struct CreditsMenuScreen : TilesetManager, GUIHandler
{
	enum Action
	{
		A_SHOW,
		A_BACK,
	};

	int action;
	int frame;

	//sf::Sprite testSprite;
	Panel *panel;
	sf::Vertex creditsQuad[4];
	Tileset *ts;
	sf::Vector2f menuOffset;

	CreditsMenuScreen();
	~CreditsMenuScreen();
	void Draw(sf::RenderTarget *target);
	bool HandleEvent(sf::Event ev);
	void Update();
	void Cancel();
	void ConfirmCallback(Panel *p);
	void CancelCallback(Panel *p);
	void Start();
	
};

#endif