#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BasicEffect.h"

using namespace std;
using namespace sf;



BasicEffect::BasicEffect ( GameSession *owner )
	:Enemy( owner, EnemyType::BASICEFFECT, false, 0 )
{
	affectCameraZoom = false;
	spawned = true;
	pauseImmune = false;
	frame = 0;
	ts = NULL;
	//ts = //owner->GetTileset( "double.png", 64, 64 ); //what
	activated = false;
	animationFactor = 1;
	stored_frame = 0;
}

void BasicEffect::ResetEnemy()
{
	frame = 0;
	activated = false;
	//owner->DeactivateEffect( this );
}

void BasicEffect::Init( Tileset *t, sf::Vector2<double> pos, double angle, int fc, int af, bool right, float p_depth )
{
	depth = p_depth;
	//cout << "init: " << this << ", " << t->sourceName << endl;
	s.setTexture( *t->texture );
	facingRight = right;

	//pos /= (double)depth;

	sf::IntRect ir = t->GetSubRect( 0 );		

	if( facingRight )
	{
		s.setTextureRect( ir );
	}
	else
	{
		s.setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
	}

	s.setOrigin( s.getLocalBounds().width / 2, s.getLocalBounds().height / 2 );
	s.setPosition( pos.x, pos.y );
	s.setRotation( angle / PI * 180 );

	animationFactor = af;
	ts = t;
	frameCount = fc;
	//position = pos;
	frame = 0;
	activated = true;
	position = pos;
}

void BasicEffect::HandleEntrant( QuadTreeEntrant *qte )
{
}

void BasicEffect::UpdatePrePhysics()
{
}

void BasicEffect::UpdatePhysics()
{
}

void BasicEffect::UpdatePostPhysics()
{
	//if( ts == NULL )
	//{
	//	cout << "problem with: " << this << endl;
	//}
	//float depth = 1.f;

	//Vector2f spritePos = Vector2f(position) * depth;
	//s.setPosition(pos.x * depth, pos.y * depth);

	sf::IntRect ir = ts->GetSubRect( frame / animationFactor );

	if( facingRight )
	{
		s.setTextureRect( ir );
	}
	else
	{
		s.setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
	}
	//s.setOrigin( s.getLocalBounds().width / 2, s.getLocalBounds().height / 2 );
	//s.setPosition( pos.x, pos.y );
	//s.setRotation( angle / PI * 180 );

	frame++;
	if( frame / animationFactor == frameCount )
	{
		owner->DeactivateEffect( this );
	}
}

void BasicEffect::Draw(sf::RenderTarget *target )
{
/*	sf::CircleShape cs;
	cs.setFillColor( Color::Blue );
	cs.setRadius( 100 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( s.getPosition().x, s.getPosition().y );
	target->draw( cs );*/

	//cout << ts->sourceName << ": " << target->endl;

	//target->draw(s);
	sf::View oldView = target->getView();
	sf::View newView = oldView;
	newView.setCenter(oldView.getCenter() / depth);
	float oldFactor = oldView.getSize().x / 960.f;
	float newFactor = oldFactor * depth;
	newView.setSize(Vector2f(960, 540) * newFactor);
	target->setView(newView);
	target->draw( s );	
	target->setView(oldView);
}

bool BasicEffect::IHitPlayer( int index )
{
	return false;
}

 pair<bool, bool> BasicEffect::PlayerHitMe( int index )
{
	return pair<bool, bool>(false,false);
}

bool BasicEffect::PlayerSlowingMe()
{
	return false;
}

void BasicEffect::UpdateSprite()
{
}

void BasicEffect::DebugDraw(sf::RenderTarget *target)
{
}

void BasicEffect::UpdateHitboxes()
{
}

bool BasicEffect::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void BasicEffect::SaveEnemyState()
{
	stored_frame = frame;
}

void BasicEffect::LoadEnemyState()
{
	frame = stored_frame;
}