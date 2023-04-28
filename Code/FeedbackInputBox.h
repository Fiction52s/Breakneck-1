#ifndef __FEEDBACK_INPUT_BOX_H__
#define __FEEDBACK_INPUT_BOX_H__

#include <SFML/Graphics.hpp>

struct Tileset;

struct FeedbackInputBox
{
	sf::Vertex bgQuad[4];
	sf::Vertex buttonIcon[4];
	sf::Text feedbackText;

	sf::Vector2f size;

	Tileset *ts_button;

	FeedbackInputBox();

	void SetTopLeft(sf::Vector2f pos);
	void SetCenter(sf::Vector2f pos);
	void Draw(sf::RenderTarget *target);
};

#endif