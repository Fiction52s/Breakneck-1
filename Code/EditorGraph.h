#ifndef __EDITORGRAPH_H__
#define __EDITORGRAPH_H__

#include <SFML/Graphics.hpp>

struct EditorGraph
{
	static int NUM_HALF_GRAPH_LINES;
	static int NUM_GRAPH_LINES;
	static int TOTAL_VERTICES;

	EditorGraph();
	~EditorGraph();
	void SetPosition( sf::Vector2f &pos );
	void SetCenterAbsolute(const sf::Vector2f &center);
	void Draw(sf::RenderTarget *target);
	void ModifyGraphSpacing(double factor);
	sf::Vertex *graphLines;
	sf::Color graphColor;
	int graphSpacing;
};

struct EditSession;
struct PolyVis
{
	PolyVis();
	sf::CircleShape *circles;
	EditSession *sess;
	void Draw(sf::RenderTarget *target);
};

#endif