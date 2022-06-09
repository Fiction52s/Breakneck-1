#include "Sequence.h"
#include <fstream>
#include <assert.h>
#include "GameSession.h"


using namespace std;
using namespace sf;

Cutscene::Cutscene( GameSession *own, sf::Vector2i &og )
	:owner( own ), init( false )
{
	origin = Vector2f( og.x, og.y );
}

bool Cutscene::LoadFromFile( const std::string &path )
{
	ifstream is;
	is.open( path );

	if( !is.is_open() )
	{
		assert( false );
		return false;
	}

	int numTilesets;
	is >> numTilesets;
	tilesets = new Tileset*[numTilesets];

	for( int i = 0; i < numTilesets; ++i )
	{
		string sourceName;
		is >> sourceName;
		int tileX, tileY;
		is >> tileX;
		is >> tileY;

		tilesets[i] = owner->GetTileset( sourceName, tileX, tileY );
	}

	int numTotalFrames;
	is >> numTotalFrames;
	totalFrames = numTotalFrames;

	cameras = new View[numTotalFrames];
	activeSprites = new list<Sprite>*[numTotalFrames];

	for( int i = 0; i < numTotalFrames; ++i )
	{
		activeSprites[i] = NULL;

		int centerX, centerY, zoomLevel, angleLevel;
		is >> centerX >> centerY >> zoomLevel >> angleLevel;

		cameras[i].setCenter( Vector2f( centerX + origin.x, centerY + origin.y ) );

		int fac = 2;
		cameras[i].setRotation( angleLevel * 2 );
		cameras[i].setSize( 192 / 4 * zoomLevel, 108 / 4 * zoomLevel );
		//cameras[i].zoomLevel = zoomLevel;
		//cameras[i].angleLevel = angleLevel;
		//ci.view.setRotation
		//cout << "here" << endl;
	}
	
	int numEntities;
	is >> numEntities;


	//map<int, sf::Sprite> * entities = new map<int, sf::Sprite>[numEntities];
	map<int, list<Sprite>> entities;
	for( int i = 0; i < numEntities; ++i )
	{ 
		string typeName;
		is >> typeName;

		int numActiveFrames;
		is >> numActiveFrames;

		for( int j = 0; j < numActiveFrames; ++j )
		{
			int trueFrame, tsIndex, tsFrame, facingRight, xPos, yPos;
			float rotation;
			float xScale, yScale;
			is >> trueFrame >> tsIndex >> tsFrame >> facingRight 
				>> xPos >> yPos >> rotation >> xScale >> yScale;

			Sprite sp;
			Tileset *ts = tilesets[tsIndex];
			sp.setTexture( *ts->texture );

			IntRect ir = ts->GetSubRect( tsFrame );
			if( !facingRight )
			{
				ir.left += ir.width;
				ir.width = -ir.width;
			}

			sp.setTextureRect( ir );
			sp.setScale( xScale, yScale );
			sp.setRotation( rotation );
			sp.setPosition( xPos + origin.x, yPos + origin.y );

			sp.setOrigin( sp.getLocalBounds().width / 2, sp.getLocalBounds().height / 2 );

			if( activeSprites[trueFrame-1] == NULL )
			{
				activeSprites[trueFrame-1] = new list<Sprite>;
			}

			activeSprites[trueFrame-1]->push_back( sp );
		}
	}

	init = true;
	return true;
}

void Cutscene::Draw( RenderTarget *target, int frame )
{
	list<Sprite> *sprList = activeSprites[frame];
	if( sprList != NULL )
	{
		list<Sprite> &sprites = *sprList;
		for( list<Sprite>::iterator it = sprites.begin(); it != sprites.end(); ++it )
		{
			target->draw( (*it) );
		}
	}
}

sf::View & Cutscene::GetView( int frame )
{
	return cameras[frame];
}

Cutscene::~Cutscene()
{
	if( init )
	{
		delete [] tilesets; //dont need to delete the tilesets themselves
		delete [] cameras;
		for( int i = 0; i < totalFrames; ++i )
		{
			delete activeSprites[i];
		}
		delete [] activeSprites;
	}
}