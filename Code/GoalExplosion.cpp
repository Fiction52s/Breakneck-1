#include "GoalExplosion.h"
#include "GameSession.h"
#include <iostream>

using namespace sf;
using namespace std;

const int GoalPulse::circlePoints = 32;

GoalPulse::GoalPulse( GameSession *p_owner, sf::Vector2f &pos )
	:owner( p_owner ), circleVA( sf::Quads, circlePoints * 4 ), innerRadius( 100 ), 
	outerRadius( 200 ), position( pos ), show( false ), frame( 0 ), pulseLength( 60 ),
	maxOuterRadius( 3000.f ), maxInnerRadius( 2800.f )
{
	minInnerRadius = innerRadius;
	minOuterRadius = outerRadius;
	UpdatePoints();	
}

void GoalPulse::Reset()
{
	frame = 0;
	show = false;
}

void GoalPulse::Update()
{
	if( show )
	{
		if( frame == pulseLength + 1 )
		{
			frame = 0;
			show = false;
		}
		else
		{
			CubicBezier innerBez( 0, 0, 1, 1 );
			CubicBezier outerBez( 0, 0, 1, 1 );

			double innerQ = innerBez.GetValue( frame / (double)pulseLength );
			double outerQ = outerBez.GetValue( frame / (double)pulseLength );

			innerRadius = minInnerRadius * (1.0 - innerQ) + maxInnerRadius * innerQ;
			outerRadius = minOuterRadius * (1.0 - outerQ) + maxOuterRadius * outerQ;

			++frame;
		}

		UpdatePoints();
	}
}

void GoalPulse::UpdatePoints()
{
	/*Transform t;
	float angle = 0;
	for( int i = 0; i < circlePoints; ++i )
	{
		circleVA[i*4+0].position = 
	}*/
	cout <<  circlePoints << endl;
	Transform tr;
	Vector2f offsetInner( 0, -innerRadius );
	Vector2f offsetOuter( 0, -outerRadius );
	for( int i = 0; i < circlePoints; ++i )
	{ 
		circleVA[i*4+0].position = position + tr.transformPoint( offsetInner );  
		circleVA[i*4+1].position = position + tr.transformPoint( offsetOuter ); 
		
		tr.rotate( 360.f / circlePoints );

		circleVA[i*4+2].position = position + tr.transformPoint( offsetOuter ); 
		circleVA[i*4+3].position = position + tr.transformPoint( offsetInner );

		circleVA[i*4+0].color = Color::Cyan;
		circleVA[i*4+1].color = Color::Cyan;
		circleVA[i*4+2].color = Color::Cyan;
		circleVA[i*4+3].color = Color::Cyan;
	}

	
}

void GoalPulse::Draw( sf::RenderTarget *target )
{
	if( show )
	{
		target->draw( circleVA );
	}
	
}