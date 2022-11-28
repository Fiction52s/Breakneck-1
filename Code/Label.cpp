#include "GUI.h"

using namespace sf;
using namespace std;

Label::Label( Panel *p )
	:PanelMember( p )
{

}

void Label::Draw(sf::RenderTarget *target)
{
	if (!hidden)
	{
		target->draw(text);
	}
}

bool Label::MouseUpdate()
{
	return true;
}



void Label::SetTopLeftPosition(sf::Vector2i &pos)
{
	auto lb = text.getLocalBounds();
	text.setOrigin(lb.left, lb.top);
	text.setPosition(Vector2f(pos));
}

void Label::SetCenterPosition(sf::Vector2i &pos)
{
	auto lb = text.getLocalBounds();
	text.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2 );
	text.setPosition(Vector2f(pos));
}

void Label::SetTopCenterPosition(sf::Vector2i &pos)
{
	auto lb = text.getLocalBounds();
	text.setOrigin(lb.left + lb.width / 2, lb.top);
	text.setPosition(Vector2f(pos));
}