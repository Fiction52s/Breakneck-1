#include "Wire.h"
#include "Actor.h"
#include "GameSession.h"
#include <iostream>
#include <assert.h>

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>

Wire::Wire( Actor *p, bool r)
	:state( IDLE ), numPoints( 0 ), framesFiring( 0 ), fireRate( 120 ), maxTotalLength( 10000 ), minSegmentLength( 50 )
	, player( p ), triggerThresh( 200 ), hitStallFrames( 10 ), hitStallCounter( 0 ), pullStrength( 10 ), right( r )
	, extraBuffer( 8 ), quads( sf::Quads, (int)((ceil( maxTotalLength / 6.0 ) + extraBuffer) * 4 ))//eventually you can split this up into smaller sections so that they don't all need to draw
	, quadHalfWidth( 3 ), ts_wire( NULL ), frame( 0 ), animFactor( 3 ), offset( 8, 18 ) //, ts_redWire( NULL ) 
{
	ts_wire = player->owner->GetTileset( "wire.png", 6, 36 );
	minSideEdge = NULL;
	minSideOther = -1;
	minSideAlong = -1;
}

void Wire::UpdateState( bool touchEdgeWithWire )
{
	//cout << "update state" << endl;
	ControllerState &currInput = player->currInput;
	ControllerState &prevInput = player->prevInput;

	//V2d playerPos = player->position;
	V2d playerPos = GetOriginPos(true);
	storedPlayerPos = playerPos;
	//cout << "setting stored player pos to: " << playerPos.x << ", " << playerPos.y << " using " << player->position.x << ", " << player->position.y << endl;
	/*V2d dir;
	if( player->ground == NULL )
	{
		dir = V2d( 0, -1 );
	}
	else
	{
		dir = player->ground->Normal();
	}*/
	//playerPos += V2d( offset.x, offset.y );
	bool triggerDown;
	bool prevTriggerDown;

	if( right )
	{
		triggerDown = currInput.rightTrigger >= triggerThresh;
		prevTriggerDown = prevInput.rightTrigger >= triggerThresh;
	}
	else
	{
		triggerDown = currInput.leftTrigger >= triggerThresh;
		prevTriggerDown = prevInput.leftTrigger >= triggerThresh;
	}


	switch( state )
	{
	case IDLE:
		{
			
			if( triggerDown && !prevTriggerDown )
			{
				//cout << "firing" << endl;
				fireDir = V2d( 0, 0 );


				if( false )
				{
					if( currInput.LLeft() )
					{
						fireDir.x -= 1;
					}
					else if( currInput.LRight() )
					{
						fireDir.x += 1;
					}
			
					if( currInput.LUp() )
					{
						if( player->reversed )
						{
							fireDir.y += 1;
						}
						else
						{
							fireDir.y -= 1;
						}
							
					}
					else if( currInput.LDown() )
					{
						if( player->reversed )
						{
							fireDir.y -= 1;
						}
						else
						{
							fireDir.y += 1;
						}
						
					}
				}
				else
				{
					fireDir.x = cos( currInput.leftStickRadians );
					fireDir.y = -sin( currInput.leftStickRadians );
				}

				if( length( fireDir ) > .1 )
				{
					if( (right && ( player->lastWire == 0 || player->lastWire == 2 )) 
						|| (!right && ( player->lastWire == 0 || player->lastWire == 1 ) ) )
					{
						if( right )
						{
							player->lastWire = 1;
						}
						else
						{
							player->lastWire = 2;
						}

						fireDir = normalize( fireDir );
						state = FIRING;
						framesFiring = 0;
						frame = 0;
					}
				}
			}
			break;
		}
	case FIRING:
		{
			//rcEdge = NULL;
		//	RayCast( this, player->owner->terrainTree->startNode, anchor.pos, player->position );
			if( rcEdge != NULL )
			{
				state = HIT;
				hitStallCounter = framesFiring;
			}

			if( framesFiring * fireRate > maxTotalLength )
			{
				Reset();
				
			}
			break;
		}
	case HIT:
		{
			double total = 0;
			
			if( numPoints > 0 )
			{
				total += length( points[0].pos - anchor.pos );
				for( int i = 1; i < numPoints; ++i )
				{
					total += length( points[i].pos - points[i-1].pos );
				}
				total += length( points[numPoints-1].pos - playerPos );
			}
			else
			{
				total += length( anchor.pos - playerPos );
			}
			totalLength = total;

			if( totalLength > maxTotalLength )
			{
				state = RELEASED;
			}

			if( player->ground == NULL && hitStallCounter >= hitStallFrames && triggerDown )
			{
				state = PULLING;
			}
			else if( player->ground != NULL && hitStallCounter >= hitStallFrames && prevTriggerDown && !triggerDown )
			{
				state = RELEASED;
			}
			break;
		}
	case PULLING:
		{
			if( !triggerDown )
			{
				state = RELEASED;
			}
			if( touchEdgeWithWire )
			{
				state = RELEASED;
			}

			double total = 0;
			
			if( numPoints > 0 )
			{
				total += length( points[0].pos - anchor.pos );
				for( int i = 1; i < numPoints; ++i )
				{
					total += length( points[i].pos - points[i-1].pos );
				}
				total += length( points[numPoints-1].pos - playerPos );
			}
			else
			{
				total += length( anchor.pos - playerPos );
			}
			totalLength = total;

			if( totalLength > maxTotalLength )
			{
				state = RELEASED;
			}
			
			break;
		}
	case RELEASED:
		{
			Reset();
			break;
		}
	}

	switch( state )
	{
	case IDLE:
		{
			//no updates
			break;
		}
	case FIRING:
		{
			rcEdge = NULL;
			//rcQuantity = 0;
			
			rayCancel = false;
			RayCast( this, player->owner->terrainTree->startNode, playerPos, playerPos + fireDir * fireRate * (double)(framesFiring + 1 ) );
			
			if( rayCancel )
			{
				Reset();
			}

			//cout << "framesFiring " << framesFiring << endl;

			if( rcEdge != NULL )
			{
				if( rcQuant < 4 )
				{
					//cout << "lock1" << endl;
					anchor.pos = rcEdge->v0;
				}
				else if( rcQuant > length( rcEdge->v1 - rcEdge->v0 ) - 4 )
				{
					//cout << "lock2" << endl;
					anchor.pos = rcEdge->v1;
				}
				else
				{
					anchor.pos = rcEdge->GetPoint( rcQuant );
				}
				
				anchor.e = rcEdge;
				anchor.quantity = rcQuant;
				numPoints = 0;
			}
			break;
		}
	case HIT:
		{


			if( hitStallCounter < hitStallFrames )
				hitStallCounter++;
			break;
		}
	case PULLING:
		{
			double total = 0;
			
			if( numPoints > 0 )
			{
				total += length( points[0].pos - anchor.pos );
				for( int i = 1; i < numPoints; ++i )
				{
					total += length( points[i].pos - points[i-1].pos );
				}
				total += length( points[numPoints-1].pos - playerPos );
				//cout << "multi: " << "numPoints: " << numPoints << " , " << total << endl;
			}
			else
			{
				total += length( anchor.pos - playerPos );
				//cout << "single: " << total << endl;
			}
			//totalLength = total;
			//if( total < totalLength )
				totalLength = total;

			V2d wn;

			if( numPoints == 0 )
			{
				//if( totalLength < segmentLength )
					segmentLength = totalLength;
				wn = normalize( anchor.pos - playerPos );
				//cout << "segment length single: " << segmentLength << endl;
			}
			else
			{
				double temp = length( points[numPoints-1].pos - playerPos );
				//if( temp < segmentLength )
				{
					segmentLength = temp;
				}
				wn = normalize( points[numPoints-1].pos - playerPos );
				//cout << "segment length multi: " << segmentLength << endl;
			}
			
			bool shrinkInput = false;

			
			if( wn.y <= 0 )
			{
				if( wn.x < 0 )
				{
					shrinkInput = currInput.LLeft();
				}
				else if( wn.x > 0 )
				{
					shrinkInput = currInput.LRight();
				}

				shrinkInput |= currInput.LUp();
			}
			else if( wn.y > 0 )
			{
				if( wn.x < 0 )
				{
					shrinkInput = currInput.LLeft();
				}
				else if( wn.x > 0 )
				{
					shrinkInput = currInput.LRight();
				}

				shrinkInput |= currInput.LDown();
			}

			shrinkInput = false;

			if( currInput.B )
			{
				shrinkInput = true;
			}
			else if( currInput.Y )
			{
				if( triggerDown && player->ground == NULL )
				{
					segmentLength += pullStrength;
					totalLength += pullStrength;
				}
			}

			if( shrinkInput && triggerDown && player->ground == NULL )
			//if( false )
			{
				//totalLength -= pullStrength;

				
				double segmentChange = pullStrength;
				if( segmentLength - pullStrength < minSegmentLength )
					segmentChange = minSegmentLength - (segmentLength - pullStrength);

				totalLength -= segmentChange;
				segmentLength -= segmentChange;
			}
			break;
		}	
	case RELEASED:
		{
			break;
		}
	}

	++frame;
	if( frame / animFactor > 5 )
	{
		frame = 0;
	}
}

void Wire::SortNewPoints( int start, int end )
{
	/*int first;
	for( int i = end; i >= start; --i )
	{
		first = start;
		for( int j = 1; j <= i; ++j )
		{
			if( points[j].angleDiff < points[first].angleDiff )
			{
				first = 
			}
		}
	}*/
}

void Wire::SwapPoints( int aIndex, int bIndex )
{
	WirePoint temp = points[aIndex];
	points[aIndex] = points[bIndex];
	points[bIndex] = temp;
}

void Wire::UpdateAnchors( V2d vel )
{
	//V2d playerPos = player->position;
	//playerPos += V2d( offset.x, offset.y );
	V2d playerPos = GetOriginPos(true);

	if( state == HIT || state == PULLING )
	{
		if( oldPos.x == storedPlayerPos.x && oldPos.y == storedPlayerPos.y )
		{
			//return;
		}
		
		/*for( int i = numPoints - 1; i >= 0; --i )
		{ 
			double result = cross( playerPos - points[numPoints-1].pos, points[i].test );
			if( result > 0 )
			{
				//cout << "removing point " << result << endl;
				numPoints--;
			}
			else
			{
				break;
			}
		}*/
		
		
		oldPos = storedPlayerPos;////playerPos - vel;//

	/*	if( vel.x == 0 && vel.y == 0 )
		{
			if( player->action == Actor::DASH )
				cout << "dash" << endl;
			else if( player->action == Actor::STAND )
				cout << "stand" << endl;
		cout << "oldpos: " << oldPos.x << ", " << oldPos.y << endl;
		cout << "player: " << playerPos.x << ", " << playerPos.y << " using " << player->position.x << ", " << player->position.y << endl;
		}*/

		double radius = length( realAnchor - playerPos ); //new position after moving

		if( numPoints == 0 )
		{
			//line->append( sf::Vertex(sf::Vector2f(anchor.pos.x, anchor.pos.y), Color::Black) );
			realAnchor = anchor.pos;
		}
		else
		{
			//line->append( sf::Vertex(sf::Vector2f(points[numPoints - 1].pos.x, points[numPoints - 1].pos.y), Color::Black) );
			realAnchor = points[numPoints-1].pos;
		}



		int counter = 0;
		while( true )
		{
			
			if( counter > 1 )
			{
				cout << "COUNTER: " << counter << endl;
			}

			V2d a = realAnchor - oldPos;
			V2d b = realAnchor - playerPos;
			double len = max( length( a ), length( b ) );

			/*double left = min( realAnchor.x, min( oldPos.x, playerPos.x ) );
			double right = max( realAnchor.x, max( oldPos.x, playerPos.x ) );
			double top = min( realAnchor.y, min( oldPos.y, playerPos.y ) );
			double bottom = max( realAnchor.y, max( oldPos.y, playerPos.y ) );*/
			V2d oldDir = oldPos - realAnchor;
			V2d dir = playerPos - realAnchor;
			double left;
			double right;
			double top;
			double bottom;

			if( ( oldDir.x < 0 && oldDir.y < 0 && dir.x > 0 && dir.y < 0 ) || ( dir.x < 0 && dir.y < 0 && oldDir.x < 0 && oldDir.y < 0 ) )
			{
				top = realAnchor.y - len;
			}
			else
			{
				top = min( realAnchor.y, min( oldPos.y, playerPos.y ) );
			}

			if( ( oldDir.x < 0 && oldDir.y < 0 && dir.x < 0 && dir.y > 0 ) || ( dir.x < 0 && dir.y < 0 && oldDir.x < 0 && oldDir.y > 0 ) )
			{
				left = realAnchor.x - len;
			}
			else
			{
				left = min( realAnchor.x, min( oldPos.x, playerPos.x ) );
			}

			if( ( oldDir.x > 0 && oldDir.y < 0 && dir.x > 0 && dir.y > 0 ) || ( dir.x > 0 && dir.y < 0 && oldDir.x > 0 && oldDir.y > 0 ) )
			{
				right = realAnchor.x + len;
			}
			else
			{
				right = max( realAnchor.x, max( oldPos.x, playerPos.x ) );
			}

			if( ( oldDir.x < 0 && oldDir.y > 0 && dir.x > 0 && dir.y > 0 ) || ( dir.x < 0 && dir.y > 0 && oldDir.x > 0 && oldDir.y > 0 ) )
			{
				bottom = realAnchor.y + len;
			}
			else
			{
				bottom = max( realAnchor.y, max( oldPos.y, playerPos.y ) );
			}

			double ex = 1;
			Rect<double> r( left - ex, top - ex, (right - left) + ex * 2, (bottom - top) + ex * 2 );
			


			


			//addedPoints = 0;
			foundPoint = false;

			player->owner->terrainTree->Query( this, r );
		

			/*for( int i = 1; i < addedPoints; ++i )
			{
				for( int j = i; j > 0; ++j )
				{

				}
			
			}*/

			if( foundPoint )
			{
				if( numPoints > 1 )
				{
					
					if( (closestPoint.x == points[numPoints-2].pos.x && closestPoint.y == points[numPoints-2].pos.y ) )
					{
						cout << "problem  point: " << closestPoint.x << ", " << closestPoint.y << endl;
						break;
					}
					//assert( !(closestPoint.x == points[numPoints-2].pos.x && closestPoint.y == points[numPoints-2].pos.y ) );
				}


				points[numPoints].pos = closestPoint;
				

				points[numPoints].test = normalize( closestPoint - realAnchor );
				if( !clockwise )
				{
					points[numPoints].test = -points[numPoints].test;
				}
				//points[numPoints].test = normalize(  
				numPoints++;
				//cout << "closestPoint: " << closestPoint.x << ", " << closestPoint.y << endl;
				//cout << "numpoints now! " << numPoints << endl;

				V2d oldAnchor = realAnchor;
				realAnchor = points[numPoints-1].pos;

				radius = radius - length( oldAnchor - realAnchor );
				oldPos = realAnchor + normalize( realAnchor - oldAnchor ) * radius;

				cout << "point added!: " << points[numPoints-1].pos.x << ", " << points[numPoints-1].pos.y << ", numpoints: " << numPoints << endl;
				counter++;
			}
			else
			{
				break;
			}
			//oldPos = 
			
		}
		
		//if( rcEdge != NULL )
		if( false )
		{
			if( rcQuant > length( rcEdge->v1 - rcEdge->v0 ) - rcQuant )
			{
				points[numPoints].pos = rcEdge->v1;
				//wirePoints[pointNum].e = rcEdge;
				points[numPoints].test = normalize(rcEdge->edge1->v1 - rcEdge->edge1->v0 );
				//cout << "over" << endl;

				numPoints++;
			}
			else
			{
				//cout << "under" << endl;
				points[numPoints].pos = rcEdge->v0;
				points[numPoints].test = normalize( rcEdge->edge0->v1 - rcEdge->edge0->v0 );
				numPoints++;
			}
		}

		for( int i = numPoints - 1; i >= 0; --i )
		{ 
			double result = cross( playerPos - points[numPoints-1].pos, points[i].test );
			if( result > 0 )
			{
				//cout << "removing point " << result << endl;
				numPoints--;
			}
			else
			{
				break;
			}
		}
	}
	else if( state == FIRING )
	{
		//oldPos = playerPos - vel;
		oldPos = storedPlayerPos;
		V2d wireVec = fireDir * fireRate * (double)(framesFiring + 1 );
		
		V2d diff = playerPos - oldPos;
		
		V2d wirePos = playerPos + wireVec; 
		V2d oldWirePos = oldPos + wireVec;

		quadOldPosA = oldPos;
		quadOldWirePosB = oldWirePos;
		quadWirePosC = wirePos;
		quadPlayerPosD = playerPos;

		double top = min( quadOldPosA.y, min( quadOldWirePosB.y, min( quadWirePosC.y, quadPlayerPosD.y ) ) );
		double bot = max( quadOldPosA.y, max( quadOldWirePosB.y, max( quadWirePosC.y, quadPlayerPosD.y ) ) );
		double left = min( quadOldPosA.x, min( quadOldWirePosB.x, min( quadWirePosC.x, quadPlayerPosD.x ) ) );
		double right = max( quadOldPosA.x, max( quadOldWirePosB.x, max( quadWirePosC.x, quadPlayerPosD.x ) ) );

		//cout << "A: " << quadOldPosA.x << ", " << quadOldPosA.y << ", B: " << quadOldWirePosB.x << ", " << quadOldWirePosB.y << 
		//	", C: " << quadWirePosC.x << ", " << quadWirePosC.y << ", D: " << quadPlayerPosD.x << ", " << quadPlayerPosD.y << endl;
		double ex = 1;
		sf::Rect<double> r( left - ex, top - ex, (right - left) + ex * 2, ( bot - top ) + ex * 2 );
		//cout << "diff: " << diff.x << ", " << diff.y << ", size: " << r.width << ", " << r.height << endl;
		/*sf::RectangleShape *rs = new RectangleShape( Vector2f( r.width, r.height ) );
		rs->setPosition( left, top );
		rs->setFillColor( Color( 0, 255, 0, 100 ) );
		progressDraw.push_back( rs );*/
		//cout << "rect is: left: " << left << ", right: " << right << ", top: " << top << ", bot:" << bot << endl;
		if( r.width == 0 || r.height == 0 )
		{

		}
		else
		{
			//cout << "query" << endl;
			minSideEdge = NULL;
			player->owner->terrainTree->Query( this, r );
			if( minSideEdge != NULL )
			{
				storedPlayerPos = playerPos;
				state = HIT;
				numPoints = 0;
				anchor.pos = minSideEdge->v0;
				anchor.quantity = 0;
				anchor.e = minSideEdge;
				UpdateAnchors( V2d( 0, 0 ) );
			}
		}
	}
	

	/*CircleShape *cs = new CircleShape;
	cs->setFillColor( Color::Red );
	cs->setRadius( 4 );
	cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
	cs->setPosition( playerPos.x, playerPos.y );
	progressDraw.push_back( cs );*/
	storedPlayerPos = playerPos;
}

void Wire::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
{
	if( edge->edgeType == Edge::BORDER || edge->edgeType == Edge::CLOSED_GATE )
	{
		rayCancel = true;
		return;
	}
	else if( edge->edgeType == Edge::OPEN_GATE )
	{
		return;
	}
	//rayPortion > 1 &&
	//V2d playerPos = player->position;
	//playerPos += V2d( offset.x, offset.y );	
	V2d playerPos = GetOriginPos(true);

	if( rayPortion > .1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - playerPos ) < length( rcEdge->GetPoint( rcQuant ) - playerPos ) ) )
	{
		rcEdge = edge;
		rcQuant = edgeQuantity;
	}
}

void Wire::TestPoint( Edge *e )
{
	V2d p = e->v0;

	/*if( abs( p.x - points[numPoints-1].pos.x ) < 1 && abs( p.y - points[numPoints-1].pos.y ) < 1 )
	{
		//cout << "that was a close one" << endl;
		return;
	}*/
	
	//V2d playerPos = player->position;
	//playerPos += V2d( offset.x, offset.y );
	V2d playerPos = GetOriginPos(true);

	if( length( p - realAnchor ) < 1 ) //if applied to moving platforms this will need to account for rounding bugs.
	{
		return;
	}

	double radius = length( realAnchor - playerPos ); //new position after moving

	double anchorDist = length( realAnchor - p );
	if( anchorDist > radius )
	{
		return;
	}
	
	//cout << "anchordist: " << anchorDist << ", radius: " << radius << endl;

	V2d oldVec = normalize( oldPos - realAnchor );
	V2d newVec = normalize( playerPos - realAnchor );

	//cout << "old: " << oldPos.x << ", " << oldPos.y << endl;
	//cout << "new: " << playerPos.x << ", " << playerPos.y << endl;

	/*sf::VertexArray *line = new VertexArray( sf::Lines, 0 );
	line->append( sf::Vertex(sf::Vector2f(realAnchor.x, realAnchor.y), Color::Black ) );
	line->append( sf::Vertex(sf::Vector2f(oldPos.x, oldPos.y), Color::Black ) );
	line->append( sf::Vertex(sf::Vector2f(realAnchor.x, realAnchor.y), Color::Black ) );
	line->append( sf::Vertex(sf::Vector2f(playerPos.x, playerPos.y), Color::Black ) );*/


	//progressDraw.push_back( line );

	V2d pVec = normalize( p - realAnchor );

	double oldAngle = atan2( oldVec.y, oldVec.x );
	
	

	double newAngle = atan2( newVec.y, newVec.x );
	

	double pAngle = atan2( pVec.y, pVec.x );
	
	double angleDiff = abs( oldAngle - pAngle );

	double maxAngleDiff = abs( newAngle - oldAngle );

	

	//if( angleDiff > maxAngleDiff )
	//	return;

	if( oldAngle < 0 )
		oldAngle += 2 * PI;
	if( newAngle < 0 )
		newAngle += 2 * PI;
	if( pAngle < 0 )
		pAngle += 2 * PI;

	/*CircleShape *cs = new CircleShape;
	cs->setFillColor( Color::Green );
	cs->setRadius( 4 );
	cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
	cs->setPosition( p.x, p.y );
	progressDraw.push_back( cs );*/

	//cout << "p: " << p.x << ", " << p.y << " old: " << oldAngle << ", new: " << newAngle << ", pangle: " << pAngle << endl;
	bool tempClockwise = false;
	if( newAngle > oldAngle )
	{
		if( newAngle - oldAngle < PI )
		{
			tempClockwise = true;
			//cw
			if( pAngle - oldAngle >= 0 && pAngle - oldAngle <= newAngle - oldAngle )
			{
				//good
			}
			else
			{
				return;
			}
		}
		else
		{
			if( pAngle >= newAngle || pAngle <= oldAngle )
			{
				//cw
			}
			else
			{
				return;
			}
		}
	}
	else if( newAngle < oldAngle )
	{
		if( oldAngle - newAngle < PI )
		{
			//ccw
			if( pAngle - newAngle >= 0 && pAngle - newAngle <= oldAngle - newAngle )
			{
				//good
			}
			else
			{
				return;
			}
		}
		else
		{
			tempClockwise = true;
			if( pAngle <= newAngle || pAngle >= oldAngle )
			{
				//ccw
			}
			else
			{
				return;
			}
		}
	}
	else
	{
		return;
	}

	

	//would be more efficient to remove this calculation and only do it once per frame
	clockwise = tempClockwise;
	
	if( !foundPoint )
	{
		foundPoint = true;
		closestDiff = angleDiff;
		closestPoint = p;
		//
	}
	else
	{
		double closestDist = length( realAnchor - closestPoint );
		//not sure if switching the order of these does anything
		if( angleDiff < closestDiff )
		{
			closestDiff = angleDiff;
			closestPoint = p;
			//cout << "closestPoint: " << p.x << ", " << p.y << endl;
		}
		else if( approxEquals( angleDiff, closestDiff ) )
		{
			if( anchorDist < closestDist )
			{
				closestDiff = angleDiff;
				closestPoint = p;
			}
		}
		
	}
}

void Wire::HandleEntrant( QuadTreeEntrant *qte )
{
	Edge *e = (Edge*)qte;

	if( state == FIRING )
	{
		V2d along = normalize( quadOldWirePosB - quadOldPosA );
		V2d other = normalize( quadOldPosA - quadPlayerPosD);

		double alongQ = dot( e->v0 - quadOldPosA, along );
		double otherQ = cross( e->v0 - quadOldPosA, along );

		//cout << "checking: " << e->v0.x << ", " << e->v0.y << ", along/other: " << alongQ << ", " << otherQ 
		//	<< ", alongLen: " << length( quadOldWirePosB - quadOldPosA ) << ", otherLen: " << length( quadOldPosA - quadPlayerPosD ) << endl;
		if( -otherQ >= 0  && -otherQ <= length( quadOldPosA - quadPlayerPosD ) )
		{
			if( alongQ >= 0 && alongQ <= length( quadOldWirePosB - quadOldPosA ) )
			{
				if( minSideEdge == NULL 
					|| ( minSideEdge != NULL 
						&& ( otherQ < minSideOther 
						|| ( otherQ == minSideOther && alongQ < minSideAlong ) ) ) )
				{
					minSideOther = otherQ;
					minSideAlong = alongQ;
					minSideEdge = e;
			//		cout << "setting to: " << e->v0.x << ", " << e->v0.y << endl;
				}
			}
		} 
	}
	else
	{
		V2d v0 = e->v0;
		V2d v1 = e->v1;
		TestPoint( e );	
	}
}

//make multiples of the quads for each edge later
void Wire::UpdateQuads()
{
	//if( state == FIRING )
	//	++framesFiring;
	
	V2d playerPos = GetOriginPos(false);

	//cout << "starting update quads" << endl;
	V2d alongDir;// = fireDir;
	V2d otherDir;// = fireDir;
	double temp;// = otherDir.x;
	//otherDir.x = otherDir.y;
	//otherDir.y = -temp;

	int tileHeight = 6;
	int startIndex = 0;
	bool hitOrPulling = (state == HIT || state == PULLING );
	bool singleRope = ( hitOrPulling && numPoints == 0 );
	
	if( state == FIRING || singleRope || (state == HIT || state == PULLING ) )
	{
		for( int pointI = numPoints; pointI >= 0; --pointI )
		{
		V2d currWirePos;
		V2d currWireStart;
		if( hitOrPulling )
		{	
			if( pointI == 0 )
			{
				if( numPoints == 0 )
				{
					currWirePos = anchor.pos;
					currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					//alongDir = normalize(currWirePos - playerPos);
				//	cout << "only rope from anchor to player" << endl;
				}
				else
				{
					currWirePos = anchor.pos;
					currWireStart = points[0].pos;
					//alongDir = normalize( anchor.pos - currWirePos );
				//	cout << "anchor to first point" << endl;
				}
			}
			else
			{
				if( pointI == numPoints )
				{
					currWirePos = points[pointI-1].pos;
					currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					//alongDir = normalize( currWirePos - playerPos );
					
				//	cout << "beginning rope from player to points" << endl;
				}
				else
				{
					currWirePos = points[pointI-1].pos;
					currWireStart = points[pointI].pos;
				//	alongDir = normalize( points[pointI-1].pos - points[pointI].pos );
				//	cout << "middle of rope point: " << pointI << " to " << pointI-1 << endl;
				}
			}
			alongDir = normalize( currWirePos - currWireStart );
			otherDir = alongDir;
			temp = otherDir.x;
			otherDir.x = otherDir.y;
			otherDir.y = -temp;
		}
		else if( state == FIRING )
		{
			alongDir = fireDir;
			otherDir = alongDir;
			temp = otherDir.x;
			otherDir.x = otherDir.y;
			otherDir.y = -temp;
			currWirePos = playerPos + fireDir * fireRate * (double)framesFiring;
			currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
		}
		
		
		int firingTakingUp = ceil( length( currWirePos - currWireStart ) / tileHeight );


		V2d endBack = currWirePos - otherDir * quadHalfWidth;
		V2d endFront = currWirePos + otherDir * quadHalfWidth;

		//cout << "fram: " << frame / animFactor << endl;
		Vector2f topLeft( 0, tileHeight * frame / animFactor );
		Vector2f topRight( 6, tileHeight * frame / animFactor );
		Vector2f bottomLeft( 0, tileHeight * (frame / animFactor + 1 ) );
		Vector2f bottomRight( 6, tileHeight * (frame / animFactor + 1 ) );
		if( firingTakingUp > quads.getVertexCount() / 4 )
		{
			cout << "firingTakingup: " << firingTakingUp << ", count: " << quads.getVertexCount() / 4 << endl;
			assert( false );
		}

		//assert( firingTakingUp <= quads.getVertexCount() / 4 );
		//startIndex is 0
		//cout << "fireTakingUp: " << firingTakingUp << endl;


		V2d startPartial;
		V2d endPartial;
		
		//cout << "startIndex: " << startIndex << ", firingTakingUp: " << firingTakingUp << endl;
		for( int j = 0; j < firingTakingUp; ++j )
		{
			startPartial = ( currWireStart + alongDir * (double)(tileHeight * j) );
			endPartial = ( currWireStart + alongDir * (double)(tileHeight * ( j + 1 )) );
			
			
			//cout << "a: " << frame / animFactor << ", " << (tileHeight * (frame / animFactor)) << "startPartial: " << startPartial.x << ", " << startPartial.y << endl;
			//cout << "b: " << frame / animFactor + 1 << ", " << (tileHeight * (frame / animFactor + 1)) << "endPartial: " << endPartial.x << ", " << endPartial.y << endl;

			int diff = tileHeight * (j + 1) - length( currWirePos - currWireStart );
		
			if( diff > 0 )
			{
			//	cout << "j: " << j << ", firetu: " << firingTakingUp - 1 << endl;
				assert( j == firingTakingUp - 1 );
				//realTopLeft.y += diff;
				//realTopRight.y += diff;
			//	cout << "diff: " << diff << " len: " << length( currWirePos - playerPos ) << " currfirepos: " << currWirePos.x << ", " 
			//		<< currWirePos.y << " pos: " << playerPos.x << ", " << playerPos.y << endl;
				endPartial = currWirePos;
			}

			V2d startPartialBack = startPartial - otherDir * quadHalfWidth;
			V2d startPartialFront = startPartial + otherDir * quadHalfWidth;

			V2d endPartialBack = endPartial - otherDir * quadHalfWidth;
			V2d endPartialFront = endPartial + otherDir * quadHalfWidth;
			int index = (j + startIndex );
			quads[index*4].position = Vector2f( startPartialBack.x, startPartialBack.y );
			quads[index*4+1].position = Vector2f( startPartialFront.x, startPartialFront.y );
			quads[index*4+2].position = Vector2f( endPartialFront.x, endPartialFront.y );
			quads[index*4+3].position = Vector2f( endPartialBack.x, endPartialBack.y );

			int trueFrame = frame / animFactor;

			Vector2f realTopLeft = topLeft;
			Vector2f realTopRight = topRight;
			Vector2f realBottomRight= bottomRight;
			Vector2f realBottomLeft = bottomLeft;

			realTopLeft.y = tileHeight * (frame/animFactor);
			realTopRight.y = tileHeight * (frame/animFactor);
			realBottomRight.y = tileHeight * (frame/animFactor + 1);
			realBottomLeft.y = tileHeight * (frame/animFactor + 1);

			if( !right )
			{
				realTopLeft.y += 36;
				realTopRight.y += 36;
				realBottomRight.y += 36;
				realBottomLeft.y += 36;
			}
			quads[index*4].texCoords = realTopLeft;
			quads[index*4+1].texCoords = realTopRight;
			quads[index*4+2].texCoords = realBottomRight;
			quads[index*4+3].texCoords = realBottomLeft;
		}

		startIndex += firingTakingUp;

		}
		//cout << "clearing: " << startIndex << " and beyond" << endl;
		for( ; startIndex < quads.getVertexCount() / 4; ++startIndex )
		{
			quads[startIndex*4].position = Vector2f( 0, 0 );
			quads[startIndex*4+1].position = Vector2f( 0, 0 );
			quads[startIndex*4+2].position = Vector2f( 0, 0 );
			quads[startIndex*4+3].position = Vector2f( 0, 0 );
		}
	}

	if( state == FIRING )
		++framesFiring;

	//cout << "ending update quads" << endl;
}

void Wire::Draw( RenderTarget *target )
{
	if( state == FIRING || state == HIT || state == PULLING )
	{
		target->draw( quads, ts_wire->texture );
	}
	
	if( state == FIRING )
	{
		sf::Vertex line[] =
		{
			sf::Vertex(sf::Vector2f(player->position.x, player->position.y), Color::Blue),
			sf::Vertex(sf::Vector2f(player->position.x + fireDir.x * 40 * framesFiring,
			player->position.y + fireDir.y * 40 * framesFiring), Color::Magenta)
		};

		//target->draw(line, 2, sf::Lines);
			
	}
	else if( state == HIT || state == PULLING )
	{
		//V2d wirePos = wireEdge->GetPoint( wireQuant );
		if( numPoints == 0 )
		{
			sf::Vertex line0[] =
			{
				sf::Vertex(sf::Vector2f( player->position.x, player->position.y ), Color::Red),
				sf::Vertex(sf::Vector2f( anchor.pos.x, anchor.pos.y ), Color::Magenta)
			};

		//	target->draw(line0, 2, sf::Lines);
		}
		else
		{
			sf::Vertex line0[] =
			{
				sf::Vertex(sf::Vector2f( points[numPoints-1].pos.x, points[numPoints-1].pos.y ), Color::Red),
				sf::Vertex(sf::Vector2f( player->position.x, player->position.y ), Color::Magenta)
			};

		//	target->draw(line0, 2, sf::Lines);
		}

		if( numPoints > 0 )
		{
			sf::Vertex line1[] =
			{
				sf::Vertex(sf::Vector2f( anchor.pos.x, anchor.pos.y ), Color::Red),
				sf::Vertex(sf::Vector2f( points[0].pos.x, points[0].pos.y ), Color::Magenta)
			};

		//	target->draw(line1, 2, sf::Lines);
		}

		for( int i = 1; i < numPoints; ++i )
		{
		
			sf::Vertex line[] =
			{
				sf::Vertex(sf::Vector2f(points[i-1].pos.x, points[i-1].pos.y ), Color::Red),
				sf::Vertex(sf::Vector2f(points[i].pos.x, points[i].pos.y ), Color::Magenta)
			};

		//	target->draw(line, 2, sf::Lines);
		}

		CircleShape cs1;
		cs1.setFillColor( Color::Red );
		cs1.setRadius( 2 );
		cs1.setOrigin( cs1.getLocalBounds().width / 2, cs1.getLocalBounds().height / 2 );
		cs1.setPosition( anchor.pos.x, anchor.pos.y );

		target->draw( cs1 );

		for( int i = 0; i < numPoints; ++i )
		{
			CircleShape cs;
			cs.setFillColor( Color::Cyan );
			cs.setRadius( 2 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( points[i].pos.x, points[i].pos.y );

			target->draw( cs );
		}
	}
}

void Wire::DebugDraw( RenderTarget *target )
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		target->draw( *(*it) );
	}
	//progressDraw.clear();
}

void Wire::ClearDebug()
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		delete (*it);
	}
	progressDraw.clear();
}

void Wire::Reset()
{
	state = IDLE;
	numPoints = 0;
	framesFiring = 0;
	frame = 0;
}

V2d Wire::GetOriginPos( bool test )
{
	offset = player->GetWireOffset();

	if( player->facingRight )
	{
		offset.x = -abs( offset.x );
	}
	else
	{
		offset.x = abs( offset.x );
	}
	V2d playerPos;
	double angle = player->GroundedAngle();
	double x = sin( angle );
	double y = -cos( angle );
	V2d gNormal( x, y ); 
	V2d other( -gNormal.y, gNormal.x );

	if( player->ground != NULL )
	{
		V2d pp = player->ground->GetPoint( player->edgeQuantity );
		playerPos = pp + gNormal * player->normalHeight;
	}
	else
	{
		playerPos = player->position;
	}

	if( test )
		playerPos = player->position;

	playerPos += gNormal * (double)offset.y + other * (double)offset.x;
	return playerPos;
}

//actor should change the offset every frame based on its info. need a before movement wire position and a post movement wire position consistently