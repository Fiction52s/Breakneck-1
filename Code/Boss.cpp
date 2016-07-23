#include "Boss.h"
#include <iostream>
#include <assert.h>
#include "GameSession.h"

using namespace std;
using namespace sf;



PortraitBox::PortraitBox()
	:state( CLOSED ), frame( 0 ), scaleMultiple( 1 )
{
	openLength = 30;
	closeLength = 30;
}

void PortraitBox::Reset()
{
	//scaleMultiple = 1;
	frame = 0;
	state = CLOSED;
}

void PortraitBox::Open()
{
	assert( state == CLOSED );
	frame = 0;
	state = OPENING;
}

void PortraitBox::Close()
{
	assert( state == OPEN );
	frame = 0;
	state = CLOSING;
}

void PortraitBox::Draw( sf::RenderTarget *target )
{
	if( state != CLOSED )
	{
		target->draw( sprite );
	}
}

void PortraitBox::SetSprite( Tileset *ts, int frame )
{
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, 
		sprite.getLocalBounds().height / 2 );
}

void PortraitBox::Update()
{
	switch( state )
	{
	case OPENING:
		if( frame == openLength )
		{
			state = OPEN;
			frame = 0;
		}
		break;
	case OPEN:
		return;
		//frame = 0;
		break;
	case CLOSING:
		if( frame == closeLength )
		{
			state = CLOSED;
			frame = 0;
		}
		break;
	case CLOSED:
		return;
		//frame = 0;
		break;
	}

	switch( state )
	{
	case OPENING:
		{
			CubicBezier bez( .71,.14,.83,.67 );
			//CubicBezier bez( 0,0,1,1 );
			double v = bez.GetValue( frame / (double)(openLength-1) );
			float r = v;
			float yScale = .1 * (1-r) + 1 * (r);
			yScale *= scaleMultiple;
			sprite.setScale( scaleMultiple, yScale );
			//cout << "yscale: " << yScale << endl;
		}
		break;
	case OPEN:
		break;
	case CLOSING:
		{
			CubicBezier bez( 0, 0, 1, 1 );
			double v = bez.GetValue( frame / (double)(closeLength-1) );
			float r = v;
			float yScale = .1 * (r) + 1 * (1-r);
			yScale *= scaleMultiple;
			sprite.setScale( scaleMultiple, yScale );
		}
		break;
	case CLOSED:
		break;
	}

	++frame;
}

void PortraitBox::SetPosition( float x, float y )
{
	sprite.setPosition( x, y );
}

void PortraitBox::SetPosition( Vector2f &p )
{
	sprite.setPosition( p );
}

//make the symbol on top a pixel shader so the HOLDING animation can affect the image?

DialogueBox::DialogueBox( GameSession *owner, Type t )
	:state( CLOSED ), frame( 0 )
{
	ts_dialog = NULL;
	type = t;
	switch( type )
	{
	case CRAWLER:
		//ts_dialog = owner->GetTileset
		break;
	case BIRD:
		ts_dialog = owner->GetTileset( "Bosses/Dialogue/02_Bird_Dialogue_256x256.png", 256, 256 );
		break;
	case COYOTE:
		break;
	case TIGER:
		break;
	case GATOR:
		break;
	case SKELETON:
		break;
	default:
		assert( false );
		break;
	}

	assert( ts_dialog != NULL );

	sprite.setTexture( *ts_dialog->texture );
	sprite.setTextureRect( ts_dialog->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );

	openingLength = 12;
	closingLength = 12;
	openLength = 1;

	openingFactor = 2;
	closingFactor = 2;
	openFactor = 1;

	symbols = NULL;

	//ts_dialog = owner->GetTileset( 
}

void DialogueBox::SetSymbols( list<SymbolInfo> *sList ) //Tileset *ts, int frame )
{
	symbols = sList;
	numSymbols = symbols->size();
	
	currSymbol = 0;
	symbolFrame = 0;

	sit = symbols->begin();
	//symbolSprite.setTexture( *ts->texture );
	//symbolSprite.setTextureRect( ts->GetSubRect( frame ) );
	//symbolSprite.setOrigin( symbolSprite.getLocalBounds().width / 2, 
	//	symbolSprite.getLocalBounds().height / 2 );
}

void DialogueBox::SetPosition( float x, float y )
{
	sprite.setPosition( x, y );
	sprite.setScale( .5, .5 );
	symbolSprite.setPosition( x, y );
	symbolSprite.setScale( .5, .5 );
}

void DialogueBox::SetPosition( sf::Vector2f &p )
{
	sprite.setPosition( p );
	sprite.setScale( .5, .5 );
	symbolSprite.setPosition( p );
	symbolSprite.setScale( .5, .5 );
}

void DialogueBox::UpdateSymbol()
{
	if( symbols == NULL )
	{
		return;
	}

	if( symbolFrame == (*sit).framesHold )
	{
		symbolFrame = 0;
		++currSymbol;
		if( currSymbol == numSymbols )
		{
			currSymbol = 0;
			sit = symbols->begin();
		}
		else
		{
			++sit;
		}
	}

	//cout << "symbolframe: " << symbolFrame << endl;
	symbolSprite.setTexture( *(*sit).ts->texture );
	symbolSprite.setTextureRect( (*sit).ts->GetSubRect( (*sit).frame ) );
	symbolSprite.setOrigin( symbolSprite.getLocalBounds().width / 2, 
		symbolSprite.getLocalBounds().height / 2 );
	
	++symbolFrame;
}

void DialogueBox::Draw( sf::RenderTarget *target )
{
	if( state != CLOSED )
	{
		target->draw( sprite );		
	}

	if( state == OPEN )
	{
		target->draw( symbolSprite );
	}
}

void DialogueBox::Open()
{
	assert( state == CLOSED );
	state = OPENING;
	frame = 0;
}

void DialogueBox::Close()
{
	assert( state == OPEN );
	state = CLOSING;
	frame = 0;
}

void DialogueBox::Update()
{
	switch( state )
	{
	case OPENING:
		if( frame == openingLength * openingFactor )
		{
			state = OPEN;
			frame = 0;
		}
		break;
	case OPEN:
		if( frame == openLength )
		{
			frame = 0;
		}
		//frame = 0;
		break;
	case CLOSING:
		if( frame == closingLength * closingFactor )
		{
			state = CLOSED;
			frame = 0;
		}
		break;
	case CLOSED:
		return;
		//frame = 0;
		break;
	}

	switch( state )
	{
	case OPENING:
		{
			sprite.setTextureRect( ts_dialog->GetSubRect( frame / openingFactor ) );
			//cout << "frame" << endl;
			//CubicBezier bez( .71,.14,.83,.67 );
			////CubicBezier bez( 0,0,1,1 );
			//double v = bez.GetValue( frame / (double)(openLength-1) );
			//float r = v;
			//float yScale = .1 * (1-r) + 1 * (r);
			//yScale *= scaleMultiple;
			//sprite.setScale( scaleMultiple, yScale );
			////cout << "yscale: " << yScale << endl;
		}
		break;
	case OPEN:
		{
			sprite.setTextureRect( ts_dialog->GetSubRect( 13 ) ); 
		}
		break;
	case CLOSING:
		{
			sprite.setTextureRect( ts_dialog->GetSubRect( (13 * openingFactor - frame) / openingFactor ) ); 
			/*CubicBezier bez( 0, 0, 1, 1 );
			double v = bez.GetValue( frame / (double)(closeLength-1) );
			float r = v;
			float yScale = .1 * (r) + 1 * (1-r);
			yScale *= scaleMultiple;
			sprite.setScale( scaleMultiple, yScale );*/
		}
		break;
	case CLOSED:
		break;
	}

	

	if( state == OPEN )
	{
		UpdateSymbol();
	//	//symbolFrame++;
	}

	++frame;
}

