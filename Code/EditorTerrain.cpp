#include "EditorTerrain.h"

using namespace std;
using namespace sf;

TerrainBrush::TerrainBrush(PolyPtr poly)
	:pointStart(NULL), pointEnd(NULL), lines(sf::Lines, poly->numPoints * 2), numPoints(0)
{
	//assert( poly->finalized );

	TerrainPoint *curr = poly->pointStart;
	left = curr->pos.x;
	right = curr->pos.x;
	top = curr->pos.y;
	bot = curr->pos.y;

	TerrainPoint *p = new TerrainPoint(*curr);
	p->gate = NULL;
	AddPoint(p);

	curr = curr->next;
	for (; curr != NULL; curr = curr->next)
	{
		if (curr->pos.x < left)
			left = curr->pos.x;
		else if (curr->pos.x > right)
			right = curr->pos.x;

		if (curr->pos.y < top)
			top = curr->pos.y;
		else if (curr->pos.y > bot)
			bot = curr->pos.y;

		TerrainPoint *tp = new TerrainPoint(*curr);
		tp->gate = NULL;
		AddPoint(tp);
	}
	UpdateLines();
	//centerPos = Vector2f( left + width / 2.f, top + height / 2.f );
}

TerrainBrush::TerrainBrush(TerrainBrush &brush)
	:pointStart(NULL), pointEnd(NULL), numPoints(0),
	lines(sf::Lines, brush.numPoints * 2)
{
	left = brush.left;
	right = brush.right;
	top = brush.top;
	bot = brush.bot;
	pointStart = NULL;
	pointEnd = NULL;



	for (TerrainPoint *tp = brush.pointStart; tp != NULL; tp = tp->next)
	{
		AddPoint(new TerrainPoint(*tp));
	}

	UpdateLines();
}

TerrainBrush::~TerrainBrush()
{
	TerrainPoint *curr = pointStart;
	while (curr != NULL)
	{
		TerrainPoint *temp = curr->next;
		delete curr;
		curr = temp;
	}
}

void TerrainBrush::UpdateLines()
{
	int index = 0;
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		TerrainPoint *prev;
		if (curr == pointStart)
			prev = pointEnd;
		else
			prev = curr->prev;

		lines[index * 2].position = Vector2f(prev->pos.x, prev->pos.y);
		lines[index * 2 + 1].position = Vector2f(curr->pos.x, curr->pos.y);

		++index;
	}
}

void TerrainBrush::Draw(sf::RenderTarget *target)
{
	target->draw(lines);

	CircleShape cs;
	cs.setRadius(5);
	cs.setFillColor(Color::Red);
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);

	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		cs.setPosition(curr->pos.x, curr->pos.y);
		target->draw(cs);
	}
}

void TerrainBrush::Move(Vector2i delta)
{
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		curr->pos.x += delta.x;
		curr->pos.y += delta.y;
	}
	UpdateLines();
	//centerPos.x += delta.x;
	//centerPos.y += delta.y;
}

void TerrainBrush::AddPoint(TerrainPoint *tp)
{
	if (pointStart == NULL)
	{
		pointStart = tp;
		pointEnd = tp;
		tp->prev = NULL;
		tp->next = NULL;
	}
	else
	{
		pointEnd->next = tp;
		tp->prev = pointEnd;
		pointEnd = tp;
		pointEnd->next = NULL;
	}

	++numPoints;
}