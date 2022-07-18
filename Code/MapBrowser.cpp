#include "MapBrowser.h"
#include "VectorMath.h"
#include <iostream>
#include "EditSession.h"
#include "WorkshopManager.h"
//#include "steam/steam_api.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

MapNode::MapNode()
{
	checkingForPreview = false;
	ts_preview = NULL;
	type = FILE;
	index = -1;
	previewTex = NULL;
	action = IDLE;
	chooseRect = NULL;
	mapDownloaded = false;
}

void MapNode::Draw(sf::RenderTarget *target)
{
	//target->draw(previewSpr, 4, sf::Quads, ts_preview->texture);
}

MapNode::~MapNode()
{
}

void MapNode::OnHTTPRequestCompleted(HTTPRequestCompleted_t *callback,
	bool bIOFailure)
{
	if (callback->m_bRequestSuccessful)
	{
		uint32 bodySize;
		SteamHTTP()->GetHTTPResponseBodySize(callback->m_hRequest, &bodySize);

		uint8 *buffer = new uint8[bodySize];

		SteamHTTP()->GetHTTPResponseBodyData(callback->m_hRequest, buffer, bodySize);

		previewTex = new sf::Texture;
		previewTex->loadFromMemory(buffer, bodySize);

		checkingForPreview = false;
		cout << "got preview successfully" << endl;

		delete[] buffer;
	}
	else
	{
		cout << "failed to get preview" << endl;
	}

	checkingForPreview = false;
}


void MapNode::RequestDownloadPreview()
{
	checkingForPreview = false;
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
			cout << "send successful" << endl;
		}
	}
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

	workshop = new WorkshopManager;
	//workshop->mapBrowser = this;

	float boxSize = 150;
	Vector2f spacing(60, 60);
	Vector2f startRects(10, 100);

	cols = p_cols;
	rows = p_rows;
	totalRects = rows * cols;

	topRow = 0;
	maxTopRow = 0;

	

	imageRects = new ImageChooseRect*[totalRects];

	panel = new Panel("mapchooser", 1920,
		1080 - 28, handler, true);

	panel->SetPosition(Vector2i(0, 28));//960 - panel->size.x / 2, 540 - panel->size.y / 2 ));

	panel->ReserveImageRects(totalRects + extraImageRects);
	panel->extraUpdater = this;

	panel->MouseUpdate();

	upButton = panel->AddButton("up", Vector2i(10, 10), Vector2f(30, 30), "up");
	playButton = panel->AddButton("play", Vector2i(100, 10), Vector2f(30, 30), "play");
	folderPathText = panel->AddLabel("folderpath", Vector2i(50, 10), 30, "");

	Vector2i pageButtonOrigin(750, 990);

	prevPageButton = panel->AddButton("prevpage", Vector2i(pageButtonOrigin), Vector2f(30, 30), "<");
	nextPageButton = panel->AddButton("nextpage", Vector2i(pageButtonOrigin + Vector2i(60, 0)), Vector2f(30, 30), ">");
	
	

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



	panel->SetAutoSpacing(true, false, Vector2i(10, 960), Vector2i(30, 0));
	//fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");



	panel->confirmButton =
		panel->AddButton("confirm", Vector2i(0, 0), Vector2f(60, 30), "confirm");
	panel->cancelButton =
		panel->AddButton("cancel", Vector2i(0, 0), Vector2f(80, 30), "Cancel");

	panel->StopAutoSpacing();

	//fileNameTextBox->HideMember();
	//panel->confirmButton->HideMember();
	//panel->cancelButton->HideMember();
}

MapBrowser::~MapBrowser()
{
	ClearNodes();
	delete panel;
	delete[] imageRects;
	delete workshop;
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
	mapNode->filePath = p_filePath;
	mapNode->type = MapNode::FILE;
	string pathStr = p_filePath.string();
	auto d = pathStr.find(".");
	string middleTest = pathStr.substr(0, d);
	string previewPath = middleTest + ".png";
	mapNode->ts_preview = GetTileset(previewPath);

	mapNode->nodeName = mapNode->filePath.filename().stem().string();

	nodes.push_back(mapNode);
}

void MapBrowser::AddFolder(const path &p_filePath)
{
	MapNode *mapNode = new MapNode;
	mapNode->filePath = p_filePath;
	mapNode->type = MapNode::FOLDER;

	string previewPath = "Resources/Menu/foldericon_100x100.png";
	mapNode->ts_preview = GetSizedTileset(previewPath);

	mapNode->nodeName = mapNode->filePath.filename().stem().string();

	nodes.push_back(mapNode);
}

void MapBrowser::ClearNodes()
{
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		if ( (*it)->type == MapNode::FILE && (*it)->ts_preview != NULL)
		{
			DestroyTileset((*it)->ts_preview);
		}
		delete (*it);
	}
	nodes.clear();
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
				maxWorkshopPages = 1 + ((workshop->queryTotalItems-1) / STEAM_MAX_ITEMS_RETURNED_IN_QUERY);

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
					(*it)->RequestDownloadPreview();
				}

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
		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			if ((*it)->checkingForPreview)
			{
				waitingForPreviewResults = true;
				break;
			}
		}

		if (!waitingForPreviewResults)
		{
			for (auto it = nodes.begin(); it != nodes.end(); ++it)
			{
				if ((*it)->previewTex != NULL)
				{
					(*it)->ts_preview = GetTileset("WorkshopPreview/" + (*it)->nodeName, (*it)->previewTex);
				}
			}

			PopulateRects();

			action = A_IDLE;
		}


		break;
	}
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

	folderPathText->setString(p_path);

	path p(p_path);
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

	int i;
	for (i = start; i < numEntries && i < start + totalRects; ++i)
	{
		icRect = imageRects[i - start];
		node = nodes[i];

		icRect->SetName(node->nodeName);
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
	//edit->RemoveActivePanel(panel);
}

void MapBrowser::Init()
{
	//ClearNodes();
	//edit->AddActivePanel(panel);
	if (mode == OPEN)
	{
		panel->confirmButton->text.setString("Open");

		/*fileNameTextBox->SetString("");
		fileNameTextBox->focused = true;
		fileNameTextBox->SetCursorIndex(0);
		panel->SetFocusedMember(fileNameTextBox);*/
	}
	else if(mode == SAVE )
	{
		panel->confirmButton->text.setString("Save");

		/*fileNameTextBox->SetString("");
		fileNameTextBox->focused = true;
		fileNameTextBox->SetCursorIndex(0);
		panel->SetFocusedMember(fileNameTextBox);*/
	}
	else if (mode == CREATE_CUSTOM_GAME)
	{
		panel->confirmButton->text.setString("Create");

		/*fileNameTextBox->SetString("");
		fileNameTextBox->focused = true;
		fileNameTextBox->SetCursorIndex(0);
		panel->SetFocusedMember(fileNameTextBox);*/
	}
	else
	{
		panel->confirmButton->text.setString("getridof");
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
}

void MapBrowser::Start(const std::string &p_ext,
	MapBrowser::Mode p_mode, const std::string &path)
{
	ext = p_ext;
	mode = p_mode;
	Init();

	upButton->ShowMember();

	SetPath(path);
}

void MapBrowser::StartRelative(const std::string &p_ext,
	MapBrowser::Mode p_mode, const std::string &path)
{
	ext = p_ext;
	mode = p_mode;
	Init();

	upButton->ShowMember();

	SetRelativePath(path);
}

void MapBrowser::StartWorkshop()
{
	mode = WORKSHOP;

	Init();

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

MapBrowserHandler::MapBrowserHandler(int cols, int rows, int extraImageRects)
{
	chooser = new MapBrowser(this, cols, rows, extraImageRects);

	ts_largePreview = NULL;

	Vector2f previewTopLeft = Vector2f(1000, 540 - 492 / 2);

	SetRectSubRect(largePreview, FloatRect(0, 0, 912, 492));
	SetRectTopLeft(largePreview, 912, 492, previewTopLeft);

	MainMenu *mm = MainMenu::GetInstance();

	Vector2f previewBotLeft = previewTopLeft + Vector2f(0, 492);

	descriptionText.setFont(mm->arial);
	descriptionText.setCharacterSize(20);
	descriptionText.setPosition(previewBotLeft + Vector2f(0, 30));
	descriptionText.setFillColor(Color::Red);
}

MapBrowserHandler::~MapBrowserHandler()
{
	delete chooser;
}

void MapBrowserHandler::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	MapNode *node = (MapNode*)cr->info;
	if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
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
			UnfocusFile(cr);
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
			ClickFile(cr);
		}
	}
}

void MapBrowserHandler::ButtonCallback(Button *b, const std::string & e)
{
	if (b == b->panel->cancelButton)
	{
		Cancel();
	}
	else if (b == b->panel->confirmButton)
	{
		Confirm();
	}
	else if (b == chooser->playButton)
	{
		if (chooser->selectedRect != NULL)
		{
			MainMenu *mm = MainMenu::GetInstance();

			MapNode *selectedNode = (MapNode*)chooser->selectedRect->info;

			if (chooser->mode == MapBrowser::WORKSHOP)
			{
				if (CheckIfSelectedItemInstalled())
				{
					mm->RunFreePlayMap(selectedNode->filePath.string());
				}
				else
				{
					uint32 itemState = SteamUGC()->GetItemState(selectedNode->publishedFileId);
					if (!(itemState & k_EItemStateSubscribed))
					{
						cout << "subbing to item" << endl;
						SteamUGC()->SubscribeItem(selectedNode->publishedFileId);
						cout << "map download started" << endl;
					}
					/*else if (itemState &k_EItemStateDownloading)
					{
					cout << "downloading map" << endl;

					}
					else if (itemState &k_EItemStateDownloadPending)
					{
					cout << "map downoad pending" << endl;
					}*/

					mm->DownloadAndRunWorkshopMap();
				}
			}
			else
			{
				mm->RunFreePlayMap(selectedNode->filePath.string());
			}
			
		}
		//chooser->SetPath(chooser->currPath.parent_path().string());
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
			chooser->QueryMaps();
		}
	}
	else if (b == chooser->upButton)
	{
		
		chooser->SetPath(chooser->currPath.parent_path().string());
	}
}

//return true if installed. sets the filepath if its installation is just being registered.
bool MapBrowserHandler::CheckIfSelectedItemInstalled()
{
	MapNode *selectedNode = (MapNode*)chooser->selectedRect->info;

	if (selectedNode == NULL)
		return false;

	uint32 itemState = SteamUGC()->GetItemState(selectedNode->publishedFileId);

	if (itemState & k_EItemStateInstalled)
	{
		if (!selectedNode->mapDownloaded)
		{
			selectedNode->mapDownloaded = true;

			uint64 fileSize;
			char path[1024];
			uint32 timestamp;
			cout << SteamUGC()->GetItemInstallInfo(selectedNode->publishedFileId, &fileSize, path, 1024, &timestamp);

			selectedNode->filePath = string(path) + "\\" + selectedNode->nodeName + ".brknk";
		}
		return true;
	}

	return false;
}

void MapBrowserHandler::ChangePath()
{
	ts_largePreview = NULL;
	chooser->ClearTilesets();
}

void MapBrowserHandler::ClearSelection()
{
	for (int i = 0; i < chooser->totalRects; ++i)
	{
		chooser->imageRects[i]->Deselect();
	}
}

void MapBrowserHandler::SelectRect(ChooseRect *cr)
{
	ClearSelection();
	cr->Select();
}

void MapBrowserHandler::Update()
{
	chooser->Update();
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
		chooser->TurnOff();
	}
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
	ts_largePreview = cr->GetAsImageChooseRect()->ts;

	MapNode *mn = (MapNode*)cr->info;

	descriptionText.setString(mn->description);
}

void MapBrowserHandler::UnfocusFile(ChooseRect *cr)
{
	ts_largePreview = NULL;

	descriptionText.setString("");
}

void MapBrowserHandler::Draw(sf::RenderTarget *target)
{
	//chooser->panel->Draw(target);
	if (ts_largePreview)
	{
		target->draw(largePreview, 4, sf::Quads, ts_largePreview->texture);

		target->draw(descriptionText);
	}
}

MapOptionsPopup::MapOptionsPopup()
{
	panel = new Panel("mapoptions", 300,
		300, this, true);

	panel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(30, 0));
	//fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");



	panel->confirmButton =
		panel->AddButton("ok", Vector2i(0, 0), Vector2f(60, 30), "Host");
	panel->cancelButton =
		panel->AddButton("cancel", Vector2i(0, 0), Vector2f(80, 30), "Cancel");

	panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	action = A_INACTIVE;

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
}

void MapOptionsPopup::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "ok")
	{
		action = A_CONFIRMED;
	}
	else if (b->name == "cancel")
	{
		action = A_CANCELLED;
	}
}

void MapOptionsPopup::Activate()
{
	action = A_ACTIVE;
}