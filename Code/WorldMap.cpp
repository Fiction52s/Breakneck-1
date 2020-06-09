#include "WorldMap.h"

#include "GameSession.h"
#include <sstream>
#include <iostream>
#include "MainMenu.h"
#include "SaveFile.h"
#include "Enemy_Shard.h"
#include "SaveMenuScreen.h"

#include "Background.h"

using namespace boost::filesystem;
using namespace sf;
using namespace std;

WorldMap::WorldMap( MainMenu *p_mainMenu )
	:font( mainMenu->arial ), mainMenu( p_mainMenu )
{
	//extraPassRect.setSize(Vector2f(1920, 1080));

	//ts_planetAndSpace = mainMenu->tilesetManager.GetTileset( "WorldMap/map_z1.jpg", 1920, 1080 );
	//planetAndSpaceTex = new Texture;
	//planetAndSpaceTex->loadFromFile( "WorldMap/map_z1.jpg" );
	
	//ts_planet = mainMenu->tilesetManager.GetTileset( "WorldMap/Map.png", 1920, 1080 );
	//planetTex = new Texture;
	//planetTex->loadFromFile( "WorldMap/map_z2.png" );

	worldSelector = new WorldSelector(p_mainMenu);

	ts_colonySelect = mainMenu->tilesetManager.GetTileset("WorldMap/w1_select.png", 1920, 1080);

	ts_colonyActive[0] = mainMenu->tilesetManager.GetTileset("WorldMap/w1_select.png", 1920, 1080);
	ts_colonyActive[1] = mainMenu->tilesetManager.GetTileset("WorldMap/w1_select.png", 1920, 1080);
	ts_colonyActiveZoomed[0] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w1_vein.png", 1920, 1080);
	ts_colonyActiveZoomed[1] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w2_vein.png", 1920, 1080);
	//ts_zoomedMapw1 = mainMenu->tilesetManager.GetTileset("WorldMap/map_w1.png", 1920, 1080);

	

	ts_space = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_bg.png", 1920, 1080);
	spaceSpr.setTexture(*ts_space->texture);
	
	ts_planet = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap.png", 1920, 1080); 
	planetSpr.setTexture(*ts_planet->texture);
	planetSpr.setOrigin(planetSpr.getLocalBounds().width / 2, planetSpr.getLocalBounds().height / 2);
	planetSpr.setPosition(960, 540);
	

	ts_colony[0] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w1.png", 1920, 1080);
	ts_colony[1] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w2.png", 1920, 1080);
	ts_colony[2] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w3.png", 1920, 1080);
	ts_colony[3] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w4.png", 1920, 1080);
	ts_colony[4] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w5.png", 1920, 1080);
	ts_colony[5] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w6.png", 1920, 1080);
	ts_colony[6] = mainMenu->tilesetManager.GetTileset("WorldMap/map_w7.png", 1920, 1080);
	
	ts_asteroids[0] = mainMenu->tilesetManager.GetTileset("WorldMap/asteroid_1_1920x1080.png", 1920, 1080);
	ts_asteroids[1] = mainMenu->tilesetManager.GetTileset("WorldMap/asteroid_2_1920x1080.png", 1920, 1080);
	ts_asteroids[2] = mainMenu->tilesetManager.GetTileset("WorldMap/asteroid_3_1920x1080.png", 1920, 1080);
	ts_asteroids[3] = mainMenu->tilesetManager.GetTileset("WorldMap/asteroid_4_1920x1080.png", 1920, 1080);

	for (int i = 0; i < 4; ++i)
	{
		IntRect ir = ts_asteroids[i]->GetSubRect(0);
		ir.width *= 3;
		SetRectSubRect(asteroidQuads + i * 4, ir);
		SetRectCenter(asteroidQuads + i * 4, ir.width, ir.height, Vector2f(960, 540));
		//SetRectColor(asteroidQuads + i * 4, Color(Color::Red));
	}
	/*for (int i = 0; i < 4; ++i)
	{
		asteroidSpr[i].setTexture(*ts_asteroids[i]->texture);
	}*/

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

	for (int i = 0; i < 2; ++i)
	{
		SetRectSubRect(worldActiveQuads + i * 4, ts_colonyActive[i]->GetSubRect(0));
		SetRectCenter(worldActiveQuads + i * 4, 1920, 1080, Vector2f(960, 540));//Vector2f(colonySpr[i].getPosition() + Vector2f( 960, 540 )));
		SetRectSubRect(worldActiveQuadsZoomed + i * 4, ts_colonyActiveZoomed[i]->GetSubRect(0));
		SetRectCenter(worldActiveQuadsZoomed + i * 4, 1920 / 8.f, 1080 / 8.f, Vector2f(colonySpr[i].getPosition() + Vector2f(960/8.f, 540/8.f)));
	}

	for (int i = 0; i < 7; ++i)
	{
		colonySpr[i].setTexture(*ts_colony[i]->texture);
		colonySpr[i].setScale(1.f / 8.f, 1.f / 8.f);

	}

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
	//	ts_section[i] = mainMenu->tilesetManager.GetTileset( ss.str(), 1920, 1080 );

	//	ss.clear();
	//	ss.str( "" );
	//	ss << "WorldMap/map_w" << (i+1) << ".png";
	//	//colonyTex[i] = new Texture;
	//	//colonyTex[i]->loadFromFile( ss.str() );
	//	ts_colony[i] = mainMenu->tilesetManager.GetTileset( ss.str(), 1920, 1080 );
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

	Reset( NULL );

	for (int i = 0; i < 7; ++i)
	{
		selectors[i] = new MapSelector(mainMenu, Vector2f(960, 540), i);
	}
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

WorldMap::~WorldMap()
{
	//delete planetAndSpaceTex;
	//delete planetTex;

	delete worldSelector;
	for (int i = 0; i < 7; ++i)
	{
		delete selectors[i];
	}

	//for( int i = 0; i < 6; ++i )
	//{
		//delete sectionTex[i];
		//delete colonyTex[i];
	//}

	ClearEntries();
	//delete [] text;
	//delete [] localPaths;
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
	mainMenu->worldMap->selectedColony = 0;
}

void WorldMap::InitSelectors()
{
	SaveFile *sFile = mainMenu->GetCurrentProgress();
	for (int i = 0; i < sFile->numWorlds; ++i)
	{
		selectors[i]->UpdateAllInfo(i);
	}
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
		if (currInput.A && !prevInput.A)
		{

			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_zoom_in"));
			state = PlANET_TO_COLONY;
			frame = 0;

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

			currSelector->sectorSelector->currIndex = startSector;

			for (int se = 0; se < currSelector->numSectors; ++se)
			{
				int numLevels = currSelector->sectors[startSector]->numLevels;
				int startLevel = 0;
				for (int i = 0; i < numLevels; ++i)
				{
					if (!currSelector->sectors[startSector]->sec->levels[i].GetComplete())
					{
						startLevel = i;
						break;
					}
				}
				currSelector->mapSelector->currIndex = startLevel;
			}
			
			break;
		}
		else if (currInput.B && !prevInput.B)
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

		if ((currInput.LDown() || currInput.PDown()) && !moveDown)
		{
			selectedColony++;
			if (selectedColony >= mainMenu->GetCurrentProgress()->numWorlds)
				selectedColony = 0;
			moveDown = true;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_change"));
			
		}
		else if ((currInput.LUp() || currInput.PUp()) && !moveUp)
		{
			selectedColony--;
			if (selectedColony < 0)
				selectedColony = mainMenu->GetCurrentProgress()->numWorlds - 1;
			moveUp = true;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_change"));
		}

		if (!(currInput.LDown() || currInput.PDown()))
		{
			moveDown = false;
		}
		if (!(currInput.LUp() || currInput.PUp()))
		{
			moveUp = false;
		}

		UpdateColonySelect();
	}
		break;
	case PlANET_TO_COLONY:
	{
		int limit = 120 / 2;
		if (frame == limit)
		{
			state = COLONY;
			frame = 0;
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
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_zoom_out"));
				state = COLONY_TO_PLANET;
				frame = 0;
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
	rt->clear();
	
	rt->setView(uiView);
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
	if (state != COLONY)
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

	if (state == COLONY )
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

		
	}
}

Sector &WorldMap::GetCurrSector()
{
	SaveFile * currFile = mainMenu->GetCurrentProgress();
	World & world = currFile->worlds[selectedColony];
	int secIndex = selectors[selectedColony]->sectorSelector->currIndex;
	return world.sectors[secIndex];
}

void WorldMap::CompleteCurrentMap( SaveFile *sf, int totalFrames )
{
	World & world = sf->worlds[selectedColony];
	int secIndex = selectors[selectedColony]->sectorSelector->currIndex;
	Sector &sec = world.sectors[secIndex];
	int levIndex = selectors[selectedColony]->mapSelector->currIndex;
	
	Level &lev = sec.levels[levIndex];

	if (!lev.GetComplete())
	{
		lev.justBeaten = true;
		lev.SetComplete(true);

		MapSector *mapSec = CurrSelector()->GetFocusedSector();
		mapSec->UpdateUnlockedLevelCount();
		mapSec->ms->mapSelector->SetTotalSize(mapSec->unlockedLevelCount);
	}
	else
	{
		lev.justBeaten = false;
	}

	bool isRecordSet = lev.TrySetRecord(totalFrames);
	if (isRecordSet)
	{
		//create a flag so that you can get hype over this
	}
}

int WorldMap::GetCurrSectorNumLevels()
{
	MapSelector *currSelector = selectors[selectedColony];
	int secIndex = currSelector->sectorSelector->currIndex;
	return currSelector->sectors[secIndex]->numLevels;
}