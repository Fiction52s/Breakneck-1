#include "Light.h"
#include <assert.h>
#include <iostream>
#include "GameSession.h"

using namespace sf;
using namespace std;


Light::Light( GameSession *own, sf::Vector2i &p, Color &c, float rad, float bright )
	:color( c ), pos( p ), next( NULL )
{
	brightness = bright;
	//radius = rad;
	owner = own;
	/*if (!sh.loadFromFile("light_shader.frag", sf::Shader::Fragment))
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "LIGHt SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "light shader not loaded" );
	}*/
	
	//sh.setParameter( "pos", 0, 0 );
	//rad = rad * 10.f;// * 10.f;
	//sh.setParameter( "lightpos", 0, -300 );
	
	//rad = 100;	
	radius = rad;//rad / (110*brightness);
	
	cs.setRadius( rad );
	cs.setFillColor( color );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	//cs.setPosition( 0, -300 );
	cs.setPosition( p.x, p.y );

	//falloff = Vector3f( .f01, .2, .1 );
	
	//radius = 1;
	
	//falloff /= 100.f;
	depth = .075f;//.f75;//.f75;//.f75;//radius * .f075;
}

void Light::HandleQuery( QuadTreeCollider * qtc )
{
	qtc->HandleEntrant( this );
}

bool Light::IsTouchingBox( const sf::Rect<float> &r )
{
	//float trueRad = 1000;
	sf::Rect<float> r2( pos.x - radius, pos.y - radius, radius * 2, radius* 2 );
	if( r.intersects( r2 ) )
		return true;

	return false;
}

void Light::Draw( RenderTarget *target )
{
	//falloff2 *= 100.f;
	//sh.setParameter( "pos", owner->cam.pos.x, owner->cam.pos.y );
	//sh.setParameter( "zoom", owner->cam.GetZoom() );
	//sh.setParameter( "resolution", owner->window->getSize().x, owner->window->getSize().y);
	//sh.setParameter( "topLeft", owner->view.getCenter().x - owner->view.getSize().x / 2, 
	//	owner->view.getCenter().y + owner->view.getSize().y / 2 );

	Vector2i vi0 = Vector2i( owner->preScreenTex->mapCoordsToPixel( cs.getPosition() ) );
		//Vector3f pos0( vi0.x / (float)window->getSize().x, ((float)window->getSize().y - vi0.y) / (float)window->getSize().y, .01f5 ); 
	Vector3f lightpos( vi0.x / (float)owner->window->getSize().x, -1 + vi0.y / (float)owner->window->getSize().y, depth );

	//sh.setParameter( "lightpos", lightpos );

	
	/*cs.setRadius( 20 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( pos.x, pos.y );*/
	//target->draw( cs );
	/*cs.setRadius( rad );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( pos.x, pos.y );*/
}