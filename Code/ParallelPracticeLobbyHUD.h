#ifndef __PARALLEL_PRACTICE_LOBBY_HUD_H__
#define __PARALLEL_PRACTICE_LOBBY_HUD_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ParallelPracticeLobbyHUD
{
	sf::Text memberNameText[4];

	ParallelPracticeLobbyHUD();
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif