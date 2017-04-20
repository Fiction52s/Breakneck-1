#include "UIWindow.h"
#include "GameSession.h"
#include "Tileset.h"

using namespace sf;
using namespace std;

UIWindow::UIWindow( GameSession *p_owner, Tileset *t, sf::Vector2f &p_windowSize )
	:owner( p_owner ), ts_window( t ), windowSize( p_windowSize )
{
	//uiwindowtest_96x30.png
	AssignTextureToCorners();
	AssignTextureToCornerEdges();
	AssignTextureToEdges();
	AssignTextureCenter();
	minSize = t->tileWidth * 2;
	tileSize = t->tileWidth;
	Resize( Vector2f( 600, 100 ) );
}

void UIWindow::Resize( sf::Vector2f &size )
{
	windowSize = size;
	SetTopLeft( GetTopLeft() );
}

void UIWindow::Resize( float x, float y )
{
	windowSize = Vector2f( x, y );
	SetTopLeft( GetTopLeft() );
}

void UIWindow::AssignTextureToCorners()
{
	sf::Vertex *cornerVA = (borderVA + CORNER_TOPLEFT * 4); 

	float th = ts_window->tileHeight;
	IntRect cornerRect( 0, th * 10, th, th );

	for( int i = 0; i < 4; ++i )
	{
		cornerVA[i*4+0].texCoords = Vector2f( cornerRect.left, cornerRect.top );
		cornerVA[i*4+1].texCoords = Vector2f( cornerRect.left + cornerRect.width, cornerRect.top );
		cornerVA[i*4+2].texCoords = Vector2f( cornerRect.left + cornerRect.width, cornerRect.top + cornerRect.height);
		cornerVA[i*4+3].texCoords = Vector2f( cornerRect.left, cornerRect.top + cornerRect.height );

		/*cornerVA[i*4+0].color = Color::Red;
		cornerVA[i*4+1].color = Color::Red;
		cornerVA[i*4+2].color = Color::Red;
		cornerVA[i*4+3].color = Color::Red;*/
	}
	/*Vector2f points[4];
	points[0] = Vector2f( cornerRect.left, cornerRect.top );
	points[1] = Vector2f( cornerRect.left + cornerRect.width, cornerRect.top );
	points[2] = Vector2f( cornerRect.left + cornerRect.width, cornerRect.top + cornerRect.height );
	points[3] = Vector2f( cornerRect.left, cornerRect.top + cornerRect.height );

	int i = 0;
	cornerVA[i*4+0].texCoords = points[0];
	cornerVA[i*4+1].texCoords = points[1];
	cornerVA[i*4+2].texCoords = points[2];
	cornerVA[i*4+3].texCoords = points[3];

	++i;
	cornerVA[i*4+0].texCoords = points[3];
	cornerVA[i*4+1].texCoords = points[0];
	cornerVA[i*4+2].texCoords = points[1];
	cornerVA[i*4+3].texCoords = points[2];

	++i;
	cornerVA[i*4+0].texCoords = points[2];
	cornerVA[i*4+1].texCoords = points[3];
	cornerVA[i*4+2].texCoords = points[0];
	cornerVA[i*4+3].texCoords = points[1];

	++i;
	cornerVA[i*4+0].texCoords = points[1];
	cornerVA[i*4+1].texCoords = points[2];
	cornerVA[i*4+2].texCoords = points[3];
	cornerVA[i*4+3].texCoords = points[0];*/
}

void UIWindow::AssignTextureCenter()
{
	float th = ts_window->tileWidth;

	IntRect sub = ts_window->GetSubRect( 11 );
	/*centerVA[0].texCoords = Vector2f( sub.left, sub.top );
	centerVA[1].texCoords = Vector2f( sub.left + th, sub.top );
	centerVA[2].texCoords = Vector2f( sub.left + th, sub.top + th );
	centerVA[3].texCoords = Vector2f( sub.left, sub.top + th );*/

	centerVA[0].color = Color::Blue;
	centerVA[1].color = Color::Blue;
	centerVA[2].color = Color::Blue;
	centerVA[3].color = Color::Blue;
}

void UIWindow::SetCenterVertices( sf::Vector2f &A,
		sf::Vector2f &B, sf::Vector2f &C,
		sf::Vector2f &D )
{
	float th = ts_window->tileWidth;
	float tw = ts_window->tileHeight;

	//goes behind the other stuff halfway
	centerVA[0].position = A;
	centerVA[1].position = B;
	centerVA[2].position = C;
	centerVA[3].position = D;
}

void UIWindow::AssignTextureToCornerEdges()
{
	Vertex *cornerEdgeVA = (borderVA);
	IntRect rect;
	for( int i = 0; i < 8; ++i )
	{
		rect = ts_window->GetSubRect( i );
		if( i % 2 == 0 ) //horizontal
		{
			/*cornerEdgeVA[i*4+0].color = Color::Blue;
			cornerEdgeVA[i*4+1].color = Color::Blue;
			cornerEdgeVA[i*4+2].color = Color::Blue;
			cornerEdgeVA[i*4+3].color = Color::Blue;*/
			cornerEdgeVA[i*4+0].texCoords = Vector2f( rect.left, rect.top + rect.height );
			cornerEdgeVA[i*4+1].texCoords = Vector2f( rect.left, rect.top );
			cornerEdgeVA[i*4+2].texCoords = Vector2f( rect.left + rect.width, rect.top );
			cornerEdgeVA[i*4+3].texCoords = Vector2f( rect.left + rect.width, rect.top + rect.height);
		}
		else //vertical
		{
			/*cornerEdgeVA[i*4+0].color = Color::Yellow;
			cornerEdgeVA[i*4+1].color = Color::Yellow;
			cornerEdgeVA[i*4+2].color = Color::Yellow;
			cornerEdgeVA[i*4+3].color = Color::Yellow;*/
			//rotated 90 degrees counter clockwise to be vertical
			

			cornerEdgeVA[i*4+0].texCoords = Vector2f( rect.left, rect.top );
			cornerEdgeVA[i*4+1].texCoords = Vector2f( rect.left + rect.width, rect.top );
			cornerEdgeVA[i*4+2].texCoords = Vector2f( rect.left + rect.width, rect.top + rect.height);
			cornerEdgeVA[i*4+3].texCoords = Vector2f( rect.left, rect.top + rect.height );
		}

		
		
	}
}

void UIWindow::AssignTextureToEdges()
{	
	//walls

	Vertex *wallVA = (borderVA + WALL_LEFT * 4);
	Vertex *flatVA = (borderVA + FLOOR * 4 );

	IntRect rect;

	//walls

	rect = ts_window->GetSubRect( 8 );
	for( int i = 0; i < 2; ++i )
	{
		/*wallVA[i*4+0].color = Color::Green;
		wallVA[i*4+1].color = Color::Green;
		wallVA[i*4+2].color = Color::Green;
		wallVA[i*4+3].color = Color::Green;*/
		wallVA[i*4+0].texCoords = Vector2f( rect.left, rect.top + rect.height );
		wallVA[i*4+1].texCoords = Vector2f( rect.left, rect.top );
		wallVA[i*4+2].texCoords = Vector2f( rect.left + rect.width, rect.top );
		wallVA[i*4+3].texCoords = Vector2f( rect.left + rect.width, rect.top + rect.height );
	}

	//ceiling and floor
	rect = ts_window->GetSubRect( 9 );
	for( int i = 0; i < 2; ++i )
	{
		flatVA[i*4+0].texCoords = Vector2f( rect.left, rect.top );
		flatVA[i*4+1].texCoords = Vector2f( rect.left + rect.width, rect.top );
		flatVA[i*4+2].texCoords = Vector2f( rect.left + rect.width, rect.top + rect.height );
		flatVA[i*4+3].texCoords = Vector2f( rect.left, rect.top + rect.height );

		/*flatVA[i*4+0].color = Color::Magenta;
		flatVA[i*4+1].color = Color::Magenta;
		flatVA[i*4+2].color = Color::Magenta;
		flatVA[i*4+3].color = Color::Magenta;*/
	}
}

void UIWindow::Draw( sf::RenderTarget *target )
{
	/*sf::CircleShape c;
	c.setRadius( 20 );
	c.setFillColor( Color::White );
	c.setOrigin( c.getLocalBounds().width / 2, c.getLocalBounds().height / 2 );
	c.setPosition( 0, 0 );*/
	
	target->draw( centerVA, 4, sf::Quads /*TODO shader here*/ );

	target->draw( borderVA, 16 * 4, sf::Quads, ts_window->texture );

	sf::RectangleShape rs;
	rs.setFillColor( Color( 0, 255, 0, 60 ) );
	rs.setSize( Vector2f( 600, 100 ) );
	rs.setPosition( 50 + ts_window->tileHeight, ts_window->tileHeight );

	target->draw( rs );


	//target->draw( c );
}

const sf::Vector2f &UIWindow::GetTopLeft()
{
	return borderVA[CORNER_TOPLEFT * 4].position;
}

void UIWindow::SetTopLeft( const sf::Vector2f &pos )
{
	//window size minimum is 2x tilewidth
	float tw = ts_window->tileWidth;
	float th = ts_window->tileHeight;
	float left = pos.x;
	float top = pos.y;
	float rightLeft = left + th + windowSize.x;
	float botTop = top + th + windowSize.y;
	
	SetCenterVertices( Vector2f( left + th / 2, top + th / 2 ),
		Vector2f( rightLeft + th / 2, top + th / 2 ),
		Vector2f( rightLeft + th / 2, botTop + th / 2 ),
		Vector2f( left + th / 2, botTop + th / 2 ) );

	SetCornerVertices( Vector2f( left, top ), 0 );
	SetCornerVertices( Vector2f( rightLeft, top ), 1 );
	SetCornerVertices( Vector2f( rightLeft, botTop ), 2 );
	SetCornerVertices( Vector2f( left, botTop ), 3 );

	SetCornerEdgesHorizVertices( Vector2f( left + th, top ), 0 );
	SetCornerEdgesHorizVertices( Vector2f( rightLeft - tw, top ), 1 );
	SetCornerEdgesHorizVertices( Vector2f( rightLeft - tw, botTop ), 2 );
	SetCornerEdgesHorizVertices( Vector2f( left + th, botTop ), 3 );

	SetCornerEdgesVertVertices( Vector2f( left, top + th ), 0 );
	SetCornerEdgesVertVertices( Vector2f( rightLeft, top + th ), 1 );
	SetCornerEdgesVertVertices( Vector2f( rightLeft, botTop - tw ), 2 );
	SetCornerEdgesVertVertices( Vector2f( left, botTop - tw ), 3 );

	SetWallVertices( Vector2f( left, top + th + tw ), 0 );
	SetWallVertices( Vector2f( rightLeft, top + th + tw ), 1 );

	SetFlatVertices( Vector2f( left + th + tw, top ), 0 );
	SetFlatVertices( Vector2f( left + th + tw, botTop ), 1 );

	//about to set up the function for setwallpos and get the window all set up tomorrow
}

void UIWindow::SetCornerVertices( sf::Vector2f &topLeft, int index )
{
	float th = ts_window->tileHeight;

	Vertex *cornerVA = (borderVA + CORNER_TOPLEFT * 4 );	
	cornerVA[index*4+0].position = Vector2f( topLeft.x, topLeft.y );
	cornerVA[index*4+1].position = Vector2f( topLeft.x + th, topLeft.y );
	cornerVA[index*4+2].position = Vector2f( topLeft.x + th, topLeft.y + th );
	cornerVA[index*4+3].position = Vector2f( topLeft.x, topLeft.y + th );
}

void UIWindow::SetWallVertices( Vector2f &topLeft, int index )
{
	float th = ts_window->tileHeight;
	float tw = ts_window->tileWidth;

	float height = (windowSize.y - 2 * tw );
	Vertex *wallVA = (borderVA + WALL_LEFT * 4 );
	wallVA[index*4+0].position = Vector2f( topLeft.x, topLeft.y );
	wallVA[index*4+1].position = Vector2f( topLeft.x + th, topLeft.y );
	wallVA[index*4+2].position = Vector2f( topLeft.x + th, topLeft.y + height );
	wallVA[index*4+3].position = Vector2f( topLeft.x, topLeft.y + height );
}

void UIWindow::SetFlatVertices( Vector2f &topLeft, int index )
{
	float th = ts_window->tileHeight;
	float tw = ts_window->tileWidth;

	float width = (windowSize.x - 2 * tw );
	Vertex *flatVA = (borderVA + FLOOR * 4 );
	flatVA[index*4+0].position = Vector2f( topLeft.x, topLeft.y );
	flatVA[index*4+1].position = Vector2f( topLeft.x + width, topLeft.y );
	flatVA[index*4+2].position = Vector2f( topLeft.x + width, topLeft.y + th );
	flatVA[index*4+3].position = Vector2f( topLeft.x, topLeft.y + th );
}

void UIWindow::SetCornerEdgesVertVertices( Vector2f &topLeft, int index )
{
	float th = ts_window->tileHeight;
	float tw = ts_window->tileWidth;

	Vertex *cornerVA = (borderVA + ( index * 2 ) * 4 );	
	cornerVA[0].position = Vector2f( topLeft.x, topLeft.y );
	cornerVA[1].position = Vector2f( topLeft.x + th, topLeft.y );
	cornerVA[2].position = Vector2f( topLeft.x + th, topLeft.y + tw );
	cornerVA[3].position = Vector2f( topLeft.x, topLeft.y + tw );
}

void UIWindow::SetCornerEdgesHorizVertices( Vector2f &topLeft, int index )
{
	float th = ts_window->tileHeight;
	float tw = ts_window->tileWidth;

	Vertex *cornerVA = (borderVA + ( (index * 2) + 1 ) * 4 );	
	cornerVA[0].position = Vector2f( topLeft.x, topLeft.y );
	cornerVA[1].position = Vector2f( topLeft.x + tw, topLeft.y );
	cornerVA[2].position = Vector2f( topLeft.x + tw, topLeft.y + th );
	cornerVA[3].position = Vector2f( topLeft.x, topLeft.y + th );
}