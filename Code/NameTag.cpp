#include "NameTag.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

NameTag::NameTag()
{
	MainMenu *mm = MainMenu::GetInstance();
	nameText.setFont(mm->arial);
	nameText.setFillColor(Color::White);
	nameText.setCharacterSize(20);
	SetName("NOT SET");

	SetHoverDelta(Vector2f(0, -100));

	SetRectColor(bgQuad, Color(50, 50, 50, 200));
}

void NameTag::SetName(const std::string &n)
{
	nameText.setString(n);
	auto lb = nameText.getLocalBounds();
	nameText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height); //center bottom
}

void NameTag::SetPos(sf::Vector2f pos)
{
	trackingPos = pos;
}

void NameTag::UpdatePixelPos(sf::RenderTarget *target)
{
	pixelPos = Vector2f(target->mapCoordsToPixel(trackingPos));
}

bool NameTag::IsActive()
{
	return active;
}

void NameTag::SetActive(bool a)
{
	active = a;
}

void NameTag::SetHoverDelta(sf::Vector2f offset)
{
	hoverDelta = offset;
}

void NameTag::MapDraw(sf::RenderTarget *target, sf::Vector2f offset, sf::Vector2f p_pixelPos)
{
	nameText.setPosition(p_pixelPos + offset);

	auto gb = nameText.getGlobalBounds();
	float extra = 10;
	SetRectTopLeft(bgQuad, gb.width + extra * 2, gb.height + extra * 2, Vector2f(gb.left - extra, gb.top - extra));

	target->draw(bgQuad, 4, sf::Quads);
	target->draw(nameText);
}

void NameTag::Draw(sf::RenderTarget *target)
{

	//Vector2f pos(target->mapCoordsToPixel(trackingPos));

	nameText.setPosition(pixelPos + hoverDelta);

	auto gb = nameText.getGlobalBounds();
	float extra = 10;
	SetRectTopLeft(bgQuad, gb.width + extra * 2, gb.height + extra * 2, Vector2f(gb.left - extra, gb.top - extra));

	target->draw(bgQuad, 4, sf::Quads);
	target->draw(nameText);
}