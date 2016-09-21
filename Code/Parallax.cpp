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
Parallax::Parallax( GameSession *p_owner )
	:owner( p_owner )
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
	repeatingSprites.push_back( new RepeatingSprite( this,
		ts, index, offset, repeatWidth, depthLevel ) );
}

void Parallax::Update()
{
	Vector2f camPos = owner->cam.pos;

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
	//oldView = target->getView();
	//newView.//( Vector2f( owner->cam.pos ), Vector2f( 1920, 1080 ) ); 
	newView.setCenter( owner->cam.pos );
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

