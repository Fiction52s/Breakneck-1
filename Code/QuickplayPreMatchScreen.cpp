#include "QuickplayPreMatchScreen.h"
#include "MainMenu.h"
#include "NetplayManager.h"
#include "LobbyManager.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;

QuickplayPreMatchScreen::QuickplayPreMatchScreen()
{
	TilesetManager::SetGameResourcesMode(false);

	mapHeader = NULL;

	Vector2f previewSize(912, 492);
	Vector2f previewPos(960 - previewSize.x / 2, 120);

	SetRectTopLeft(previewQuad, previewSize.x, previewSize.y, previewPos);

	previewBottomLeft = previewPos + Vector2f(0, previewSize.y);

	MainMenu *mm = MainMenu::GetInstance();

	nameText.setFont(mm->arial);
	nameText.setFillColor(Color::White);
	nameText.setCharacterSize(40);
	nameText.setPosition(previewPos + Vector2f(0, -100));

	descriptionText.setFont(mm->arial);
	descriptionText.setFillColor(Color::Red);
	descriptionText.setCharacterSize(20);
	descriptionText.setPosition(previewBottomLeft + Vector2f(0, 20));


	SetRectColor(bgQuad, Color::Cyan);
	SetRectTopLeft(bgQuad, 1920, 1080, Vector2f(0, 0));

	ts_preview = NULL;

	action = A_STARTUP;
}

QuickplayPreMatchScreen::~QuickplayPreMatchScreen()
{
	if (mapHeader != NULL)
	{
		delete mapHeader;
		mapHeader = NULL;
	}
}

void QuickplayPreMatchScreen::Update()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	switch (action)
	{
	case A_STARTUP:
	{
		action = A_SHOW;
		frame = 0;
		break;
	}
	case A_SHOW:
	{
		if (frame == 60 * 2)
		{
			action = A_DONE;
			frame = 0;
		}
		break;
	}
	case A_DONE:
	{

		break;
	}
	}

	switch (action)
	{
	case A_STARTUP:
	{
		break;
	}
	case A_SHOW:
	{
		break;
	}
	case A_DONE:
	{

		break;
	}
	}

	++frame;
}

void QuickplayPreMatchScreen::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);

	if (ts_preview != NULL)
	{
		target->draw(previewQuad, 4, sf::Quads, ts_preview->texture);
	}

	target->draw(nameText);
	target->draw(descriptionText);
}

void QuickplayPreMatchScreen::UpdateMapHeader(const std::string &mapPath)
{
	if (mapHeader != NULL)
	{
		delete mapHeader;
		mapHeader = NULL;
	}

	ifstream is;
	is.open(mapPath);

	if (is.is_open())
	{
		mapHeader = new MapHeader;

		mapHeader->Load(is);

		nameText.setString(mapHeader->description);
		descriptionText.setString(mapHeader->fullName);

		is.close();
	}
	else
	{
		cout << "cannot update map header for waiting room. file at: " << mapPath << endl;
	}
}

void QuickplayPreMatchScreen::SetPreview(const std::string &previewPath)
{
	ClearPreview();

	ts_preview = GetTileset(previewPath);

	if (ts_preview == NULL)
	{
		cout << "preview path failed: " << previewPath << endl;
		return;
	}

	SetRectSubRect(previewQuad, ts_preview->GetSubRect(0));

	//cout << "setting waiting room preview: " << previewPath << endl;

	if (ts_preview != NULL)
	{
		//	cout << "preview successfully set" << endl;
	}
}

void QuickplayPreMatchScreen::ClearPreview()
{
	if (ts_preview != NULL)
	{
		DestroyTileset(ts_preview);
		ts_preview = NULL;
	}
}

void QuickplayPreMatchScreen::Clear()
{
	action = A_STARTUP;

	ClearPreview();

	if (mapHeader != NULL)
	{
		delete mapHeader;
		mapHeader = NULL;
	}

	descriptionText.setString("");
	nameText.setString("");
}

void QuickplayPreMatchScreen::SetAction(Action a)
{
	action = a;
	frame = 0;

}
