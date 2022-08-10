#include "GoalExplosion.h"
#include "GameSession.h"
#include <iostream>

using namespace sf;
using namespace std;

const int GoalPulse::circlePoints = 32;

GoalPulse::GoalPulse()
	:circleVA( sf::Quads, circlePoints * 4 ), innerRadius( 100 ), 
	outerRadius( 200 ), pulseLength( 60 ),
	maxOuterRadius( 3000.f ), maxInnerRadius( 2800.f )
{
	data.show = false;
	data.frame = 0;
	minInnerRadius = innerRadius;
	minOuterRadius = outerRadius;
	//UpdatePoints();	
}

void GoalPulse::SetPosition(sf::Vector2f &pos)
{
	position = pos;
}

void GoalPulse::Reset()
{
	data.frame = 0;
	data.show = false;
}

void GoalPulse::StartPulse()
{
	data.frame = 0;
	data.show = true;
}

void GoalPulse::Update()
{
	if(data.show )
	{
		if(data.frame == pulseLength + 1 )
		{
			data.frame = 0;
			data.show = false;
		}
		else
		{
			CubicBezier innerBez( 0, 0, 1, 1 );
			CubicBezier outerBez( 0, 0, 1, 1 );

			double innerQ = innerBez.GetValue(data.frame / (double)pulseLength );
			double outerQ = outerBez.GetValue(data.frame / (double)pulseLength );

			innerRadius = minInnerRadius * (1.0 - innerQ) + maxInnerRadius * innerQ;
			outerRadius = minOuterRadius * (1.0 - outerQ) + maxOuterRadius * outerQ;

			++data.frame;
		}

		UpdatePoints();
	}
}

void GoalPulse::UpdatePoints()
{
	if (!data.show)
	{
		return;
	}
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
	if(data.show )
	{
		target->draw( circleVA );
	}
	
}

int GoalPulse::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void GoalPulse::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void GoalPulse::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	bytes += sizeof(MyData);
}