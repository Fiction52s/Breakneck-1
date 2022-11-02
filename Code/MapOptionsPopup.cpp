#include "MapOptionsPopup.h"
#include "MapHeader.h"
#include "LobbyManager.h"
#include "MapBrowser.h"
#include "md5.h"

using namespace std;
using namespace sf;


MapOptionsPopup::MapOptionsPopup()
{
	//panel = new Panel("mapoptions", 500,
	//	300, this, true);

	

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

	creatorByLabel = panel->AddLabel( "creatorbylabel", previewPos + Vector2i(0, -50 + 5), 25, "By: ");
	creatorLabel = panel->AddLabel("creatorlabel", previewPos + Vector2i(50, -50), 25, "");
	creatorLink = panel->AddHyperLink("creatorlink", previewPos + Vector2i(50, -50), 25, "", "");

	descriptionText = panel->AddLabel("description", previewBotLeft + Vector2i(0, 20), 20, "");
	descriptionText->setFillColor(Color::Red);

	action = A_INACTIVE;

	chosenGameModeType = -1;
	chosenGameModeType = MatchParams::GAME_MODE_FIGHT;

	currMapHeader = new MapHeader;
	currLobbyParams = new LobbyParams;

	gameModeOptions.reserve(MatchParams::GAME_MODE_Count);

	gameModeDropdownModes.reserve(MatchParams::GAME_MODE_Count);

	playerNumOptions.reserve(4);

	panel->SetAutoSpacing(false, true, rightColumStart, Vector2i(0, 20));

	std::vector<string> blankOptions;
	blankOptions.push_back("");

	modeDropdown = panel->AddDropdown("gamemodedropdown", Vector2i(0, 0), Vector2i(400, 28), blankOptions, 0);
	modeDropdown->SetToolTip("Choose Map Mode");

	numPlayersDropdown = panel->AddDropdown("numplayersdropdown", Vector2i(0, 0), Vector2i(400, 28), blankOptions, 0);
	numPlayersDropdown->SetToolTip("Choose Num Players");

	panel->SetAutoSpacing(true, false, rightColumStart + Vector2i(0, panel->size.y - 50), Vector2i(30, 0));

	panel->confirmButton =
		panel->AddButton("ok", Vector2i(0, 0), Vector2f(60, 30), "Host");
	panel->cancelButton =
		panel->AddButton("cancel", Vector2i(0, 0), Vector2f(80, 30), "Cancel");

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
	delete currLobbyParams;
}

void MapOptionsPopup::Update()
{
	panel->MouseUpdate();
}

void MapOptionsPopup::HandleEvent(sf::Event ev)
{
	panel->HandleEvent(ev);
}

void MapOptionsPopup::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
	target->draw(previewSpr);
}

void MapOptionsPopup::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "ok")
	{
		currLobbyParams->randSeed = time(0);
		currLobbyParams->creatorID = currMapHeader->creatorID;
		//currLobbyParams->maxMembers = 2;
		currLobbyParams->maxMembers = playerNumOptions[numPlayersDropdown->selectedIndex];
		currLobbyParams->gameModeType = gameModeDropdownModes[modeDropdown->selectedIndex];//MatchParams::GAME_MODE_FIGHT; //eventually option set by popup

		cout << "game mode confirmed as: " << currLobbyParams->gameModeType << endl;

		action = A_CONFIRMED;
	}
	else if (b->name == "cancel")
	{
		action = A_CANCELLED;
	}
}

void MapOptionsPopup::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	if (dropdown == modeDropdown )
	{
		UpdateNumPlayerOptions();
	}
}

bool MapOptionsPopup::Activate(MapNode *mp)
{
	//mapOptionsPopup->Activate(boost::filesystem::relative(selectedMap->filePath).string());
	if (mp->isWorkshop)
	{
		currLobbyParams->mapPath = mp->filePath.string();
	}
	else
	{
		currLobbyParams->mapPath = boost::filesystem::relative(mp->filePath).string();
	}

	ts_preview = mp->ts_preview;
	if (ts_preview != NULL && ts_preview->texture != NULL)
	{
		ts_preview->SetSpriteTexture(previewSpr);	
	}

	descriptionText->setString(mp->description);
	
	if (mp->isWorkshop)
	{
		nameLabel->setString("");

		nameLink->SetString(mp->fullMapName);
		nameLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(mp->publishedFileId));
	}
	else
	{
		nameLabel->setString(mp->fullMapName);

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
		creatorLabel->setString("");
	}
	else
	{
		creatorLink->HideMember();
		creatorLabel->setString(mp->creatorName);
		//creatorLink->SetLinkURL("https://steamcommunity.com/profiles/" + to_string(mp->creatorId));
		//creatorLink->SetString(mp->creatorName);
	}
	

	//mapLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(uploadID));


	//currLobbyParams->randSeed = time(0);


	std::ifstream is;
	is.open(currLobbyParams->mapPath);

	assert(is.is_open());
	std::string content((std::istreambuf_iterator<char>(is)),
		(std::istreambuf_iterator<char>()));
	currLobbyParams->fileHash = md5(content);

	is.clear();
	is.seekg(0, ios::beg);

	currMapHeader->Load(is);
	is.close();

	/*for (auto it = panel->dropdowns.begin(); it != panel->dropdowns.end(); ++it)
	{
		delete (*it).second;
	}
	panel->dropdowns.clear();*/

	
	panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	gameModeOptions.clear();
	gameModeDropdownModes.clear();

	for (int i = 0; i < MatchParams::GAME_MODE_Count; ++i)
	{
		if ( MatchParams::IsMultiplayerMode(i) && currMapHeader->CanRunAsMode(i))
		{
			gameModeOptions.push_back(MatchParams::GetGameModeName(i));
			gameModeDropdownModes.push_back(i);
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


	

	UpdateNumPlayerOptions();
	

	//numPlayersDropdown = panel->AddDropdown( "numplayersdropdown", Vector2f( 0, 0 ), Vector2f( 50, 28 ), 

	currLobbyParams->isWorkshopMap = mp->isWorkshop;
	currLobbyParams->publishedFileId = mp->publishedFileId;

	action = A_ACTIVE;

	return true;
}

void MapOptionsPopup::UpdateNumPlayerOptions()
{
	playerNumOptions = MatchParams::GetNumPlayerOptions(gameModeDropdownModes[modeDropdown->selectedIndex]);

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