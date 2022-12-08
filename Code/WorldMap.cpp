#include "WorldMap.h"

#include "GameSession.h"
#include <sstream>
#include <iostream>
#include "MainMenu.h"
#include "SaveFile.h"
#include "Enemy_Shard.h"
#include "SaveMenuScreen.h"

#include "Background.h"
#include "KinBoostScreen.h"
#include "LoadingBackpack.h"
#include "UIMouse.h"

using namespace boost::filesystem;
using namespace sf;
using namespace std;

WorldMap::WorldMap( MainMenu *p_mainMenu )
	:font( mainMenu->arial ), mainMenu( p_mainMenu )
{
	allUnlocked = true;

	
	kinBoostScreen = new KinBoostScreen(mainMenu, this);

	worldSelector = new WorldSelector(p_mainMenu);

	SetRectColor(infoQuadBG, Color(0, 0, 0, 100));
	SetRectColor(infoNameBG, Color(0, 0, 0, 200));

	Vector2f infoQuadPos(1416, 292);
	SetRectTopLeft(infoQuadBG, 256, 192, infoQuadPos);
	SetRectTopLeft(infoNameBG, 256, 64, infoQuadPos + Vector2f( 0, -64 ));

	sectorsCompleteText.setFillColor(Color::White);
	sectorsCompleteText.setCharacterSize(36);
	sectorsCompleteText.setFont(mainMenu->arial);
	sectorsCompleteText.setPosition( infoQuadPos + Vector2f(10, 10));

	shardsCapturedText.setFillColor(Color::White);
	shardsCapturedText.setCharacterSize(36);
	shardsCapturedText.setFont(mainMenu->arial);
	shardsCapturedText.setPosition(infoQuadPos + Vector2f(10, 10 + 60));

	

	worldNameText.setFillColor(Color::White);
	worldNameText.setCharacterSize(40);
	worldNameText.setFont(mainMenu->arial);
	worldNameText.setPosition(infoQuadPos + Vector2f(256/2, 10) + Vector2f( 0, -64 ));
	
	

	ts_colonySelect = GetTileset("WorldMap/w1_select.png", 1920, 1080);

	ts_colonyActive[0] = GetTileset("WorldMap/w1_select.png", 1920, 1080);
	ts_colonyActive[1] = GetTileset("WorldMap/w1_select.png", 1920, 1080);
	ts_colonyActiveZoomed[0] = GetTileset("WorldMap/map_w1_vein.png", 1920, 1080);
	ts_colonyActiveZoomed[1] = GetTileset("WorldMap/map_w2_vein.png", 1920, 1080);
	//ts_zoomedMapw1 = GetTileset("WorldMap/map_w1.png", 1920, 1080);

	

	ts_space = GetTileset("WorldMap/worldmap_bg.png", 1920, 1080);
	spaceSpr.setTexture(*ts_space->texture);
	
	ts_planet = GetTileset("WorldMap/worldmap.png", 1920, 1080); 
	planetSpr.setTexture(*ts_planet->texture);
	planetSpr.setOrigin(planetSpr.getLocalBounds().width / 2, planetSpr.getLocalBounds().height / 2);
	planetSpr.setPosition(960, 540);
	

	ts_colony[0] = GetTileset("WorldMap/map_w1.png", 1920, 1080);
	ts_colony[1] = GetTileset("WorldMap/map_w2.png", 1920, 1080);
	ts_colony[2] = GetTileset("WorldMap/map_w3.png", 1920, 1080);
	ts_colony[3] = GetTileset("WorldMap/map_w4.png", 1920, 1080);
	ts_colony[4] = GetTileset("WorldMap/map_w5.png", 1920, 1080);
	ts_colony[5] = GetTileset("WorldMap/map_w6.png", 1920, 1080);
	ts_colony[6] = GetTileset("WorldMap/map_w7.png", 1920, 1080);
	ts_colony[7] = GetTileset("WorldMap/map_w7.png", 1920, 1080);
	
	ts_asteroids[0] = GetTileset("WorldMap/asteroid_1_1920x1080.png", 1920, 1080);
	ts_asteroids[1] = GetTileset("WorldMap/asteroid_2_1920x1080.png", 1920, 1080);
	ts_asteroids[2] = GetTileset("WorldMap/asteroid_3_1920x1080.png", 1920, 1080);
	ts_asteroids[3] = GetTileset("WorldMap/asteroid_4_1920x1080.png", 1920, 1080);

	for (int i = 0; i < 4; ++i)
	{
		IntRect ir = ts_asteroids[i]->GetSubRect(0);
		ir.width *= 3;
		SetRectSubRect(asteroidQuads + i * 4, ir);
		SetRectCenter(asteroidQuads + i * 4, ir.width, ir.height, Vector2f(960, 540));
		//SetRectColor(asteroidQuads + i * 4, Color(Color::Red));
	}

	if (!asteroidShader.loadFromFile("Resources/Shader/menuasteroid.frag", sf::Shader::Fragment))
	{
		cout << "asteroid SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}
	asteroidShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	zoomView.setCenter(960, 540);
	zoomView.setSize(1920, 1080);

	
	colonySpr[0].setPosition(1087, 331);
	//colonySpr[0].setPosition(841, 473);
	colonySpr[1].setPosition(1087, 614);
	colonySpr[2].setPosition(842, 756);
	colonySpr[3].setPosition(595, 614);
	colonySpr[4].setPosition(595, 331);
	colonySpr[5].setPosition(841, 189);
	colonySpr[6].setPosition(841, 473);
	colonySpr[7].setPosition(841, 600);

	for (int i = 0; i < 2; ++i)
	{
		SetRectSubRect(worldActiveQuads + i * 4, ts_colonyActive[i]->GetSubRect(0));
		SetRectCenter(worldActiveQuads + i * 4, 1920, 1080, Vector2f(960, 540));//Vector2f(colonySpr[i].getPosition() + Vector2f( 960, 540 )));
		SetRectSubRect(worldActiveQuadsZoomed + i * 4, ts_colonyActiveZoomed[i]->GetSubRect(0));
		SetRectCenter(worldActiveQuadsZoomed + i * 4, 1920 / 8.f, 1080 / 8.f, Vector2f(colonySpr[i].getPosition() + Vector2f(960/8.f, 540/8.f)));
	}

	for (int i = 0; i < MAX_NUM_WORLDS; ++i)
	{
		colonySpr[i].setTexture(*ts_colony[i]->texture);
		colonySpr[i].setScale(1.f / 8.f, 1.f / 8.f);

	}

	colonyRadius = 192 / 2;

	colonySelectSprZoomed.setScale(1.f / 8.f, 1.f / 8.f );
	

	//ts_colonySelectZoomed[0]->texture->setSmooth(true);

	//colonySelectSpr.setTexture( *ts_colonySelect[0]->texture );
	//colonySelectSpr.setScale(5.f, 5.f);
	
	//if (!zoomShader.loadFromFile( "zoomblur_shader.vert", "zoomblur_shader.frag" ) )
	if (!zoomShader.loadFromFile("Resources/Shader/zoomblur_shader.frag", sf::Shader::Fragment))
	{
		cout << "zoom blur SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}
	zoomShader.setUniform("texSize", Vector2f(1920, 1080));
	zoomShader.setUniform("radial_blur", 1.f);
	zoomShader.setUniform("radial_bright", .1f);
	zoomShader.setUniform("radial_origin", Vector2f( .5, .5 ) );
	zoomShader.setUniform("radial_size", Vector2f( 1.f / 1920, 1.f / 1080 ));


	int width = 1920;//1920 - w->getSize().x;
	int height = 1080; //1080 - w->getSize().y;
	uiView = View(sf::Vector2f(width / 2, height / 2), sf::Vector2f(width, height));

	//for( int i = 0; i < 6; ++i )
	//{
	//	stringstream ss;
	//	ss << "WorldMap/map_z3_" << (i+1) << ".png";
	//	ts_section[i] = GetTileset( ss.str(), 1920, 1080 );

	//	ss.clear();
	//	ss.str( "" );
	//	ss << "WorldMap/map_w" << (i+1) << ".png";
	//	//colonyTex[i] = new Texture;
	//	//colonyTex[i]->loadFromFile( ss.str() );
	//	ts_colony[i] = GetTileset( ss.str(), 1920, 1080 );
	//}

	fontHeight = 24;
	menuPos = Vector2f( 300, 300 );
	yspacing = 40;
	entries = NULL;
	text = NULL;
	dirNode = NULL;
	localPaths = NULL;
	leftBorder = 20;
	//numTotalEntries = 0;

	currLevelTimeText.setFillColor(Color::White);
	currLevelTimeText.setCharacterSize(40);
	currLevelTimeText.setFont(mainMenu->arial);
	currLevelTimeText.setOrigin(currLevelTimeText.getLocalBounds().left + currLevelTimeText.getLocalBounds().width / 2, 0);
	currLevelTimeText.setPosition(1300, 200);
	currLevelTimeText.setString("");

	Reset(NULL);

	adventureFile.Load("Resources/Adventure", "tadventure");
	adventureFile.LoadMapHeaders();
	planet = new Planet(adventureFile);

	int numWorlds = planet->numWorlds;
	selectors = new MapSelector*[numWorlds];

	for (int i = 0; i < numWorlds; ++i)
	{
		selectors[i] = new MapSelector(this, &(planet->worlds[i]),
			mainMenu, Vector2f(960, 540));
	}
	/*for (int i = 0; i < 8; ++i)
	{
		selectors[i] = new MapSelector( planet->worlds[mainMenu, Vector2f(960, 540), i);
	}*/
}

WorldMap::~WorldMap()
{
	//delete planetAndSpaceTex;
	//delete planetTex;

	delete worldSelector;
	for (int i = 0; i < planet->numWorlds; ++i)
	{
		delete selectors[i];
	}
	delete[] selectors;

	delete planet;
	//for( int i = 0; i < 6; ++i )
	//{
	//delete sectionTex[i];
	//delete colonyTex[i];
	//}

	delete kinBoostScreen;

	ClearEntries();
	//delete [] text;
	//delete [] localPaths;
}

void WorldMap::UpdateWorldStats()
{
	int sel = selectedColony + 1;

	worldNameText.setString("World " + to_string(sel));

	FloatRect worldNameTextLocalBounds = worldNameText.getLocalBounds();
	worldNameText.setOrigin(worldNameTextLocalBounds.width / 2
		+ worldNameTextLocalBounds.left, 0);

	SaveFile *saveFile = mainMenu->GetCurrentProgress();

	AdventureWorld &aw = adventureFile.GetWorld(selectedColony);


	World &world = planet->worlds[selectedColony];
	int numTotalSectors = world.numSectors;
	int numCompletedSectors = 0;
	for (int i = 0; i < numTotalSectors; ++i)
	{
		if (saveFile->IsCompleteSector(&world.sectors[i]))
		{
			++numCompletedSectors;
		}
	}

	stringstream ss;

	ss << "Sectors: " << numCompletedSectors << "/" << numTotalSectors;

	sectorsCompleteText.setString(ss.str());

	int numTotalShards = 0;
	int totalCaptured = 0;
	int numLevels;
	int levIndex;
	int numShardsInLevel;

	for (int i = 0; i < numTotalSectors; ++i)
	{
		numLevels = world.sectors[i].numLevels;
		for (int j = 0; j < numLevels; ++j)
		{
			levIndex = world.sectors[i].GetLevelIndex(j);
			AdventureMapHeaderInfo &amhi = adventureFile.GetMapHeaderInfo(levIndex);
			numShardsInLevel = amhi.shardInfoVec.size();
			numTotalShards += numShardsInLevel;
			for (int k = 0; k < numShardsInLevel; ++k)
			{
				if (saveFile->ShardIsCaptured(amhi.shardInfoVec[k].GetTrueIndex()))
				{
					++totalCaptured;
				}
			}
		}
	}

	ss.str("");
	ss.clear();

	ss << "Shards: " << totalCaptured << "/" << numTotalShards;

	shardsCapturedText.setString(ss.str());
}

void WorldMap::UpdateColonySelect()
{
	//colonySelectSpr.setTextureRect( ts_colonySelect[0]->GetSubRect( 0 ) );
	/*colonySelectSpr.setOrigin( colonySelectSpr.getLocalBounds().width / 2,
		colonySelectSpr.getLocalBounds().height / 2 );*/

	//colonySelectSpr.setPosition(colonySpr[0].getPosition());
	worldSelector->SetPosition(Vector2f(colonySpr[selectedColony].getPosition() + Vector2f(960 / 8.f, 540 / 8.f)));
	//colonySelectSprZoomed.setTexture(*ts_colonySelectZoomed[selectedColony]->texture);
	//colonySelectSprZoomed.setPosition(colonySpr[selectedColony].getPosition());
}

bool WorldMap::IsInAllUnlockedMode()
{
	return allUnlocked;
}

void WorldMap::RunSelectedMap()
{
	MapSelector *ms = CurrSelector();
	ms->RunSelectedMap();

}

void WorldMap::Reset( SaveFile *sf )
{
	fontHeight = 24;
	state = SPACE;
	frame = 0;
	asteroidFrame = 0;

	selectedColony = 0;
	selectedLevel = 0;
	/*if( sf != NULL )
	{
		
	}
	else
	{
		selectedColony = 0;
		selectedLevel = 0;
	}*/
	
	ClearEntries();
	moveDown = false;
	moveUp = false;

	UpdateColonySelect();
}

const std::string & WorldMap::GetSelected()
{
	return localPaths[selectedLevel];
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

	if (dirNode != NULL)
	{

		delete[] dirNode;
	}
		
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

MapSelector *WorldMap::CurrSelector()
{
	return selectors[selectedColony];
}

void WorldMap::UpdateMapList()
{
	ClearEntries();
	
	//std::string path = "test/";
	//std::string file = "map_online.brknk";
 //  bool goodDownload = levelServer.DownloadFile( path, file );
	stringstream ss;
	ss << "Resources/Maps/W" << (selectedColony+1);
	cout << "stuff: " << ss.str() << endl;
	UpdateMapList( entries, ss.str() );

	text = new Text[numTotalEntries];

	localPaths = new string[numTotalEntries];

	dirNode = new TreeNode*[numTotalEntries];

	


	Tex( 0, 0, entries );
}

void WorldMap::SetDefaultSelections()
{
	selectedColony = 0;
}

void WorldMap::InitSelectors()
{
	for (int i = 0; i < planet->numWorlds; ++i)
	{
		selectors[i]->Init();
		//selectors[i]->UpdateAllInfo(i);
	}
}

bool WorldMap::MouseIsOnSelectedColony()
{
	int tempSelected = -1;
	Vector2f mousePos = MOUSE.GetFloatPos();
	Vector2f colMiddle = colonySpr[selectedColony].getPosition() + Vector2f(colonySpr[selectedColony].getGlobalBounds().width / 2,
		colonySpr[selectedColony].getGlobalBounds().height / 2);

	if (length(mousePos - colMiddle) < colonyRadius)
	{
		return true;
	}

	return false;
}

void WorldMap::Update( ControllerState &prevInput, ControllerState &currInput )
{
	int trans = 20;
	switch( state )
	{
	case SPACE:
		{
			
			/*if( frame == trans )
			{
				state = SPACE_TO_PLANET;
				frame = 0;
			}*/
		}
		break;
	case SPACE_TO_PLANET:
		if( frame == trans )
		{
			state = PLANET;
			frame = 0;
		}
		else
		{

		}
		break;
	case PLANET:
	{
		if ((MouseIsOnSelectedColony() && MOUSE.IsMouseLeftClicked())
			|| CONTROLLERS.ButtonPressed_A() )//currInput.A && !prevInput.A)
		{
			state = PlANET_TO_COLONY;
			frame = 0;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_zoom_in"));

			//mainMenu->loadingBackpack->SetScale(.25f);
			//mainMenu->loadingBackpack->SetPosition(Vector2f(1920 - 260, 1080 - 200));


			//bgLoadFinished = false;
			

			//state = PlANET_TO_COLONY;
			//frame = 0;

			//testSelector->UpdateAllInfo();
			MapSelector *currSelector = CurrSelector();

			int startSector = 0;
			for (int i = 0; i < currSelector->numSectors; ++i)
			{
				if (!currSelector->sectors[i]->state == MapSector::COMPLETE)
				{
					startSector = i;
					break;
				}
			}

			currSelector->sectorSASelector->currIndex = startSector;

			//currSelector->sectors[startSector]->CreateBG();

			for (int se = 0; se < currSelector->numSectors; ++se)
			{
				int numLevels = currSelector->sectors[startSector]->numLevels;
				int startLevel = 0;
				for (int i = 0; i < numLevels; ++i)
				{
					/*if (!currSelector->sectors[startSector]->sec->levels[i].GetComplete())
					{
						startLevel = i;
						break;
					}*/
				}
				//currSelector->mapSelector->currIndex = startLevel;
			}
			
			break;
		}
		else if (CONTROLLERS.ButtonPressed_B() )//currInput.B && !prevInput.B)
		{
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("main_menu_back"));
			mainMenu->SetMode(MainMenu::SAVEMENU);
			mainMenu->saveMenu->Reset();
			mainMenu->saveMenu->action = SaveMenuScreen::FADEIN;
			mainMenu->saveMenu->transparency = 1.f;
			//mainMenu->saveMenu->kinFace.setTextureRect(mainMenu->saveMenu->ts_kinFace->GetSubRect(0));
			//mainMenu->saveMenu->kinJump.setTextureRect(mainMenu->saveMenu->ts_kinJump1->GetSubRect(0));
			state = SPACE;
			//transition back up later instead of just turning off
			frame = 0;
			break;
		}

		int numCompletedWorlds;
		if (allUnlocked)
		{
			numCompletedWorlds = planet->numWorlds;
		}
		else
		{
			SaveFile *saveFile = mainMenu->GetCurrentProgress();
			numCompletedWorlds = saveFile->GetNumCompleteWorlds(planet);
		}

		/*int tempSelected = -1;
		if (currInput.LUp())
		{
			if (currInput.LLeft())
			{
				tempSelected = 4;
			}
			else if (currInput.LRight())
			{
				tempSelected = 0;
			}
			else
			{
				tempSelected = 5;
			}
		}
		else if (currInput.LDown())
		{
			if (currInput.LLeft())
			{
				tempSelected = 3;
			}
			else if (currInput.LRight())
			{
				tempSelected = 1;
			}
			else
			{
				tempSelected = 2;
			}
		}*/

		bool left = currInput.LLeft() && prevInput.IsLeftNeutral();//!prevInput.LLeft();
		bool up = currInput.LUp() && prevInput.IsLeftNeutral();//!prevInput.LUp();
		bool down = currInput.LDown() && prevInput.IsLeftNeutral();//!prevInput.LDown();
		bool right = currInput.LRight() && prevInput.IsLeftNeutral();//!prevInput.LRight();

		int tempSelected = -1;
		Vector2f mousePos = MOUSE.GetFloatPos();
		Vector2f colMiddle;
		for (int i = 0; i < numCompletedWorlds; ++i)
		{
			colMiddle = colonySpr[i].getPosition() + Vector2f(colonySpr[i].getGlobalBounds().width / 2,
				colonySpr[i].getGlobalBounds().height / 2);
			if (length(mousePos - colMiddle) < colonyRadius)
			{
				tempSelected = i;
				break;
			}
		}


	
		/*if (selectedColony == 0)
		{
			if (left || up )
			{
				tempSelected = 5;
			}
			else if (down)
			{
				tempSelected = 1;
			}
		}
		else if (selectedColony == 1)
		{
			if (up)
			{
				tempSelected = 0;
			}
			else if (left || down )
			{
				tempSelected = 2;
			}
		}
		else if (selectedColony == 2)
		{
			if (left)
			{
				tempSelected = 3;
			}
			else if (right)
			{
				tempSelected = 1;
			}
		}
		else if (selectedColony == 3)
		{
			if (down || right )
			{
				tempSelected = 2;
			}
			else if (up)
			{
				tempSelected = 4;
			}
		}
		else if (selectedColony == 4)
		{
			if (down)
			{
				tempSelected = 3;
			}
			else if (up || right )
			{
				tempSelected = 5;
			}
		}
		else if (selectedColony == 5)
		{
			if (left)
			{
				tempSelected = 4;
			}
			else if (right)
			{
				tempSelected = 0;
			}
		}*/

		if (tempSelected >= 0 && tempSelected <= numCompletedWorlds && tempSelected != selectedColony )
		{
			selectedColony = tempSelected;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_change"));
			UpdateWorldStats();
		}

		/*if ((currInput.LDown() || currInput.PDown()) && !moveDown)
		{
			


			selectedColony++;
			if (selectedColony > numCompletedWorlds
				|| selectedColony >= planet->numWorlds )
				selectedColony = 0;
			
			moveDown = true;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_change"));
			UpdateWorldStats();
		}
		else if ((currInput.LUp() || currInput.PUp()) && !moveUp)
		{
			int numCompletedWorlds;
			if (allUnlocked)
			{
				numCompletedWorlds = planet->numWorlds;
			}
			else
			{
				SaveFile *saveFile = mainMenu->GetCurrentProgress();
				numCompletedWorlds = saveFile->GetNumCompleteWorlds(planet);
			}

			selectedColony--;
			if (selectedColony < 0)
			{
				if (numCompletedWorlds == planet->numWorlds)
				{
					selectedColony = planet->numWorlds - 1;
				}
				else
				{
					selectedColony = numCompletedWorlds;
				}
				
			}
				

			moveUp = true;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_change"));
			UpdateWorldStats();
		}

		if (!(currInput.LDown() || currInput.PDown()))
		{
			moveDown = false;
		}
		if (!(currInput.LUp() || currInput.PUp()))
		{
			moveUp = false;
		}*/

		UpdateColonySelect();
	}
		break;
	case PlANET_TO_COLONY:
	{
		int limit = 120 / 2;
		if (frame == limit)
		{
			//state = COLONY;
			//frame = 0;
			//state = PLANET;
			//frame = 0;
			mainMenu->LoadMode(MainMenu::WORLDMAP_COLONY);
			return;
			//worldSelector->SetAlpha(1.f - a);
		}
		else
		{
			

			float a = frame / (float)(limit - 1);

			worldSelector->SetAlpha(1.f - a * 2);

			int mLimit = 50 / 2;
			int mFrame = min(frame, mLimit);

			float aMove = mFrame / (float)(mLimit - 1);

			CubicBezier cb(.83, .27, .06, .63);//(0, 0, 1, 1);
			CubicBezier cbMove(0, 0, 1, 1);

			oldZoomCurvePos = zoomCurvePos;

			zoomCurvePos = cb.GetValue(a);


			float f = zoomCurvePos;

			float fz = cbMove.GetValue(aMove);

			//float test = f * 5.f;


			if (frame == 0)
				zoomShader.setUniform("sampleStrength", 0.f);


			Vector2f colMiddle = Vector2f(colonySpr[selectedColony].getGlobalBounds().width / 2,
				colonySpr[selectedColony].getGlobalBounds().height / 2);
			Vector2f endPos = colonySpr[selectedColony].getPosition() + colMiddle;
			float endScale = colonySpr[selectedColony].getScale().x * .2f;

			Vector2f startPos(960, 540);
			float startScale = 1.f;


			float oldA = currScale;



			currScale = startScale * (1.f - f) + endScale * f;

			currCenter = startPos * (1.f - fz) + endPos * fz;

			zoomView.setCenter(currCenter);
			zoomView.setSize(Vector2f(1920, 1080) * currScale);

			if (frame > 0)
			{
				float diff = zoomCurvePos - oldZoomCurvePos;//abs(currScale - oldScale);

															//float diffFactor = diff / abs(endScale - startScale) / limit;
				float multiple = limit;
				diff *= multiple;
				diff += 1.0;

				zoomShader.setUniform("sampleStrength", diff);
			}
		}
	}
		break;
	case COLONY:
	{
		worldSelector->SetAlpha(0);
		Vector2f colMiddle = Vector2f(colonySpr[selectedColony].getGlobalBounds().width / 2,
			colonySpr[selectedColony].getGlobalBounds().height / 2);
		Vector2f endPos = colonySpr[selectedColony].getPosition() + colMiddle;
		
		zoomView.setCenter(endPos);
		zoomView.setSize(Vector2f(1920, 1080) * colonySpr[selectedColony].getScale().x * .2f);
		break;
	}
		
		//if( currInput.A && !prevInput.A )
		//{
		//	state = OFF;
		//	frame = 0;
		//	return false; //start a map!
		//}
		//frame = 0;
		
	case COLONY_TO_PLANET:
	{
		int limit = 60;//120 / 2;
		if (frame == limit)
		{
			state = PLANET;
			frame = 0;
			currScale = 1.f;
			zoomView.setSize(Vector2f(1920, 1080) * currScale);
			zoomView.setCenter(960, 540);
			worldSelector->SetAlpha(1.f);
		}
		else
		{
			
			float a = frame / (float)(limit - 1);
			worldSelector->SetAlpha(a);
			int mLimit = 60;//16 / 2;
			int mFrame = min(frame, mLimit);

			float aMove = mFrame / (float)(mLimit - 1);

			CubicBezier cb(0, 0, 1, 1);//(.12, .66, .85, .4);//.83, .27, .06, .63);//(0, 0, 1, 1);
			CubicBezier cbMove(0, 0, 1, 1);

			oldZoomCurvePos = zoomCurvePos;

			zoomCurvePos = cb.GetValue(a);


			float f = zoomCurvePos;

			float fz = cbMove.GetValue(aMove);

			//float test = f * 5.f;


			if (frame == 0)
				zoomShader.setUniform("sampleStrength", 0.f);


			Vector2f colMiddle = Vector2f(colonySpr[selectedColony].getGlobalBounds().width / 2,
				colonySpr[selectedColony].getGlobalBounds().height / 2);
			Vector2f startPos = colonySpr[selectedColony].getPosition() + colMiddle;
			float startScale = colonySpr[selectedColony].getScale().x;//.2f;

			Vector2f endPos(960, 540);
			float endScale = 1.f;


			float oldA = currScale;



			currScale = startScale * (1.f - f) + endScale * f;

			currCenter = startPos * (1.f - fz) + endPos * fz;

			zoomView.setCenter(currCenter);
			zoomView.setSize(Vector2f(1920, 1080) * currScale);

			if (frame > 0)
			{
				float diff = zoomCurvePos - oldZoomCurvePos;
				float multiple = limit;
				diff *= multiple;
				diff += 1.0;

				//zoomShader.setUniform("sampleStrength", diff);
			}
		}
	}
		break;
	}

	

	switch( state )
	{
	case SPACE:
		{
			if( frame == 0 )
			{
				//back.setTexture( *planetAndSpaceTex );
			//	back.setTexture( *ts_planetAndSpace->texture );
			//	back.setColor( Color( 255, 255, 255, 255 ) );
			}
			break;
		}
	case SPACE_TO_PLANET:
		{
			if( frame == 0 )
			{
				//front.setTexture( *planetTex );
				//front.setTexture( *ts_planet->texture );
				//front.setColor( Color( 255, 255, 255, 255 ) );
			}

			float z = (float)frame / trans;
			int c = floor( z * 255.0 + .5 );
			int c0 = 255 - c;
			//back.setColor( Color( 255, 255, 255, c0 ) );
			//front.setColor( Color( 255, 255, 255, c ) );
			break;
		}
	case PLANET:
		{
			worldSelector->SetAlpha(1.f);//need every frame?
			//if( frame == 0 )
			{
				//back.setTexture( *planetTex );
				/*back.setTexture( *ts_planet->texture );
				back.setColor( Color( 255, 255, 255, 255 ) );*/
			}
			break;
		}
	case PlANET_TO_COLONY:
		{
			if( frame == 0 )
			{/*
				UpdateMapList();
				front.setTexture( *ts_colony[selectedColony]->texture );
				front.setColor( Color( 255, 255, 255, 255 ) );*/
			}
			break;
		}
	case COLONY:
		{
			if (!CurrSelector()->Update(currInput, prevInput))
			{
				mainMenu->LoadMode(MainMenu::WORLDMAP);
				//mainMenu->SetMode(MainMenu::WORLDMAP);

			}
			break;
		}
	case COLONY_TO_PLANET:
	{
		/*if (frame == trans)
		{
			state = PLANET;
			frame = 0;
			currScale = 1.f;
			zoomView.setSize(Vector2f(1920, 1080) * currScale);
			zoomView.setCenter(960, 540);
		}*/
		break;
	}
	}

	worldSelector->Update();

	++frame;
	++asteroidFrame;
}

void WorldMap::Draw( RenderTarget *target )
{
	sf::RenderTexture *rt = MainMenu::extraScreenTexture;
	rt->clear(sf::Color::Transparent);
	
	rt->setView(uiView);


	if (state == COLONY)
	{
		rt->clear(Color::Transparent);
		rt->setView(uiView);
		CurrSelector()->Draw(rt);
		rt->draw(currLevelTimeText);
		rt->display();
		const sf::Texture &ttex = rt->getTexture();
		selectorExtraPass.setTexture(ttex);

		float dur = 30;
		if (frame <= dur)
		{
			selectorExtraPass.setColor(Color(255, 255, 255, 255.f * (frame / dur)));
		}
		else
		{
			//selectorExtraPass.setColor(Color(255, 255, 255, 40));
			selectorExtraPass.setColor(Color::White);
		}
		target->draw(selectorExtraPass);

		return;
	}

	rt->draw(spaceSpr);

	int scrollSeconds[] = { 400, 300, 140, 120 };
	float astFactor[] = { .1f, .3f, 1.5f, 2.f };

	//rt->draw(asteroidSpr[0]);
	//rt->draw(asteroidSpr[1]);

	float z = zoomView.getSize().x / 1920.f;
	View vvv = zoomView;

	RenderStates rs;

	rs.shader = &asteroidShader;
		

	for (int i = 0; i < 2; ++i)
	{
		rs.texture = ts_asteroids[i]->texture;
		float aZ = 1.f - z;
		aZ *= astFactor[i];
		aZ = 1.f - aZ;
		//float aZ = z * astFactor[i];
		assert(aZ <= 1.f);
		if (aZ > 0)
		{
			float xDiff = zoomView.getCenter().x - 960;
			float yDiff = zoomView.getCenter().y - 540;
			xDiff *= 1.f - aZ;
			yDiff *= 1.f - aZ;
			vvv.setSize(aZ * 1920.f, aZ * 1080.f);
			vvv.setCenter(960 + xDiff, 540 + yDiff);
			rt->setView(vvv);
			asteroidShader.setUniform("quant", -(asteroidFrame % (scrollSeconds[i] * 60)) / (float)(scrollSeconds[i] * 60));
			rt->draw(asteroidQuads + i * 4, 4, sf::Quads, rs);
			//rt->draw(asteroidSpr[i], &asteroidShader);
		}
	}

	rt->setView(zoomView);

	rt->draw(planetSpr);

	for (int i = 0; i < 7; ++i)
	{
		rt->draw(colonySpr[i]);
	}



	//int breatheTi

	int energyBreathe = 240;
	Color selectColor = Color::White;
	int ff = asteroidFrame % (energyBreathe);
	if (ff < energyBreathe / 2)
	{
		float factor = (float)ff / (energyBreathe / 2);
		selectColor.a = 150 * factor;
		//c.a = std::max(20.f, (float)c.a);
	}
	else
	{
		float factor = 1.f - (float)(ff - energyBreathe / 2) / (energyBreathe / 2);
		selectColor.a = 150 * factor;
		//c.a = std::max(20.f, (float)c.a);
	}

	for (int i = 0; i < 2; ++i)
	{
		SetRectColor(worldActiveQuads + i * 4, selectColor);
		SetRectColor(worldActiveQuadsZoomed + i * 4, selectColor);
		rt->draw(worldActiveQuads + i * 4, 4, sf::Quads, ts_colonyActive[i]->texture);
		rt->draw(worldActiveQuadsZoomed + i * 4, 4, sf::Quads, ts_colonyActiveZoomed[i]->texture);
	}
	//colonySelectSpr.setColor(selectColor);
	//colonySelectSprZoomed.setColor(selectColor);

	//rt->draw(colonySelectSpr);
	//rt->draw(colonySelectSprZoomed);
	if (state != COLONY )
	{
		worldSelector->Draw(rt);
	}
	

	for (int i = 2; i < 4; ++i)
	{
		rs.texture = ts_asteroids[i]->texture;

		float aZ = 1.f - z;
		aZ *= astFactor[i];
		aZ = 1.f - aZ;
		assert(aZ <= 1.f);
		if (aZ > 0)
		{
			float xDiff = zoomView.getCenter().x - 960;
			float yDiff = zoomView.getCenter().y - 540;
			xDiff *= 1.f - aZ;
			yDiff *= 1.f - aZ;

			vvv.setCenter(960 + xDiff, 540 + yDiff);
			vvv.setSize(aZ * 1920.f, aZ * 1080.f);

			rt->setView(vvv);
			asteroidShader.setUniform("quant", (asteroidFrame % (scrollSeconds[i] * 60)) / (float)(scrollSeconds[i] * 60));
			rt->draw(asteroidQuads + i * 4, 4, sf::Quads, rs);
			//rt->draw(asteroidSpr[i] , &asteroidShader);
		}
	}

	if (state != COLONY && state != PlANET_TO_COLONY
		&& state != COLONY_TO_PLANET )
	{
		rt->draw(infoQuadBG, 4, sf::Quads);
		rt->draw(infoNameBG, 4, sf::Quads);
		rt->draw(sectorsCompleteText);
		rt->draw(shardsCapturedText);
		rt->draw(worldNameText);
	}

	/*if (state == LOAD_COLONY)
	{
		mainMenu->loadingBackpack->Draw(rt);
	}*/

	//rt->draw(asteroidSpr[2]);
	//asteroidSpr[2].setScale(5.f, 5.f);
	//asteroidSpr[3].setScale(5.f, 5.f);
	
	//float astFac0 = 1.5f;
	//float astFac1 = 2.f;
	//int scrollFrames0 = 60 * 60;
	//int scrollFrames1 = 60 * 45;
	//int scrollFrames2 = 60 * 60;
	//int scrollFrames3 = 60 * 60;
	//float z0 = zoomView.getSize().x / 1920.f;
	//z = 1.f - z;
	//z *= astFac0;
	//z = 1.f - z;

	//if (z > 0)
	//{
	//	

	//	

	//	vvv.setSize(z * 1920.f, z * 1080.f);

	//	rt->setView(vvv);

	//	asteroidShader.setUniform("quant", (asteroidFrame % scrollFrames1) / (float)scrollFrames1);
	//	rt->draw(asteroidSpr[2], &asteroidShader);
	//}
	//z = 1.f - z;
	

	rt->display();
	const sf::Texture &tex = rt->getTexture();
	extraPassSpr.setTexture(tex);
	
	zoomShader.setUniform("zoomTex", sf::Shader::CurrentTexture );
	extraPassSpr.setPosition(0, 0);

	if ((state == PlANET_TO_COLONY /*|| state == COLONY_TO_PLANET*/ ) && frame > 20 )
	{
		target->draw(extraPassSpr, &zoomShader);
	}
	else
	{
		target->draw(extraPassSpr);
	}

	
}

Sector & WorldMap::GetCurrSector()
{
	//SaveFile * currFile = mainMenu->GetCurrentProgress();
	//World & world = currFile->worlds[selectedColony];
	int secIndex = selectors[selectedColony]->sectorSASelector->currIndex;
	return planet->worlds[selectedColony].sectors[secIndex];
	//return currFile->adventureFile.GetSector(selectedColony, secIndex);
}

void WorldMap::CompleteCurrentMap( Level *lev, int totalFrames )
{
	SaveFile *saveFile = mainMenu->GetCurrentProgress();
	//World & world = sf->worlds[selectedColony];
	//int worldIndex = selectedColony;
	//int secIndex = selectors[selectedColony]->sectorSASelector->currIndex;
	//int levIndex = selectors[selectedColony]->FocusedSector()->mapSASelector->currIndex;
	
	//Level &lev = sec.levels[levIndex];

	if ( !saveFile->IsCompleteLevel( lev ) )
	{
		saveFile->CompleteLevel(lev);

		MapSector *mapSec = CurrSelector()->FocusedSector();
		mapSec->UpdateUnlockedLevelCount();
		mapSec->mapSASelector->SetTotalSize(mapSec->unlockedLevelCount);
	}
	else
	{
		//lev.justBeaten = false;
	}

	bool isRecordSet = saveFile->TrySetRecordTime(totalFrames, lev);
	if (isRecordSet)
	{
		//create a flag so that you can get hype over this
	}
}

int WorldMap::GetCurrSectorNumLevels()
{
	MapSelector *currSelector = selectors[selectedColony];
	int secIndex = currSelector->sectorSASelector->currIndex;
	return currSelector->sectors[secIndex]->numLevels;
}