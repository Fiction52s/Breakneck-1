#include "WorkshopMapPopup.h"
#include "MapHeader.h"
#include "LobbyManager.h"
#include "MapBrowser.h"
#include "md5.h"
#include "MainMenu.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace sf;

using namespace boost::filesystem;



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

	panel->AddLabel("statuslabel", Vector2i(0, 0), 20, "Status:");

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

	currStatus = -1;
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

	CheckStatus();

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
		action = A_SAVE_AND_EDIT;
	}
	else if (b == playButton)
	{
		action = A_PLAY;
	}
	else if (b == hostButton)
	{
		action = A_HOST;
	}
	else if (b == saveLocalCopyButton)
	{
		action = A_SAVE;

		//if (currMapNode->CheckIfFullyInstalled())
		//{
		//	string userNumber = currMapNode->folderPath.parent_path().filename().string();
		//	path workshopDownloadFolder = current_path().string() + string("/Resources/Maps/WorkshopDownloads/");//currMapNode->folderPath.parent_path();
		//	path userFolder = workshopDownloadFolder.string() + userNumber + "/";


		//	try
		//	{
		//		if (!boost::filesystem::exists(userFolder))
		//		{
		//			if (!boost::filesystem::create_directory(userFolder))
		//			{
		//				cout << "directory creation failure" << endl;
		//				assert(0);
		//				action = A_ACTIVE;
		//				return;
		//			}
		//		}

		//		//assume directory exists now
		//		string mapNumber = currMapNode->folderPath.filename().string();
		//		path mapFolder = userFolder.string() + mapNumber + "/";

		//		if (boost::filesystem::exists(mapFolder))
		//		{
		//			//tell the user that 
		//		}

		//	}
		//	catch (boost::filesystem::filesystem_error const & e)
		//	{
		//		cout << "fs error: " << e.what() << "\n";
		//	}
		//	

		//	//boost::filesystem::copy_file(from to, copy_option::fail_if_exists);
		//}
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


	currStatus = -1;
	CheckHeader();
	CheckStatus();

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

void WorkshopMapPopup::CheckStatus()
{
	if (currMapNode == NULL)
	{
		panel->labels["statuslabel"]->setString("Status: ");
		return;
	}

	int oldStatus = currStatus;
	currStatus = SteamUGC()->GetItemState(currMapNode->publishedFileId);

	if (currStatus == oldStatus)
		return;


	bool fullyInstalled = (currStatus & k_EItemStateInstalled) && (currStatus & k_EItemStateSubscribed);

	string statusStr = "";
	if (fullyInstalled)
	{
		statusStr = "Installed";
	}
	else if (currStatus & k_EItemStateNeedsUpdate)
	{
		statusStr = "Attempting Download";

		if (currMapNode->mapUpdating && currMapNode->downloadResult != -1)
		{
			switch (currMapNode->downloadResult)
			{
			case k_EResultOK:
			{

				break;
			}
			case k_EResultTimeout:
			{
				statusStr = "ERROR - TIMEOUT";
				break;
			}
			default:
			{
				statusStr = "ERROR - " + to_string(currMapNode->downloadResult);
			}
			}
		}
	}
	else if (currStatus & k_EItemStateDownloadPending)
	{
		statusStr = "Download Pending";
	}
	else if (currStatus & k_EItemStateDownloading)
	{
		statusStr = "Downloading";

		uint64 bytesDownloaded;
		uint64 bytesTotal;
		bool infoAvailable = SteamUGC()->GetItemDownloadInfo(currMapNode->publishedFileId, &bytesDownloaded, &bytesTotal);
		if (infoAvailable)
		{
			int percentDownloaded = roundf((((float)bytesDownloaded) / bytesTotal) * 100.f);
			statusStr += " " + to_string(percentDownloaded) + "%";
		}
		else
		{
			cout << "info not available!" << endl;
		}
	}
	else if (!(currStatus &k_EItemStateSubscribed))
	{
		statusStr = "Unsubscribed";
	}
	else
	{
		statusStr = to_string(currStatus);
	}

	panel->labels["statuslabel"]->setString("Status: " + statusStr);
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

	if (!installed )
	{
		currMapNode->TryUpdate();
	}

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