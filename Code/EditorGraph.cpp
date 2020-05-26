#include "EditorGraph.h"
#include "EditSession.h"

using namespace std;
using namespace sf;


int EditorGraph::NUM_HALF_GRAPH_LINES = 30;
int EditorGraph::NUM_GRAPH_LINES = NUM_HALF_GRAPH_LINES * 2 + 1;
int EditorGraph::TOTAL_VERTICES = NUM_GRAPH_LINES * 4 * 2; //2 vert, 2 horiz

EditorGraph::EditorGraph()
{
	graphColor = Color(200, 50, 50, 100);
	graphSpacing = 32;
	//graphLines = new Vertex[TOTAL_VERTICES];
	graphQuads = new Vertex[TOTAL_VERTICES];
	defaultLineWidth = 2;
	lineWidth = 2;
	//SetPosition();
}

EditorGraph::~EditorGraph()
{
	delete[] graphQuads;
	//delete[] graphLines;
}

void EditorGraph::ModifyGraphSpacing(double factor)
{
	int newSpacing = graphSpacing * factor;
	if (newSpacing < 4)
		return;
	graphSpacing *= factor;
}

void EditorGraph::Draw(RenderTarget *target)
{
	//target->draw(graphLines, TOTAL_VERTICES, sf::Lines);
	target->draw(graphQuads, TOTAL_VERTICES, sf::Quads);
}

void EditorGraph::SetCenterAbsolute(const Vector2f &center, float zoomFactor )
{
	int gX, gY;
	float x = center.x;
	float y = center.y;

	x /= graphSpacing;
	y /= graphSpacing;

	if (x > 0)
		x += .5f;
	else if (x < 0)
		x -= .5f;

	if (y > 0)
		y += .5f;
	else if (y < 0)
		y -= .5f;

	gX = ((int)x) * graphSpacing;
	gY = ((int)y) * graphSpacing;

	Vector2f gPos = Vector2f(gX, gY);

	lineWidth = zoomFactor * defaultLineWidth;

	SetPosition(gPos);
}

void EditorGraph::SetPosition( Vector2f &pos )
{
	int graphMax = graphSpacing * NUM_HALF_GRAPH_LINES;
	int temp = -graphMax;

	//horiz
	int quadIndex;
	for (int i = 0; i < NUM_GRAPH_LINES; i++)
	{
		quadIndex = i * 4;
		SetRectTopLeft(graphQuads + quadIndex,
			graphMax * 2, lineWidth, Vector2f(-graphMax, temp - lineWidth / 2) + pos);
		SetRectColor(graphQuads + quadIndex, graphColor);
		//graphLines[i] = sf::Vertex(Vector2f(-graphMax, temp) + pos, graphColor);
		//graphLines[i + 1] = sf::Vertex(Vector2f(graphMax, temp) + pos, graphColor);
		temp += graphSpacing;
	}

	////vert
	temp = -graphMax;
	
	for (int i = 0; i < NUM_GRAPH_LINES; i++)
	{
		quadIndex = (i + NUM_GRAPH_LINES) * 4;
		SetRectTopLeft(graphQuads + quadIndex,
			lineWidth, graphMax * 2, Vector2f( temp - lineWidth / 2, -graphMax) + pos);
		SetRectColor(graphQuads + quadIndex, graphColor);
		//graphLines[i] = sf::Vertex(Vector2f(temp, -graphMax) + pos, graphColor);
		//graphLines[i + 1] = sf::Vertex(Vector2f(temp, graphMax) + pos, graphColor);
		temp += graphSpacing;
	}
}

int EditorGraph::GetSpacing()
{
	return graphSpacing;
}

void EditorGraph::SetSpacing(int spacing)
{
	if (spacing < 4)
	{
		return;
	}

	graphSpacing = spacing;
}

PolyVis::PolyVis()
{
	sess = EditSession::GetSession();
}

void PolyVis::Draw(sf::RenderTarget *target)
{

}