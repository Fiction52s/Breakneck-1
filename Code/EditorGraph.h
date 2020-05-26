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
	int GetSpacing();
	void SetPosition( sf::Vector2f &pos );
	void SetCenterAbsolute(const sf::Vector2f &center,
		float zoomFactor );
	void Draw(sf::RenderTarget *target);
	void ModifyGraphSpacing(double factor);
	void SetSpacing(int spacing);
	//sf::Vertex *graphLines;
	sf::Vertex *graphQuads;
	sf::Color graphColor;
	int graphSpacing;
	float lineWidth;
	float defaultLineWidth;
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