#include "LevelSelector.h"
#include <iostream>
#include <SFML/Network.hpp>
#include "MainMenu.h"
#include <iomanip>
#include <time.h>
#include "CustomMapClient.h"

using namespace boost::filesystem;
using namespace std;
using namespace sf;

int TreeNode::GetLevel()
{
	int level = 0;
	TreeNode *iter = parent;
	while( iter != NULL )
	{
		++level;
		iter = iter->parent;
	}
	return level;
}

TreeNode::TreeNode()
{
	expanded = false;
	parent = NULL;
	next = NULL;
}

string TreeNode::GetLocalPath()
{
	TreeNode *rent = this;
	string full; //filePath.filename().string();
	while( rent != NULL )
	{
		full = rent->name + full;//"/" + full;
		rent = rent->parent;
	}
	//cout << "returning: " << full << endl;
	return full;
}

LevelSelector::LevelSelector( MainMenu *p_mainMenu )
{
	customMapClient = NULL;

	//customMapClient = new CustomMapClient;
	//customMapClient->AnonymousInit();
	//customMapClient->AttemptUserLogin("test", "Shephard123~");

	mainMenu = p_mainMenu;
	previewSpr.setPosition(500, 500);
	width = 400;
	height = 1080;
	
	ts_previewNotFound = mainMenu->tilesetManager.GetTileset("Maps/notfound.png", 912, 492);
	drawPanel.create( width, height );
	drawPanel.clear();
	entries = NULL;
	numTotalEntries = 0;
	font = mainMenu->arial;
	text = NULL;
	mouseOverIndex = -1;
	position = Vector2f( 0, 0 );
	selectedIndex = 0;
	fontHeight = 24;
	xspacing = 40;
	yspacing = 40;
	mouseDownIndex = -1;
	localPaths = NULL;
	fullPaths = NULL;
	mouseDown = false;
	dirNode = NULL;
	viewOffset = 0;
	//entries = new TreeNode;
	//entries->name = "Maps";
	//entries->next = NULL;
	///entries->parent = NULL;
}

LevelSelector::~LevelSelector()
{
	if (customMapClient != NULL)
		delete customMapClient;
	//entries
	//text
	//localPaths
	//dirNode
	ClearEntries();
}

void LevelSelector::UpdateMapList()
{
	ClearEntries();

	/*sf::Ftp ftp;
	Ftp::Response re = ftp.connect("ftp.breakneckgame.com", 21, sf::seconds( 5 ) );
	if( re.isOk() )
	{
		ftp.login("test@breakneckgame.com", "password123");
		sf::Ftp::DirectoryResponse response = ftp.getWorkingDirectory();
		if (response.isOk())
		cout << "Current directory: " << response.getDirectory() << endl;
		
		sf::Ftp::ListingResponse res = ftp.getDirectoryListing();
		if (res.isOk())
		{
			const std::vector<std::string>& listing = res.getListing();
			for (std::vector<std::string>::const_iterator it = listing.begin(); it != listing.end(); ++it)
			{
				ftp.download( (*it),"Maps/Downloads", sf::Ftp::Ascii); 
			}
		}
		//ftp.download("map online.brknk", "Maps/Downloads", sf::Ftp::Ascii);

		ftp.disconnect();
	}
	else
	{
		cout << "COULDNT CONNECT WITH SERVER" << endl;
	}*/
	//sf::Http &http = levelServer.http;
	//customMapClient->AttempGetMapListFromServer();
	/*for (auto it = customMapClient->mapEntries.begin(); it != customMapClient->mapEntries.end(); ++it)
	{
		customMapClient->AttemptDownloadMapFromServer("Resources/Maps/DownloadedMaps/", (*it));
	}*/

	//std::string path = "Maps/";
	//std::string file = "gateblank9.brknk";
	//bool goodDownload = levelServer.DownloadFile( path, file );

   
	UpdateMapList( entries, "Resources/Maps" );

	text = new Text[numTotalEntries];

	localPaths = new string[numTotalEntries];
	fullPaths = new string[numTotalEntries];

	dirNode = new TreeNode*[numTotalEntries];

	


	Tex( 0, 0, entries );
}

void LevelSelector::MouseUpdate( sf::Vector2f mousePos )
{

	//Vector2f realPos( mousePos.x / windowStretch.x, mousePos.y / windowStretch.y );

	Vector2f adjPos( mousePos.x - position.x, mousePos.y - position.y );
	if( adjPos.x >= 0 && adjPos.x <= width && adjPos.y >= 0 && adjPos.y < yspacing * numTotalEntries )
	{
		mouseOverIndex = (int)adjPos.y / yspacing + viewOffset;
		//cout << "selectedIndex: " << selectedIndex << endl;
	}
	else
	{
		mouseOverIndex = -1;
	}
}

void LevelSelector::GetPreview( const std::string &pName, const std::string &mName, bool update)
{
	string fileName;
	if (pName == "Maps")
	{
		fileName = "Maps/" + mName + ".png";
	}
	else
	{
		fileName = "Maps/" + pName + "/" + mName + ".png";
	}
	
	if (update)
	{
		previewTS[mName] = mainMenu->tilesetManager.GetUpdatedTileset(fileName, 912, 492);
	}
	else
	{
		previewTS[mName] = mainMenu->tilesetManager.GetTileset(fileName, 912, 492);
	}
	
}

void LevelSelector::LeftClick( bool click, sf::Vector2f mousePos )
{
	if( click )
	{
		if( !mouseDown )
		{
			mouseDown = true;
			//this needs to be different at different resolutions!!
			//mouseDownPos = Vector2f( mousePos.x / windowStretch.x, mousePos.y / windowStretch.y );
			//Vector2f realPos( mousePos.x / windowStretch.x, mousePos.y / windowStretch.y );

			Vector2f adjPos( mousePos.x - position.x, mousePos.y - position.y );
			if( adjPos.x >= 0 && adjPos.x <= width && adjPos.y >= 0 && adjPos.y < yspacing * numTotalEntries )
			{
				mouseDownIndex = (int)adjPos.y / yspacing + viewOffset;
				//cout << "selectedIndex: " << selectedIndex << endl;
			}
			else
			{
				mouseDownIndex = -1;
			}
		}
	}
	else
	{
		if( mouseDown )
		{
			//Vector2f realPos( mousePos.x / windowStretch.x, mousePos.y / windowStretch.y );

			Vector2f adjPos( mousePos.x - position.x, mousePos.y - position.y );
			if( adjPos.x >= 0 && adjPos.x <= width && adjPos.y >= 0 && adjPos.y < yspacing * numTotalEntries )
			{
				int testIndex = (int)adjPos.y / yspacing + viewOffset;
				if( mouseDownIndex == testIndex )
				{
					selectedIndex = testIndex;
					//cout << "selected index: " << selectedIndex << endl;
					string indexText = text[selectedIndex].getString().toAnsiString();
					//string localPathTest = localPaths[selectedIndex];
					string fullPathTest = fullPaths[selectedIndex];
					Tileset *&currTS = previewTS[indexText];

					if (text[selectedIndex].getFillColor() == Color::Red)
					{
						TreeNode *entry = GetEntryByName(fullPathTest);
						entry->expanded = !entry->expanded;
						for (int i = 0; i < numTotalEntries; ++i)
						{
							text[i].setString("");
						}
						//Text &t0 = text[index];
						Tex(0, 0, entries);

						UpdateSelectedPreview();
					}
					else if( currTS == NULL )
					{
						UpdateSelectedPreview();
					}
					
					//needs cleanup later
					if( currTS != NULL )
						previewSpr.setTexture(*currTS->texture);
					else
					{
						previewSpr.setTexture(*ts_previewNotFound->texture);
					}
				}
				//cout << "selectedIndex: " << selectedIndex << endl;
			}
			else
			{
				//cout << "negative!" << endl;
				//selectedIndex = -1;
			}

			mouseDown = false;
			//selectedIndex = -1;
		}
	}
}

const std::string &LevelSelector::GetSelectedPath()
{
	return fullPaths[selectedIndex];
}

void LevelSelector::UpdateSelectedPreview()
{
	string indexText = text[selectedIndex].getString().toAnsiString();
	string test = localPaths[selectedIndex];

	Tileset *&currTS = previewTS[indexText];

	if ( text[selectedIndex].getFillColor() != Color::Red)
	{
		if (currTS == NULL)
		{
			GetPreview(localPaths[selectedIndex], indexText, false);
		}
		else
		{
			GetPreview(localPaths[selectedIndex], indexText, true);
		}
		//currTS = previewTS[indexText];
	}
	

	if (currTS != NULL)
		previewSpr.setTexture(*currTS->texture);
	else
	{
		currTS = ts_previewNotFound;
		previewSpr.setTexture(*ts_previewNotFound->texture);
	}
}

TreeNode *LevelSelector::GetEntryByName(const std::string &dirName)
{
	return entryMap[dirName];
}

void LevelSelector::SetLocalPath(int index, TreeNode *entry)
{
	//TreeNode *par = entry->parent;
	if (entry->name != "Maps/" )
	{
		localPaths[index] = entry->name;
	}
	else
	{
		localPaths[index] = "";
	}
}

int LevelSelector::Tex(int index, int level, TreeNode *entry)
{
	int innerLevel = level + 1;
	if (entry->name == "Maps/")
	{
		innerLevel = level;
	}

	Text &t0 = text[index];
	t0.setFont(font);
	t0.setCharacterSize(fontHeight);
	t0.setString(entry->name);
	t0.setFillColor(Color::Red);
	t0.setPosition(level * xspacing, index * yspacing);
	fullPaths[index] = string("Resources/") + entry->GetLocalPath();

	SetLocalPath(index, entry);
	
	dirNode[index] = NULL;
	++index; //1 for me

	if (entry->expanded)
	{
		for (list<TreeNode*>::iterator it = entry->dirs.begin(); it != entry->dirs.end(); ++it)
		{
			index = Tex(index, innerLevel, (*it));	//this does itself
		}
	}
	

	if (entry->expanded)
	{
		for (list<path>::iterator it = entry->files.begin(); it != entry->files.end(); ++it)
		{
			Text &t = text[index];
			t.setFont(font);
			t.setCharacterSize(fontHeight);

			string name = (*it).filename().string();
			name = name.substr(0, name.size() - 6);


			t.setString(name);
			t.setFillColor(Color::White);
			t.setPosition(innerLevel * xspacing, index * yspacing);
			fullPaths[index] = string("Resources/") + (entry->GetLocalPath() / (*it).filename()).string();
			SetLocalPath(index, entry);
			
			dirNode[index] = entry;

			++index; //1 for each file
		}
	}

	return index;
}

void LevelSelector::ClearEntries()
{
	selectedIndex = 0;
	mouseOverIndex = -1;
	numTotalEntries = 0;
	if( entries != NULL )
	{
		ClearEntries( entries );
		entries = NULL;
	}
	if( text != NULL )
	{
		delete [] text;
	}

	if( localPaths != NULL )
	{
		delete [] localPaths;
	}

	if (fullPaths != NULL)
	{
		delete[] fullPaths;
	}

	if (dirNode != NULL)
	{
		/*for (int i = 0; i < numTotalEntries; ++i)
		{
			dirNode[i]->
		}*/
		delete[] dirNode;
	}
		
}

void LevelSelector::ClearEntries(TreeNode *n)
{
	entryMap.clear();
	for( list<TreeNode*>::iterator it = n->dirs.begin(); it != n->dirs.end(); ++it )
	{
		ClearEntries( (*it) );
	}

	previewTS.clear();
	
	delete n;
}

void LevelSelector::UpdateMapList( TreeNode *parentNode, const std::string &relativePath )
{
	path p( current_path() / relativePath );
	//string pFile =  + (*it).relative_path().stem().string() + );
	vector<path> v;
	try
	{
		//previewTS.clear();
		if (exists(p))    // does p actually exist?
		{
			if (is_regular_file(p))        // is p a regular file?   
			{
				if( p.extension().string() == ".brknk" )
				{
					//string name = p.filename().string();
					parentNode->files.push_back( p );//name.substr( 0, name.size() - 6 ) );
					numTotalEntries++;

					string pathFolder = p.parent_path().stem().string();
					string relPath = p.relative_path().string();
					string mapName = pathFolder + "/" + p.relative_path().stem().string();
					previewTS[p.relative_path().stem().string()] = NULL;

					//this is for loading and resaving. dont remove it. just turn it
					//back on when I need to use the functionality.

					//string mapPath = "Resources/Maps/" + pathFolder + "/" + p.filename().string();
					//allMapPaths.push_back(mapPath);
				}
			}
			else if (is_directory(p))      // is p a directory?
			{
				TreeNode *newDir = new TreeNode;
				newDir->parent = parentNode;
				newDir->next = NULL;
				newDir->name = p.filename().string() + "/";
				newDir->filePath = p;

				assert(entryMap.count(newDir->name) == 0);
				string queryTest = string("Resources/") + newDir->GetLocalPath();
				
				entryMap[queryTest] = newDir;
				//entryMap[newDir->name] = newDir;

				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				sort(v.begin(), v.end());             // sort, since directory iteration
														// is not ordered on some file systems

				if( parentNode == NULL )
				{
					entries = newDir;
					newDir->expanded = true;
				}
				else
				{
					parentNode->dirs.push_back( newDir );
				}
				numTotalEntries++;
			
				
				for (vector<path>::const_iterator it (v.begin()); it != v.end(); ++it)
				{
					UpdateMapList( newDir, relativePath + "/" + (*it).filename().string() );
					//this writes out the name of every map
					//cout << "   " << *it << '\n';
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

void LevelSelector::Draw( RenderTarget *target )
{
	int totalShown = 27;

	drawPanel.clear(sf::Color::Black);
	View v;
	v.setCenter(drawPanel.getSize().x / 2, drawPanel.getSize().y / 2 + yspacing * viewOffset);
	v.setSize(drawPanel.getSize().x, drawPanel.getSize().y);
	drawPanel.setView(v);
//	View v;
//	)
	///drawPanel.setView( )

	int tSelected = selectedIndex;// -viewOffset;
	if(tSelected >= 0 )
	{
		sf::RectangleShape rs;
		rs.setFillColor( Color::Blue );
		rs.setSize( Vector2f( width, yspacing ) );
		rs.setPosition( position.x, position.y + yspacing * tSelected);
		drawPanel.draw( rs );
	}
	
	if( mouseOverIndex >= 0 && mouseOverIndex != tSelected )
	{
		sf::RectangleShape rs;
		rs.setFillColor( Color::Magenta );
		rs.setSize( Vector2f( width, yspacing ) );
		rs.setPosition( position.x, position.y + yspacing * mouseOverIndex );
		drawPanel.draw( rs );
	}

	
	for( int i = 0; i < numTotalEntries; ++i )
	{
		if (i < viewOffset || i >= numTotalEntries || i > viewOffset + totalShown )
		{
			continue;
		}
		drawPanel.draw( text[i] );
	}

	/*if( mouseDown )
	{
		CircleShape cs;
		cs.setRadius( 5 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds(). height / 2 );
		cs.setPosition( mouseDownPos.x, mouseDownPos.y );
		cs.setFillColor( Color::Yellow );
		drawPanel.draw( cs );
	}*/

	drawPanel.display();
	sf::Sprite dSprite;
	dSprite.setTexture( drawPanel.getTexture() );
	target->draw( dSprite );

	target->draw(previewSpr);
}

void LevelSelector::Print()
{
	PrintDir( entries );
}

void LevelSelector::PrintDir( TreeNode * dir )
{
	cout << "directory: " << dir->name << endl;
	cout << "containing files: " << endl;
	for( list<path>::iterator it = dir->files.begin(); it != dir->files.end(); ++it )
	{
		cout << "-- " << (*it) << endl;
	}

	cout << "containing dirs: " << endl;
	for( list<TreeNode*>::iterator it = dir->dirs.begin(); it != dir->dirs.end(); ++it )
	{
		cout << "+ " << (*it)->name << endl;
		PrintDir( (*it) );
	}
}

void LevelSelector::ChangeViewOffset(int delta)
{
	viewOffset += delta;
	if (viewOffset < 0)
		viewOffset = 0;
	else if (viewOffset + 27 > numTotalEntries)
	{
		viewOffset = numTotalEntries - 27;
	}
}

void LevelSelector::BackupAllMapsAndPreviews()
{
	time_t t = time(NULL);
	tm *timeInfo = localtime(&t);
	stringstream ss;
	ss << "Resources/Maps_Backup_";

	int year = timeInfo->tm_year + 1900;
	int mon = timeInfo->tm_mon + 1;
	ss << year << "-" << mon << "-" << timeInfo->tm_mday
		<< "_" << timeInfo->tm_hour << "-" << timeInfo->tm_min << "-" << timeInfo->tm_sec;

	string destStr = ss.str();

	copyDirectoryRecursively("Resources/Maps", destStr);
}

void LevelSelector::LoadAndRewriteAllMaps()
{
	BackupAllMapsAndPreviews();

	for (auto it = allMapPaths.begin(); it != allMapPaths.end(); ++it)
	{
		cout << (*it) << endl;
		mainMenu->LoadAndResaveMap((*it));
	}
}


namespace fs = boost::filesystem;
void copyDirectoryRecursively(const fs::path& sourceDir, const fs::path& destinationDir)
{
	if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir))
	{
		throw std::runtime_error("Source directory " + sourceDir.string() + " does not exist or is not a directory");
	}
	if (fs::exists(destinationDir))
	{
		throw std::runtime_error("Destination directory " + destinationDir.string() + " already exists");
	}
	if (!fs::create_directory(destinationDir))
	{
		throw std::runtime_error("Cannot create destination directory " + destinationDir.string());
	}

	for (const auto& dirEnt : fs::recursive_directory_iterator{ sourceDir })
	{
		const auto& path = dirEnt.path();
		auto relativePathStr = path.string();
		boost::replace_first(relativePathStr, sourceDir.string(), "");
		fs::copy(path, destinationDir / relativePathStr);
	}
}