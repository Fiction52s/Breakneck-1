#include "WorkshopMapPopup.h"
#include "MapHeader.h"
#include "LobbyManager.h"
#include "MapBrowser.h"
#include "md5.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;


WorkshopMapPopup::WorkshopMapPopup()
{
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

	creatorByLabel = panel->AddLabel("creatorbylabel", previewPos + Vector2i(0, -50 + 6), 25, "By: ");
	creatorLabel = panel->AddLabel("creatorlabel", previewPos + Vector2i(40, -50), 25, "");
	creatorLink = panel->AddHyperLink("creatorlink", previewPos + Vector2i(40, -50), 25, "", "");

	descriptionText = panel->AddLabel("description", previewBotLeft + Vector2i(0, 20), 20, "");
	descriptionText->setFillColor(Color::Red);

	action = A_INACTIVE;

	chosenGameModeType = -1;
	chosenGameModeType = MatchParams::GAME_MODE_FIGHT;

	currMapHeader = new MapHeader;

	panel->SetAutoSpacing(false, true, rightColumStart, Vector2i(0, 20));

	fileLink = panel->AddLabeledHyperLink("filelink", Vector2i(0, 0), 20, "", "", "File:");

	panel->AddLabel("maxnumplayerslabel", Vector2i(0, 0), 20, "Max Players: ");
	panel->AddLabel("numverticeslabel", Vector2i(0, 0), 20, "Number of Vertices: ");
	panel->AddLabel("numgameobjectslabel", Vector2i(0, 0), 20, "Number of objects: ");
	panel->AddLabel("sizelabel", Vector2i(0, 0), 20, "Size: ");

	//panel->AddLabel("numobjectslabel", Vector2i(0, 0), 20, "Number of Objects: ");

	std::vector<string> blankOptions;
	blankOptions.push_back("");

	panel->SetAutoSpacing(false, true, rightColumStart + Vector2i(0, 300), Vector2i(0, 20));

	editLocalCopyButton = panel->AddButton("edit", Vector2i(0, 0), Vector2f(400, 30), "Edit Local Copy");
	playButton = panel->AddButton("play", Vector2i(0, 0), Vector2f(400, 30), "Play");
	hostButton = panel->AddButton("workshophost", Vector2i(0, 0), Vector2f(400, 30), "Host Lobby");
	saveLocalCopyButton = panel->AddButton("save", Vector2i(0, 0), Vector2f(400, 30), "Download Local Copy");

	backButton = panel->AddButton("back", Vector2i(0, 0), Vector2f(400, 30), "Back");
	panel->SetCancelButton(backButton);

	panel->StopAutoSpacing();
	//panel->SetAutoSpacing(false, true, rightColumStart + Vector2i(0, 300), Vector2i(0, 20));
	subscribeButton = panel->AddButton("subscribe", rightColumStart + Vector2i(0, 600), Vector2f(400, 50), "Subscribe");
	unsubscribeButton = panel->AddButton("unsubscribe", rightColumStart + Vector2i(0, 600), Vector2f(400, 50), "Unsubscribe");
}

WorkshopMapPopup::~WorkshopMapPopup()
{
	delete panel;

	if (currMapHeader != NULL)
	{
		delete currMapHeader;
	}
}

void WorkshopMapPopup::Update()
{
	CheckSubscription();

	CheckHeader();

	panel->MouseUpdate();
}

void WorkshopMapPopup::HandleEvent(sf::Event ev)
{
	panel->HandleEvent(ev);
}

void WorkshopMapPopup::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
	target->draw(previewSpr);
}

void WorkshopMapPopup::ButtonCallback(Button *b, const std::string & e)
{
	if (b == editLocalCopyButton)
	{

	}
	else if (b == playButton)
	{
		action = A_PLAY;

		MainMenu *mm = MainMenu::GetInstance();

		if (currMapNode->CheckIfFullyInstalled())
		{
			mm->RunFreePlayMap(currMapNode->filePath.string());
		}
		else
		{
			mm->DownloadAndRunWorkshopMap();
		}

	}
	else if (b == hostButton)
	{
		action = A_HOST;
	}
	else if (b == saveLocalCopyButton)
	{

	}
	else if (b == backButton)
	{
		action = A_BACK;
	}
	else if (b == subscribeButton)
	{
		currMapNode->Subscribe();

		unsubscribeButton->HideMember();
		subscribeButton->HideMember();
	}
	else if (b == unsubscribeButton)
	{
		currMapNode->Unsubscribe();

		unsubscribeButton->HideMember();
		subscribeButton->HideMember();
	}
}

bool WorkshopMapPopup::Activate(MapNode *mp)
{
	currMapNode = mp;

	if (currMapHeader != NULL)
	{
		delete currMapHeader;
		currMapHeader = NULL;
	}

	ts_preview = mp->ts_preview;
	if (ts_preview != NULL && ts_preview->texture != NULL)
	{
		ts_preview->SetSpriteTexture(previewSpr);
	}

	descriptionText->setString(mp->description);

	nameLabel->setString("");

	nameLink->ShowMember();
	nameLink->SetString(mp->fullMapName);
	nameLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(mp->publishedFileId));

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
	}

	CheckHeader();

	action = A_ACTIVE;
	return true;
}

void WorkshopMapPopup::Deactivate()
{
	action = A_INACTIVE;
}

void WorkshopMapPopup::CheckSubscription()
{
	if (currMapNode->IsSubscribed())
	{
		subscribeButton->HideMember();
		unsubscribeButton->ShowMember();
	}
	else
	{
		if (currMapNode->subscribing)
		{
			subscribeButton->HideMember();
			unsubscribeButton->HideMember();
		}
		else if (!unsubscribeButton->hidden)
		{
			unsubscribeButton->HideMember();
			subscribeButton->ShowMember();
		}
	}
}

void WorkshopMapPopup::CheckHeader()
{
	if (currMapNode == NULL )
	{
		return;
	}

	bool installed = currMapNode->CheckIfFullyInstalled();
	if (currMapHeader != NULL && !installed )
	{
		delete currMapHeader;
		currMapHeader = NULL;
	}

	if (currMapHeader != NULL)
		return;

	if (installed )
	{
		std::ifstream is;
		is.open(currMapNode->filePath.string());
		currMapHeader = new MapHeader;
		currMapHeader->Load(is);
		is.close();

		fileLink->ShowMember();
		fileLink->SetLinkFileAndFolder(currMapNode->filePath.string(), currMapNode->folderPath.string());
		fileLink->SetString(currMapNode->filePath.filename().string());

		panel->labels["maxnumplayerslabel"]->setString("Number of Player Spawns: " + to_string(currMapHeader->numPlayerSpawns));
		panel->labels["numverticeslabel"]->setString("Number of Vertices: " + to_string(currMapHeader->numVertices));

		string numGameObjectsStr;

		if (currMapHeader->numGameObjects == -1)
		{
			numGameObjectsStr = "?";
		}
		else
		{
			numGameObjectsStr = to_string(currMapHeader->numGameObjects);
		}

		panel->labels["numgameobjectslabel"]->setString("Number of objects: " + numGameObjectsStr);

		string sizeStr;
		if (currMapHeader->functionalWidth == -1 || currMapHeader->functionalHeight == -1)
		{
			sizeStr = "? x ?";
		}
		else
		{
			sizeStr = to_string(currMapHeader->functionalWidth) + " x " + to_string(currMapHeader->functionalHeight);
		}

		panel->labels["sizelabel"]->setString("Size: " + sizeStr);
	}
	else
	{
		fileLink->HideMember();
		//fileLink->SetLinkFileAndFolder(currMapNode->filePath.string(), currMapNode->folderPath.string());
		//fileLink->SetString("");

		panel->labels["maxnumplayerslabel"]->setString("Number of Player Spawns: ?");
		panel->labels["numverticeslabel"]->setString("Number of Vertices: ?");
		panel->labels["numgameobjectslabel"]->setString("Number of objects: ?");
		panel->labels["sizelabel"]->setString("Size: ? x ?");
	}
}

void WorkshopMapPopup::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup")
	{
		action = A_BACK;
	}
}