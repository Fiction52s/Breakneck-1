#include "FileChooser.h"
#include "VectorMath.h"
#include <iostream>
#include "EditSession.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

void FileNode::Draw(sf::RenderTarget *target)
{
	//target->draw(previewSpr, 4, sf::Quads, ts_preview->texture);
}



FolderNode::~FolderNode()
{
	for (auto it = fileNodes.begin(); it != fileNodes.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = childFolders.begin(); it != childFolders.end(); ++it)
	{
		delete (*it);
	}
}

void FolderNode::Draw(sf::RenderTarget *target)
{
	for (auto it = fileNodes.begin(); it != fileNodes.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void FolderNode::AddFile(const path &p_filePath)
{
	FileNode *fileNode = new FileNode;
	fileNode->filePath = p_filePath;
	//SetRectColor(fileNode->previewSpr, Color::Red);
	fileNodes.push_back(fileNode);
}

void FolderNode::AddFolder(FolderNode *fn)
{
	childFolders.push_back(fn);
}

std::string FolderNode::GetRelPath()
{
	if (parentFolder == NULL)
	{
		return "";
	}

	return parentFolder->GetRelPath() + "\\" + folderName + "\\";
}

void FolderNode::DebugPrint( int indent )
{
	for (int i = 0; i < indent; ++i)
	{
		cout << "\t";
	}
	cout << "folder: " << folderName << "\n";

	for (auto it = fileNodes.begin(); it != fileNodes.end(); ++it)
	{
		for (int i = 0; i < indent + 1; ++i)
		{
			cout << "\t";
		}

		cout << "file: " << (*it)->filePath.filename().string() << "\n";
	}

	for (auto it = childFolders.begin(); it != childFolders.end(); ++it)
	{
		(*it)->DebugPrint(indent + 1);
	}
}

FolderTree::FolderTree(const std::string &p_path, const std::string &p_ext)
	:ext( p_ext )
{
	treePath = p_path;
	treeRoot = NULL;

	SetupTree();
}

FolderTree::~FolderTree()
{
	delete treeRoot;
}

void FolderTree::SetupTree()
{
	SetupEntry(NULL, "");
}

void FolderTree::SetupEntry( FolderNode *fn, const std::string &relPath )
{
	string fnPath = "";
	if (fn != NULL)
		fnPath = fn->GetRelPath();
	string fullRelPath = fnPath + relPath;
	path p(treePath / fullRelPath);
	vector<path> v;
	try
	{
		if (exists(p))
		{
			if (is_regular_file(p))
			{
				if (p.extension().string() == ext)
				{
					//cout << "adding map file: " << p.filename().string() << endl;
					fn->AddFile( p );
				}
			}
			else if (is_directory(p))      // is p a directory?
			{
				FolderNode *newFolder = new FolderNode;
				newFolder->parentFolder = fn;
				if (fn == NULL)
				{
					treeRoot = newFolder;
				}
				else
				{
					newFolder->folderName = p.filename().string();
					fn->AddFolder(newFolder);
				}

				

				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				sort(v.begin(), v.end());             // sort, since directory iteration
													  // is not ordered on some file systems

				//cout << "setting up new folder: " << p.filename().string() << endl;
				for (vector<path>::const_iterator it(v.begin()); it != v.end(); ++it)
				{
					SetupEntry(newFolder, (*it).filename().string());
				}
			}
			else
				cout << p << " exists, but is neither a regular file nor a directory\n";
		}
		else
			cout << p << " does not exist\n";
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
	}
}

void FolderTree::DebugPrintTree()
{
	treeRoot->DebugPrint(0);
}

FileChooser::FileChooser()
{
	TilesetManager::SetGameResourcesMode(false); //allows tilesets to be outside
	//of the resources folder
	ext = ".brknk";

	edit = EditSession::GetSession();

	float boxSize = 150;
	Vector2f spacing(60, 60);
	Vector2f startRects(10, 100);

	cols = 4;
	rows = 4;
	totalRects = rows * cols;

	SetRectSubRect(largePreview, FloatRect(0, 0, 912, 492));
	
	topRow = 0;
	maxTopRow = 0;

	imageRects = new ImageChooseRect*[totalRects];

	panel = new Panel("filechooser", 1920,
		1080 - 28, this, true);

	panel->SetPosition(Vector2i(0, 28));//960 - panel->size.x / 2, 540 - panel->size.y / 2 ));

	panel->ReserveImageRects(totalRects);
	panel->extraUpdater = this;
	
	ts_largePreview = NULL;
	
	upButton = panel->AddButton("up", Vector2i(10, 10), Vector2f(30, 30), "up");
	folderPathText = panel->AddLabel("folderpath", Vector2i(50, 10), 30, "");

	int x, y;
	for (int i = 0; i < totalRects; ++i)
	{
		x = i % cols;
		y = i / cols;
		imageRects[i] = panel->AddImageRect(ChooseRect::ChooseRectIdentity::I_FILESELECT,
			Vector2f(
				startRects.x + x * (boxSize + spacing.x ), 
				startRects.y +  y * (boxSize + spacing.y )), 
			NULL, 0, boxSize);
		imageRects[i]->SetShown(true);
		imageRects[i]->Init();
	}

	SetRectTopLeft(largePreview, 912, 492, Vector2f(1000, 540 - 492 / 2));

	panel->SetAutoSpacing(true, false, Vector2i(10, 960), Vector2i(30, 0));
	fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");

	//panel->AddButton("open", Vector2i(0, 0), Vector2f(60, 30), "Open");
	panel->confirmButton = 
		panel->AddButton("save", Vector2i(0, 0), Vector2f(60, 30), "Save");
	panel->cancelButton = 
		panel->AddButton("cancel", Vector2i(0, 0), Vector2f(80, 30), "Cancel");
}

FileChooser::~FileChooser()
{
	ClearNodes();
	delete panel;
	delete[] imageRects;
}

bool FileChooser::MouseUpdate()
{
	return true;
}

void FileChooser::Draw(sf::RenderTarget *target)
{
	if (ts_largePreview)
	{
		target->draw(largePreview, 4, sf::Quads, ts_largePreview->texture);
	}
}

void FileChooser::AddFile(const path &p_filePath)
{
	FileNode *fileNode = new FileNode;
	fileNode->filePath = p_filePath;
	fileNode->type = FileNode::FILE;

	string pathStr = p_filePath.string();
	auto d = pathStr.find(".");
	string middleTest = pathStr.substr(0, d);
	string previewPath = middleTest + ".png";
	fileNode->ts_preview = GetTileset(previewPath);

	nodes.push_back(fileNode);
}

void FileChooser::AddFolder(const path &p_filePath)
{
	FileNode *folderNode = new FileNode;
	folderNode->filePath = p_filePath;
	folderNode->type = FileNode::FOLDER;

	string previewPath = "Resources/Menu/foldericon_100x100.png";
	folderNode->ts_preview = GetSizedTileset(previewPath);

	nodes.push_back(folderNode);
}

void FileChooser::ClearNodes()
{
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		delete (*it);
	}
	nodes.clear();
}

void FileChooser::SetRelativePath(const std::string &p_relPath)
{
	string actualPath = current_path().string() + "/" + p_relPath;
	SetPath(actualPath);
}

void FileChooser::SetPath(const std::string &p_path)
{
	ClearNodes();
	ClearTilesets();

	topRow = 0;

	ts_largePreview = NULL;

	folderPathText->setString(p_path);

	path p(p_path);
	currPath = p;

	assert(exists(p));
	assert(is_directory(p));
	vector<path> v;
	copy(directory_iterator(p), directory_iterator(), back_inserter(v));

	sort(v.begin(), v.end() );

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
		if (is_directory((*it)) )
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

void FileChooser::PopulateRects()
{
	Tileset *ts;
	ImageChooseRect *icRect;
	FileNode *node;
	path *folderPath;

	int start = topRow * cols;

	int i;
	for ( i = start; i < numEntries && i < start + totalRects; ++i )
	{
		icRect = imageRects[i - start];
		node = nodes[i];

		icRect->SetName(node->filePath.filename().stem().string());
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

void FileChooser::Print()
{
	/*cout << "folders: " << "\n";
	for (auto it = childFolders.begin(); it != childFolders.end(); ++it)
	{
		cout << "\t" << (*it).filename() << "\n";
	}

	cout << "files: " << "\n";
	for (auto it = fileNodes.begin(); it != fileNodes.end(); ++it)
	{
		cout << "\t" << (*it)->filePath.filename().string() << "\n";
	}*/
}

void FileChooser::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	FileNode *node = (FileNode*)cr->info;
	if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
	{
		if (node->type == FileNode::FILE)
		{
			ts_largePreview = cr->GetAsImageChooseRect()->ts;
		}
	}
	else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
	{
		ts_largePreview = NULL;
	}
	else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
	{
		if (node->type == FileNode::FOLDER)
		{
			SetPath(node->filePath.string());
		}
		else if (node->type == FileNode::FILE)
		{
			string fileName = cr->nameText.getString().toAnsiString();
			if (fMode == OPEN)
			{
				bool found = false;
				for (int i = 0; i < nodes.size(); ++i)
				{
					if (fileName == nodes[i]->filePath.stem().string())
					{
						found = true;
						break;
					}
				}

				if (found)
				{
					edit->ChooseFileOpen(this, fileName);
					TurnOff();
				}
			}
			else if (fMode == SAVE)
			{
				edit->ChooseFileSave(this, fileName);
				TurnOff();
			}
		}
	}
}

void FileChooser::ButtonCallback(Button *b, const std::string & e)
{
	if (b == b->panel->cancelButton)
	{
		TurnOff();
	}
	else if (b == b->panel->confirmButton)
	{
		string fileName = fileNameTextBox->GetString();
		if (fileName != "")
		{
			if (fMode == OPEN)
			{
				edit->ChooseFileOpen(this, fileName);
			}
			else if (fMode == SAVE)
			{
				edit->ChooseFileSave(this, fileName);
			}
			TurnOff();
		}
	}
	else if (b == upButton)
	{
		SetPath(currPath.parent_path().string());
	}
}

void FileChooser::Deactivate()
{
	ClearNodes();
	ClearTilesets();
}

void FileChooser::TurnOff()
{
	edit->RemoveActivePanel(panel);
}

void FileChooser::Init()
{
	edit->AddActivePanel(panel);
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

void FileChooser::Start(const std::string &p_ext,
	FileChooser::Mode p_fMode, const std::string &path)
{
	ext = p_ext;
	fMode = p_fMode;
	Init();
	
	//SetRelativePath("Resources/Maps/W2");

	SetPath(path);
}

void FileChooser::StartRelative(const std::string &p_ext,
	FileChooser::Mode p_fMode, const std::string &path)
{
	ext = p_ext;
	fMode = p_fMode;
	Init();

	SetRelativePath(path);
}

void FileChooser::MouseScroll(int delta)
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