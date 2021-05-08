#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"
#include "MusicSelector.h"

using namespace sf;
using namespace std;

TextChooseRect::TextChooseRect(ChooseRectIdentity ident,
	sf::Vertex *v, sf::Vector2f &p_pos,
	const std::string &str, sf::Vector2f &p_boxSize,
	Panel *p)
	:ChooseRect(ident, ChooseRectType::TEXT, v, p_boxSize, p_pos, p)
{
	SetText(str);
	//SetImage(p_ts, p_tileIndex);
}

void TextChooseRect::UpdatePanelPos()
{
	Vector2f truePos = GetGlobalPos() + Vector2f(boxSize.x / 2.f, boxSize.y / 2.f);

	//float test;
	//FloatRect aabb = spr.getGlobalBounds();
	//float max = std::max(aabb.height, aabb.width) * 1.2f; //.8 to give the box a little room
	//test = max / boxSize;
	//view.setCenter(Vector2f(960 * test - truePos.x * test, 540 * test - truePos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
	//view.setSize(Vector2f(1920 * test, 1080 * test));
}

void TextChooseRect::UpdateTextPosition()
{
	nameText.setPosition(Vector2f(pos.x, pos.y + boxSize.y / 2));// +boxSize + 2));
}

void TextChooseRect::SetName(const std::string &name)
{
	nameText.setString(name);
	nameText.setOrigin(nameText.getLocalBounds().left,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height /2);
}

void TextChooseRect::SetSize(sf::Vector2f &bSize)
{
	ChooseRect::SetSize(bSize);
	UpdatePanelPos();
}

void TextChooseRect::SetText(const std::string &str)
{
	SetName(str);
	UpdateTextPosition();
}

void TextChooseRect::Draw(RenderTarget *target)
{
	if (show)
	{
		target->draw(nameText);
	}
}