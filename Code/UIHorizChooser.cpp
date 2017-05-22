#include "UIWindow.h"
#include "GameSession.h"
#include "Tileset.h"
#include <string>

using namespace sf;
using namespace std;

UIBar::UIBar( UIControl *p_parent, TilesetManager *tsMan, sf::Font *f, int p_width, int p_height, int p_textHeight )
	:UIControl( p_parent, NULL, UI_BAR ), width( p_width ), textOffset( 10, 10 ), alignment( LEFT )
{
	sideWidth = 20;
	ts_bar = tsMan->GetTileset( "Menu/ui_bar_32x80.png", 32, 80 );
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
			currText.setOrigin( 0, 0 ); 
			currText.setPosition( topLeft );
			break;
		}
	case MIDDLE:
		{
			Vector2f topMiddle = GetTopLeftGlobal() + Vector2f( width / 2, 0 ) + off;
			currText.setOrigin( currText.getLocalBounds().width / 2, 0 );
			currText.setPosition( topMiddle );
			break;
		}
	case RIGHT:
		{
			Vector2f topRight = GetTopLeftGlobal() + Vector2f( width, 0 ) + off;
			currText.setOrigin( currText.getLocalBounds().width, 0 );
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
	nameBar->SetText( label, Vector2i( 0, 0 ), UIBar::Alignment::MIDDLE );
	
	dimensions = bar->dimensions;

	/*currIndexText.setFont( *f );
	currIndexText.setCharacterSize( 40 );
	currIndexText.setColor( Color::White );*/
	bar->SetText( names[defaultIndex] ); 

	waitFrames[0] = 10;
	waitFrames[1] = 5;
	waitFrames[2] = 2;

	waitModeThresh[0] = 2;
	waitModeThresh[1] = 2;

	currWaitLevel = 0;
	flipCounterLeft = 0;
	flipCounterRight = 0;
	framesWaiting = 0;
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

void UIHorizSelector::Focus()
{
	UIControl::Focus();
	bar->Focus();
	nameBar->Focus();
}

void UIHorizSelector::Unfocus()
{
	UIControl::Unfocus();
	bar->Unfocus();
	nameBar->Unfocus();
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
	bool upOrDown = curr.LUp() || curr.LDown();
	bool left = curr.LLeft() && !upOrDown;
	bool right = curr.LRight() && !upOrDown;

	if( right )
	{
		if( flipCounterRight == 0 
			|| ( flipCounterRight > 0 && framesWaiting == waitFrames[currWaitLevel] )
			)
		{
			if( flipCounterRight == 0 )
			{
				currWaitLevel = 0;
			}

			++flipCounterRight;

			if( flipCounterRight == waitModeThresh[currWaitLevel] && currWaitLevel < 2 )
			{
				currWaitLevel++;
			}

			flipCounterLeft = 0;
			framesWaiting = 0;

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
		else
		{
			++framesWaiting;
		}
		
	}
	else if( left )
	{
		if( flipCounterLeft == 0 
			|| ( flipCounterLeft > 0 && framesWaiting == waitFrames[currWaitLevel] )
			)
		{
			if( flipCounterLeft == 0 )
			{
				currWaitLevel = 0;
			}

			++flipCounterLeft;

			if( flipCounterLeft == waitModeThresh[currWaitLevel] && currWaitLevel < 2 )
			{
				currWaitLevel++;
			}

			flipCounterRight = 0;
			framesWaiting = 0;
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
		else
		{
			++framesWaiting;
		}
		
	}
	else
	{
		flipCounterLeft = 0;
		flipCounterRight = 0;
		currWaitLevel = 0;
		framesWaiting = 0;
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

void UIHorizSelector::Draw( sf::RenderTarget *target )
{
	nameBar->Draw( target );
	bar->Draw( target );
}

UIHorizSelectorInt::UIHorizSelectorInt( UIControl *p_parent, UIEventHandlerBase *p_handler, 
	TilesetManager *tsMan, Font *f, int p_numOptions, std::string *p_names,
	const std::string &p_label, int p_labelWidth,
		int *p_results, bool p_loop, int p_defaultIndex, int p_chooserWidth )
		:UIHorizSelector( p_parent, p_handler, UI_HORIZ_SELECTOR_INT, tsMan, f, p_numOptions, p_names, 
		p_label, p_labelWidth, Type::INT, p_loop, p_defaultIndex,
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
	const std::string &p_label, int p_labelWidth, std::string *p_results, bool p_loop, 
	int p_defaultIndex, int p_chooserWidth )
		:UIHorizSelector( p_parent, p_handler, UI_HORIZ_SELECTOR_STR, tsMan, f, p_numOptions, p_names, 
		p_label, p_labelWidth, Type::STR, p_loop, p_defaultIndex, p_chooserWidth )
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