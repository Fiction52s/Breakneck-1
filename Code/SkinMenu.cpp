#include "SkinMenu.h"
#include "SaveMenuScreen.h"
#include "Tileset.h"
#include "VectorMath.h"
#include "Actor.h"
#include "MainMenu.h"

using namespace sf;

SkinMenu::SkinMenu(SaveMenuScreen *p_saveScreen)
{
	skinScale = 2.0;
	saveScreen = p_saveScreen;
	ts_skins = saveScreen->GetSizedTileset("Menu/Skin/skin_64x64.png");

	//unlock all skins for debug
	for (int i = 0; i < Actor::SKIN_Count; ++i)
	{
		saveScreen->UnlockSkin(i);
	}

	for (int i = 0; i < Actor::SKIN_Count; ++i)
	{
		if (saveScreen->IsSkinUnlocked(i))
		{
			ts_skins->SetQuadSubRect(skinQuads + i * 4, i);
		}
	}

	

	gridSpacing = Vector2f(80 * skinScale, 80 * skinScale);

	float totalWidth = gridSpacing.x * 8;


	SetGridTopLeft(Vector2f(960 - totalWidth / 2.f, 0));

	rect.setFillColor(Color::White);
	rect.setSize(Vector2f(64 * skinScale, 64 * skinScale));

	SetRectColor(bgQuad, Color( 0, 0, 0, 50 ) );
}

void SkinMenu::Reset()
{
}

void SkinMenu::SetSelectedIndex(int index)
{
	selectedPos.y = index / 8;
	selectedPos.x = index % 8;
}


int SkinMenu::GetSelectedIndex()
{
	return selectedPos.y * 8 + selectedPos.x;
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
			SetRectTopLeft(skinQuads + i * 4, 64 * skinScale, 64 * skinScale,
				Vector2f(pos.x + gridSpacing.x * x, pos.y + gridSpacing.y * y));
		}
	}

	SetRectTopLeft(bgQuad, gridSpacing.x * 8, gridSpacing.y * 8, pos);
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

	int selectedIndex = GetSelectedIndex();

	if (selectedIndex < Actor::SKIN_Count
		&& saveScreen->IsSkinUnlocked(selectedIndex))
	{
		saveScreen->SetSkin(selectedIndex);

		if (currInput.A && !prevInput.A)
		{
			saveScreen->SaveCurrSkin();
			return false;
			//select skin
		}
	}
	
	if ((currInput.B && !prevInput.B)
		||( currInput.rightShoulder && !prevInput.rightShoulder ) )
	{
		return false;
	}

	return true;
}

void SkinMenu::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(rect);
	target->draw(skinQuads, 64 * 4, sf::Quads, ts_skins->texture);
}