#include "WorldMap.h"

#include "GameSession.h"
#include <sstream>
#include <iostream>

using namespace sf;
using namespace std;

WorldMap::WorldMap()
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
	back.setOrigin( 1920 / 2, 1080 / 2 );
	front.setPosition( 0, 0 );
	front.setOrigin( 1920 / 2, 1080 / 2 );
	selectedColony = 1;
}

void WorldMap::Reset()
{
	state = OFF;
	frame = 0;
	selectedColony = 1;
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
}

void WorldMap::Update()
{
	
	if( state == OFF )
		return;


	int trans = 40;
	switch( state )
	{
	case PLANET_AND_SPACE:
		if( frame == trans )
		{
			state = PLANET_TRANSITION;
			frame = 0;
		}
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
			break;
		}
	}

	if( state != COLONY )
	{
		//cout << "a: " << (int)front.getColor().a << endl;
		//cout << "frame: " << frame << ", state: " << (int)state << endl;
	}
	++frame;
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
}