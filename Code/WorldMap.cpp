#include "WorldMap.h"

#include "GameSession.h"
#include <sstream>
#include <iostream>
#include "MainMenu.h"

using namespace boost::filesystem;
using namespace sf;
using namespace std;

WorldMap::WorldMap( MainMenu *mainMenu )
	:font( mainMenu->arial )
{
	ts_planetAndSpace = mainMenu->tilesetManager.GetTileset( "WorldMap/map_z1.jpg", 1920, 1080 );
	//planetAndSpaceTex = new Texture;
	//planetAndSpaceTex->loadFromFile( "WorldMap/map_z1.jpg" );
	
	ts_planet = mainMenu->tilesetManager.GetTileset( "WorldMap/map_z2.png", 1920, 1080 );
	//planetTex = new Texture;
	//planetTex->loadFromFile( "WorldMap/map_z2.png" );

	ts_colonySelect = mainMenu->tilesetManager.GetTileset( "WorldMap/map_select_512x512.png"
		, 512, 512 );

	colonySelectSprite.setTexture( *ts_colonySelect->texture );
	

	for( int i = 0; i < 6; ++i )
	{
		stringstream ss;
		ss << "WorldMap/map_z3_" << (i+1) << ".png";
		//sectionTex[i] = new Texture;//owner->GetTileset( ss.str(), 1920, 1080 );
		//sectionTex[i]->loadFromFile( ss.str() );
		ts_section[i] = mainMenu->tilesetManager.GetTileset( ss.str(), 1920, 1080 );

		ss.clear();
		ss.str( "" );
		ss << "WorldMap/map_w" << (i+1) << ".png";
		//colonyTex[i] = new Texture;
		//colonyTex[i]->loadFromFile( ss.str() );
		ts_colony[i] = mainMenu->tilesetManager.GetTileset( ss.str(), 1920, 1080 );
	}

	back.setPosition( 0, 0 );
	//back.setOrigin( 0, 0 );
	front.setPosition( 0, 0 );
	//front.setOrigin( 1920 / 2, 1080 / 2 );
	//selectedColony = 1;

	fontHeight = 24;
	menuPos = Vector2f( 300, 300 );
	yspacing = 40;
	entries = NULL;
	text = NULL;
	dirNode = NULL;
	localPaths = NULL;
	leftBorder = 20;
	//numTotalEntries = 0;


	Reset( NULL );

	//UpdateColonySelect();
}

void WorldMap::UpdateColonySelect()
{
	colonySelectSprite.setTextureRect( ts_colonySelect->GetSubRect( selectedColony ) );
	colonySelectSprite.setOrigin( colonySelectSprite.getLocalBounds().width / 2,
		colonySelectSprite.getLocalBounds().height / 2 );
	switch( selectedColony )
	{
	case 0:
		{
			colonySelectSprite.setPosition( 1260, 335 );
			break;
		}
	case 1:
		{
			colonySelectSprite.setPosition( 1245, 720 );
			break;
		}
	case 2:
		{
			colonySelectSprite.setPosition( 980, 890 );
			break;
		}
	case 3:
		{
			colonySelectSprite.setPosition( 670, 690 );
			break;
		}
	case 4:
		{
			colonySelectSprite.setPosition( 680, 350 );
			break;
		}
	case 5:
		{
			colonySelectSprite.setPosition( 970, 170 );
			break;
		}
	}
}

void WorldMap::Reset( SaveFile *sf )
{
	fontHeight = 24;
	state = OFF;
	frame = 0;

	if( sf != NULL )
	{
		selectedColony = 1;
		selectedLevel = 0;
	}
	else
	{
		selectedColony = 1;
		selectedLevel = 0;
	}
	
	ClearEntries();
	moveDown = false;
	moveUp = false;

	UpdateColonySelect();
}

const std::string & WorldMap::GetSelected()
{
	return localPaths[selectedLevel];
}

WorldMap::~WorldMap()
{
	//delete planetAndSpaceTex;
	//delete planetTex;



	//for( int i = 0; i < 6; ++i )
	//{
		//delete sectionTex[i];
		//delete colonyTex[i];
	//}

	delete [] text;
	delete [] localPaths;
}

void WorldMap::ClearEntries()
{
	numTotalEntries = 0;
	selectedLevel = 0;
	//mouseOverIndex = -1;
	numTotalEntries = 0;
	if( entries != NULL )
	{
		delete entries;
		//ClearEntries( entries );
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

int WorldMap::Tex( int index, int level, TreeNode *entry )
{
	//Text &t0 = text[index];
	//t0.setFont( font );
	//t0.setCharacterSize( fontHeight );
	//t0.setString( entry->name );
	//t0.setColor( Color::Red );
	//t0.setPosition( menuPos.x, menuPos.y + index * yspacing );
	//localPaths[index] = entry->GetLocalPath();//entry->filePath;
	//dirNode[index] = NULL;
	//++index; //1 for me
	//for( list<TreeNode*>::iterator it = entry->dirs.begin(); it != entry->dirs.end(); ++it )
	//{
	//	index = Tex( index, level + 1, (*it) );	//this does itself
	//}

	bgRect.setSize( Vector2f( 300, yspacing * numTotalEntries ) );
	Color c( 0, 50, 0 );
	bgRect.setFillColor( c );
	bgRect.setPosition( menuPos );

	
	
	
	

	for( list<path>::iterator it = entry->files.begin(); it != entry->files.end(); ++it )
	{
		Text &t = text[index];
		t.setFont( font );
		t.setCharacterSize( fontHeight );

		string name = (*it).filename().string();
		name = name.substr( 0, name.size() - 6 );


		t.setString( name );
		t.setFillColor( Color::White );
		t.setPosition( menuPos.x + leftBorder, menuPos.y + index * yspacing );
		localPaths[index] = ( entry->GetLocalPath() / (*it).filename()).string();
		dirNode[index] = entry;

		++index; //1 for each file
	}

	return index;
}

void WorldMap::UpdateMapList( TreeNode *parentNode, const std::string &relativePath )
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
			else if (is_directory(p) )      // is p a directory?
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
				//numTotalEntries++;
			
				
				for (vector<path>::const_iterator it (v.begin()); it != v.end(); ++it)
				{
					UpdateMapList( newDir, relativePath + "/" + (*it).filename().string() );
					cout << "   " << *it << '\n';
				}
			}
			else
			{
				cout << p << " exists, but is neither a regular file nor a directory\n";
				assert( false );
			}
		}
		else
			cout << p << " does not exist\n";
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
	}
}

void WorldMap::UpdateMapList()
{
	ClearEntries();
	
	//std::string path = "test/";
	//std::string file = "map_online.brknk";
 //  bool goodDownload = levelServer.DownloadFile( path, file );
	stringstream ss;
	ss << "Maps/W" << (selectedColony+1);
	cout << "stuff: " << ss.str() << endl;
	UpdateMapList( entries, ss.str() );

	text = new Text[numTotalEntries];

	localPaths = new string[numTotalEntries];

	dirNode = new TreeNode*[numTotalEntries];

	


	Tex( 0, 0, entries );
}

bool WorldMap::Update( ControllerState &prevInput, ControllerState &currInput )
{
	
	if( state == OFF )
		return false;


	int trans = 20;
	switch( state )
	{
	case PLANET_AND_SPACE:
		{
			if( frame == trans )
			{
				state = PLANET_TRANSITION;
				frame = 0;
			}
			//cout << "currInput.A: " << currInput.A << ", prevInput.A: " << prevInput.A << endl;
		}
		//frame = 0;
		/*if( frame == trans )
		{
			state = PLANET_TRANSITION;
			frame = 0;
		}*/
		//frame = 0;
		break;
	case PLANET_TRANSITION:
		if( frame == trans )
		{
			state = PLANET;
			frame = 0;
		}
		break;
	case PLANET:
		/*if( frame == trans )
		{
			state = SECTION_TRANSITION;
			frame = 0;
		}*/
		if( currInput.A && !prevInput.A )
		{
			state = COLONY_TRANSITION;//SECTION_TRANSITION;
			frame = 0;
			break;
		}
		else if( currInput.B && !prevInput.B )
		{
			state = OFF;
			frame = 0;
			break;
		}
			
		if( (currInput.LDown() || currInput.PDown()) && !moveDown )
		{
			selectedColony++;
			//currentMenuSelect++;
			if( selectedColony > 5 )
				selectedColony = 0;
			moveDown = true;
			UpdateColonySelect();
		}
		else if( ( currInput.LUp() || currInput.PUp() ) && !moveUp )
		{
			selectedColony--;
			if( selectedColony < 0 )
				selectedColony = 5;
			moveUp = true;
			UpdateColonySelect();
		}

		if( !(currInput.LDown() || currInput.PDown()) )
		{
			moveDown = false;
		}
		if( ! ( currInput.LUp() || currInput.PUp() ) )
		{
			moveUp = false;
		}
		break;
	case SECTION_TRANSITION:
		if( frame == trans )
		{
			state = SECTION;
			frame = 0;

		}
		break;
	case SECTION:
		if( frame == trans )
		{
			state = COLONY_TRANSITION;
			frame = 0;
		}
		break;
	case COLONY_TRANSITION:
		if( frame == trans )
		{
			state = COLONY;
			frame = 0;
			
		}
		break;
	case COLONY:
		if( currInput.A && !prevInput.A )
		{
			state = OFF;
			frame = 0;
			return false;
		}

		frame = 0;
		break;
	}


	switch( state )
	{
	case PLANET_AND_SPACE:
		{
			if( frame == 0 )
			{
				//back.setTexture( *planetAndSpaceTex );
				back.setTexture( *ts_planetAndSpace->texture );
				back.setColor( Color( 255, 255, 255, 255 ) );
			}
			break;
		}
	case PLANET_TRANSITION:
		{
			if( frame == 0 )
			{
				//front.setTexture( *planetTex );
				front.setTexture( *ts_planet->texture );
				front.setColor( Color( 255, 255, 255, 255 ) );
			}

			float z = (float)frame / trans;
			int c = floor( z * 255.0 + .5 );
			int c0 = 255 - c;
			//back.setColor( Color( 255, 255, 255, c0 ) );
			front.setColor( Color( 255, 255, 255, c ) );
			break;
		}
	case PLANET:
		{
			if( frame == 0 )
			{
				//back.setTexture( *planetTex );
				back.setTexture( *ts_planet->texture );
				back.setColor( Color( 255, 255, 255, 255 ) );
			}
			break;
		}
	case SECTION_TRANSITION:
		{
			if( frame == 0 )
			{
				//front.setTexture( *sectionTex[selectedColony] );
				front.setTexture( *ts_section[selectedColony]->texture );
				front.setColor( Color( 255, 255, 255, 255 ) );
			}

			float z = (float)frame / trans;
			int c = floor( z * 255.0 + .5 );
			int c0 = 255 - c;
			//back.setColor( Color( 255, 255, 255, c0 ) );
			front.setColor( Color( 255, 255, 255, c ) );
			break;
		}
	case SECTION:
		{
			if( frame == 0 )
			{
				back.setTexture( *ts_section[selectedColony]->texture );
				back.setColor( Color( 255, 255, 255, 255 ) );
			}
			break;
		}
	case COLONY_TRANSITION:
		{
			if( frame == 0 )
			{
				UpdateMapList();
				front.setTexture( *ts_colony[selectedColony]->texture );
				front.setColor( Color( 255, 255, 255, 255 ) );
			}

			float z = (float)frame / trans;
			int c = floor( z * 255.0 + .5 );
			int c0 = 255 - c;
			//back.setColor( Color( 255, 255, 255, c0 ) );
			front.setColor( Color( 255, 255, 255, c ) );
			break;
		}
	case COLONY:
		{
			if( frame == 0 )
			{
				back.setTexture( *ts_colony[selectedColony]->texture );
				back.setColor( Color( 255, 255, 255, 255 ) );
			}

			//cout << "currInput.ldown: " << currInput.LDown() << ", prevldown: " << prevInput.LDown() << endl;
			if( currInput.LDown() && !prevInput.LDown() )
			{
				++selectedLevel;
				if( selectedLevel == numTotalEntries )
					selectedLevel = 0;
			}
			else if( currInput.LUp() && !prevInput.LUp() )
			{
				--selectedLevel;
				if( selectedLevel == -1 )
				{
					selectedLevel = numTotalEntries - 1;
				}
			}

			selectedRect.setSize( Vector2f( 300, yspacing ) );
			selectedRect.setFillColor( Color::Green );
			selectedRect.setPosition( menuPos.x, 
				menuPos.y + yspacing * selectedLevel );

			break;
		}
	}

	if( state != COLONY )
	{
		//cout << "a: " << (int)front.getColor().a << endl;
		//cout << "frame: " << frame << ", state: " << (int)state << endl;
	}
	++frame;

	return true;
}

void WorldMap::Draw( RenderTarget *target )
{
	if( state == OFF )
	{

		return;
	}

	//
	if( state == PLANET_AND_SPACE || state == PLANET || state == SECTION || state == COLONY )
	{
		//cout << "drawing" << endl;
		target->draw( back );
	}
	else
	{
		//cout << "drawing" << endl;
		target->draw( back );
		target->draw( front );
	}

	if( state == COLONY )
	{
		target->draw( bgRect );
		target->draw( selectedRect );

		for( int i = 0; i < numTotalEntries; ++i )
		{
			target->draw( text[i] );
		}
	}

	if( state == PLANET )
	{
		target->draw( colonySelectSprite );
	}
}