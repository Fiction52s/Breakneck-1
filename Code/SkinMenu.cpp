#include "SkinMenu.h"
#include "SaveMenuScreen.h"
#include "Tileset.h"
#include "VectorMath.h"

using namespace sf;

SkinMenu::SkinMenu(SaveMenuScreen *saveScreen)
{
	ts_skins = saveScreen->GetSizedTileset("Menu/Skin/skin_64x64.png");
	for (int i = 0; i < 64; ++i)
	{
		if (i == 10)
		{
			break;
		}
		ts_skins->SetQuadSubRect(skinQuads + i * 4, i);
	}

	gridSpacing = Vector2f(80, 80);



	SetGridTopLeft(Vector2f(200, 200));

	rect.setFillColor(Color::White);
	rect.setSize(Vector2f(64, 64));
	
}

void SkinMenu::Reset()
{
}

void SkinMenu::SetGridTopLeft(sf::Vector2f &pos)
{
	gridTopLeft = pos;
	int i;
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			i = y * 8 + x;
			SetRectTopLeft(skinQuads + i * 4, 64, 64,
				Vector2f(pos.x + gridSpacing.x * x, pos.y + gridSpacing.y * y));
		}
	}
}

bool SkinMenu::Update(ControllerState &currInput, ControllerState &prevInput)
{
	if (currInput.LUp() && !prevInput.LUp())
	{
		selectedPos.y--;
		if (selectedPos.y == -1)
		{
			selectedPos.y = 7;
		}
	}
	else if (currInput.LDown() && !prevInput.LDown())
	{
		selectedPos.y++;
		if (selectedPos.y == 8)
		{
			selectedPos.y = 0;
		}
	}

	if (currInput.LLeft() && !prevInput.LLeft())
	{
		selectedPos.x--;
		if (selectedPos.x == -1)
		{
			selectedPos.x = 7;
		}
	}
	else if (currInput.LRight() && !prevInput.LRight())
	{
		selectedPos.x++;
		if (selectedPos.x == 8)
		{
			selectedPos.x = 0;
		}
	}

	rect.setPosition(gridTopLeft + Vector2f( selectedPos.x * gridSpacing.x,
		selectedPos.y * gridSpacing.y));

	if (currInput.A && !prevInput.A)
	{
		//select skin
	}

	if (currInput.B && !prevInput.B)
	{
		return false;
	}

	return true;
}

void SkinMenu::Draw(sf::RenderTarget *target)
{
	target->draw(rect);
	target->draw(skinQuads, 64 * 4, sf::Quads, ts_skins->texture);
}