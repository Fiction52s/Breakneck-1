#include "BossHealth.h"
#include "VectorMath.h"
#include "Boss.h"

using namespace std;
using namespace sf;

BossHealth::BossHealth(Boss *p_boss)
	:boss( p_boss )
{
	bgWidth = 80;
	bgHeight = 500;
	healthPadding = 10;

	SetRectColor(bgQuad, Color::Black);
	SetRectColor(healthQuad, Color::Cyan);
}

void BossHealth::Reset()
{

}

void BossHealth::SetTopLeft(sf::Vector2f &pos)
{
	topLeftPosition = pos;
	SetRectTopLeft(bgQuad, bgWidth, bgHeight, pos);

	float fullHeight = bgHeight - healthPadding * 2;
	float currHealthProp = boss->numHealth / (float)boss->maxHealth;
	float height = fullHeight * currHealthProp;
	float bottom = (pos.y + bgHeight) - healthPadding;
	SetRectTopLeft(healthQuad, bgWidth - healthPadding * 2,
		height, Vector2f(pos.x + healthPadding, 
			pos.y + healthPadding + (fullHeight - height)));
	//SetRectTopLeft(healthQuad, bgWidth - healthPadding * 2, bgHeight - healthPadding * 2, pos + Vector2f( healthPadding, healthPadding ) );
}

void BossHealth::Update()
{
	SetTopLeft(topLeftPosition);
}

void BossHealth::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(healthQuad, 4, sf::Quads);
}