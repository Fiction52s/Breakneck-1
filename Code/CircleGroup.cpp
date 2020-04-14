#include "CircleGroup.h"
#include "VectorMath.h"
#include <assert.h>

using namespace sf;
using namespace std;

CircleGroup::CircleGroup(int num, int rad, sf::Color col, int m_pointsPerCircle)
{
	numCircles = num;
	pointsPerCircle = m_pointsPerCircle;
	radius = rad;
	color = col;
	numVerts = numCircles * pointsPerCircle * 3;
	va = new Vertex[numVerts];
	circleCenters = new Vector2f[numCircles];

	for (int i = 0; i < numVerts; ++i)
	{
		va[i].color = color;
		va[i].position = Vector2f(0, 0);
	}
	for (int i = 0; i < numCircles; ++i)
	{
		circleCenters[i] = Vector2f(0, 0);
	}
	HideAll();
}

CircleGroup::~CircleGroup()
{
	delete[] va;
	delete[] circleCenters;
}

void CircleGroup::SetColor(int index, sf::Color c)
{
	sf::Vertex *circleStart = (va + index * pointsPerCircle * 3);
	for (int i = 0; i < pointsPerCircle * 3; ++i)
	{
		circleStart[i].color = c;
	}
}

void CircleGroup::ResetColor()
{
	for (int i = 0; i < numVerts; ++i)
	{
		va[i].color = color;
	}
}

void CircleGroup::SetVisible(int index, bool vis)
{
	sf::Vertex *circleStart = (va + index * pointsPerCircle * 3);
	Color newColor;
	if (vis)
	{
		newColor = color;
	}
	else
	{
		newColor = Color::Transparent;
	}

	for (int i = 0; i < pointsPerCircle * 3; ++i)
	{
		circleStart[i].color = newColor;
	}
}

void CircleGroup::SetPosition(int index, Vector2f &pos)
{
	Vector2f center = pos;
	circleCenters[index] = center;
	sf::Vertex *circleStart = (va + index * pointsPerCircle * 3);

	int circleIndex = index * pointsPerCircle * 3;

	sf::Transform t;
	t.rotate(360.f / pointsPerCircle);
	Vector2f diff(0, radius);

	for (int i = 0; i < pointsPerCircle; ++i)
	{
		circleStart[i * 3 + 0].position = center;
		circleStart[i * 3 + 1].position = center + diff;

		diff = t.transformPoint(diff);
		circleStart[i * 3 + 2].position = center + diff;
	}
}

void CircleGroup::ShowAll()
{
	for (int i = 0; i < numCircles; ++i)
	{
		SetVisible(i, true);
	}
}

void CircleGroup::HideAll()
{
	for (int i = 0; i < numCircles; ++i)
	{
		SetVisible(i, false);
	}
}

void CircleGroup::Draw(sf::RenderTarget *target)
{
	target->draw(va, numVerts, sf::Triangles);
}

float CircleGroup::GetRadius()
{
	return radius;
}

const sf::Vector2f CircleGroup::GetPosition(int index)
{
	assert(index >= 0 && index < numCircles);
	return circleCenters[index];
}

//bool CircleGroup::ContainsPoint(int index, sf::Vector2f &p)
//{
//	Vector2f center = GetPosition(index);
//	float lenSqr = lengthSqr(p - center);
//	return (lenSqr <= radius * radius);
//}