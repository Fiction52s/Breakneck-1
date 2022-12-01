#include "MapOptionsPopup.h"
#include "MapHeader.h"
#include "LobbyManager.h"
#include "MapBrowser.h"
#include "md5.h"

using namespace std;
using namespace sf;


MapOptionsPopup::MapOptionsPopup( int p_mode )
{
	//panel = new Panel("mapoptions", 500,
	//	300, this, true);
	mode = p_mode;
	

	previewPos = Vector2i(10, 110);
	Vector2i previewSize(912, 492);
	Vector2i previewBotLeft = previewPos + Vector2i(0, previewSize.y);

	Vector2i rightColumStart(previewPos.x + previewSize.x + 10, 10);

	panel = new Panel("mapoptions", rightColumStart.x + 410,
		800, this, true);


	panel->SetCenterPos(Vector2i(960, 540));

	
	previewSpr.setPosition(Vector2f(previewPos + panel->pos));

	nameLink = panel->AddHyperLink("namelink", previewPos + Vector2i(0, -100), 40, "", "");
	nameLabel = panel->AddLabel("namelabel", previewPos + Vector2i(0, -100), 40, "");

	creatorByLabel = panel->AddLabel( "creatorbylabel", previewPos + Vector2i(0, -50 + 6), 25, "By: ");
	creatorLabel = panel->AddLabel("creatorlabel", previewPos + Vector2i(40, -50), 25, "");
	creatorLink = panel->AddHyperLink("creatorlink", previewPos + Vector2i(40, -50), 25, "", "");

	descriptionLabel = panel->AddLabel("description", previewBotLeft + Vector2i(0, 20), 20, "");
	descriptionLabel->text.setFillColor(Color::Red);

	action = A_INACTIVE;

	chosenGameModeType = -1;
	chosenGameModeType = MatchParams::GAME_MODE_FIGHT;

	currMapHeader = new MapHeader;
	currLobbyData = new LobbyData;

	gameModeOptions.reserve(MatchParams::GAME_MODE_Count);

	gameModeDropdownModes.reserve(MatchParams::GAME_MODE_Count);

	playerNumOptions.reserve(4);

	

	panel->SetAutoSpacing(false, true, rightColumStart, Vector2i(0, 20));

	fileLink = panel->AddLabeledHyperLink("filelink", Vector2i(0, 0), 20, "", "", "File:");

	panel->AddLabel("maxnumplayerslabel", Vector2i(0, 0), 20, "Max Players: ");
	panel->AddLabel("numverticeslabel", Vector2i(0, 0), 20, "Number of Vertices: ");
	panel->AddLabel("numgameobjectslabel", Vector2i(0, 0), 20, "Number of objects: ");
	panel->AddLabel("sizelabel", Vector2i(0, 0), 20, "Size: ");

	//panel->AddLabel("numobjectslabel", Vector2i(0, 0), 20, "Number of Objects: ");

	std::vector<string> blankOptions;
	blankOptions.push_back("");


	panel->AddLabel("gamemodelabel", Vector2i(0,0), 20, "Game Mode:");

	modeDropdown = panel->AddDropdown("gamemodedropdown", Vector2i(0, -15), Vector2i(400, 28), blankOptions, 0);
	modeDropdown->SetToolTip("Choose Map Mode");

	numPlayersLabel = panel->AddLabel("playernumberlabel", Vector2i(0, 0), 20, "Number of Players:");

	numPlayersDropdown = panel->AddDropdown("numplayersdropdown", Vector2i(0, -15), Vector2i(400, 28), blankOptions, 0);
	numPlayersDropdown->SetToolTip("Choose Num Players");

	//panel->SetAutoSpacing(true, false, rightColumStart + Vector2i(0, panel->size.y - 50), Vector2i(30, 0));

	
	panel->SetAutoSpacing(false, true, rightColumStart + Vector2i(0, 300), Vector2i(0, 20));

	createLobbyHostButton = panel->AddButton("host", Vector2i(0, 0), Vector2f(60, 30), "Host");
	createLobbyCancelButton = panel->AddButton("cancel", Vector2i(0, 0), Vector2f(80, 30), "Cancel");

	panel->confirmButton = createLobbyHostButton;
	panel->cancelButton = createLobbyCancelButton;

	panel->StopAutoSpacing();
	/*panel->ReserveImageRects(totalRects + extraImageRects);
	panel->extraUpdater = this;*/

	//panel->MouseUpdate();

	/*upButton = panel->AddButton("up", Vector2i(10, 10), Vector2f(30, 30), "up");
	playButton = panel->AddButton("play", Vector2i(100, 10), Vector2f(30, 30), "play");
	folderPathText = panel->AddLabel("folderpath", Vector2i(50, 10), 30, "");

	Vector2i pageButtonOrigin(750, 990);

	prevPageButton = panel->AddButton("prevpage", Vector2i(pageButtonOrigin), Vector2f(30, 30), "<");
	nextPageButton = panel->AddButton("nextpage", Vector2i(pageButtonOrigin + Vector2i(60, 0)), Vector2f(30, 30), ">");*/
}

MapOptionsPopup::~MapOptionsPopup()
{
	delete panel;
	delete currMapHeader;
	delete currLobbyData;
}

void MapOptionsPopup::Update()
{
	panel->MouseUpdate();
}

bool MapOptionsPopup::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void MapOptionsPopup::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
	target->draw(previewSpr);
}

void MapOptionsPopup::ButtonCallback(Button *b, const std::string & e)
{
	if( b == createLobbyHostButton )
	{
		currLobbyData->randSeed = time(0);
		currLobbyData->creatorId = currMapHeader->creatorID;
		//currLobbyParams->maxMembers = 2;
		if (numPlayersDropdown->selectedIndex < 0)
		{
			currLobbyData->maxMembers = -1; //for freeplay/offline mode
		}
		else
		{
			currLobbyData->maxMembers = playerNumOptions[numPlayersDropdown->selectedIndex];
		}
		
		currLobbyData->gameModeType = gameModeDropdownModes[modeDropdown->selectedIndex];//MatchParams::GAME_MODE_FIGHT; //eventually option set by popup

		cout << "game mode confirmed as: " << currLobbyData->gameModeType << endl;

		action = A_HOST;
	}
	else if (b == createLobbyCancelButton )
	{
		action = A_CANCELLED;
	}
}

void MapOptionsPopup::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	if (dropdown == modeDropdown )
	{
		if (!numPlayersDropdown->hidden)
		{
			UpdateNumPlayerOptions();
		}
	}
}

void MapOptionsPopup::CancelCallback(Panel *p)
{
	action = A_CANCELLED;
}

void MapOptionsPopup::PanelCallback(Panel *p, const std::string &e)
{
	if (e == "leftclickoffpopup")
	{
		CancelCallback(p);
		//action = A_BACK;
	}
}

bool MapOptionsPopup::Activate(MapNode *mp, int numP)
{
	// set the name of the lobby if it's ours
	string lobbyName = SteamFriends()->GetPersonaName();
	lobbyName += "'s lobby";

	currLobbyData->lobbyName = lobbyName;

	if (mp->isWorkshop)
	{
		currLobbyData->mapPath = mp->filePath.string();
	}
	else
	{
		currLobbyData->mapPath = boost::filesystem::relative(mp->filePath).string();
	}

	ts_preview = mp->ts_preview;
	if (ts_preview != NULL && ts_preview->texture != NULL)
	{
		ts_preview->SetSpriteTexture(previewSpr);
	}

	descriptionLabel->text.setString(mp->description);
	
	if (mp->isWorkshop)
	{
		nameLabel->text.setString("");

		nameLink->ShowMember();
		nameLink->SetString(mp->fullMapName);
		nameLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(mp->publishedFileId));
	}
	else
	{
		nameLabel->text.setString(mp->fullMapName);

		nameLink->HideMember();

		if (!mp->creatorNameRetrieved)
		{
			bool needsToRequestInfo = SteamFriends()->RequestUserInformation(mp->creatorId, true);

			if (needsToRequestInfo)
			{
				mp->checkingForCreatorName = true;
			}
			else
			{
				mp->creatorNameRetrieved = true;
				mp->creatorName = SteamFriends()->GetFriendPersonaName(mp->creatorId);
			}
		}
	}

	if (mp->creatorNameRetrieved)
	{
		creatorLink->ShowMember();
		creatorLink->SetLinkURL("https://steamcommunity.com/profiles/" + to_string(mp->creatorId));
		creatorLink->SetString(mp->creatorName);
		creatorLabel->text.setString("");
	}
	else
	{
		creatorLink->HideMember();
		creatorLabel->text.setString(mp->creatorName);
		//creatorLink->SetLinkURL("https://steamcommunity.com/profiles/" + to_string(mp->creatorId));
		//creatorLink->SetString(mp->creatorName);
	}
	
	//panel->labels["filenamelabel"]->setString("File: " + mp->filePath.filename().string());
	fileLink->SetLinkFileAndFolder(mp->filePath.string(), mp->folderPath.string());
	fileLink->SetString(mp->filePath.filename().string());
	//mapLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(uploadID));


	//currLobbyParams->randSeed = time(0);


	std::ifstream is;
	is.open(currLobbyData->mapPath);

	assert(is.is_open());
	std::string content((std::istreambuf_iterator<char>(is)),
		(std::istreambuf_iterator<char>()));
	currLobbyData->fileHash = md5(content);

	is.clear();
	is.seekg(0, ios::beg);

	currMapHeader->Load(is);
	is.close();

	/*for (auto it = panel->dropdowns.begin(); it != panel->dropdowns.end(); ++it)
	{
		delete (*it).second;
	}
	panel->dropdowns.clear();*/

	//currMapHeader->

	panel->labels["maxnumplayerslabel"]->text.setString( "Number of Player Spawns: " + to_string(currMapHeader->numPlayerSpawns));
	panel->labels["numverticeslabel"]->text.setString("Number of Vertices: " + to_string( currMapHeader->numVertices));


	string numGameObjectsStr;

	if (currMapHeader->numGameObjects == -1)
	{
		numGameObjectsStr = "?";
	}
	else
	{
		numGameObjectsStr = to_string(currMapHeader->numGameObjects);
	}

	panel->labels["numgameobjectslabel"]->text.setString("Number of objects: " + numGameObjectsStr);

	string sizeStr;
	if (currMapHeader->functionalWidth == -1 || currMapHeader->functionalHeight == -1)
	{
		sizeStr = "? x ?";
	}
	else
	{
		sizeStr = to_string(currMapHeader->functionalWidth) + " x " + to_string(currMapHeader->functionalHeight);
	}

	panel->labels["sizelabel"]->text.setString("Size: " + sizeStr);

	
	panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	gameModeOptions.clear();
	gameModeDropdownModes.clear();

	if (numP == 0)
	{
		for (int i = 0; i < MatchParams::GAME_MODE_Count; ++i)
		{
			if (MatchParams::IsMultiplayerMode(i) && currMapHeader->CanRunAsMode(i))
			{
				gameModeOptions.push_back(MatchParams::GetGameModeName(i));
				gameModeDropdownModes.push_back(i);
			}
		}
	}
	else
	{
		for (int i = 0; i < MatchParams::GAME_MODE_Count; ++i)
		{
			if (currMapHeader->CanRunAsMode(numP, i))//MatchParams::IsMultiplayerMode(i) && currMapHeader->CanRunAsMode(i))
			{
				gameModeOptions.push_back(MatchParams::GetGameModeName(i));
				gameModeDropdownModes.push_back(i);
			}
		}
	}
	

	if (gameModeOptions.empty())
	{
		return false;
	}

	
	

	modeDropdown->SetOptions(gameModeOptions);
	modeDropdown->UpdateOptions();
	modeDropdown->selectedIndex = -1;
	modeDropdown->SetSelectedIndex(0);
	
	if (numP > 0)
	{
		//currMapHeader->numPlayerSpawns
		playerNumOptions.clear();

		numPlayersDropdown->selectedIndex = -1;

		numPlayersDropdown->HideMember();
		numPlayersLabel->HideMember();
		//numPlayersDropdown->SetOptions(playerNumOptionsStr);
		//numPlayersDropdown->UpdateOptions();
		//numPlayersDropdown->selectedIndex = -1;
		//numPlayersDropdown->SetSelectedIndex(startIndex);
	}
	else
	{
		UpdateNumPlayerOptions();
	}
	
	

	//numPlayersDropdown = panel->AddDropdown( "numplayersdropdown", Vector2f( 0, 0 ), Vector2f( 50, 28 ), 

	currLobbyData->isWorkshopMap = mp->isWorkshop;
	currLobbyData->publishedFileId = mp->publishedFileId;

	action = A_ACTIVE;

	

	return true;
}

void MapOptionsPopup::UpdateNumPlayerOptions()
{
	numPlayersDropdown->ShowMember();
	numPlayersLabel->ShowMember();
	//currMapHeader->numPlayerSpawns
	playerNumOptions = MatchParams::GetNumPlayerOptions(gameModeDropdownModes[modeDropdown->selectedIndex], currMapHeader->numPlayerSpawns);

	std::vector<string> playerNumOptionsStr;
	for (auto it = playerNumOptions.begin(); it != playerNumOptions.end(); ++it)
	{
		playerNumOptionsStr.push_back(to_string((*it)));
	}

	int startIndex = 0;
	int ind = 0;
	for (auto it = playerNumOptionsStr.begin(); it != playerNumOptionsStr.end(); ++it)
	{
		if ((*it) == numPlayersDropdown->GetSelectedText())
		{
			startIndex = ind;
			break;
		}
		++ind;
	}

	numPlayersDropdown->SetOptions(playerNumOptionsStr);
	numPlayersDropdown->UpdateOptions();
	numPlayersDropdown->selectedIndex = -1;
	numPlayersDropdown->SetSelectedIndex(startIndex);
	
}