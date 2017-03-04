#include "Parallax.h"
#include "GameSession.h"
#include <iostream>

using namespace std;
using namespace sf;

RepeatingSprite::RepeatingSprite( Parallax *p_parent, Tileset *ts, int index,
	Vector2f &offset, int p_repeatWidth, int p_depthLevel )
	:parent( p_parent ), relPos( offset ), repeatWidth( p_repeatWidth ),
	depth( p_depthLevel * .01f )
{
	spr.setTexture( *ts->texture );
	spr.setTextureRect( ts->GetSubRect( index ) );
	spr.setOrigin( spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2 );
	//relPos = Vector2f( 0, 0 );
	//repeatWidth = 1920 * 2;
}



void RepeatingSprite::Update( Vector2f &camPos )
{
	int p = floor( camPos.x * depth + relPos.x + .5f );
	int px = abs( p );
	int pxx = (px % repeatWidth) - repeatWidth / 2;

	if( p > 0 )
		pxx = -pxx;

	//cout << "pxx: " << pxx << ", modified:  " << (pxx / .5f) << endl;
	//pxx = floor( pxx * .5 + .5 );

	Vector2f realPos( camPos.x + pxx, camPos.y + relPos.y );
	//cout << "pxx: " << pxx << "realpos: " << realPos.x << ",  "<< realPos.y << ", campos: " << camPos.x << ", " << camPos.y << endl;
	spr.setPosition( realPos );
}

void RepeatingSprite::Draw( RenderTarget *target )
{
	target->draw( spr );
}

//depth level of 0 doesnt move at all
//depth level of 100 moves at the same rate as the terrain
Parallax::Parallax()
{
	depth = depthLevel / .01;

	int numImages;
	/*switch( arrange )
	{
	case ABAB_2:
		numImages = 2;
		break;
	case AABB_2:
		numImages = 2;
		break;
	case ABCABC_3:
		numImages = 3;
		break;
	default:
		assert( 0 );
	}*/
	
	
	//vaArray = new Sprite[numImages];
	//testSprite.setTexture( *owner->GetTileset( "Parallax/w2_tree_01_1920x1080.png", 1920, 1080 )->texture );
	//testSprite.setOrigin( testSprite.getLocalBounds().width / 2, testSprite.getLocalBounds().height
	//	 / 2 );
	//testSprite1.setTexture( *owner->GetTileset( "Parallax/w2_tree_01_1920x1080.png", 1920, 1080 )->texture );
	//testSprite1.setOrigin( testSprite.getLocalBounds().width / 2, testSprite.getLocalBounds().height
	//	 / 2 );
}

void Parallax::AddRepeatingSprite( Tileset *ts, int index,
	Vector2f &offset, int repeatWidth, int depthLevel )
{
	int repeatsPerScreen = 1920 / repeatWidth;
	repeatingSprites.push_back( new RepeatingSprite( this,
		ts, index, offset, repeatWidth, depthLevel ) );
}

void Parallax::Update( Vector2f &camPos )
{
	for( list<RepeatingSprite*>::iterator it = repeatingSprites.begin();
		it != repeatingSprites.end(); ++it )
	{
		(*it)->Update( camPos );
	}
	
//	int p = floor( camPos.x + .5f );
//	int px = abs( p );
//	int repeatWidth = 1920 * 2;
//
//	int leftpx = px;// - 960;
//	int rightpx = px + 960;
//	int lpxx = (leftpx % (1920*2)) - 1920;
//	//int rpxx = (rightpx % (1920*2)) - 1920;
//
//	//cout << "pxx: " << pxx << "camPos.x: " << camPos.x << endl;
////	if( camPos.x > 0 )
////		pxx = -pxx;
//	if( p > 0 )
//		lpxx = -lpxx;
//
//
//	//camPos.x += pxx;
//	camPos.x += lpxx;
//
//	cout <<"real diff: " << lpxx << ", campos: " << camPos.x << endl;
//	
//	//Vector2f left = camPos;
//	//Vector2f right = camPos;
//	//left.x -= 960;
//	//right.x += 960;
//	testSprite.setPosition( camPos );
	//testSprite1.setPosition( right );
}

void Parallax::Draw( RenderTarget *target )
{
	oldView.setCenter( target->getView().getCenter() );
	oldView.setSize( target->getView().getSize() );
	
	newView.setCenter( oldView.getCenter() );
	newView.setSize( Vector2f( 1920, 1080 ) );
	target->setView( newView );

	for( list<RepeatingSprite*>::iterator it = repeatingSprites.begin();
		it != repeatingSprites.end(); ++it )
	{
		(*it)->Draw( target );
	}
	//target->draw( testSprite );
	//target->draw( testSprite1 );
	target->setView( oldView );
}

Parallax::~Parallax()
{
	for( list<RepeatingSprite*>::iterator it = repeatingSprites.begin();
		it != repeatingSprites.end(); ++it )
	{
		delete (*it);
	}
	/*for( list<VertexArray*>::iterator it = vaList.begin(); it != vaList.end(); ++it )
	{
		delete (*it);
	}*/
}

//void Parallax::AddSprite( Sprite &spr )
//{
//
//}




ScrollingBackground::ScrollingBackground( Tileset *p_ts, int index,
		int p_depthLevel )
		:ts( p_ts ), va( sf::Quads, 2 * 4 ), depthLevel( p_depthLevel ),
		depth( p_depthLevel * .01f ), tsIndex( index )
{
	SetTileIndex( tsIndex );
	SetLeftPos( Vector2f( 0, 0 ) );
}

void ScrollingBackground::Update( Vector2f &camPos )
{
	int repeatWidth = 1920 * 2;
	int p = floor( camPos.x * depth + .5f );

	int px = abs( p );
	int pxx = (px % repeatWidth) - repeatWidth / 2;

	float camXAbs = abs( camPos.x * depth );
	int m = 0;
	while( camXAbs > 1920 )
	{
		camXAbs -= 1920;
		++m;
	}
	
	float off = camXAbs;

	if( camPos.x > 0 )
		off = -off;
	else if( camPos.x < 0 )
	{
		off = off - 1920;
	}
	
	//cout << "off: " << off << endl;

	//if( p > 0 )
	//	off = -off;

	if( p > 0 )
		pxx = -pxx;

	//cout << "pxx: " << pxx << ", modified:  " << (pxx / .5f) << endl;
	//pxx = floor( pxx * .5 + .5 );

	//Vector2f realPos( camPos.x + pxx, camPos.y + relPos.y );
	Vector2f realPos( camPos.x + off, camPos.y );
	//realPos = camPos;
	realPos.x -= 960;
	SetLeftPos( realPos );//camPos.x );//realPos.x );
}

void ScrollingBackground::SetLeftPos( Vector2f &pos )
{
	xPos = pos.x;
	float currX = pos.x;
	float top = pos.y - 540;
	float width = 1920;
	float height = 1080;
	for( int i = 0; i < 2; ++i )
	{
		va[i*4+0].position = Vector2f( currX, top );
		va[i*4+1].position = Vector2f( currX + width, top );
		va[i*4+2].position = Vector2f( currX + width, top + height );
		va[i*4+3].position = Vector2f( currX, top + height );

		currX += width;
	}	
}

void ScrollingBackground::SetTileIndex( int index )
{
	tsIndex = index;
	IntRect subRect = ts->GetSubRect( tsIndex );
	for( int i = 0; i < 2; ++i )
	{
		va[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		va[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		va[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		va[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}	
}

void ScrollingBackground::Draw( RenderTarget *target )
{
	oldView.setCenter( target->getView().getCenter() );
	oldView.setSize( target->getView().getSize() );
	
	newView.setCenter( oldView.getCenter() );
	newView.setSize( Vector2f( 1920, 1080 ) );
	target->setView( newView );

	target->draw( va, ts->texture );

	target->setView( oldView );
}