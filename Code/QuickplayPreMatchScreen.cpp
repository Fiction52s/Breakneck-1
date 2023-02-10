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

	Vector2f previewBottomLeft = previewPos + Vector2f(0, previewSize.y);
	Vector2f previewBottomCenter(previewBottomLeft.x + previewSize.x / 2, previewBottomLeft.y);

	Vector2f leftNamePos(previewPos.x / 2, previewPos.y);
	Vector2f rightNamePos(1920 - leftNamePos.x, previewPos.y);

	Vector2f mapNamePos = previewBottomCenter + Vector2f(0, 50);
	Vector2f modeNamePos = previewPos + Vector2f(previewSize.x, -100);

	MainMenu *mm = MainMenu::GetInstance();


	modeText.setFont(mm->arial);
	modeText.setFillColor(Color::Red);
	modeText.setCharacterSize(50);
	modeText.setPosition(modeNamePos);

	mapNameText.setFont(mm->arial);
	mapNameText.setFillColor(Color::Red);
	mapNameText.setCharacterSize(50);
	mapNameText.setPosition(mapNamePos);

	leftPlayerName.setFont(mm->arial);
	leftPlayerName.setFillColor(Color::White);
	leftPlayerName.setCharacterSize(40);
	leftPlayerName.setPosition(leftNamePos);

	rightPlayerName.setFont(mm->arial);
	rightPlayerName.setFillColor(Color::White);
	rightPlayerName.setCharacterSize(40);
	rightPlayerName.setPosition(rightNamePos);

	SetRectColor(bgQuad, Color::Black);
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

	target->draw(modeText);
	target->draw(mapNameText);
	target->draw(leftPlayerName);
	target->draw(rightPlayerName);
}

void QuickplayPreMatchScreen::SetToNetplayMatchParams()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	string mapPath = netplayManager->matchParams.mapPath.string();

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

		modeText.setString("FIGHT"); //implement tomorrow //netplayManager->matchParams.gameModeType);
		mapNameText.setString(mapHeader->fullName);
		leftPlayerName.setString(netplayManager->netplayPlayers[0].name);
		rightPlayerName.setString(netplayManager->netplayPlayers[1].name);

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

	modeText.setString("");
	mapNameText.setString("");
	leftPlayerName.setString("");
	rightPlayerName.setString("");
}

void QuickplayPreMatchScreen::SetAction(Action a)
{
	action = a;
	frame = 0;

}
