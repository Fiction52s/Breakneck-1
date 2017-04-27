#include "UIWindow.h"
#include "GameSession.h"
#include "Tileset.h"
#include <string>

using namespace sf;
using namespace std;

UIBar::UIBar( UIControl *p_parent, TilesetManager *tsMan, sf::Font *f, int p_width )
	:UIControl( p_parent, NULL, UI_BAR ), width( p_width ), textOffset( 10, 10 ), alignment( LEFT )
{
	//ts_bar = tsMan->GetTileset( "blahbar" );
	AssignTexture();
	SetTopLeft( 0, 0 );
	currText.setFont( *f );
	currText.setCharacterSize( 40 );
	currText.setColor( Color::White );

	dimensions = Vector2f( p_width, 50 ); //50 is just a random constant
}

void UIBar::AssignTexture()
{
	barVA[LEFT*4+0].color = Color::Red;
	barVA[LEFT*4+1].color = Color::Red;
	barVA[LEFT*4+2].color = Color::Red;
	barVA[LEFT*4+3].color = Color::Red;

	barVA[MIDDLE*4+0].color = Color::Green;
	barVA[MIDDLE*4+1].color = Color::Green;
	barVA[MIDDLE*4+2].color = Color::Green;
	barVA[MIDDLE*4+3].color = Color::Green;

	barVA[RIGHT*4+0].color = Color::Blue;
	barVA[RIGHT*4+1].color = Color::Blue;
	barVA[RIGHT*4+2].color = Color::Blue;
	barVA[RIGHT*4+3].color = Color::Blue;
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
	float tw = 20;
	float th = 80;

	barVA[LEFT*4+0].position = Vector2f( x, y );
	barVA[LEFT*4+1].position = Vector2f( x + tw, y );
	barVA[LEFT*4+2].position = Vector2f( x + tw, y + th );
	barVA[LEFT*4+3].position = Vector2f( x, y + th );

	barVA[MIDDLE*4+0].position = barVA[LEFT*4+1].position;
	barVA[MIDDLE*4+1].position = barVA[LEFT*4+1].position + Vector2f( width, 0 );
	barVA[MIDDLE*4+2].position = barVA[LEFT*4+2].position + Vector2f( width, 0 );
	barVA[MIDDLE*4+3].position = barVA[LEFT*4+2].position;

	barVA[RIGHT*4+0].position = barVA[MIDDLE*4+1].position;
	barVA[RIGHT*4+1].position = barVA[MIDDLE*4+1].position + Vector2f( tw, 0 );
	barVA[RIGHT*4+2].position = barVA[MIDDLE*4+2].position + Vector2f( tw, 0 );
	barVA[RIGHT*4+3].position = barVA[MIDDLE*4+2].position;

	SetTextAlignment( alignment );
}

void UIBar::SetText( const std::string &text )
{
	currText.setString( text );
	SetTextAlignment( alignment );
}

void UIBar::SetText( const std::string &text, Alignment align )
{
	currText.setString( text );
	SetTextAlignment( align );
}

const sf::Vector2f &UIBar::GetTopLeftGlobal()
{
	return barVA[0].position;
}

void UIBar::SetTextAlignment( Alignment align )
{
	alignment = align;
	switch( align )
	{
	case LEFT:
		{
			Vector2f topLeft = GetTopLeftGlobal() + textOffset;
			currText.setOrigin( 0, 0 ); 
			currText.setPosition( topLeft );
			break;
		}
	case MIDDLE:
		{
			Vector2f topMiddle = GetTopLeftGlobal() + Vector2f( width, textOffset.y );
			currText.setOrigin( currText.getLocalBounds().width / 2, 0 );
			currText.setPosition( topMiddle );
			break;
		}
	case RIGHT:
		{
			Vector2f topRight = GetTopLeftGlobal() + Vector2f( width - textOffset.x, textOffset.y );
			currText.setOrigin( currText.getLocalBounds().width, 0 );
			currText.setPosition( topRight );
			break;
		}
	}
}

void UIBar::Draw( sf::RenderTarget *target )
{
	target->draw( barVA, 12, sf::Quads ); //ts_bar->texture
	target->draw( currText );
}

bool UIBar::Update( ControllerState &curr,
		ControllerState &prev )
{
	return false;
	//nothing here
}

UIHorizSelector::UIHorizSelector( UIControl *p_parent, UIEventHandlerBase *p_handler, 
	UIControlType p_cType, TilesetManager *tsMan, sf::Font *f, int p_numOptions, 
	std::string *p_names, const std::string &label, int p_labelWidth, Type p_chooserType,
	bool p_loop, int p_defaultIndex, int p_chooserWidth )
	:UIControl( p_parent, p_handler, p_cType ), numOptions( p_numOptions ), chooserType( p_chooserType ), loop( p_loop ), 
	defaultIndex( p_defaultIndex )
{

	//ts_arrows = tsMan->GetTileset( "arrows_" );
	names = new std::string[numOptions];
	for( int i = 0; i < numOptions; ++i )
	{
		names[i] = p_names[i];
	}
	currIndex = defaultIndex;

	AssignArrowTexture();

	bar = new UIBar( p_parent, tsMan, f, p_chooserWidth );

	nameBar = new UIBar( p_parent, tsMan, f, p_labelWidth );
	nameBar->SetText( label, UIBar::Alignment::MIDDLE );
	
	dimensions = bar->dimensions;

	/*currIndexText.setFont( *f );
	currIndexText.setCharacterSize( 40 );
	currIndexText.setColor( Color::White );*/
	bar->SetText( names[defaultIndex] );
}

UIHorizSelector::~UIHorizSelector()
{
	delete bar;
	delete nameBar;
	delete [] names;
}

const sf::Vector2f &UIHorizSelector::GetTopLeftRel()
{
	return bar->GetTopLeftRel();
}

const sf::Vector2f &UIHorizSelector::GetTopLeftGlobal()
{
	return bar->GetTopLeftGlobal();
}

void UIHorizSelector::AssignArrowTexture()
{
	IntRect sub;
	for( int i = 0; i < 2; ++i )
	{
		/*sub = ts_arrow->GetSubRect( i );
		arrows[i*4+0].texCoords = Vector2f( sub.left, sub.top );
		arrows[i*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top );
		arrows[i*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
		arrows[i*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );*/

		arrows[i*4+0].color = Color::Blue;
		arrows[i*4+1].color = Color::Cyan;
		arrows[i*4+2].color = Color::Yellow;
		arrows[i*4+3].color = Color::Red;
	}
}

bool UIHorizSelector::Update( ControllerState &curr, ControllerState &prev )
{
	bool left = curr.LLeft();
	bool right = curr.LRight();

	if( right )
	{
		if( currIndex < numOptions - 1 )
		{
			currIndex++;
		}
		else
		{
			if( loop )
			{
				currIndex = 0;
			}
		}
	}
	else if( left )
	{
		if( currIndex > 0 )
		{
			currIndex--;
		}
		else
		{
			if( loop )
			{
				currIndex = numOptions - 1;
			}
		}
	}

	bar->SetText( names[currIndex] );


	return false;
}

void UIHorizSelector::SetTopLeft( float x, float y )
{
	int extraSpacing = 20;
	relTopLeft = Vector2f( x, y );
	nameBar->SetTopLeft( x, y );
	bar->SetTopLeft( x + nameBar->GetWidth() + extraSpacing, y );

	//set arrows
	//currIndexText.setPosition( x, y );
}

void UIHorizSelector::UpdateSprite()
{
	
}

void UIHorizSelector::Draw( sf::RenderTarget *target )
{
	bar->Draw( target );
}

UIHorizSelectorInt::UIHorizSelectorInt( UIControl *p_parent, UIEventHandlerBase *p_handler, 
	TilesetManager *tsMan, Font *f, int p_numOptions, std::string *p_names,
		int *p_results, bool p_loop, int p_defaultIndex, int p_chooserWidth )
		:UIHorizSelector( p_parent, p_handler, UI_HORIZ_SELECTOR_INT, tsMan, f, p_numOptions, p_names, Type::INT, p_loop, p_defaultIndex,
		p_chooserWidth )
{
	results = new int[p_numOptions];
	for( int i = 0; i < p_numOptions; ++i )
	{
		results[i] = p_results[i];
	}
}

int UIHorizSelectorInt::GetResult( int index )
{
	return results[index];
}

UIHorizSelectorStr::UIHorizSelectorStr( UIControl *p_parent, UIEventHandlerBase *p_handler,
	TilesetManager *tsMan, Font *f, int p_numOptions, std::string *p_names,
		std::string *p_results, bool p_loop, int p_defaultIndex, int p_chooserWidth )
		:UIHorizSelector( p_parent, p_handler, UI_HORIZ_SELECTOR_STR, tsMan, f, p_numOptions, p_names, Type::STR, p_loop, p_defaultIndex,
		p_chooserWidth )
{
	results = new std::string[p_numOptions];
	for( int i = 0; i < p_numOptions; ++i )
	{
		results[i] = p_results[i];
	}
}

const std::string &UIHorizSelectorStr::GetResult( int index )
{
	return results[index];
}