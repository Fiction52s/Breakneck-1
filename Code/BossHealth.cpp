#include "BossHealth.h"
#include "VectorMath.h"

using namespace std;
using namespace sf;

BossHealth::BossHealth(Boss *p_boss)
	:boss( p_boss )
{
	bgWidth = 80;
	bgHeight = 500;
	healthPadding = 10;
}

void BossHealth::Reset()
{

}

void BossHealth::SetTopLeft(sf::Vector2f &pos)
{
	SetRectTopLeft(bgQuad, bgWidth, bgHeight, pos);
	SetRectTopLeft(healthQuad, bgWidth - healthPadding * 2, bgHeight - healthPadding * 2, pos + Vector2f( pos.x + healthPadding, pos.y + healthPadding ) );
}

void BossHealth::Update()
{

}

void BossHealth::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(healthQuad, 4, sf::Quads);
}