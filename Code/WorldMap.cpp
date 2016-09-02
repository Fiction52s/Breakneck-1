#include "WorldMap.h"

#include "GameSession.h"
#include <sstream>
#include <iostream>

using namespace boost::filesystem;
using namespace sf;
using namespace std;

WorldMap::WorldMap( sf::Font &p_font )
	:font( p_font )
{
	planetAndSpaceTex = new Texture;
	planetAndSpaceTex->loadFromFile( "WorldMap/map_z1.jpg" );
	
	planetTex = new Texture;
	planetTex->loadFromFile( "WorldMap/map_z2.png" );

	for( int i = 0; i < 6; ++i )
	{
		stringstream ss;
		ss << "WorldMap/map_z3_" << (i+1) << ".png";
		sectionTex[i] = new Texture;//owner->GetTileset( ss.str(), 1920, 1080 );
		sectionTex[i]->loadFromFile( ss.str() );

		ss.clear();
		ss.str( "" );
		ss << "WorldMap/map_z4_" << (i+1) << ".png";
		colonyTex[i] = new Texture;
		colonyTex[i]->loadFromFile( ss.str() );
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

	Reset();
}

void WorldMap::Reset()
{
	fontHeight = 24;
	state = OFF;
	frame = 0;
	selectedColony = 1;
	selectedLevel = 0;
	ClearEntries();
}

const std::string & WorldMap::GetSelected()
{
	return localPaths[selectedLevel];
}

WorldMap::~WorldMap()
{
	delete planetAndSpaceTex;
	delete planetTex;



	for( int i = 0; i < 6; ++i )
	{
		delete sectionTex[i];
		delete colonyTex[i];
	}

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
	bgRect.setFillColor( Color::Green );
	bgRect.setPosition( menuPos );

	
	
	
	

	for( list<path>::iterator it = entry->files.begin(); it != entry->files.end(); ++it )
	{
		Text &t = text[index];
		t.setFont( font );
		t.setCharacterSize( fontHeight );

		string name = (*it).filename().string();
		name = name.substr( 0, name.size() - 6 );


		t.setString( name );
		t.setColor( Color::White );
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

   
	UpdateMapList( entries, "Maps/W2" );

	text = new Text[numTotalEntries];

	localPaths = new string[numTotalEntries];

	dirNode = new TreeNode*[numTotalEntries];

	


	Tex( 0, 0, entries );
}

bool WorldMap::Update()
{
	
	if( state == OFF )
		return false;


	int trans = 40;
	switch( state )
	{
	case PLANET_AND_SPACE:
		{
			//cout << "currInput.A: " << currInput.A << ", prevInput.A: " << prevInput.A << endl;
			if( currInput.A && !prevInput.A )
			{
				state = PLANET_TRANSITION;
				frame = 0;
				break;
			}
			else if( currInput.B && !prevInput.B )
			{
				state = OFF;
				frame = 0;
				break;
			}
			
		}
		frame = 0;
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
		if( frame == trans )
		{
			state = SECTION_TRANSITION;
			frame = 0;
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
				back.setTexture( *planetAndSpaceTex );
				back.setColor( Color( 255, 255, 255, 255 ) );
			}
			break;
		}
	case PLANET_TRANSITION:
		{
			if( frame == 0 )
			{
				front.setTexture( *planetTex );
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
				back.setTexture( *planetTex );
				back.setColor( Color( 255, 255, 255, 255 ) );
			}
			break;
		}
	case SECTION_TRANSITION:
		{
			if( frame == 0 )
			{
				front.setTexture( *sectionTex[selectedColony] );
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
				back.setTexture( *sectionTex[selectedColony] );
				back.setColor( Color( 255, 255, 255, 255 ) );
			}
			break;
		}
	case COLONY_TRANSITION:
		{
			if( frame == 0 )
			{
				front.setTexture( *colonyTex[selectedColony] );
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
				back.setTexture( *colonyTex[selectedColony] );
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
			selectedRect.setFillColor( Color::Black );
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
}