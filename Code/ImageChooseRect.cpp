#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

EditorDecorInfo * ImageChooseRect::CreateDecor()
{
	EditorDecorInfo *edi = new EditorDecorInfo(decorName, ts, tileIndex, 0, Vector2f(0, 0),
		0, Vector2f(1, 1));
	return edi;
}

void ImageChooseRect::SetImage(Tileset *p_ts, const sf::IntRect &subRect)
{
	if (p_ts == NULL)
	{
		SetShown(false);
		ts = p_ts;
		tileIndex = -1;
		return;
	}

	ts = p_ts;
	tileIndex = -1;

	ts->SetSpriteTexture(spr);
	spr.setTextureRect(subRect);
	spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);

	SetSize(boxSize);
}

void ImageChooseRect::SetImage(Tileset *p_ts, int p_index)
{
	if (p_ts == NULL)
	{
		SetShown(false);
		ts = p_ts;
		tileIndex = p_index;
		return;
	}

	if (!(p_ts == ts && p_index == tileIndex))
	{
		ts = p_ts;
		tileIndex = p_index;


		ts->SetSpriteTexture(spr);
		ts->SetSubRect(spr, tileIndex);
		spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);

		SetSize(boxSize);
	}
}

ImageChooseRect::ImageChooseRect(ChooseRectIdentity ident, sf::Vertex *v, Vector2f &p_pos, Tileset *p_ts,
	int p_tileIndex, int bSize, Panel *p)
	:ChooseRect(ident, ChooseRectType::IMAGE, v, bSize, p_pos, p)
{
	ts = NULL;
	SetImage(p_ts, p_tileIndex);
}

ImageChooseRect::ImageChooseRect(ChooseRectIdentity ident, sf::Vertex *v,
	sf::Vector2f &p_pos, Tileset *p_ts, const sf::IntRect &subRect, int bSize, Panel *p)
	:ChooseRect(ident, ChooseRectType::IMAGE, v, bSize, p_pos, p)
{
	ts = NULL;
	SetImage(p_ts, subRect);
}

void ImageChooseRect::UpdatePanelPos()
{
	Vector2f truePos = GetGlobalPos() + Vector2f(boxSize / 2.f, boxSize / 2.f);

	float test;
	FloatRect aabb = spr.getGlobalBounds();
	float max = std::max(aabb.height, aabb.width) * 1.2f; //.8 to give the box a little room
	test = max / boxSize;
	view.setCenter(Vector2f(960 * test - truePos.x * test, 540 * test - truePos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
	view.setSize(Vector2f(1920 * test, 1080 * test));
}

void ImageChooseRect::SetSize(float s)
{
	ChooseRect::SetSize(s);
	UpdatePanelPos();
}

void ImageChooseRect::Draw(RenderTarget *target)
{
	if (show)
	{
		sf::View oldView = target->getView();
		target->setView(view);

		target->draw(spr);

		target->setView(oldView);

		target->draw(nameText);
	}
}

void ImageChooseRect::UpdateSprite(int frameUpdate)
{
	//animate eventually
}