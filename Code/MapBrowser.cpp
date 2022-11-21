#include "MapBrowser.h"
#include "VectorMath.h"
#include <iostream>
#include "EditSession.h"
#include "WorkshopManager.h"
#include "LobbyManager.h"
#include "md5.h"
#include "UIController.h"
//#include "steam/steam_api.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

MapNode::MapNode()
{
	checkingForPreview = false;
	myBrowser = NULL;
	type = FILE;
	index = -1;
	ts_preview = NULL;
	previewTex = NULL;
	action = IDLE;
	chooseRect = NULL;
	mapDownloaded = false;
	publishedFileId = 0;
	isWorkshop = false;
	nameAndDescriptionUpdated = false;
	creatorNameRetrieved = false;
	checkingForCreatorName = false;
	creatorId = 0;
	mapUpdating = false;
	subscribing = false;
	unsubscribing = false;
	downloadResult = -1;
}

void MapNode::Draw(sf::RenderTarget *target)
{
	//target->draw(previewSpr, 4, sf::Quads, ts_preview->texture);
}

MapNode::~MapNode()
{
	//was maybe getting a memory leak here, put this here to see if it happens, and it doesn't...can't reproduce sigh
	/*if (previewTex != NULL)
	{
		delete previewTex;
	}*/
}

bool MapNode::IsSubscribed()
{
	if (!isWorkshop)
		return true;

	uint32 itemState = SteamUGC()->GetItemState(publishedFileId);

	if ((itemState & k_EItemStateSubscribed))
	{
		return true;
	}

	return false;
}

void MapNode::Subscribe()
{
	if (!isWorkshop)
		return;

	if (!IsSubscribed())
	{
		SteamAPICall_t call = SteamUGC()->SubscribeItem(publishedFileId);
		onRemoteStorageSubscribePublishedFileResultCallResult.Set(call, this, &MapNode::OnSubscribe);
		subscribing = true;
	}
}

void MapNode::Unsubscribe()
{
	if (!isWorkshop)
		return;

	if( IsSubscribed() )
	{
		SteamAPICall_t call = SteamUGC()->UnsubscribeItem(publishedFileId);
		onRRemoteStorageUnsubscribePublishedFileResultCallResult.Set(call, this, &MapNode::OnUnsubscribe);
		unsubscribing = true;
	}
}

void MapNode::ClearPreview()
{
	if (type == MapNode::FILE )
	{
		if (ts_preview != NULL)
		{
			assert(myBrowser != NULL);
			myBrowser->DestroyTileset(ts_preview);
			ts_preview = NULL;
			previewTex = NULL;
			if (chooseRect != NULL)
			{
				chooseRect->SetImage(NULL, 0);
			}
		}
		
		checkingForPreview = false; //even if you receive a preview after this you dont want it.
		
	}
}

void MapNode::TryUpdate()
{
	if (!isWorkshop || mapUpdating || !IsSubscribed() )
		return;

	uint32 itemState = SteamUGC()->GetItemState(publishedFileId);

	bool installed = itemState & k_EItemStateInstalled && itemState & k_EItemStateSubscribed;
	bool needsUpdate = itemState & k_EItemStateNeedsUpdate;

	downloadResult = -1;

	if (needsUpdate && !mapUpdating)
	{
		bool result = SteamUGC()->DownloadItem(publishedFileId, true);
		if (result)
		{
			mapUpdating = true;
			cout << "starting download update" << endl;
			//started download
		}
		else
		{
			cout << "update cannot be downloaded.." << endl;
		}
	}
	/*else if (needsUpdate && mapUpdating)
	{
		uint64 bytesDownloaded;
		uint64 bytesTotal;
		bool infoAvailable = SteamUGC()->GetItemDownloadInfo(publishedFileId, &bytesDownloaded, &bytesTotal);
		if (infoAvailable)
		{
			cout << "downloaded: " << bytesDownloaded << " of " << bytesTotal << "\n";
		}
		else
		{
			cout << "info not available!" << endl;
		}
	}*/
}

void MapNode::OnSubscribe(RemoteStorageSubscribePublishedFileResult_t *callback, bool bIOFailure)
{
	subscribing = false;
	if (callback->m_eResult == k_EResultOK)
	{
		
	}
}

void MapNode::OnUnsubscribe(RemoteStorageUnsubscribePublishedFileResult_t *callback, bool bIOFailure)
{
	unsubscribing = false;
	if (callback->m_eResult == k_EResultOK)
	{
		mapDownloaded = false;
		mapUpdating = false;

		filePath = "";
		folderPath = "";
	}
}

void MapNode::OnHTTPRequestCompleted(HTTPRequestCompleted_t *callback,
	bool bIOFailure)
{
	if (checkingForPreview)
	{
		if (callback->m_bRequestSuccessful)
		{
			uint32 bodySize;
			SteamHTTP()->GetHTTPResponseBodySize(callback->m_hRequest, &bodySize);

			uint8 *buffer = new uint8[bodySize];

			SteamHTTP()->GetHTTPResponseBodyData(callback->m_hRequest, buffer, bodySize);

			assert(previewTex == NULL);

			//cout << "creating texture " << to_string(publishedFileId) << "\n";
			previewTex = new sf::Texture;
			if (!previewTex->loadFromMemory(buffer, bodySize))
			{
				delete previewTex;
				previewTex = NULL;
			}
			else
			{
				assert(myBrowser != NULL);
				ts_preview = myBrowser->GetTileset("WorkshopPreview/" + to_string(publishedFileId), previewTex);
			}

			delete[] buffer;

			if (ts_preview != NULL)
			{
				if (chooseRect != NULL)
				{
					chooseRect->SetImage(ts_preview, 0);
				}
			}
		}
		else
		{
			//cout << "failed to get preview" << endl;
		}

		checkingForPreview = false;
	}
}

void MapNode::RequestDownloadPreview()
{
	checkingForPreview = false;

	if (ts_preview != NULL)
	{
		return;
	}

	if (previewURL == "")
	{
		return;
	}

	HTTPRequestHandle rh = SteamHTTP()->CreateHTTPRequest(
		EHTTPMethod::k_EHTTPMethodGET,
		previewURL.c_str());

	if (rh == INVALID_HTTPREQUEST_HANDLE)
	{
		cout << "http request failed" << endl;
	}
	else
	{
		previewRequestHandle = rh;
		checkingForPreview = true;

		SteamAPICall_t call;
		bool httpResult = SteamHTTP()->SendHTTPRequest(rh, &call);
		if (!httpResult)
		{
			cout << "failed send" << endl;
		}
		else
		{
			OnHTTPRequestCompletedCallResult.Set(call, this,
				&MapNode::OnHTTPRequestCompleted);
			//cout << "send successful" << endl;
		}
	}
}

void MapNode::RequestCreatorInfo()
{
	//not used rn might clean up later
	//SteamFriends()->RequestUserInformation(details.m_ulSteamIDOwner, true);
}

void MapNode::UpdateInstallInfo()
{
	if (!mapDownloaded)
	{
		mapDownloaded = true;
		mapUpdating = false;

		uint64 fileSize;
		char path[1024];
		uint32 timestamp;
		cout << SteamUGC()->GetItemInstallInfo(publishedFileId, &fileSize, path, 1024, &timestamp);

		filePath = string(path) + "\\" + fileName + ".brknk";
		folderPath = path;
	}
}

void MapNode::CheckDownloadResult()
{
	if (mapUpdating)
	{
		if (downloadResult != -1)
		{
			if (downloadResult == k_EResultOK)
			{
				cout << "download success" << endl;
				mapUpdating = false;
				UpdateInstallInfo();
			}
			else if (downloadResult == k_EResultNoConnection)
			{
			cout << "download no connection" << endl;
			}
			else if (downloadResult == k_EResultTimeout)
			{
			cout << "download timeout" << endl;
			}
			else
			{
			cout << "download result: " << downloadResult << endl;
			}
		}
	}
}

bool MapNode::CheckIfFullyInstalled()
{
	if (!isWorkshop)
		return true;

	uint32 itemState = SteamUGC()->GetItemState(publishedFileId);

	bool installed = itemState & k_EItemStateInstalled && itemState & k_EItemStateSubscribed;
	bool needsUpdate = itemState & k_EItemStateNeedsUpdate;
	if ( installed && !needsUpdate )
	{
		UpdateInstallInfo();
		return true;
	}
	//else if (needsUpdate && !mapUpdating )
	//{
	//	cout << "not sure if this update code is needed" << endl;
	//	bool result = SteamUGC()->DownloadItem(publishedFileId, true);
	//	if (result)
	//	{
	//		mapUpdating = true;
	//		cout << "starting download update" << endl;
	//		//started download
	//	}
	//	else
	//	{
	//		cout << "update cannot be downloaded.." << endl;
	//	}
	//}
	//else if (needsUpdate && mapUpdating)
	//{
	//	uint64 bytesDownloaded;
	//	uint64 bytesTotal;
	//	bool infoAvailable = SteamUGC()->GetItemDownloadInfo(publishedFileId, &bytesDownloaded, &bytesTotal);
	//	if (infoAvailable)
	//	{
	//		cout << "downloaded: " << bytesDownloaded << " of " << bytesTotal << "\n";
	//	}
	//	else
	//	{
	//		cout << "info not available!" << endl;
	//	}
	//}

	return false;
}

MapBrowser::MapBrowser(MapBrowserHandler *p_handler,
	int p_cols, int p_rows, int extraImageRects)
	:handler(p_handler)
{
	handler->chooser = this;
	TilesetManager::SetGameResourcesMode(false); //allows tilesets to be outside
												 //of the resources folder
	ext = ".brknk";

	action = A_IDLE;

	workshop = MainMenu::GetInstance()->workshopManager;//new WorkshopManager;
	//workshop->mapBrowser = this;

	float boxSize = 150;
	Vector2f spacing(20, 20);
	Vector2f startRects(10, 60);

	cols = p_cols;
	rows = p_rows;
	totalRects = rows * cols;

	topRow = 0;
	maxTopRow = 0;

	isWorkshop = false;

	

	imageRects = new ImageChooseRect*[totalRects];

	panel = new Panel("mapchooser", 1920,
		1080 - 28, handler, true);



	panel->SetPosition(Vector2i(0, 0));//960 - panel->size.x / 2, 540 - panel->size.y / 2 ));

	panel->ReserveImageRects(totalRects + extraImageRects);
	panel->extraUpdater = this;

	panel->MouseUpdate();


	vector<string> tabNames = { "Local", "Workshop" };
	panel->AddTabGroup("tabs", Vector2i(0, 0), tabNames, 200, 30);

	upButton = panel->AddButton("up", Vector2i(10, 10), Vector2f(30, 30), "up");
	folderPathText = panel->AddLabel("folderpath", Vector2i(50, 10), 30, "");

	Vector2i pageButtonOrigin(750, 990);

	
	
	
	int x, y;
	for (int i = 0; i < totalRects; ++i)
	{
		x = i % cols;
		y = i / cols;
		imageRects[i] = panel->AddImageRect(ChooseRect::ChooseRectIdentity::I_FILESELECT,
			Vector2f(
				startRects.x + x * (boxSize + spacing.x),
				startRects.y + y * (boxSize + spacing.y)),
			NULL, 0, boxSize);
		imageRects[i]->SetShown(true);
		imageRects[i]->Init();
	}

	int boxesTotalWidth = ((boxSize + spacing.x) * cols) - spacing.x;
	int boxesTotalHeight = ((boxSize + spacing.y) * rows) - spacing.y;
	Vector2i belowRects = Vector2i(startRects) + Vector2i(0, boxesTotalHeight);
	belowRects += Vector2i(0, 20);

	Vector2i rightOfRects = Vector2i(startRects) + Vector2i(boxesTotalWidth, 0);
	rightOfRects += Vector2i(20, 0);


	mapScroller = panel->AddScrollBar("scroll", rightOfRects, Vector2i(30, boxesTotalHeight), 4, 4);

	//panel->SetAutoSpacing(true, false, belowRects, Vector2i(30, 0));
	//AddLabel(name + "label", labelStart, 24, labelText);
	fileNameTextBox = panel->AddLabeledTextBox("filename", belowRects, true, 1, 30, 20, 30, "", "Filename:");
	fileNameTextBoxLabel = panel->labels["filenamelabel"];
	
	saveButton = panel->AddButton("save", belowRects + Vector2i(0, 40), Vector2f(60, 30), "Save");
	cancelButton = panel->AddButton("cancel", belowRects + Vector2i(120, 40), Vector2f(80, 30), "Cancel");

	panel->cancelButton = cancelButton;

		//panel->confirmButton =
			//panel->cancelButton =

	panel->StopAutoSpacing();

	openButton = panel->AddButton("open", saveButton->pos + Vector2i(0, 0), Vector2f(60, 30), "Open");
	createLobbyButton = panel->AddButton("createlobby", saveButton->pos + Vector2i(0, 0), Vector2f(100, 30), "Create Lobby");

	belowRects += Vector2i((boxSize + spacing.x) * cols, 0) + Vector2i( -100, 0 );

	prevPageButton = panel->AddButton("prevpage", Vector2i(belowRects), Vector2f(30, 30), "<");
	nextPageButton = panel->AddButton("nextpage", Vector2i(belowRects + Vector2i(45, 0)), Vector2f(30, 30), ">");

	//fileNameTextBox->HideMember();
	//panel->confirmButton->HideMember();
	//panel->cancelButton->HideMember();
}

MapBrowser::~MapBrowser()
{
	ClearNodes();
	delete panel;
	delete[] imageRects;
	//delete workshop;
}

//bool MapBrowser::MouseUpdate()
//{
//	return handler->MouseUpdate();
//}

void MapBrowser::Draw(sf::RenderTarget *target)
{
	handler->Draw(target);
}

void MapBrowser::AddFile(const path &p_filePath)
{
	MapNode *mapNode = new MapNode;
	mapNode->myBrowser = this;
	mapNode->filePath = p_filePath;
	mapNode->type = MapNode::FILE;
	string pathStr = p_filePath.string();
	auto d = pathStr.find(".");
	string middleTest = pathStr.substr(0, d);
	string previewPath = middleTest + ".png";
	mapNode->ts_preview = GetTileset(previewPath);

	mapNode->fileName = mapNode->filePath.filename().stem().string();
	mapNode->nodeName = mapNode->fileName;//mapNode->filePath.filename().stem().string();

	nodes.push_back(mapNode);
}

void MapBrowser::AddFolder(const path &p_filePath)
{
	MapNode *mapNode = new MapNode;
	mapNode->myBrowser = this;
	mapNode->filePath = p_filePath;
	mapNode->type = MapNode::FOLDER;

	string previewPath = "Resources/Menu/foldericon_100x100.png";
	mapNode->ts_preview = GetSizedTileset(previewPath);

	mapNode->fileName = mapNode->filePath.filename().stem().string();
	mapNode->nodeName = mapNode->fileName;//mapNode->filePath.filename().stem().string();

	nodes.push_back(mapNode);
}

void MapBrowser::ClearNodes()
{
	ClearPreviews();
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		delete (*it);
	}
	nodes.clear();
}

void MapBrowser::RequestAllPreviews()
{
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		(*it)->RequestDownloadPreview();
	}
}

const static int STEAM_MAX_ITEMS_RETURNED_IN_QUERY = 50;
void MapBrowser::Update()
{
	switch (action)
	{
	case A_WAITING_FOR_QUERY_RESULTS:
	{
		if (workshop->queryState == WorkshopManager::QS_NOT_QUERYING)
		{
			if (workshop->queryTotalItems > 0)
			{
				maxWorkshopPages = 1 + ((workshop->queryTotalItems - 1) / STEAM_MAX_ITEMS_RETURNED_IN_QUERY);

				if (maxWorkshopPages > 1)
				{
					prevPageButton->ShowMember();
					nextPageButton->ShowMember();
				}
				else
				{
					prevPageButton->HideMember();
					nextPageButton->HideMember();
				}

				//maxWorkshopPages = 1;

				for (auto it = nodes.begin(); it != nodes.end(); ++it)
				{
					(*it)->myBrowser = this;
				}

				//RequestAllPreviews();

				numEntries = nodes.size();

				int numRowsTaken = ceil(((float)numEntries) / cols);
				maxTopRow = numRowsTaken - rows;
				if (maxTopRow < 0)
					maxTopRow = 0;

				action = A_WAITING_FOR_PREVIEW_RESULTS;

			}
			else
			{
				action = A_IDLE;
			}
			//PopulateRects();
		}
		break;
	}
	case A_WAITING_FOR_PREVIEW_RESULTS:
	{
		bool waitingForPreviewResults = false;
		/*for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			if ((*it)->checkingForPreview)
			{
				waitingForPreviewResults = true;
				break;
			}
		}*/

		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			if ((*it)->checkingForCreatorName)
			{
				waitingForPreviewResults = true;
				break;
			}
		}

		if (!waitingForPreviewResults)
		{
			//for (auto it = nodes.begin(); it != nodes.end(); ++it)
			//{
			//	if ((*it)->previewTex != NULL)
			//	{
			//		//using publishedFileId allows for unique items, and not getting confused on duplicate names.
			//		(*it)->ts_preview = GetTileset("WorkshopPreview/" + to_string((*it)->publishedFileId), (*it)->previewTex);
			//	}
			//}

			PopulateRects();

			action = A_IDLE;
		}


		break;
	}
	case A_IDLE:
	{
		//request the previews in order
		int nodeIndex = 0;
		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			if ((*it)->ts_preview == NULL)
			{
				if (!(*it)->checkingForPreview)
				{
					(*it)->RequestDownloadPreview();
				}
				break;
			}
			++nodeIndex;
		}
		break;
	}
	}

	int scroll = UICONTROLLER.ConsumeScroll();
	if (scroll != 0)
	{
		MouseScroll(scroll);
	}


	panel->UpdateSlide(1);
	panel->MouseUpdate();
	panel->UpdateSprites();
}

void MapBrowser::SetRelativePath(const std::string &p_relPath)
{
	string actualPath = current_path().string() + "\\" + p_relPath;
	SetPath(actualPath);
}

void MapBrowser::SetPath(const std::string &p_path)
{
	ClearNodes();

	handler->ClearSelection();

	topRow = 0;

	handler->ChangePath();

	//int maxPathShownLength = 50;

	folderPathText->setString(p_path);

	path p(p_path);


	//get to formatting this later!
	//int panelRight = panel->size.x;

	
	/*if (folderPathText->getGlobalBounds().width + folderPathText->getGlobalBounds().left > panel->size.x)
	{
		int numFolders = 2;
		string *parentNames = new string[numFolders];
		int numFolders = 2;
		
		path p2 = p;
		for (int i = 0; i < numFolders; ++i)
		{
			if (p2.has_parent_path())
			{
				parentNames[i] = p2.parent_path().filename().string();
			}
		}
		
		if (p2.has_parent_path())
		{

		}
		
	}*/

	
	currPath = p;

	assert(exists(p));
	assert(is_directory(p));
	vector<path> v;
	copy(directory_iterator(p), directory_iterator(), back_inserter(v));

	sort(v.begin(), v.end());

	nodes.reserve(v.size());

	for (vector<path>::const_iterator it(v.begin()); it != v.end(); ++it)
	{
		if (is_regular_file((*it)))
		{
			if ((*it).extension().string() == ext)
			{
				AddFile((*it));
			}
		}
		if (is_directory((*it)))
		{
			AddFolder((*it));
		}
	}

	numEntries = nodes.size();

	/*sort(fileNodes.begin(), fileNodes.end(),
	[](FileNode *a, FileNode * b) -> bool
	{
	return a->filePath.stem().string() > b->filePath.stem().string();
	});*/


	//this currently ignores folders since they arent given a space yet.
	int numRowsTaken = ceil(((float)numEntries) / cols);
	maxTopRow = numRowsTaken - rows;
	if (maxTopRow < 0)
		maxTopRow = 0;

	PopulateRects();

	//MOUSE.ResetMouse(); //can load rects and mouse can be clicked down on one of thema lready and its weird.
}

void MapBrowser::ClearPreviews()
{
	//cout << "clear previews" << endl;
	for( auto it = nodes.begin(); it != nodes.end(); ++it )
	{
		(*it)->ClearPreview();
	}
}

void MapBrowser::ClearAllPreviewsButSelected()
{
	if (selectedRect == NULL)
	{
		ClearPreviews();
		return;
	}	

	MapNode *selectedNode = (MapNode*)selectedRect->info;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		if ((*it) != selectedNode)
		{
			(*it)->ClearPreview();
		}
	}
}

void MapBrowser::QueryMaps()
{
	action = A_WAITING_FOR_QUERY_RESULTS;

	ClearNodes();
	for (int i = 0; i < totalRects; ++i)
	{
		imageRects[i]->SetShown(false);
	}

	topRow = 0;

	workshop->Query(&nodes, currWorkshopPage);
}

void MapBrowser::PopulateRects()
{
	Tileset *ts;
	ImageChooseRect *icRect;
	MapNode *node;
	path *folderPath;

	int start = topRow * cols;
	int maxNameCharsShown = 15;

	int defaultNameSize = 18;

	mapScroller->SetRows(ceil(numEntries / (float)cols), rows);
	mapScroller->SetIndex(topRow);

	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		(*it)->chooseRect = NULL;
	}

	int i;
	for (i = start; i < numEntries && i < start + totalRects; ++i)
	{
		icRect = imageRects[i - start];
		node = nodes[i];

		icRect->SetName(node->nodeName);
		icRect->SetNameSize(defaultNameSize);

		bool check = false;
		for (int z = 1; z <= 4; ++z)
		{
			if (icRect->nameText.getGlobalBounds().width > icRect->boxSize.x)
			{
				icRect->SetNameSize(defaultNameSize - z);
				check = true;
			}
			else
			{
				break;
			}
		}

		if (check)
		{
			if (icRect->nameText.getGlobalBounds().width > icRect->boxSize.x)
			{
				string adjustedNodeName = node->nodeName.substr(0, maxNameCharsShown) + "...";
				icRect->SetName(adjustedNodeName);
			}
		}
		
		ts = node->ts_preview;
		icRect->SetInfo(node);
		node->chooseRect = icRect;

		if (ts != NULL)
			icRect->SetImage(ts, ts->GetSubRect(0));
		else
		{
			icRect->SetImage(NULL, 0);
		}

		icRect->SetShown(true);
	}

	for (; i < start + totalRects; ++i)
	{
		icRect = imageRects[i - start];
		icRect->SetShown(false);
	}
}

void MapBrowser::Deactivate()
{
	ClearNodes();
	//ClearTilesets();
}

void MapBrowser::TurnOff()
{
	action = A_CANCELLED;
	//edit->RemoveActivePanel(panel);
}

void MapBrowser::ShowFileNameTextBox()
{
	fileNameTextBox->ShowMember();
	fileNameTextBoxLabel->setString("Filename:");
}

void MapBrowser::HideFileNameTextBox()
{
	fileNameTextBox->HideMember();
	fileNameTextBoxLabel->setString("");
}

void MapBrowser::Init()
{
	//ClearNodes();
	//edit->AddActivePanel(panel);
	if (mode == OPEN)
	{
		openButton->ShowMember();
		saveButton->HideMember();
		createLobbyButton->HideMember();
		
		panel->confirmButton = openButton;
		ShowFileNameTextBox();
		fileNameTextBox->SetString("");
	}
	else if(mode == SAVE )
	{
		openButton->HideMember();
		saveButton->ShowMember();
		createLobbyButton->HideMember();
		panel->confirmButton = saveButton;
		ShowFileNameTextBox();
		fileNameTextBox->SetString("");
	}
	else if (mode == CREATE_CUSTOM_GAME)
	{
		openButton->HideMember();
		saveButton->HideMember();
		createLobbyButton->ShowMember();
		panel->confirmButton = createLobbyButton;
		HideFileNameTextBox();
	}

	
	

	nextPageButton->HideMember();
	prevPageButton->HideMember();

	if (mode == WORKSHOP)
	{
		upButton->HideMember();


		openButton->HideMember();
		saveButton->HideMember();
		createLobbyButton->HideMember();

		HideFileNameTextBox();

		panel->cancelButton = NULL;
		panel->confirmButton = NULL;

		cancelButton->HideMember();
	}
	else
	{
		upButton->ShowMember();
		cancelButton->ShowMember();
	}

	if (panel->confirmButton != NULL)
	{
		panel->cancelButton = cancelButton;
		cancelButton->SetPos(panel->confirmButton->pos + Vector2i(panel->confirmButton->size.x + 30, 0));
	}
	
	if (mode == OPEN)
	{
		
	}
	else if (mode == SAVE)
	{

	}
	else if (mode == CREATE_CUSTOM_GAME)
	{

	}
	else if (mode == WORKSHOP)
	{

	}

	currWorkshopPage = 1;
	selectedRect = NULL;
	action = A_IDLE;
}

void MapBrowser::Start(const std::string &p_ext,
	MapBrowser::Mode p_mode, const std::string &path)
{
	ext = p_ext;
	mode = p_mode;
	Init();

	isWorkshop = false;

	upButton->ShowMember();

	SetPath(path);
}

void MapBrowser::StartRelative(const std::string &p_ext,
	MapBrowser::Mode p_mode, const std::string &path)
{
	ext = p_ext;
	mode = p_mode;
	Init();

	isWorkshop = false;

	upButton->ShowMember();

	SetRelativePath(path);
}

void MapBrowser::StartWorkshop( MapBrowser::Mode p_mode )
{
	mode = p_mode;
	
	isWorkshop = true;

	Init();

	folderPathText->setString("");

	currWorkshopPage = 1;

	upButton->HideMember();

	QueryMaps();
}

void MapBrowser::MouseScroll(int delta)
{
	int oldTopRow = topRow;
	if (delta < 0)
	{
		topRow -= delta;
		if (topRow > maxTopRow)
			topRow = maxTopRow;
	}
	else if (delta > 0)
	{
		topRow -= delta;
		if (topRow < 0)
			topRow = 0;
	}

	if (topRow != oldTopRow)
	{
		PopulateRects();
	}
}

MapBrowserHandler::MapBrowserHandler(int cols, int rows, bool p_showPreview, int extraImageRects)
{
	chooser = new MapBrowser(this, cols, rows, extraImageRects);

	focusedRect = NULL;

	showPreview = p_showPreview;

	if (!showPreview)
	{
		chooser->panel->SetSize(Vector2f(850, 840));
	}

	ts_largePreview = NULL;

	Vector2f previewTopLeft = Vector2f(1000, 540 - 492 / 2);

	Vector2f previewSize(912, 492);

	SetRectSubRect(largePreview, FloatRect(0, 0, previewSize.x, previewSize.y));
	SetRectTopLeft(largePreview, previewSize.x, previewSize.y, previewTopLeft);

	SetRectSubRect(noPreviewQuad, FloatRect(0, 0, previewSize.x, previewSize.y));
	SetRectTopLeft(noPreviewQuad, previewSize.x, previewSize.y, previewTopLeft);
	SetRectColor(noPreviewQuad, Color::Black);

	

	MainMenu *mm = MainMenu::GetInstance();

	Vector2f previewBotLeft = previewTopLeft + Vector2f(0, previewSize.y);

	descriptionText.setFont(mm->arial);
	descriptionText.setCharacterSize(20);
	descriptionText.setPosition(previewBotLeft + Vector2f(0, 30));
	descriptionText.setFillColor(Color::Red);

	fullNameText.setFont(mm->arial);
	fullNameText.setCharacterSize(40);
	fullNameText.setPosition(previewTopLeft + Vector2f(0, -50));
	fullNameText.setFillColor(Color::Black);

	creatorLink = chooser->panel->AddHyperLink("creatorlink", Vector2i(previewTopLeft + Vector2f(0, -100)), 40, "", "");

	if (!showPreview)
	{
		creatorLink->HideMember();
	}
}

MapBrowserHandler::~MapBrowserHandler()
{
	delete chooser;
}

void MapBrowserHandler::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	MapNode *node = (MapNode*)cr->info;
	if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED
		|| eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)//left released so that it catches the mouse when first loading
	{
		if (node->type == MapNode::FILE)
		{
			FocusFile(cr);
		}
	}
	else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
	{
		if (node->type == MapNode::FILE)
		{
			if (focusedRect == cr)
			{
				ClearFocus();
			}
		}
	}
	else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
	{
		if (node->type == MapNode::FOLDER)
		{
			chooser->SetPath(node->filePath.string());
		}
		else if (node->type == MapNode::FILE)
		{
			//FocusFile(cr);
			ClickFile(cr);
		}
	}
}

void MapBrowserHandler::ButtonCallback(Button *b, const std::string & e)
{
	if (b == chooser->cancelButton )
	{
		Cancel();
	}
	else if (b == chooser->saveButton || b == chooser->openButton || b == chooser->createLobbyButton)
	{
		Confirm();
	}
	else if (b == chooser->prevPageButton)
	{
		chooser->currWorkshopPage--;
		if (chooser->currWorkshopPage < 1)
		{
			chooser->currWorkshopPage = 1;
		}
		else
		{
			ts_largePreview = NULL;
			chooser->QueryMaps();
		}
	}
	else if (b == chooser->nextPageButton)
	{
		chooser->currWorkshopPage++;
		if (chooser->currWorkshopPage > chooser->maxWorkshopPages)
		{
			chooser->currWorkshopPage = chooser->maxWorkshopPages;
		}
		else
		{
			ts_largePreview = NULL;
			chooser->QueryMaps();
		}
	}
	else if (b == chooser->upButton)
	{
		chooser->SetPath(chooser->currPath.parent_path().string());
	}
}

void MapBrowserHandler::TabGroupCallback(TabGroup *tg, const std::string &e)
{
	//need to revisit the whole Create_Custom_Game thing eventually
	if (tg->currTabIndex == 0)
	{
		chooser->StartRelative(".brknk", MapBrowser::CREATE_CUSTOM_GAME, "Resources\\Maps");
	}
	else if (tg->currTabIndex == 1)
	{
		chooser->StartWorkshop(MapBrowser::CREATE_CUSTOM_GAME);
	}
}

void MapBrowserHandler::ScrollBarCallback(ScrollBar *sb, const std::string &e)
{
	chooser->topRow = sb->currIndex;
	chooser->PopulateRects();
}

void MapBrowserHandler::CancelCallback(Panel *p)
{
	Cancel();
}

//return true if installed. sets the filepath if its installation is just being registered.
bool MapBrowserHandler::CheckIfSelectedItemInstalled()
{
	MapNode *selectedNode = (MapNode*)chooser->selectedRect->info;

	if (selectedNode == NULL)
		return false;

	if (!chooser->isWorkshop)
		return true;

	return selectedNode->CheckIfFullyInstalled();
}

void MapBrowserHandler::SubscribeToItem()
{
	MapNode *selectedNode = (MapNode*)chooser->selectedRect->info;

	selectedNode->Subscribe();
}

void MapBrowserHandler::UnsubscribeFromItem()
{
	MapNode *selectedNode = (MapNode*)chooser->selectedRect->info;

	selectedNode->Unsubscribe();
}

void MapBrowserHandler::ChangePath()
{
	ts_largePreview = NULL;
	//chooser->ClearTilesets();
}

void MapBrowserHandler::ClearSelection()
{
	chooser->selectedRect = NULL;

	for (int i = 0; i < chooser->totalRects; ++i)
	{
		chooser->imageRects[i]->Deselect();
	}	
}

void MapBrowserHandler::SelectRect(ChooseRect *cr)
{
	ClearSelection();
	cr->Select();
	chooser->selectedRect = cr->GetAsImageChooseRect();
}

void MapBrowserHandler::Update()
{
	chooser->Update();

	if (focusedRect != NULL && ts_largePreview == NULL)
	{	
		MapNode *mn = (MapNode*)focusedRect->info;
		if (mn->type == MapNode::FILE)
		{
			if (mn->ts_preview != NULL)
			{
				ts_largePreview = mn->ts_preview;
			}
		}
	}
}

void MapBrowserHandler::Cancel()
{
	chooser->TurnOff();
}

void MapBrowserHandler::Confirm()
{
	string fileName = chooser->fileNameTextBox->GetString();
	if (fileName != "")
	{
		if (chooser->mode == MapBrowser::OPEN)
		{
			//chooser->edit->ChooseFileOpen(chooser, fileName);
		}
		else if (chooser->mode == MapBrowser::SAVE)
		{
			//chooser->edit->ChooseFileSave(chooser, fileName);
		}
		//chooser->action = MapBrowser::A_CONFIRMED; //hopefully this doesnt add any weird bugs
		//chooser->TurnOff();
	}

	
	chooser->TurnOff();

	chooser->action = MapBrowser::A_CONFIRMED; //hopefully this doesnt add any weird bugs
}

void MapBrowserHandler::ClickFile(ChooseRect *cr)
{
	string fileName = cr->nameText.getString().toAnsiString();

	MapNode *mn = (MapNode*)cr->info;

	MainMenu *mm = MainMenu::GetInstance();

	SelectRect(cr);

	if (chooser->mode == MapBrowser::OPEN)
	{
		bool found = false;
		for (int i = 0; i < chooser->nodes.size(); ++i)
		{
			if (fileName == chooser->nodes[i]->filePath.stem().string())
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			//chooser->edit->ChooseFileOpen(chooser, fileName);
			//chooser->TurnOff();
		}
	}
	else if (chooser->mode == MapBrowser::SAVE)
	{
		chooser->fileNameTextBox->SetString(mn->fileName);
		//chooser->edit->ChooseFileSave(chooser, fileName);
		//chooser->TurnOff();
	}
	else if (chooser->mode == MapBrowser::WORKSHOP)
	{
		chooser->selectedRect = cr->GetAsImageChooseRect();	
	}
	else if (chooser->mode == MapBrowser::CREATE_CUSTOM_GAME)
	{
		chooser->selectedRect = cr->GetAsImageChooseRect();
	}
}

void MapBrowserHandler::FocusFile(ChooseRect *cr)
{
	if (!showPreview)
		return;
	
	
	//ts_largePreview = cr->GetAsImageChooseRect()->ts;
	
	MapNode *mn = (MapNode*)cr->info;

	focusedRect = cr->GetAsImageChooseRect();

	ts_largePreview = mn->ts_preview;//cr->GetAsImageChooseRect()->ts;

	if ( !mn->isWorkshop && !mn->nameAndDescriptionUpdated)
	{
		ifstream is;
		is.open(mn->filePath.string());

		if (is.is_open())
		{
			MapHeader mh;
			mh.Load(is);

			if (mh.fullName == "----")
			{
				//need to figure out how to get the workshop name here.
				mn->fullMapName = mn->nodeName;
			}
			else
			{
				mn->fullMapName = mh.fullName;
			}

			mn->description = mh.description;

			is.close();

			mn->nameAndDescriptionUpdated = true;

			mn->creatorId = mh.creatorID;
			mn->creatorName = mh.creatorName;
		}
		else
		{

		}
	}

	fullNameText.setString(mn->fullMapName);
	descriptionText.setString(mn->description);

	if (mn->isWorkshop)
	{
		//mapLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(uploadID));
		creatorLink->SetLinkURL("https://steamcommunity.com/profiles/" + to_string(mn->creatorId));
		creatorLink->SetString(mn->creatorName);
		creatorLink->ShowMember();
	}
	else
	{
		creatorLink->HideMember();
	}
}

void MapBrowserHandler::ClearFocus()
{
	ts_largePreview = NULL;
	focusedRect = NULL;
	descriptionText.setString("");
	fullNameText.setString("");
	creatorLink->HideMember();
}

void MapBrowserHandler::Draw(sf::RenderTarget *target)
{
	//chooser->panel->Draw(target);
	if (ts_largePreview)
	{
		target->draw(largePreview, 4, sf::Quads, ts_largePreview->texture);
	}
	else
	{
		target->draw(noPreviewQuad, 4, sf::Quads);
	}

	target->draw(descriptionText);
	target->draw(fullNameText);
}