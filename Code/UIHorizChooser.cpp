#include "UIWindow.h"
#include "GameSession.h"
#include "Tileset.h"
#include <string>

using namespace sf;
using namespace std;

UIBar::UIBar( UIControl *p_parent, TilesetManager *tsMan, sf::Font *f, int p_width, int p_height, int p_textHeight )
	:UIControl( p_parent, NULL, UI_BAR ), width( p_width ), textOffset( 10, 30 ), alignment( LEFT )
{
	sideWidth = 20;
	//depreciated
	ts_bar = NULL;//tsMan->GetTileset( "Menu/ui_bar_32x80.png", 32, 80 );
	//ts_bar = tsMan->GetTileset( "blahbar" );
	AssignTexture(0);
	SetTopLeft( 0, 0 );
	currText.setFont( *f );
	currText.setCharacterSize( p_textHeight );
	currText.setFillColor( Color::White );

	bState = BAR_UNFOCUSED;

	dimensions = Vector2f( p_width, p_height ); //50 is just a random constant

	SetState( BAR_UNFOCUSED );
}

void UIBar::SetTextHeight( int height )
{
	currText.setCharacterSize( height );
	SetTextAlignment( alignment, textOffset );
}

void UIBar::AssignTexture(int tileIndex )
{
	sf::IntRect irLeft = ts_bar->GetSubRect( tileIndex * 3 + 0 );
	sf::IntRect irMiddle = ts_bar->GetSubRect( tileIndex * 3 + 1 );
	sf::IntRect irRight = ts_bar->GetSubRect( tileIndex * 3 + 2 );
	

	/*barVA[LEFT*4+0].color = Color::Red;
	barVA[LEFT*4+1].color = Color::Red;
	barVA[LEFT*4+2].color = Color::Red;
	barVA[LEFT*4+3].color = Color::Red;*/

	barVA[LEFT*4+0].texCoords = Vector2f( irLeft.left, irLeft.top );
	barVA[LEFT*4+1].texCoords = Vector2f( irLeft.left + irLeft.width, irLeft.top );
	barVA[LEFT*4+2].texCoords = Vector2f( irLeft.left + irLeft.width, irLeft.top + irLeft.height );
	barVA[LEFT*4+3].texCoords = Vector2f( irLeft.left, irLeft.top + irLeft.height );

	/*barVA[RIGHT*4+0].color = Color::Blue;
	barVA[RIGHT*4+1].color = Color::Blue;
	barVA[RIGHT*4+2].color = Color::Blue;
	barVA[RIGHT*4+3].color = Color::Blue;*/

	barVA[RIGHT*4+0].texCoords = Vector2f( irRight.left, irRight.top );
	barVA[RIGHT*4+1].texCoords = Vector2f( irRight.left + irRight.width, irRight.top );
	barVA[RIGHT*4+2].texCoords = Vector2f( irRight.left + irRight.width, irRight.top + irRight.height );
	barVA[RIGHT*4+3].texCoords = Vector2f( irRight.left, irRight.top + irRight.height );

	barVA[MIDDLE*4+0].texCoords = Vector2f( irMiddle.left, irMiddle.top );
	barVA[MIDDLE*4+1].texCoords = Vector2f( irMiddle.left + irMiddle.width, irMiddle.top );
	barVA[MIDDLE*4+2].texCoords = Vector2f( irMiddle.left + irMiddle.width, irMiddle.top + irMiddle.height );
	barVA[MIDDLE*4+3].texCoords = Vector2f( irMiddle.left, irMiddle.top + irMiddle.height );
}

void UIBar::Focus()
{
	UIControl::Focus();
	SetState( BAR_FOCUSED );


	//temporary
}

void UIBar::Unfocus()
{
	UIControl::Unfocus();
	SetState( BAR_UNFOCUSED );

	//temporary
	
}

void UIBar::SetTopLeft( float x, float y )
{
	relTopLeft = Vector2f( x, y );

	if( parent != NULL )
	{
		Vector2f pTopLeft = parent->GetTopLeftGlobal();
		x += pTopLeft.x;
		y += pTopLeft.y;
	}
	//float tw = ts_bar->tileWidth;
	//float th = ts_bar->tileHeight;
	float tw = sideWidth;
	float th = dimensions.y;

	barVA[LEFT*4+0].position = Vector2f( x, y );
	barVA[LEFT*4+1].position = Vector2f( x + tw, y );
	barVA[LEFT*4+2].position = Vector2f( x + tw, y + th );
	barVA[LEFT*4+3].position = Vector2f( x, y + th );

	barVA[MIDDLE*4+0].position = barVA[LEFT*4+1].position;
	barVA[MIDDLE*4+1].position = barVA[LEFT*4+1].position + Vector2f( width - sideWidth * 2, 0 );
	barVA[MIDDLE*4+2].position = barVA[LEFT*4+2].position + Vector2f( width - sideWidth * 2, 0 );
	barVA[MIDDLE*4+3].position = barVA[LEFT*4+2].position;

	barVA[RIGHT*4+0].position = barVA[MIDDLE*4+1].position;
	barVA[RIGHT*4+1].position = barVA[MIDDLE*4+1].position + Vector2f( tw, 0 );
	barVA[RIGHT*4+2].position = barVA[MIDDLE*4+2].position + Vector2f( tw, 0 );
	barVA[RIGHT*4+3].position = barVA[MIDDLE*4+2].position;

	SetTextAlignment( alignment, textOffset );
}

void UIBar::SetText( const std::string &text )
{
	currText.setString( text );
	SetTextAlignment( alignment, textOffset );
}

void UIBar::SetText( const std::string &text, sf::Vector2i offset, Alignment align )
{
	currText.setString( text );
	SetTextAlignment( align, offset );
}

void UIBar::SetState( BarState state )
{
	bState = state;
	UpdateSprite();
}

const sf::String &UIBar::GetString()
{
	return currText.getString();
}

void UIBar::UpdateSprite()
{
	int sub = 0;
	switch( bState )
	{
	case BAR_UNFOCUSED:
		sub = 0;
		break;
	case BAR_FOCUSED:
		sub = 1;
		break;
	case BAR_ALT0:
		sub = 2;
		break;
	case BAR_ALT1:
		sub = 3;
		break;
	case BAR_ALT2:
		sub = 4;
		break;
	case BAR_ALT3:
		sub = 5;
		break;
	}

	AssignTexture( sub );

	//barVA[MIDDLE*4+0].color = currColor;
	//barVA[MIDDLE*4+1].color = currColor;
	//barVA[MIDDLE*4+2].color = currColor;
	//barVA[MIDDLE*4+3].color = currColor;
}

const sf::Vector2f &UIBar::GetTopLeftGlobal()
{
	return barVA[0].position;
}

void UIBar::SetTextAlignment( Alignment align, sf::Vector2i &offset )
{
	textOffset = offset;
	Vector2f off( textOffset.x, textOffset.y );
	alignment = align;
	switch( align )
	{
	case LEFT:
		{
			Vector2f topLeft = GetTopLeftGlobal() + off;
			currText.setOrigin(currText.getLocalBounds().left, currText.getLocalBounds().top);
			currText.setPosition( topLeft );
			break;
		}
	case MIDDLE:
		{
			Vector2f topMiddle = GetTopLeftGlobal() + Vector2f( width / 2, 0 ) + off;
			currText.setOrigin(currText.getLocalBounds().left + currText.getLocalBounds().width / 2, 0 );
			currText.setPosition( topMiddle );
			break;
		}
	case RIGHT:
		{
			Vector2f topRight = GetTopLeftGlobal() + Vector2f( width, 0 ) + off;
			currText.setOrigin(currText.getLocalBounds().left + currText.getLocalBounds().width, 0 );
			currText.setPosition( topRight);
			break;
		}
	}
}

void UIBar::Draw( sf::RenderTarget *target )
{
	//target->draw( barVA, 12, sf::Quads ); //ts_bar->texture
	target->draw( barVA, 12, sf::Quads, ts_bar->texture );
	target->draw( currText );
}

bool UIBar::Update( ControllerState &curr,
		ControllerState &prev )
{
	return false;
	//nothing here
}



