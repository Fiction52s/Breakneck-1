#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

ToolTip::ToolTip(const std::string &str)
{
	toolTipText.setCharacterSize(14);
	toolTipText.setFont(MainMenu::GetInstance()->arial);
	toolTipText.setFillColor(Color::Black);
	toolTipText.setString(str);

	SetRectColor(quad, Color::White);
}

void ToolTip::SetFromMousePos(const sf::Vector2i &pos)
{
	Vector2f myPos = Vector2f(pos.x + 20, pos.y + 20);
	float width = toolTipText.getLocalBounds().width;
	float height = toolTipText.getLocalBounds().height;
	float extra = 10;
	SetRectTopLeft(quad, width + extra * 2, height + extra * 2, myPos + Vector2f(-extra, -extra));
	toolTipText.setPosition(myPos);
}

void ToolTip::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
	target->draw(toolTipText);
}