#include "MapBrowser.h"
#include "VectorMath.h"
#include <iostream>
#include "EditSession.h"
#include "WorkshopManager.h"
//#include "steam/steam_api.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

void MapNode::Draw(sf::RenderTarget *target)
{
	//target->draw(previewSpr, 4, sf::Quads, ts_preview->texture);
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
	folderPathText = panel->AddLabel("folderpath", Vector2i(50, 10), 30, "");

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
	fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");

	panel->confirmButton =
		panel->AddButton("save", Vector2i(0, 0), Vector2f(60, 30), "Save");
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

bool MapBrowser::MouseUpdate()
{
	return handler->MouseUpdate();
}

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

}

void MapBrowser::AddFolder(const path &p_filePath)
{
	MapNode *mapNode = new MapNode;
	mapNode->filePath = p_filePath;
	mapNode->type = MapNode::FOLDER;

	string previewPath = "Resources/Menu/foldericon_100x100.png";
	mapNode->ts_preview = GetSizedTileset(previewPath);

	nodes.push_back(mapNode);
}

void MapBrowser::ClearNodes()
{
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		delete (*it);
	}
	nodes.clear();
}

void MapBrowser::Update()
{
	switch (action)
	{
	case A_WAITING_FOR_RESULTS:
	{
		if (workshop->queryState == WorkshopManager::QS_NOT_QUERYING)
		{
			action = A_IDLE;

			for (auto it = nodes.begin(); it != nodes.end(); ++it)
			{
				//string previewPath = (*it)->filePath.string() + ".png";
				//(*it)->ts_preview = GetTileset(previewPath);
			}

			numEntries = nodes.size();

			int numRowsTaken = ceil(((float)numEntries) / cols);
			maxTopRow = numRowsTaken - rows;
			if (maxTopRow < 0)
				maxTopRow = 0;

			PopulateRects();
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

void MapBrowser::SetToWorkshop()
{
	fMode = WORKSHOP;
	ClearNodes();

	action = A_WAITING_FOR_RESULTS;
	workshop->Query(&nodes);
	//PopulateRects();
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

		icRect->SetName(node->mapName);
		ts = node->ts_preview;
		icRect->SetInfo(node);

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
	//edit->AddActivePanel(panel);
	if (fMode == OPEN)
	{
		panel->confirmButton->text.setString("Open");
	}
	else
	{
		panel->confirmButton->text.setString("Save");
	}
	fileNameTextBox->SetString("");
	fileNameTextBox->focused = true;
	fileNameTextBox->SetCursorIndex(0);
	panel->SetFocusedMember(fileNameTextBox);
}

void MapBrowser::Start(const std::string &p_ext,
	MapBrowser::Mode p_fMode, const std::string &path)
{
	ext = p_ext;
	fMode = p_fMode;
	Init();

	SetPath(path);
}

void MapBrowser::StartRelative(const std::string &p_ext,
	MapBrowser::Mode p_fMode, const std::string &path)
{
	ext = p_ext;
	fMode = p_fMode;
	Init();

	SetRelativePath(path);
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

void MapBrowser::LateDraw(sf::RenderTarget *target)
{
	handler->LateDraw(target);
}

MapBrowserHandler::MapBrowserHandler(int cols, int rows, int extraImageRects)
{
	chooser = new MapBrowser(this, cols, rows, extraImageRects);
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
	else if (b == chooser->upButton)
	{
		chooser->SetPath(chooser->currPath.parent_path().string());
	}
}

DefaultMapBrowserHandler::DefaultMapBrowserHandler()
	:MapBrowserHandler(4, 4)
{
	ts_largePreview = NULL;
	SetRectSubRect(largePreview, FloatRect(0, 0, 912, 492));
	SetRectTopLeft(largePreview, 912, 492, Vector2f(1000, 540 - 492 / 2));
}

void DefaultMapBrowserHandler::ChangePath()
{
	ts_largePreview = NULL;
	chooser->ClearTilesets();
}

void DefaultMapBrowserHandler::Update()
{
	chooser->Update();
}

void DefaultMapBrowserHandler::Cancel()
{
	chooser->TurnOff();
}

void DefaultMapBrowserHandler::Confirm()
{
	string fileName = chooser->fileNameTextBox->GetString();
	if (fileName != "")
	{
		if (chooser->fMode == MapBrowser::OPEN)
		{
			//chooser->edit->ChooseFileOpen(chooser, fileName);
		}
		else if (chooser->fMode == MapBrowser::SAVE)
		{
			//chooser->edit->ChooseFileSave(chooser, fileName);
		}
		chooser->TurnOff();
	}
}

void DefaultMapBrowserHandler::ClickFile(ChooseRect *cr)
{
	string fileName = cr->nameText.getString().toAnsiString();
	if (chooser->fMode == MapBrowser::OPEN)
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
			chooser->TurnOff();
		}
	}
	else if (chooser->fMode == MapBrowser::SAVE)
	{
		//chooser->edit->ChooseFileSave(chooser, fileName);
		chooser->TurnOff();
	}
}

void DefaultMapBrowserHandler::FocusFile(ChooseRect *cr)
{
	ts_largePreview = cr->GetAsImageChooseRect()->ts;
}

void DefaultMapBrowserHandler::UnfocusFile(ChooseRect *cr)
{
	ts_largePreview = NULL;
}

void DefaultMapBrowserHandler::Draw(sf::RenderTarget *target)
{
	//chooser->panel->Draw(target);
	if (ts_largePreview)
	{
		target->draw(largePreview, 4, sf::Quads, ts_largePreview->texture);
	}
}