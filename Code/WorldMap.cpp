#include "WorldMap.h"

#include "GameSession.h"
#include <sstream>
#include <iostream>
#include "MainMenu.h"
#include "SaveFile.h"
#include "Enemy_Shard.h"
#include "SaveMenuScreen.h"

using namespace boost::filesystem;
using namespace sf;
using namespace std;

//MapNode::MapNode()
//	:leftLinkIndex(-1),rightLinkIndex(-1),upLinkIndex(-1),downLinkIndex(-1),index(-1)
//{
//
//}
//
//int MapNode::GetNextIndex(ControllerState &curr,
//	ControllerState &prev)
//{
//	if (curr.LLeft() && !prev.LLeft())
//	{
//		if (leftLinkIndex >= 0)
//		{
//			return leftLinkIndex;
//		}
//	}
//	else if (curr.LRight() && !prev.LRight())
//	{
//		if (rightLinkIndex >= 0)
//		{
//			return rightLinkIndex;
//		}
//	}
//	else if (curr.LUp() && !prev.LUp())
//	{
//		if (upLinkIndex >= 0)
//		{
//			return upLinkIndex;
//		}
//	}
//	else if (curr.LDown() && !prev.LDown())
//	{
//		if (downLinkIndex >= 0)
//		{
//			return downLinkIndex;
//		}
//	}
//
//	return index;
//}



//void MapNode::Draw(sf::RenderTarget *target)
//{
//	//just for debug
//
//	//draw node
//	sf::CircleShape cs;
//	cs.setPosition(pos.x, pos.y);
//	cs.setFillColor(Color::Blue);
//	cs.setRadius(100);
//	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
//	target->draw(cs);
//}
//
//void MapNode::StartMap()
//{
//
//}

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

	ts_colonySelect = mainMenu->tilesetManager.GetTileset( "WorldMap/map_select_512x512.png"
		, 512, 512 );

	//ts_zoomedMapw1 = mainMenu->tilesetManager.GetTileset("WorldMap/map_w1.png", 1920, 1080);

	ts_space = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_bg.png", 1920, 1080);
	spaceSpr.setTexture(*ts_space->texture);
	
	ts_planet = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap.png", 1920, 1080); 
	planetSpr.setTexture(*ts_planet->texture);
	planetSpr.setOrigin(planetSpr.getLocalBounds().width / 2, planetSpr.getLocalBounds().height / 2);
	planetSpr.setPosition(960, 540);

	ts_colony[0] = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_w1.png", 1920, 1080);
	ts_colony[1] = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_w2.png", 1920, 1080);
	ts_colony[2] = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_w3.png", 1920, 1080);
	ts_colony[3] = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_w4.png", 1920, 1080);
	ts_colony[4] = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_w5.png", 1920, 1080);
	ts_colony[5] = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_w6.png", 1920, 1080);
	ts_colony[6] = mainMenu->tilesetManager.GetTileset("WorldMap/worldmap_w7.png", 1920, 1080);
	
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

	if (!asteroidShader.loadFromFile("Shader/menuasteroid.frag", sf::Shader::Fragment))
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

	for (int i = 0; i < 7; ++i)
	{
		colonySpr[i].setTexture(*ts_colony[i]->texture);
		colonySpr[i].setScale(1.f / 8.f, 1.f / 8.f);
	}

	colonySelectSprite.setTexture( *ts_colonySelect->texture );
	
	//if (!zoomShader.loadFromFile( "zoomblur_shader.vert", "zoomblur_shader.frag" ) )
	if (!zoomShader.loadFromFile("Shader/zoomblur_shader.frag", sf::Shader::Fragment))
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


	Reset( NULL );

	testSelector = new MapSelector(mainMenu, Vector2f(960, 540));
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
			state = PlANET_TO_COLONY;
			frame = 0;
			break;
		}
		else if (currInput.B && !prevInput.B)
		{
			mainMenu->menuMode = MainMenu::SAVEMENU;
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
			if (selectedColony > 5)
				selectedColony = 0;
			moveDown = true;
			UpdateColonySelect();
		}
		else if ((currInput.LUp() || currInput.PUp()) && !moveUp)
		{
			selectedColony--;
			if (selectedColony < 0)
				selectedColony = 5;
			moveUp = true;
			UpdateColonySelect();
		}

		if (!(currInput.LDown() || currInput.PDown()))
		{
			moveDown = false;
		}
		if (!(currInput.LUp() || currInput.PUp()))
		{
			moveUp = false;
		}
	}
		break;
	case PlANET_TO_COLONY:
	{
		int limit = 120 / 2;
		if (frame == limit)
		{
			state = COLONY;
			frame = 0;

		}
		else
		{
			float a = frame / (float)(limit - 1);

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


			Vector2f colMiddle = Vector2f(colonySpr[0].getGlobalBounds().width / 2,
				colonySpr[0].getGlobalBounds().height / 2);
			Vector2f endPos = colonySpr[0].getPosition() + colMiddle;
			float endScale = colonySpr[0].getScale().x;//.2f;

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
		Vector2f colMiddle = Vector2f(colonySpr[0].getGlobalBounds().width / 2,
			colonySpr[0].getGlobalBounds().height / 2);
		Vector2f endPos = colonySpr[selectedColony].getPosition() + colMiddle;

		zoomView.setCenter(endPos);
		zoomView.setSize(Vector2f(1920, 1080) * colonySpr[0].getScale().x);
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
		}
		else
		{
			float a = frame / (float)(limit - 1);

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


			Vector2f colMiddle = Vector2f(colonySpr[0].getGlobalBounds().width / 2,
				colonySpr[0].getGlobalBounds().height / 2);
			Vector2f startPos = colonySpr[0].getPosition() + colMiddle;
			float startScale = colonySpr[0].getScale().x;//.2f;

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
		//if (currInput.A && !prevInput.A)
		//{
		//	state = OFF;
		//	frame = 0;
		//	return false; //start a map!
		//}
		if (currInput.B && !prevInput.B)
		{
			state = COLONY_TO_PLANET;
			frame = 0;
		}
		else
		{
			testSelector->Update(currInput, prevInput);
		}
			//if( frame == 0 )
			//{
			//	back.setTexture( *ts_colony[selectedColony]->texture );
			//	back.setColor( Color( 255, 255, 255, 255 ) );
			//}

			////cout << "currInput.ldown: " << currInput.LDown() << ", prevldown: " << prevInput.LDown() << endl;
			//if( currInput.LDown() && !prevInput.LDown() )
			//{
			//	++selectedLevel;
			//	if( selectedLevel == numTotalEntries )
			//		selectedLevel = 0;
			//}
			//else if( currInput.LUp() && !prevInput.LUp() )
			//{
			//	--selectedLevel;
			//	if( selectedLevel == -1 )
			//	{
			//		selectedLevel = numTotalEntries - 1;
			//	}
			//}

			//selectedRect.setSize( Vector2f( 300, yspacing ) );
			//selectedRect.setFillColor( Color::Green );
			//selectedRect.setPosition( menuPos.x, 
			//	menuPos.y + yspacing * selectedLevel );

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



	++frame;
	++asteroidFrame;
}

void WorldMap::Draw( RenderTarget *target )
{
	sf::RenderTexture *rt = MainMenu::extraScreenTexture;
	rt->clear();
	
	rt->setView(uiView);
	rt->draw(spaceSpr);

	int scrollSeconds[] = { 400, 180, 140, 120 };
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

	if (state == COLONY)
	{
		testSelector->Draw(target);
	}
}

MapSelector::MapSelector( MainMenu *mm, sf::Vector2f &pos )
	:centerPos( pos )
{
	state = S_IDLE;
	worldIndex = 0;
	mainMenu = mm;
	//numNodeColumns = 10;
	//nodeSelectorWidth = 400;
	ts_node = mm->tilesetManager.GetTileset("Worldmap/node_w1_80x80.png", 80, 80);
	Tileset *ts_bottom = mm->tilesetManager.GetTileset("Worldmap/levelselect_672x256.png", 672, 256);

	ts_sectorKey = mm->tilesetManager.GetTileset("Worldmap/sectorkey_80x80.png", 80, 80);
	ts_sectorOpen = new Tileset*[1];
	ts_sectorOpen[0] = mm->tilesetManager.GetTileset("Worldmap/sectoropen_256x256.png", 256, 256);

	ts_shoulderIcons = mainMenu->tilesetManager.GetTileset("Menu/xbox_button_icons_128x128.png", 128, 128);
	SetRectCenter(shoulderIcons, 128, 128, Vector2f(200, 200));
	SetRectCenter(shoulderIcons + 4, 128, 128, Vector2f(1920 - 200, 200));
	SetRectSubRect(shoulderIcons, ts_shoulderIcons->GetSubRect(6));
	SetRectSubRect(shoulderIcons + 4, ts_shoulderIcons->GetSubRect(4));

	ts_path = mm->tilesetManager.GetTileset("Worldmap/nodepath_96x96.png", 96, 96);

	Tileset *ts_thumb = mm->tilesetManager.GetTileset("Worldmap/mapthumb_w1_1_256x256.png", 256, 256);
	Tileset *ts_shard = mm->tilesetManager.GetTileset("Worldmap/worldmap_shards_272x256.png", 272, 256);
	bottomBG.setTexture(*ts_bottom->texture);
	thumbnailBG.setTexture(*ts_thumb->texture);
	shardBG.setTexture(*ts_shard->texture);

	bottomBG.setPosition(624, 545);
	thumbnailBG.setPosition(640, 225);
	shardBG.setPosition(1008, 225);

	numSectors = 0;
	sectors = NULL;
	//numSectors = 7;
	//MapSector *ms;
	sectorCenter = Vector2f(960, 550);/*bottomBG.getPosition()
	+ Vector2f(bottomBG.getLocalBounds().width / 2,
		bottomBG.getLocalBounds().height / 2);*/

	saSelector = NULL;
	slideDuration = 50;
	////bottomCenter.x = 0;
	//sectors = new MapSector*[numSectors];
	//for (int i = 0; i < numSectors; ++i)
	//{
	//	ms = new MapSector( this, 6 );
	//	//load sectors externally
	//	sectors[i] = ms;
	//	ms->SetCenter(bottomCenter);
	//	//ms->ts_thumb 
	//	
	//}

	


	//bottomBG.setOrigin(bottomBG.getLocalBounds().width / 2, bottomBG.getLocalBounds().height / 2);
	//thumbnailBG.setOrigin(thumbnailBG.getLocalBounds().width / 2, thumbnailBG.getLocalBounds().height / 2);
	//shardBG.setOrigin(shardBG.getLocalBounds().width / 2, shardBG.getLocalBounds().height / 2);






	//saSelectorLevel = new SingleAxisSelector(numLevelsInWorld, waitFrames, 2, waitModeThresh, 0, 0);
	//Tileset *ts_thumb = mm->tilesetManager.GetTileset("WorldMap/thumbnail01.png", 256, 256);
	//thumbnailSpr.setTexture(ts_thumb);

}


void MapSelector::UpdateSprites()
{

}

void MapSelector::Draw(sf::RenderTarget *target)
{
	//target->draw(bottomBG);
	//target->draw(thumbnailBG);
	//target->draw(shardBG);
	
	if (state == S_SLIDINGLEFT || state == S_SLIDINGRIGHT)
	{
		sectors[saSelector->oldCurrIndex]->Draw(target);
	}
	else
	{
		target->draw(shoulderIcons, 8, sf::Quads, ts_shoulderIcons->texture);
	}

	sectors[saSelector->currIndex]->Draw(target);
	//target->draw(thumbnail);

	//target->draw( paths, sectors[currSectorIndex]->numLevels  )
}

void MapSelector::Update(ControllerState &curr,
	ControllerState &prev)
{
	if (state == S_IDLE)
	{
		bool left = curr.LLeft();
		bool right = curr.LRight();

		int changed = saSelector->UpdateIndex(curr.leftShoulder, curr.rightShoulder);

		if (changed > 0)
		{
			state = S_SLIDINGRIGHT;
			frame = 0;
		}
		else if (changed < 0)
		{
			state = S_SLIDINGLEFT;
			frame = 0;
		}
		else
		{
			sectors[saSelector->currIndex]->Update(curr, prev);
		}
	}
	else if (state == S_SLIDINGLEFT)
	{
		if (frame == slideDuration+1)
		{
			state = S_IDLE;
			frame = 0;
			//sectors[saSelector->currIndex]->SetXCenter(sectorCenter.x);
		}
		else
		{
			float diff = 1920.f * (float)frame / slideDuration;
			float oldCenter = sectorCenter.x + diff;
			float newCenter = -960.f + diff;
			sectors[saSelector->oldCurrIndex]->SetXCenter(oldCenter);
			sectors[saSelector->currIndex]->SetXCenter(newCenter);
		}
		
	}
	else if (state == S_SLIDINGRIGHT)
	{
		if (frame == slideDuration+1)
		{
			state = S_IDLE;
			frame = 0;
			//sectors[saSelector->currIndex]->SetXCenter(sectorCenter.x);
		}
		else
		{
			float diff = 1920.f * (float)frame / slideDuration;
			float oldCenter = sectorCenter.x - diff;
			float newCenter = (1920.f + 960.f) - diff;
			sectors[saSelector->oldCurrIndex]->SetXCenter(oldCenter);
			sectors[saSelector->currIndex]->SetXCenter(newCenter);
		}
		
	}
	++frame;
}

void MapSelector::UpdateAllInfo()
{
	SaveFile *sf = mainMenu->GetCurrentProgress();
	World & w = sf->worlds[worldIndex];

	MapSector *mSector;
	//Vector2f bottomCenter = bottomBG.getPosition() + Vector2f(bottomBG.getLocalBounds().width / 2, bottomBG.getLocalBounds().height / 2);

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	

	if ((sectors != NULL && numSectors != w.numSectors))
	{
		for (int i = 0; i < numSectors; ++i)
		{
			delete sectors[i];
		}
		delete[] sectors;
		sectors = NULL;
		
		delete saSelector;
		saSelector = NULL;
		
	}

	if (sectors == NULL )
	{
		numSectors = w.numSectors;
		sectors = new MapSector*[numSectors];
		
		for (int i = 0; i < numSectors; ++i)
		{
			mSector = new MapSector(this, i);
			//load sectors externally
			sectors[i] = mSector;
			mSector->Init(&(w.sectors[i]));
		}

		saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, numSectors, 0);
	}
	else
	{
		for (int i = 0; i < numSectors; ++i)
		{
			sectors[i]->Init(&(w.sectors[i]));
		}
	}

	
	//int numLevelsInWorld = 7;
	//if (saSelector != NULL)
		
	
	//saSelector->totalItems = numLevels;

}

MapSector::MapSector(MapSelector *p_ms, int index )
	:numLevels(-1), ms(p_ms), sectorIndex( index )
{
	unlockedIndex = -1;
	numUnlockConditions = -1;
	nodeSize = 80;
	pathLen = 16;
	frame = 0;
	nodes = NULL;
	paths = NULL;
	saSelector = NULL;
	unlockCondText = NULL;
	stringstream ss;
	ss.str("");
	ss << "WorldMap/mapthumb_w" << ms->worldIndex + 1 << "_" << sectorIndex + 1 << "_256x256.png";
	ts_thumb = ms->mainMenu->tilesetManager.GetTileset(ss.str(), 256, 256);
	assert(ts_thumb != NULL);

	ts_shards = ms->mainMenu->tilesetManager.GetTileset("Menu/shards_48x48.png", 48, 48);

	thumbnail.setTexture(*ts_thumb->texture);
	thumbnail.setOrigin(thumbnail.getLocalBounds().width / 2, thumbnail.getLocalBounds().height / 2);
	

	SetRectColor(levelBG, Color(100, 100, 100, 100));
	SetRectColor(statsBG, Color(100, 100, 100, 100));
	SetRectColor(sectorStatsBG, Color(100, 100, 100, 100));


	ts_energyCircle = ms->mainMenu->tilesetManager.GetTileset("WorldMap/node_energy_circle_80x80.png", 80, 80); 
	ts_energyTri = ms->mainMenu->tilesetManager.GetTileset("WorldMap/node_energy_tri_80x80.png", 80, 80);
	ts_energyMask = ms->mainMenu->tilesetManager.GetTileset("WorldMap/node_energy_mask_80x80.png", 80, 80);
	//SetRectCenter( shoulderIcons, 
	nodeHighlight.setTexture(*ms->ts_node->texture);

	endSpr.setTexture(*ms->ts_sectorKey->texture);
	endSpr.setTextureRect(ms->ts_sectorKey->GetSubRect(0));
	endSpr.setOrigin(endSpr.getLocalBounds().width / 2, endSpr.getLocalBounds().height / 2);
	//SetXCenter(960);
}

void MapSector::Draw(sf::RenderTarget *target)
{
	target->draw(sectorNameText);
	target->draw(levelBG, 4, sf::Quads);
	target->draw(statsBG, 4, sf::Quads);
	target->draw(sectorStatsBG, 4, sf::Quads);

	for (int i = 0; i < numLevels; ++i)
	{
		if (sec->levels[i].GetComplete() )
		{
			target->draw(paths[i]);
		}
	}
	//target->draw( paths, numLevels )
	if (sec->IsUnlocked()) //just for testing
	{
		if (ms->state == MapSelector::S_IDLE)
		{
			target->draw(nodeEnergy);
		}
		DrawStats(target);
		DrawLevelStats(target);
		for (int i = 0; i < numLevels; ++i)
		{
			target->draw(topBonusNodes[i]);
			target->draw(nodes[i]);
			target->draw(botBonusNodes[i]);
		}

		target->draw(endSpr);

		if (ms->state == MapSelector::S_IDLE)
		{
			target->draw(nodeHighlight);
		}
		//target->draw(nodeHighlight);
		//target->draw(nodes, numLevels * 4 * 3, sf::Quads, ms->ts_node->texture);
	}
	else
	{
		DrawUnlockConditions(target);
	}

	target->draw(thumbnail);
	
}

void MapSector::DrawLevelStats(sf::RenderTarget *target)
{
	target->draw(levelCollectedShards, 4 * 16, sf::Quads, ts_shards->texture);
	target->draw(levelPercentCompleteText);
}

void MapSector::SetXCenter( float x )
{
	xCenter = x;

	left = Vector2f(xCenter, ms->sectorCenter.y);
	int numLevelsPlus = numLevels + 1;
	if (numLevelsPlus % 2 == 0)
	{
		left.x -= pathLen / 2 + nodeSize + (pathLen + nodeSize) * (numLevelsPlus / 2 - 1);
	}
	else
	{
		left.x -= nodeSize / 2 + (pathLen + nodeSize) * (numLevelsPlus / 2);
	}


	
	thumbnail.setPosition(Vector2f(x - 150, ms->sectorCenter.y - 325));
	sectorNameText.setPosition(x, 50);
	Vector2f sectorStatsCenter = Vector2f(x + 150, ms->sectorCenter.y - 325);
	Vector2f sectorStatsSize(256, 256);

	Vector2f levelStatsSize(500, 256);
	Vector2f levelStatsCenter = Vector2f(x, ms->sectorCenter.y + 300);
	Vector2f levelStatsTopLeft = levelStatsCenter - Vector2f( levelStatsSize.x / 2, levelStatsSize.y / 2 );
	SetRectCenter(sectorStatsBG, sectorStatsSize.x, sectorStatsSize.y, sectorStatsCenter );
	SetRectCenter(levelBG, 700, 256, Vector2f( x, ms->sectorCenter.y ));
	SetRectCenter(statsBG, levelStatsSize.x, levelStatsSize.y, levelStatsCenter );
	
	Vector2f sectorStatsTopLeft(sectorStatsCenter.x - sectorStatsSize.x/2, sectorStatsCenter.y - sectorStatsSize.y/2);

	int numUnlock = sec->numUnlockConditions;
	for (int i = 0; i < numUnlock; ++i)
	{
		unlockCondText[i].setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * i);
	}

	shardsCollectedText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * 0);
	completionPercentText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * 1);

	

	Vector2f shardGridOffset;
	Vector2f gridTopLeft = levelStatsTopLeft + shardGridOffset;
	float gridSpacing = 20;

	float gridTotalRight = gridTopLeft.x + (ts_shards->tileWidth * 4) + (gridSpacing * 3);
	
	for (int i = 0; i < 16; ++i)
	{
		SetRectCenter(levelCollectedShards + i * 4, 48, 48, Vector2f(gridTopLeft.x + (ts_shards->tileWidth + gridSpacing ) * (i % 4) + ts_shards->tileWidth / 2.f,
			gridTopLeft.y + ( ts_shards->tileHeight + gridSpacing ) * (i / 4) + ts_shards->tileHeight / 2.f));
	}

	Vector2f levelStatsActualTopLeft(gridTotalRight, gridTopLeft.y);

	levelPercentCompleteText.setPosition(levelStatsActualTopLeft + Vector2f(50, 50));

	
	UpdateNodePosition();
}

void MapSector::DrawStats(sf::RenderTarget *target)
{
	target->draw(completionPercentText);
	target->draw(shardsCollectedText);
}

void MapSector::DrawUnlockConditions(sf::RenderTarget *target)
{
	int numUnlock = sec->numUnlockConditions;
	for (int i = 0; i < numUnlock; ++i)
	{
		target->draw(unlockCondText[i]);
	}
}

void MapSector::UpdateStats()
{
	stringstream ss;

	int numTotalShards = sec->GetNumTotalShards();
	int numShardsCaptured = sec->GetNumShardsCaptured();

	ss << numShardsCaptured << " / " << numTotalShards;

	shardsCollectedText.setString(ss.str());

	ss.str("");

	int percent = floor(sec->GetCompletionPercentage());

	ss << percent << "%";

	completionPercentText.setString(ss.str());
}

void MapSector::UpdateLevelStats()
{

	for (int i = 0; i < 16; ++i)
	{
		levelCollectedShards[i * 4 + 0].texCoords = Vector2f(0, 0);
		levelCollectedShards[i * 4 + 1].texCoords = Vector2f(0, 0);
		levelCollectedShards[i * 4 + 2].texCoords = Vector2f(0, 0);
		levelCollectedShards[i * 4 + 3].texCoords = Vector2f(0, 0);
		SetRectSubRect(levelCollectedShards + i * 4, ts_shards->GetSubRect(20));
	}

	int gridIndex = 0;
	int uncaptured = 0;
	auto &snList = sec->levels[saSelector->currIndex].shardNameList;
	for (auto it = snList.begin(); it != snList.end(); ++it)
	{
		ShardType sType = Shard::GetShardType((*it));
		if (sec->world->sf->ShardIsCaptured(sType))
		{
			SetRectSubRect(levelCollectedShards + gridIndex * 4, ts_shards->GetSubRect(sType));
		}
		else
		{
			SetRectSubRect(levelCollectedShards + gridIndex * 4, ts_shards->GetSubRect(40));
		}
		++gridIndex;
	}

	stringstream ss;
	int percent = floor(sec->levels[saSelector->currIndex].GetCompletionPercentage());

	ss << percent << "%";

	levelPercentCompleteText.setString(ss.str());
	/*for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{

	}*/
}

sf::Vector2f MapSector::GetNodePos(int node)
{
	return left + Vector2f(nodeSize / 2, 0) + Vector2f((pathLen + nodeSize) * node, 0);
}

sf::Vector2f MapSector::GetTopNodePos(int n)
{
	Vector2f res = GetNodePos(n);
	res.y -= pathLen + nodeSize;
	return res;
}

sf::Vector2f MapSector::GetBotNodePos(int n)
{
	Vector2f res = GetNodePos(n);
	res.y += pathLen + nodeSize;
	return res;
}

void MapSector::UpdateNodePosition()
{
	for (int i = 0; i < numLevels; ++i)
	{
		topBonusNodes[i].setPosition(GetTopNodePos(i));
		nodes[i].setPosition(GetNodePos(i));
		botBonusNodes[i].setPosition(GetBotNodePos(i));
	}

	for (int i = 0; i < numLevels; ++i)
	{
		paths[i].setPosition(nodes[i].getPosition().x 
			+ (nodeSize / 2 + pathLen / 2), nodes[i].getPosition().y);
	}

	endSpr.setPosition(GetNodePos(numLevels));
	//endSpr.
}

void MapSector::Update(ControllerState &curr,
	ControllerState &prev)
{
	int unlockedLevelCount = numLevels;
	for (int i = 0; i < numLevels-1; ++i)
	{
		if (!sec->levels[i].GetComplete())
		{
			unlockedLevelCount = max( 1, i+1);
		}
	}

	if (unlockedLevelCount != saSelector->totalItems)
	{
		saSelector->SetTotalSize(unlockedLevelCount);
		UpdateNodes();
	}
	
	/*if (state == NORMAL && sec->IsComplete() )
	{
		state = JUSTCOMPLETE;
		stateFrame = 0;
	}*/

	if (state == NORMAL)
	{
		for (int i = 0; i < numLevels; ++i)
		{
			if (sec->levels[i].justBeaten)
			{
				if (i < numLevels - 1)
				{
					state = LEVELJUSTCOMPLETE;
				}
				else
				{
					state = JUSTCOMPLETE;
				}				
				stateFrame = 0;
				unlockedIndex = i;//saSelector->currIndex;
				//unlockFrame = frame;
				sec->levels[i].justBeaten = false;
				break;
			}
		}
	}

	if (state != EXPLODECOMPLETE)
	{
		int old = saSelector->currIndex;

		bool left = curr.LLeft();
		bool right = curr.LRight();

		int changed = saSelector->UpdateIndex(left, right);

		if (changed != 0)
		{
			UpdateLevelStats();
		}

		int oldYIndex = selectedYIndex;

		if ((selectedYIndex == 0 && !HasTopBonus(saSelector->currIndex)
			|| (selectedYIndex == 2 && !HasBotBonus(saSelector->currIndex))))
		{
			selectedYIndex = 1;
		}

		if (curr.LUp() && !prev.LUp())
		{
			if (selectedYIndex == 2 || (selectedYIndex == 1 && HasTopBonus(saSelector->currIndex)))
			{
				--selectedYIndex;
			}
		}
		else if (curr.LDown() && !prev.LDown())
		{
			if (selectedYIndex == 0 || (selectedYIndex == 1 && HasBotBonus(saSelector->currIndex)))
			{
				++selectedYIndex;
			}
		}
		if (changed != 0 || oldYIndex != selectedYIndex)
		{
			UpdateNodes();
			//UpdateNodes();
			//Vertex *n = (nodes + selectedYIndex * numLevels * 4 + old * 4);
			//	SetRectSubRect( n, )
		}

		if (curr.A && !prev.A)
		{
			if (selectedYIndex == 1)
			{
				//string levelPath = sec->levels[saSelector->currIndex].GetFullName();// name;
				//View oldView = ms->mainMenu->window->getView();

				//GameSession *gs = new GameSession(NULL, ms->mainMenu, levelPath);

				//ms->mainMenu->menuMode = MainMenu::LOADINGMAP;

				//ms->loadThread = new boost::thread(GameSession::sLoad, gs);

				//GameSession *gs = new GameSession(NULL, ms->mainMenu, level);
				//GameSession::sLoad(gs);
				//int result = gs->Run();

				//delete gs;

				//ms->mainMenu->window->setView(oldView);
				if (sec->IsComplete())
				{
					state = COMPLETE;
				}
				else
				{
					state = NORMAL;
				}
				ms->mainMenu->AdventureLoadLevel(&(sec->levels[saSelector->currIndex]));
			}
		}
	}

	

	
	int n = GetNodeSubIndex(saSelector->currIndex);
	
	int enAnimFactor = 6;
	if (n % 3 == 0)
	{
		nodeEnergy.setTexture(*ts_energyCircle->texture);
		nodeEnergy.setTextureRect(ts_energyCircle->GetSubRect((frame / enAnimFactor) % 10));
		nodeHighlight.setTextureRect(ms->ts_node->GetSubRect(9));
	}
	else if (n % 3 == 1)
	{
		nodeEnergy.setTexture(*ts_energyTri->texture);
		nodeEnergy.setTextureRect(ts_energyTri->GetSubRect((frame / enAnimFactor) % 15));
		nodeHighlight.setTextureRect(ms->ts_node->GetSubRect(10));
	}
	else if (n % 3 == 2)
	{
		nodeEnergy.setTexture(*ts_energyMask->texture);
		nodeEnergy.setTextureRect(ts_energyCircle->GetSubRect((frame / enAnimFactor) % 20));
		nodeHighlight.setTextureRect(ms->ts_node->GetSubRect(11));
	}
	nodeEnergy.setOrigin(nodeEnergy.getLocalBounds().width / 2, nodeEnergy.getLocalBounds().height / 2);
	nodeHighlight.setOrigin(nodeHighlight.getLocalBounds().width / 2, nodeHighlight.getLocalBounds().height / 2);
	if (selectedYIndex == 0)
	{
		nodeEnergy.setPosition(GetTopNodePos(saSelector->currIndex));
	}
	else if (selectedYIndex == 1)
	{
		nodeEnergy.setPosition(GetNodePos(saSelector->currIndex));
	}
	else
	{
		nodeEnergy.setPosition(GetBotNodePos(saSelector->currIndex));
	}
	nodeHighlight.setPosition(nodeEnergy.getPosition());

	int breathe = 180;
	float trans = (float)(frame%breathe) / (breathe/2);
	if (trans > 1)
	{
		trans = 2.f - trans;
	}
	nodeHighlight.setColor(Color(255, 255, 255, 255 * trans));

	if (state == LEVELJUSTCOMPLETE || state == JUSTCOMPLETE )//unlockedIndex != -1)
	{
		int ff = stateFrame / 7;
		if (ff == 16)
		{
			if (state == LEVELJUSTCOMPLETE)
				state = NORMAL;
			else
			{
				state = EXPLODECOMPLETE;
			}
			stateFrame = 0;
		}
		else
		{
			paths[unlockedIndex].setTextureRect(ms->ts_path->GetSubRect(ff));
		}
		
	}

	if( state == EXPLODECOMPLETE )
	{
		int explodeFactor = 4;
		if (stateFrame == 16 * explodeFactor)
		{
			state = COMPLETE;
			stateFrame = 0;
			endSpr.setTexture(*ms->ts_sectorKey->texture);
			endSpr.setTextureRect(ms->ts_sectorKey->GetSubRect(0));
			endSpr.setOrigin(endSpr.getLocalBounds().width / 2, endSpr.getLocalBounds().width / 2);
		}
		else
		{
			if (stateFrame == 0)
			{ 
				endSpr.setTexture(*ms->ts_sectorOpen[0]->texture);
			}
			

			endSpr.setTextureRect(ms->ts_sectorOpen[0]->GetSubRect(stateFrame / explodeFactor));
			endSpr.setOrigin(endSpr.getLocalBounds().width / 2, endSpr.getLocalBounds().width / 2);
		}
	}

	++frame;
	++stateFrame;
}

bool MapSector::HasTopBonus(int node)
{
	return sec->HasTopBonus(node);
}
bool MapSector::HasBotBonus(int node)
{
	return sec->HasTopBonus(node);
}

void MapSector::UpdateNodes()
{
	for (int i = 0; i < numLevels; ++i)
	{
		//Vertex *n = (nodes + 1 * numLevels * 4 + i * 4);
		//Vertex *nTop = (nodes + 0 * numLevels * 4 + i * 4);
		//Vertex *nBot = (nodes + 2 * numLevels * 4 + i * 4);
		if (HasTopBonus(i) && sec->levels[i].TopBonusUnlocked())
		{
			//SetRectSubRect(nTop, ms->ts_node->GetSubRect(GetNodeBonusIndexTop(i)));
			//SetRectColor(nTop, Color(Color::White));
			topBonusNodes[i].setTextureRect(ms->ts_node->GetSubRect(GetNodeBonusIndexTop(i)));
			topBonusNodes[i].setColor(Color::White);
		}
		else
		{
			topBonusNodes[i].setColor(Color::Transparent);
		}

		if (HasBotBonus(i) && sec->levels[i].BottomBonusUnlocked())
		{
			botBonusNodes[i].setTextureRect(ms->ts_node->GetSubRect(GetNodeBonusIndexBot(i)));
			
			botBonusNodes[i].setColor(Color::White);
		}
		else
		{
			botBonusNodes[i].setColor(Color::Transparent);
		}

		nodes[i].setTextureRect(ms->ts_node->GetSubRect(GetNodeSubIndex(i)));
		//SetRectSubRect(n, ms->ts_node->GetSubRect(GetNodeSubIndex(i)));
	}
}


void MapSector::Load()
{

}

int MapSector::GetNodeSubIndex(int node)
{
	if (!sec->IsLevelUnlocked( node ) )
	{
		return 0;
	}
	else
	{
		if (selectedYIndex == 1 && saSelector->currIndex == node)
		{
			if (sec->levels[node].IsOneHundredPercent())
			{
				return 8;
			}
			else if (sec->levels[node].GetComplete())
			{
				return 7;
			}
			else
			{
				return 6;
			}
		}
		else
		{
			if (sec->levels[node].IsOneHundredPercent())
			{
				return 5;
			}
			else if (sec->levels[node].GetComplete())
			{
				return 4;
			}
			else
			{
				return 3;
			}
		}
	}

	/*if (sec->levels[node].completed)
	{
		return 1;
	}
	else
	{
		return 0;
	}*/
}

void MapSector::UpdateUnlockConditions()
{
	int numUnlock = sec->numUnlockConditions;
	stringstream ss;
	for (int i = 0; i < numUnlock; ++i)
	{
		ss.str("");
		int completeOfType = sec->world->GetNumSectorTypeComplete(sec->conditions[i]);
		int numNeededOfType = sec->numTypesNeeded[i];
		ss << completeOfType << " / " << numNeededOfType;
		unlockCondText[i].setString(ss.str());
	}
}

int MapSector::GetNodeBonusIndexTop(int node)
{
	if (selectedYIndex == 0 && saSelector->currIndex == node)
	{
		return 0;
	}
	else
	{
		return 0;
	}
	/*if (sec->levels[node].completed)
	{
		return 64;
	}
	else
	{
		return 65;
	}*/
}

int MapSector::GetNodeBonusIndexBot(int node)
{
	if (selectedYIndex == 2 && saSelector->currIndex == node)
	{
		return 0;
	}
	else
	{
		return 0;
	}
}

void MapSector::Init(Sector *m_sec)
{
	unlockedIndex = -1;
	sec = m_sec;
	numLevels = sec->numLevels;

	stringstream ss;
	ss << "Sector " << sec->index+1;

	sectorNameText.setString(ss.str());

	int waitFrames[3] = { 30, 10, 5 };
	int waitModeThresh[2] = { 2, 2 };

	bool blank = saSelector == NULL;
	bool diffNumLevels = false;
	if (saSelector != NULL && saSelector->totalItems != numLevels)
	{
		diffNumLevels = true;
	}

	if (diffNumLevels)
	{
		delete saSelector;
		saSelector = NULL;
	}
	
	if( saSelector == NULL )
		saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, numLevels, 0);

	selectedYIndex = 1;

	if (diffNumLevels)
	{
		delete[] nodes;
		nodes = NULL;

		delete[] paths;
		paths = NULL;

		delete[] topBonusNodes;
		topBonusNodes = NULL;

		delete[] botBonusNodes;
		botBonusNodes = NULL;
	}
	if (nodes == NULL)
	{
		nodes = new Sprite[numLevels];
		paths = new Sprite[numLevels];
		topBonusNodes = new Sprite[numLevels];
		botBonusNodes = new Sprite[numLevels];

		for (int i = 0; i < numLevels; ++i)
		{
			nodes[i].setTexture(*ms->ts_node->texture);
			nodes[i].setTextureRect(ms->ts_node->GetSubRect(0));
			nodes[i].setOrigin(nodes[i].getLocalBounds().width / 2, nodes[i].getLocalBounds().height / 2);
			topBonusNodes[i].setTexture(*ms->ts_node->texture);
			topBonusNodes[i].setTextureRect(ms->ts_node->GetSubRect(0));
			topBonusNodes[i].setOrigin(topBonusNodes[i].getLocalBounds().width / 2, topBonusNodes[i].getLocalBounds().height / 2);
			botBonusNodes[i].setTexture(*ms->ts_node->texture);
			botBonusNodes[i].setTextureRect(ms->ts_node->GetSubRect(0));
			botBonusNodes[i].setOrigin(botBonusNodes[i].getLocalBounds().width / 2, botBonusNodes[i].getLocalBounds().height / 2);
		}

		for (int i = 0; i < numLevels; ++i)
		{
			paths[i].setTexture(*ms->ts_path->texture);
			paths[i].setTextureRect(ms->ts_path->GetSubRect(15));
			paths[i].setOrigin(nodes[i].getLocalBounds().width / 2, nodes[i].getLocalBounds().height / 2);
		}
	}

	if (numUnlockConditions != -1 && sec->numUnlockConditions != numUnlockConditions)
	{
		delete[] unlockCondText;
		unlockCondText = NULL;
	}

	if (unlockCondText == NULL)
	{
		numUnlockConditions = sec->numUnlockConditions;
		unlockCondText = new Text[numUnlockConditions];

		for (int i = 0; i < numUnlockConditions; ++i)
		{
			unlockCondText[i].setFont(ms->mainMenu->arial);
			unlockCondText[i].setCharacterSize(40);
			unlockCondText[i].setFillColor(Color::White);
		}
	}
	
	if (blank)
	{
		shardsCollectedText.setFont(ms->mainMenu->arial);
		completionPercentText.setFont(ms->mainMenu->arial);

		shardsCollectedText.setCharacterSize(40);
		completionPercentText.setCharacterSize(40);

		shardsCollectedText.setFillColor(Color::White);
		completionPercentText.setFillColor(Color::White);


		levelPercentCompleteText.setFillColor(Color::White);
		levelPercentCompleteText.setCharacterSize(40);
		levelPercentCompleteText.setFont(ms->mainMenu->arial);

		sectorNameText.setFillColor(Color::White);
		sectorNameText.setCharacterSize(40);
		sectorNameText.setFont(ms->mainMenu->arial);
		sectorNameText.setOrigin(sectorNameText.getLocalBounds().width / 2, 0);
	}

	if (sec->IsComplete())
	{
		state = COMPLETE;
	}
	else
	{
		state = NORMAL;
	}

	SetXCenter(960);
	UpdateNodes();
	UpdateUnlockConditions();
	UpdateStats();
	UpdateLevelStats();
}