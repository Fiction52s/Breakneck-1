#include "LevelSelector.h"
#include <iostream>
#include <SFML/Network.hpp>
#include "MainMenu.h"

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

string TreeNode::GetLocalPath()
{
	TreeNode *rent = this;
	string full; //filePath.filename().string();
	while( rent != NULL )
	{
		full = rent->name + "/" + full;
		rent = rent->parent;
	}
	//cout << "returning: " << full << endl;
	return full;
}

LevelSelector::LevelSelector( MainMenu *mainMenu )
{
	width = 400;
	height = 1080;
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
	mouseDown = false;
	dirNode = NULL;
	viewOffset = 0;
	//entries = new TreeNode;
	//entries->name = "Maps";
	//entries->next = NULL;
	///entries->parent = NULL;
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
	
	
	std::string path = "test/";
	std::string file = "map_online.brknk";
 //  bool goodDownload = levelServer.DownloadFile( path, file );

   
	UpdateMapList( entries, "Resources/Maps" );

	text = new Text[numTotalEntries];

	localPaths = new string[numTotalEntries];

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
					cout << "selected index: " << selectedIndex << endl;
				}
				//cout << "selectedIndex: " << selectedIndex << endl;
			}
			else
			{
				cout << "negative!" << endl;
				//selectedIndex = -1;
			}

			mouseDown = false;
			//selectedIndex = -1;
		}
	}
}

const std::string &LevelSelector::GetSelectedPath()
{
	return localPaths[selectedIndex];
}


int LevelSelector::Tex(int index, int level, TreeNode *entry)
{
	Text &t0 = text[index];
	t0.setFont( font );
	t0.setCharacterSize( fontHeight );
	t0.setString( entry->name );
	t0.setFillColor( Color::Red );
	t0.setPosition( level * xspacing, index * yspacing );
	localPaths[index] = string( "Resources/" ) + entry->GetLocalPath();//entry->filePath;
	dirNode[index] = NULL;
	++index; //1 for me
	for( list<TreeNode*>::iterator it = entry->dirs.begin(); it != entry->dirs.end(); ++it )
	{
		index = Tex( index, level + 1, (*it) );	//this does itself
	}

	for( list<path>::iterator it = entry->files.begin(); it != entry->files.end(); ++it )
	{
		Text &t = text[index];
		t.setFont( font );
		t.setCharacterSize( fontHeight );

		string name = (*it).filename().string();
		name = name.substr( 0, name.size() - 6 );


		t.setString( name );
		t.setFillColor( Color::Blue );
		t.setPosition( (level + 1) * xspacing, index * yspacing );
		localPaths[index] = string("Resources/") + (entry->GetLocalPath() / (*it).filename()).string();
		dirNode[index] = entry;

		++index; //1 for each file
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

	if( dirNode != NULL  )
		delete [] dirNode;
}

void LevelSelector::ClearEntries(TreeNode *n)
{
	for( list<TreeNode*>::iterator it = n->dirs.begin(); it != n->dirs.end(); ++it )
	{
		ClearEntries( (*it) );
	}

	delete n;
}

void LevelSelector::UpdateMapList( TreeNode *parentNode, const std::string &relativePath )
{
	path p( current_path() / relativePath );
	
	vector<path> v;
	try
	{
		if (exists(p))    // does p actually exist?
		{
			if (is_regular_file(p))        // is p a regular file?   
			{
				if( p.extension().string() == ".brknk" )
				{
					//string name = p.filename().string();
					parentNode->files.push_back( p );//name.substr( 0, name.size() - 6 ) );
					numTotalEntries++;
				}
			}
			else if (is_directory(p))      // is p a directory?
			{
				//cout << p << " is a directory containing:\n";

				TreeNode *newDir = new TreeNode;
				newDir->parent = parentNode;
				newDir->next = NULL;
				newDir->name = p.filename().string();
				newDir->filePath = p;

				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				sort(v.begin(), v.end());             // sort, since directory iteration
														// is not ordered on some file systems

				if( parentNode == NULL )
				{
					entries = newDir;
				}
				else
				{
					parentNode->dirs.push_back( newDir );
				}
				numTotalEntries++;
			
				
				for (vector<path>::const_iterator it (v.begin()); it != v.end(); ++it)
				{
					UpdateMapList( newDir, relativePath + "/" + (*it).filename().string() );
					cout << "   " << *it << '\n';
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

	drawPanel.clear(sf::Color::Green);
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