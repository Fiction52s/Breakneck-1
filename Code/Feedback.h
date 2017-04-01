#ifndef __FEEDBACK_H__
#define __FEEDBACK_H__

#include <sfml/Graphics.hpp>


struct Tileset;
struct GameSession;

struct Feedback
{
	Feedback( GameSession *owner );
	void Load( Tileset *t, int animFactor );
	void Draw( sf::RenderTarget *target );
	void Update();
	sf::RenderTexture *rt;
	GameSession *owner;
	Tileset *t;
	int animFactor;
	int totalFrames;
	int currFrame;
	sf::Sprite feedbackSprite;
};

#endif