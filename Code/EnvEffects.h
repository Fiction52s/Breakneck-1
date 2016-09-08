#ifndef __ENV_EFFECTS_H__
#define __ENV_EFFECTS_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct GameSession;
struct Rain
{
	Rain( GameSession *owner );
	void Draw( sf::RenderTarget *target );
	void Reset();
	sf::VertexArray va;

	GameSession *owner;

	Tileset *ts_rain;

	void Update();
	int frame;
	int loopLength;
	int animFactor;
	//sf::Vector2f basePos;

	const static int TILE_WIDTH;
	const static int TILE_HEIGHT;
	const static int NUM_ROWS;
	const static int NUM_COLS;
	const static int TOTAL_QUADS;
	const static int ANIM_FACTOR;
};

#endif