#include "ImageText.h"
#include <assert.h>

using namespace std;
using namespace sf;

ImageText::ImageText( int p_maxDigits, Tileset *ts_tex )
{
	maxDigits = p_maxDigits;
	ts = ts_tex;
	int numVertices = maxDigits * 4;
	vert = new Vertex[numVertices];
	value = 0;
	
	
	numShowZeroes = 0;
	activeDigits = 1;
	
}

void ImageText::UpdateSprite()
{
	for( int i = 0; i < maxDigits; ++i )
	{
		vert[i*4 + 0].position = Vector2f( -i * ts->tileWidth, 0 ) + topRight + Vector2f( -ts->tileWidth, 0 );
		vert[i*4 + 1].position = Vector2f( -i * ts->tileWidth, 0 ) + topRight;
		vert[i*4 + 2].position = Vector2f( -i * ts->tileWidth, 0 ) + topRight + Vector2f( 0, ts->tileHeight );
		vert[i*4 + 3].position = Vector2f( -i * ts->tileWidth, 0 ) + topRight + Vector2f( -ts->tileWidth, ts->tileHeight );
	}

	int div = 10;
	int val = value;
	int ind = 0;
	
	while( true )
	{
		int res = val % div;

		IntRect subRect = ts->GetSubRect( res );
		vert[ind*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		vert[ind*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		vert[ind*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		vert[ind*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

		++ind;

		val /= div;

		if( val == 0 )
		{
			break;
		}
	}

	for(; ind < numShowZeroes; ++ind )
	{
		IntRect subRect = ts->GetSubRect( 0 );
		vert[ind*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		vert[ind*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		vert[ind*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		vert[ind*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}

	activeDigits = ind;
}

void ImageText::Draw( sf::RenderTarget *target )
{
	target->draw( vert, activeDigits * 4, sf::Quads, ts->texture );
}

void ImageText::ShowZeroes( int numZ )
{
	assert( numZ <= maxDigits && numZ >= 0 );
	numShowZeroes = numZ;
}

void ImageText::SetNumber( int num )
{
	value = num;
}