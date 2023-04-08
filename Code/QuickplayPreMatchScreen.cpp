#include "QuickplayPreMatchScreen.h"
#include "MainMenu.h"
#include "NetplayManager.h"
#include "LobbyManager.h"
#include "MapHeader.h"


using namespace std;
using namespace sf;

QuickplayPreMatchScreen::QuickplayPreMatchScreen()
	:p0Shader( "player" ), p1Shader( "player")
{
	ts_player0 = GetSizedTileset("Menu/pause_kin_400x836.png");
	ts_player1 = GetSizedTileset("Menu/pause_kin_400x836.png");

	TilesetManager::SetGameResourcesMode(false);
	//be careful of this^

	mapHeader = NULL;

	Vector2f previewSize(912, 492);
	Vector2f previewPos(960 - previewSize.x / 2, 120);

	SetRectTopLeft(previewQuad, previewSize.x, previewSize.y, previewPos);

	Vector2f previewBottomLeft = previewPos + Vector2f(0, previewSize.y);
	Vector2f previewBottomCenter(previewBottomLeft.x + previewSize.x / 2, previewBottomLeft.y);

	Vector2f leftNamePos(previewPos.x / 2, previewPos.y);
	Vector2f rightNamePos(1920 - leftNamePos.x, previewPos.y);

	Vector2f mapNamePos = previewBottomCenter + Vector2f(0, 50);
	Vector2f modeNamePos = previewPos + Vector2f(previewSize.x / 2, -100);

	MainMenu *mm = MainMenu::GetInstance();

	modeText.setFont(mm->arial);
	modeText.setFillColor(Color::Red);
	modeText.setCharacterSize(50);
	modeText.setPosition(modeNamePos);

	mapNameText.setFont(mm->arial);
	mapNameText.setFillColor(Color::White);
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

	p0Spr.setPosition(Vector2f(leftNamePos.x - ts_player0->tileWidth / 2, leftNamePos.y + 40));
	p1Spr.setPosition(Vector2f(rightNamePos.x - ts_player1->tileWidth / 2, rightNamePos.y + 40));
	//SetRectTopLeft(p0Quad, ts_player0->tileWidth, ts_player0->tileHeight, Vector2f(leftNamePos.x - ts_player0->tileWidth / 2, leftNamePos.y + 100));
	//SetRectTopLeft(p1Quad, ts_player1->tileWidth, ts_player1->tileHeight, Vector2f(rightNamePos.x - ts_player1->tileWidth / 2, rightNamePos.y + 100));

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
		if (frame == 60 * 3)
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

	target->draw(p0Spr, &p0Shader.pShader);
	target->draw(p1Spr, &p1Shader.pShader);
	//target->draw(p0Quad, 4, sf::Quads, &p0Shader.pShader);
	//target->draw(p1Quad, 4, sf::Quads, &p1Shader.pShader);

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

		string modeStr;
		switch (netplayManager->matchParams.gameModeType)
		{
		case MatchParams::GAME_MODE_FIGHT:
			modeStr = "FIGHT";
			break;
		case MatchParams::GAME_MODE_PARALLEL_RACE:
		{
			modeStr = "SPEEDRUN RACE";
			break;
		}
		default:
		{
			modeStr = "Mode not set";
			break;
		}
		}

		modeText.setString(modeStr); //implement tomorrow //netplayManager->matchParams.gameModeType);
		auto lbModeText = modeText.getLocalBounds();
		modeText.setOrigin(lbModeText.left + lbModeText.width / 2, 0);

		mapNameText.setString(mapHeader->fullName);
		auto lbMapNameText = mapNameText.getLocalBounds();
		mapNameText.setOrigin(lbMapNameText.left + lbMapNameText.width / 2,0);


		leftPlayerName.setString(netplayManager->netplayPlayers[0].name);
		auto lbLeftPlayerName = leftPlayerName.getLocalBounds();
		leftPlayerName.setOrigin(lbLeftPlayerName.left + lbLeftPlayerName.width / 2, 0 );


		rightPlayerName.setString(netplayManager->netplayPlayers[1].name);
		auto lbRightPlayerName = rightPlayerName.getLocalBounds();
		rightPlayerName.setOrigin(lbRightPlayerName.left + lbRightPlayerName.width / 2, 0);

		is.close();
	}
	else
	{
		cout << "cannot update map header for waiting room. file at: " << mapPath << endl;
	}

	//Vector2f texSize(ts_player0->texture->getSize());
	//SetRectSubRectGL(p0Quad, ts_player0->GetSubRect(0), texSize);
	//SetRectSubRectGL(p1Quad, ts_player1->GetSubRect(0), texSize);

	p0Shader.SetTileset(ts_player0);
	p1Shader.SetTileset(ts_player1);

	p0Spr.setTexture(*ts_player0->texture);
	p1Spr.setTexture(*ts_player1->texture);

	ts_player0->SetSubRect(p0Spr, 0);
	ts_player1->SetSubRect(p1Spr, 0);

	p0Shader.SetSubRect(ts_player0, ts_player0->GetSubRect(0));
	p1Shader.SetSubRect(ts_player1, ts_player1->GetSubRect(0));

	p0Shader.SetSkin(netplayManager->matchParams.playerSkins[0]);
	p1Shader.SetSkin(netplayManager->matchParams.playerSkins[1]);
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
