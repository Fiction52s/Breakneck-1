#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BasicEffect.h"

using namespace std;
using namespace sf;



BasicEffect::BasicEffect ()
	:Enemy( EnemyType::EN_BASICEFFECT, NULL )//, false, 0, false )
{
	spawned = true;
	pauseImmune = false;
	frame = 0;
	ts = NULL;
	activated = false;
	animationFactor = 1;
	stored_frame = 0;
}

BasicEffect::BasicEffect(BasicEffect &be)
	:Enemy( EnemyType::EN_BASICEFFECT, NULL)//, false, 0, false )
{
	depth = be.depth;
	frameCount = be.frameCount;
	s = be.s;
	ts = be.ts;
	pauseImmune = be.pauseImmune;
	activated = be.activated;
	animationFactor = be.animationFactor;
	facingRight = be.facingRight;
	layer = be.layer;
	stored_frame = be.stored_frame;
	startFrame = be.startFrame;
}

void BasicEffect::HandleNoHealth()
{

}

bool BasicEffect::CanTouchSpecter()
{
	return false;
}

void BasicEffect::ProcessState()
{
	if (frame / animationFactor == frameCount)
	{
		sess->DeactivateEffect(this);
		dead = true;
	}
}

void BasicEffect::ResetEnemy()
{
	frame = 0;
	activated = false;
}

void BasicEffect::Init( Tileset *t, sf::Vector2<double> pos, double angle, int fc, int af, bool right, float p_depth )
{
	depth = p_depth;
	//cout << "init: " << this << ", " << t->sourceName << endl;
	s.setTexture( *t->texture );
	facingRight = right;

	//pos /= (double)depth;

	sf::IntRect ir = t->GetSubRect( startFrame );		

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

	//ResetSlow();

	animationFactor = af;
	ts = t;
	frameCount = fc;
	//position = pos;
	frame = 0;
	activated = true;
	currPosInfo.position = pos;
	dead = false;
}

void BasicEffect::EnemyDraw(sf::RenderTarget *target )
{
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

void BasicEffect::UpdateSprite()
{
	sf::IntRect ir = ts->GetSubRect(frame / animationFactor + startFrame);

	if (facingRight)
	{
		s.setTextureRect(ir);
	}
	else
	{
		s.setTextureRect(sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height));
	}
}

void BasicEffect::DebugDraw(sf::RenderTarget *target)
{
}

void BasicEffect::UpdateHitboxes()
{
}