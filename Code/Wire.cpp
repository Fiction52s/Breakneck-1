#include "Wire.h"
#include "Actor.h"
#include "GameSession.h"
#include <iostream>
#include <assert.h>

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>

Wire::Wire( Actor *p, bool r)
	:state( IDLE ), numPoints( 0 ), framesFiring( 0 ), fireRate( 200/*120*/ ), maxTotalLength( 10000 ), maxFireLength( 5000 ), minSegmentLength( 128 )//50 )
	, player( p ), hitStallFrames( 10 ), hitStallCounter( 0 ), pullStrength( 10 ), right( r )
	, extraBuffer( 64 ), 
	quads( sf::Quads, (int)((ceil( maxTotalLength / 8.0 ) + extraBuffer) * 4 )), 
	minimapQuads( sf::Quads, (int)((ceil( maxTotalLength / 8.0 ) + extraBuffer) * 4 )),
	//eventually you can split this up into smaller sections so that they don't all need to draw
  quadHalfWidth( 4 ), ts_wire( NULL ), frame( 0 ), animFactor( 2 ), offset( 8, 18 ),
  numTotalCharges( 0 ), chargeVA( sf::Quads, MAX_CHARGES * 4 )
{
	//ts_wire = player->owner->GetTileset( "wire.png", 6, 36 );
	ts_wire = player->owner->GetTileset( "wire_01_10x8.png", 10, 8 );
	if( r )
	{
		ts_miniHit = player->owner->GetTileset( "rain_64x64.png", 64, 64 );
	}
	else
	{
		ts_miniHit = player->owner->GetTileset( "rain_64x64.png", 64, 64 );
	}

	ts_wireCharge = player->owner->GetTileset( "wirecharge_32x32.png", 32, 32 );

	minSideEdge = NULL;
	minSideOther = -1;
	minSideAlong = -1;

	triggerDown = false;
	prevTriggerDown = false;

	retractSpeed = 20;//60;

	numAnimFrames = 8;

	//numTotalCharges = 0;

	activeChargeList = NULL;
	inactiveChargeList = NULL;
	//charges start at the earliest retraction points so they hit the most targets.
	for( int i = 0; i < MAX_CHARGES; ++i )
	{
		CreateWireCharge();
	}
	

	//lockEdge = NULL;
}

void Wire::ActivateCharges()
{
	for( int i = 0; i < numPoints; ++i )
	{
		ActivateWireCharge( i );
	}
}

void Wire::UpdateState( bool touchEdgeWithWire )
{
	//cout << "update state" << endl;
	ControllerState &currInput = player->currInput;
	ControllerState &prevInput = player->prevInput;

	//V2d playerPos = player->position;
	
	V2d playerPos;
	if( state == RETRACTING )
	{
		playerPos = retractPlayerPos;
	}
	else
	{
		playerPos = GetOriginPos(true);
	}
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

	if( right )
	{
		triggerDown = currInput.RightTriggerPressed();//currInput.rightTrigger >= triggerThresh;
		prevTriggerDown = prevInput.RightTriggerPressed();//prevInput.rightTrigger >= triggerThresh;
	}
	else
	{
		triggerDown = currInput.LeftTriggerPressed();//currInput.leftTrigger >= triggerThresh;
		prevTriggerDown = prevInput.LeftTriggerPressed();//prevInput.leftTrigger >= triggerThresh;
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
					double angle = currInput.leftStickRadians;

					double degs = angle / PI * 180.0;
					double sec = 360.0 / 64.0;
					int mult = floor( (degs / sec) + .5 );
					angle = (PI / 32.0) * mult;


					/*angle = angle / ( 360.0  / 64.0 );
					int mult = floor( angle );
					double remain = angle - ( mult * PI / 32.0 );
					if( remain >= PI / 64.0 )
					{
						mult++;
					}

					angle = mult * PI / 32.0;*/


					/*fireDir.x = cos( currInput.leftStickRadians );
					fireDir.y = -sin( currInput.leftStickRadians );*/

					fireDir.x = cos( angle );
					fireDir.y = -sin( angle );
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
				if( !triggerDown )
				{
					canRetractGround = true;
				}
				else
				{
					canRetractGround = false;
				}
				hitStallCounter = framesFiring;
			}

			if( framesFiring * fireRate > maxFireLength )
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
				break;
			}
			else
			{
				if( canRetractGround && !triggerDown && prevTriggerDown )
				{
					state = RETRACTING;

					ActivateCharges();
					//cout << "beginning retracting" << endl;
					retractPlayerPos = playerPos;
					fusePointIndex = numPoints;
					if( numPoints == 0 )
					{
						fuseQuantity = length( anchor.pos - retractPlayerPos );
					}
					else
					{
						if( right )
						{
							fuseQuantity = length( retractPlayerPos - points[numPoints-1].pos );
						}
						else
						{
							fuseQuantity = length( anchor.pos - points[0].pos );
						}
					}
					
					break;
				}
				else
				{
					if( !triggerDown )
					{
						canRetractGround = true;
					}
				}
			}

			if( player->ground == NULL && !touchEdgeWithWire && hitStallCounter >= hitStallFrames && triggerDown
				&& player->oldAction != Actor::WALLCLING && player->action != Actor::WALLCLING 
				&& player->oldAction != Actor::WALLATTACK && player->action != Actor::WALLATTACK 
				&& ( !player->bounceFlameOn || player->framesSinceBounce > 8 || player->oldBounceEdge == NULL ) && player->bounceEdge == NULL )
				//&& player->oldAction != Actor::bouncewa&& player->action != Actor::WALLATTACK )
			{
				//cout << "playeraction: " << player->action << endl;
				//cout << "set state pulling" << endl;
				state = PULLING;
			}
			else
			{
				//cout << "bounce edge: " << player->bounceEdge << ", " << player->framesSinceBounce << ", old: " <<
				//	player->oldBounceEdge << endl;
			}
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
			else
			{
				if( !triggerDown && player->ground == NULL )
				{
					state = RETRACTING;
					ActivateCharges();
					retractPlayerPos = playerPos;
					fusePointIndex = numPoints;
					if( numPoints == 0 )
					{
						fuseQuantity = length( anchor.pos - retractPlayerPos );
					}
					else
					{
						if( right )
						{
							fuseQuantity = length( retractPlayerPos - points[numPoints-1].pos );
						}
						else
						{
							fuseQuantity = length( anchor.pos - points[0].pos );
						}
					}
				}
				if( triggerDown && ( touchEdgeWithWire || player->action == Actor::WALLCLING ) )
				{
					state = HIT;
					if( !triggerDown )
					{
						canRetractGround = true;
					}
					else
					{
						canRetractGround = false;
					}
				}
			}
			break;
		}
	case RETRACTING:
		{
			if( triggerDown && !prevTriggerDown )
			{
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
					double angle = currInput.leftStickRadians;

					double degs = angle / PI * 180.0;
					double sec = 360.0 / 64.0;
					int mult = floor( (degs / sec) + .5 );
					angle = (PI / 32.0) * mult;


					/*angle = angle / ( 360.0  / 64.0 );
					int mult = floor( angle );
					double remain = angle - ( mult * PI / 32.0 );
					if( remain >= PI / 64.0 )
					{
						mult++;
					}

					angle = mult * PI / 32.0;*/


					/*fireDir.x = cos( currInput.leftStickRadians );
					fireDir.y = -sin( currInput.leftStickRadians );*/

					fireDir.x = cos( angle );
					fireDir.y = -sin( angle );
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

				//player->owner->ActivateEffect( ts_miniHit, rcEdge->GetPoint( rcQuant ), true, 0, , 3, facingRight );

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

			if( currInput.A )
			{
				shrinkInput = true;
			}
			else if( currInput.B )
			{
				if( triggerDown && player->ground == NULL )
				{
					segmentLength += pullStrength;
					totalLength += pullStrength;
				}
			}

			bool bounceWindow = (player->action == Actor::BOUNCEAIR && player->framesSinceBounce > 10)
				|| player->action != Actor::BOUNCEAIR;
			if( shrinkInput && triggerDown && player->ground == NULL && totalLength > 128 && bounceWindow )
			{
				double segmentChange = pullStrength;
				if( segmentLength - pullStrength < minSegmentLength )
					segmentChange = minSegmentLength - (segmentLength - pullStrength);

				totalLength -= segmentChange;
				segmentLength -= segmentChange;
			}
			break;
		}
	case RETRACTING:
		{
			UpdateFuse();
			break;
		}
	case RELEASED:
		{
			break;
		}
	}

	++frame;
	if( frame / animFactor == numAnimFrames )
	{
		frame = 0;
	}
}

void Wire::ClearCharges()
{
	cout << "clearing charges" << endl;
	WireCharge *curr = activeChargeList;
	WireCharge *temp;
	while( curr != NULL )
	{
		temp = curr->next;
		DeactivateWireCharge( curr );
		curr = temp;
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
	V2d playerPos = GetOriginPos(true);

	if( state == HIT || state == PULLING )
	{
		if( oldPos.x == storedPlayerPos.x && oldPos.y == storedPlayerPos.y )
		{
			//return;
		}
		
		oldPos = storedPlayerPos;

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
			
			foundPoint = false;

			player->owner->terrainTree->Query( this, r );

			if( foundPoint )
			{
				if( numPoints > 1 )
				{
					if( (closestPoint.x == points[numPoints-2].pos.x && closestPoint.y == points[numPoints-2].pos.y ) )
					{
						cout << "problem  point: " << closestPoint.x << ", " << closestPoint.y << endl;
						assert( 0 );
						break;
					}
				}


				points[numPoints].pos = closestPoint;
				

				points[numPoints].test = normalize( closestPoint - realAnchor );
				if( !clockwise )
				{
					points[numPoints].test = -points[numPoints].test;
				}
				numPoints++;


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
		}
		
		if( false )
		{
			if( rcQuant > length( rcEdge->v1 - rcEdge->v0 ) - rcQuant )
			{
				points[numPoints].pos = rcEdge->v1;
				points[numPoints].test = normalize(rcEdge->edge1->v1 - rcEdge->edge1->v0 );
				numPoints++;
			}
			else
			{
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

		double ex = 1;
		sf::Rect<double> r( left - ex, top - ex, (right - left) + ex * 2, ( bot - top ) + ex * 2 );

		if( r.width == 0 || r.height == 0 )
		{

		}
		else
		{
			minSideEdge = NULL;
			player->owner->terrainTree->Query( this, r );
			if( minSideEdge != NULL )
			{
				storedPlayerPos = playerPos;
				state = HIT;
				if( !triggerDown )
				{
					canRetractGround = true;
				}
				else
				{
					canRetractGround = false;
				}
				numPoints = 0;
				anchor.pos = minSideEdge->v0;
				anchor.quantity = 0;
				anchor.e = minSideEdge;
				UpdateAnchors( V2d( 0, 0 ) );
			}
		}
	}
	
	storedPlayerPos = playerPos;
}

void Wire::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
{
	if( edge->edgeType == Edge::BORDER )
	{

		rayCancel = true;
		return;
	}
	else if( edge->edgeType == Edge::CLOSED_GATE )
	{
		Gate *g = (Gate*)edge->info;
		if( g->type != Gate::BLACK && g->gState != Gate::LOCKFOREVER && g->gState != Gate::HARD )
		{
			rayCancel = true;
			return;
		}
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
	V2d playerPos;
	if( state == RETRACTING )
	{
		playerPos = retractPlayerPos;
	}
	else
	{
		playerPos = GetOriginPos(false);
	}

	V2d alongDir;
	V2d otherDir;
	double temp;

	
	int tileHeight = 8;//6;
	int tileWidth = 10;
	int startIndex = 0;
	bool hitOrPulling = (state == HIT || state == PULLING || state == RETRACTING );
	bool singleRope = ( hitOrPulling && numPoints == 0 );
	int cap = 0;

	int currNumPoints = numPoints;

	//fusePointIndex = numPoints;// - 1;
	//fuseQuantity = 50;
	if( state == FIRING || singleRope || (state == HIT || state == PULLING ) || state == RETRACTING )
	{
		if( state == RETRACTING )
		{
			if( right )
			{
				currNumPoints = fusePointIndex;
				//cap++;
			}
			else
			{
				cap = numPoints - fusePointIndex;
				//currNumPoints = fusePointIndex;
			}
		}
		for( int pointI = currNumPoints; pointI >= cap; --pointI )
		{
		V2d currWirePos;
		V2d currWireStart;
		if( hitOrPulling )
		{	
			if( pointI == cap )
			{
				//cout << "cap" << endl;
				if( numPoints == 0 )
				{

					currWirePos = anchor.pos;
					if( state == RETRACTING )
					{
						if( right )
						{
							V2d start = anchor.pos;//points[pointI-1].pos;
							V2d end = retractPlayerPos;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * fuseQuantity; 
						}
						else
						{
							currWirePos = retractPlayerPos;
							V2d start = retractPlayerPos;//points[pointI-1].pos;
							V2d end = anchor.pos;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * fuseQuantity; 
						}
					}
					else
					{
						currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					}
					//currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
				}
				else
				{
					currWirePos = anchor.pos;
					//currWireStart = points[0].pos;
					if( state == RETRACTING && ( (right && fusePointIndex == 0) || ( !right ) ) )
					{
						if( right )
						{
							V2d start = anchor.pos;//points[0].pos;//points[pointI-1].pos;
							V2d end = points[pointI].pos;
						
							V2d dir = normalize( end - start );
							currWireStart = start + dir * fuseQuantity; 
						}
						else
						{
							V2d start;
							V2d end;
							if( cap == 0 )
							{
								//cout << "cap zero" << endl;
								start = points[0].pos;
								end = anchor.pos;
							}
							else if( cap == numPoints )
							{
								//cout << "cap max" << endl;
								start = retractPlayerPos;
								end = points[cap-1].pos;
								//currWirePos = points[
							}
							else
							{
								//cout << "cap mid" << endl;
								start = points[cap].pos;;
								end = points[cap-1].pos;
							}

							V2d dir = normalize( end - start );

							currWireStart = start;

							currWirePos = start + dir * fuseQuantity;
							//currWirePos = end;
							//currWirePos = end;
							
							//currWireStart = retractPlayerPos;
							//V2d start = retractPlayerPos;
							//V2d end = points[numPoints-1].pos;
							//currWireStart = retractPlayerPos;
							//V2d start = retractPlayerPos;//anchor.pos;
							//V2d end = points[numPoints-1].pos;//points[pointI].pos;

							//V2d dir = normalize( end - start );
							//cout << "final: " << fuseQuantity;
							////currWireStart = start + dir * fuseQuantity; 
							//if( cap == numPoints )
							//{
							//	currWirePos = start + dir * fuseQuantity; 
							//}
							//else
							//{
							//	currWirePos = end;
							//}
							
						}
					}
					/*else if( state == RETRACTING && !right  )
					{

					}*/
					else
					{
						currWireStart = points[0].pos;//playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					}
				}
			}
			else
			{
				//cout << "not cap" << endl;
				if( pointI == currNumPoints )
				{
					
					currWirePos = points[pointI-1].pos;
					
					if( state == RETRACTING )
					{
						currWirePos = points[0].pos;
						if( right )
						{
							V2d start = points[pointI-1].pos;
							V2d end;
							if( pointI == numPoints )
							{
								end = retractPlayerPos;
							}
							else
							{
								end = points[pointI].pos;
							}
							currWirePos = start;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * fuseQuantity; 
						}
						else
						{
							V2d start = retractPlayerPos;
							V2d end = points[numPoints-1].pos;

							V2d dir = normalize( end - start );
							currWirePos = end;
							currWireStart = start;
							//currWirePos = points[0].pos;
							////currWireStart = retractPlayerPos;
							//V2d start = currWirePos;//points[pointI-1].pos;
							//V2d end = anchor.pos;
							//
							//V2d dir = normalize( end - start );
							////if( cap == 0 )
							//{
							//	currWireStart = start + dir * fuseQuantity;
							//}
							////else
							//{
							//	//currWireStart = end;
							//}



							//if( cap == currNumPoints )
							//{
								
							//}
							//else
							//{
							//	currWireStart = end;
							//}
							
							//cout << "STARTING REVERSE: " << fuseQuantity << endl;
						}
					}
					else
					{
						currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					}
				}
				else
				{
					/*if( state == RETRACTING && !right )
					{
						currWireStart = points[pointI-1].pos;
						currWirePos = points[pointI].pos;
					}
					else
					{*/
					if( false )//if( state == RETRACTING && !right )
					{
						cout << "pos ind: " << (numPoints-1)-(pointI-1) << endl;
						cout << "start ind: " << (numPoints-1)-(pointI) << endl;
						currWirePos = points[(numPoints-1)-(pointI-1)].pos;
						currWireStart = points[(numPoints-1) -(pointI)].pos;
					}
					else
					{
						currWirePos = points[pointI-1].pos;
						currWireStart = points[pointI].pos;
					}
						
					//}
					
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
		
		
		firingTakingUp = ceil( length( currWirePos - currWireStart ) / tileHeight );

		
		V2d endBack = currWirePos - otherDir * quadHalfWidth;
		V2d endFront = currWirePos + otherDir * quadHalfWidth;

		//cout << "fram: " << frame / animFactor << endl;
		//Vector2f topLeft( 0, tileHeight * frame / animFactor );
		//Vector2f topRight( tileWidth, tileHeight * frame / animFactor );
		//Vector2f bottomLeft( 0, tileHeight * (frame / animFactor + 1 ) );
		//Vector2f bottomRight( tileWidth, tileHeight * (frame / animFactor + 1 ) );
		if( firingTakingUp > quads.getVertexCount() / 4 )
		{
			cout << "wirestart: " << currWireStart.x << ", " << currWireStart.y << ", curr: " << currWirePos.x << ", " << currWirePos.y << endl;
			cout << "firingTakingup: " << firingTakingUp << ", count: " << quads.getVertexCount() / 4 << endl;
			assert( false );
		}

		V2d startPartial;
		V2d endPartial;
		
		//cout << "startIndex: " << startIndex << ", firingTakingUp: " << firingTakingUp << endl;
		for( int j = 0; j < firingTakingUp; ++j )
		{
			startPartial = ( currWireStart + alongDir * (double)(tileHeight * j) );
			endPartial = ( currWireStart + alongDir * (double)(tileHeight * ( j + 1 )) );

			int diff = tileHeight * (j + 1) - length( currWirePos - currWireStart );
		
			if( diff > 0 )
			{
				assert( j == firingTakingUp - 1 );
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

			double miniExtraWidth = 20;
			startPartialBack -= otherDir * miniExtraWidth;
			startPartialFront += otherDir * miniExtraWidth;
			endPartialBack -= otherDir * miniExtraWidth;
			endPartialFront += otherDir * miniExtraWidth;
			minimapQuads[index*4].position = Vector2f( startPartialBack.x, startPartialBack.y );
			minimapQuads[index*4+1].position = Vector2f( startPartialFront.x, startPartialFront.y );
			minimapQuads[index*4+2].position = Vector2f( endPartialFront.x, endPartialFront.y );
			minimapQuads[index*4+3].position = Vector2f( endPartialBack.x, endPartialBack.y );

			Color miniColor;
			if( right )
			{
				miniColor = Color::Red;
				
			}
			else
			{
				miniColor = Color( 0, 100, 255 );
			}

			minimapQuads[index*4].color = miniColor;
			minimapQuads[index*4+1].color = miniColor;
			minimapQuads[index*4+2].color = miniColor;
			minimapQuads[index*4+3].color = miniColor;

			int trueFrame = frame / animFactor;

			//Vector2f realTopLeft = topLeft;
			//Vector2f realTopRight = topRight;
			//Vector2f realBottomRight= bottomRight;
			//Vector2f realBottomLeft = bottomLeft;

			int fr  = frame/animFactor;
			int ifr = (numAnimFrames-1) - fr;
			int f = ifr;
			if( right )
			{
				f = fr;
			}
			else
			{
				//f = fr;
				//cout << "f: " << f << endl;
				f += numAnimFrames;
			}
			IntRect subRect = ts_wire->GetSubRect( f );
			/*realTopLeft.y = subRect.left;
			realTopRight.y = subRect.left + subRect.width;
			realBottomRight.y = subRect.top + ;
			realBottomLeft.y = tileHeight * (f+1);*/

			/*if( !right )
			{
				realTopLeft.y += tileHeight * numAnimFrames;
				realTopRight.y += tileHeight * numAnimFrames;
				realBottomRight.y += tileHeight * numAnimFrames;
				realBottomLeft.y += tileHeight * numAnimFrames;
			}*/
			quads[index*4].texCoords = Vector2f( subRect.left, subRect.top );//realTopLeft;
			quads[index*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
			quads[index*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
			quads[index*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
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

			minimapQuads[startIndex*4].position = Vector2f( 0, 0 );
			minimapQuads[startIndex*4+1].position = Vector2f( 0, 0 );
			minimapQuads[startIndex*4+2].position = Vector2f( 0, 0 );
			minimapQuads[startIndex*4+3].position = Vector2f( 0, 0 );
		}

		if( state == FIRING )
			++framesFiring;
	}

	if( state == RETRACTING )
	{
		cout << "calling to update all sprites" << endl;
		UpdateChargesSprites();
	}
}

void Wire::Draw( RenderTarget *target )
{
	if( state == FIRING || state == HIT || state == PULLING || state == RETRACTING )
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

	if( state == RETRACTING )
	{
		target->draw( chargeVA, ts_wireCharge->texture );
	}
}

void Wire::DrawMinimap( sf::RenderTarget *target )
{
	if( state == FIRING || state == HIT || state == PULLING || state == RETRACTING )
	{
		target->draw( minimapQuads );
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

void Wire::ActivateRetractionCharges()
{
	if( right )
	{
		int limit = min( numPoints, MAX_CHARGES );
		for( int i = 0; i < limit; ++i )
		{
			ActivateWireCharge( i );
		}
	}
	else
	{
		int limit = min( numPoints, MAX_CHARGES );
		for( int i = 0; i < limit; ++i )
		{
			ActivateWireCharge( (numPoints-1)-i );
		}
	}
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
	ClearCharges();
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

void Wire::UpdateFuse()
{
	int currPoints = fusePointIndex;
	double momentum = retractSpeed;
	while( !approxEquals( momentum, 0 ) )
	{
		if( right )
		{
			if( fuseQuantity > momentum )
			{
				fuseQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				momentum = momentum - fuseQuantity;
				fusePointIndex--;
				if( fusePointIndex == -1 )
				{
					fuseQuantity = 0;
					state = RELEASED;
					ClearCharges();
					//cout << "setting released" << endl;
					return;
				}
				else if( fusePointIndex == 0 )
				{
					fuseQuantity = length( points[fusePointIndex].pos - anchor.pos );
				}
				else
				{
					fuseQuantity = length( points[fusePointIndex].pos - points[fusePointIndex-1].pos );
				}
			}
		}
		else
		{
			if( fuseQuantity > momentum )
			{
				fuseQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				momentum = momentum - fuseQuantity;
				fusePointIndex--;
				if( fusePointIndex == -1 )
				{
					fuseQuantity = 0;
					state = RELEASED;
					ClearCharges();
					return;
				}
				else if( fusePointIndex == 0 )
				{
					//cout << "zero" << endl;
					fuseQuantity = length( points[numPoints - 1].pos - retractPlayerPos );
					//fuseQuantity = length( points[fusePointIndex].pos - anchor.pos );
				}
				else
				{
					//cout << "other: " << fusePointIndex << endl;
					//fuseQuantity = length( points[fusePointIndex].pos - points[fusePointIndex-1].pos );
					fuseQuantity = length( points[numPoints-1 - fusePointIndex].pos 
					- points[(numPoints-1) - (fusePointIndex-1)].pos );
					//cout << "length: " << fuseQuantity << endl;
					
					
				}
			}
		}
	}
}

Wire::WireCharge::WireCharge( Wire*w, int vIndex )
	:vaIndex( vIndex ), wire( w ),edgeIndex( -1 )
{
	prev = NULL;
	next = NULL;
	Reset();
}

void Wire::WireCharge::Reset()
{
	ClearSprite();
	action = INACTIVE;
}

//actor should change the offset every frame based on its info. need a before movement wire position and a post movement wire position consistently
void Wire::WireCharge::UpdatePhysics()
{
	int currPoints = edgeIndex;
	double momentum = wire->retractSpeed / NUM_STEPS;
	while( !approxEquals( momentum, 0 ) )
	{
		if( wire->right )
		{
			if( edgeQuantity > momentum )
			{
				edgeQuantity -= momentum;
				cout << "changed: " << edgeQuantity << ", moment: " << momentum << endl;
				momentum = 0;
			}
			else
			{
				cout << "OTHER: " << edgeQuantity << ", moment: " << momentum << endl;
				momentum = momentum - edgeQuantity;
				edgeIndex--;
				if( edgeIndex == -1 )
				{
					action = INACTIVE;
					ClearSprite();
					wire->DeactivateWireCharge( this );
					return;
				}
				else if( edgeIndex == 0 )
				{
					edgeQuantity = length( wire->points[edgeIndex].pos - wire->anchor.pos );
				}
				else
				{
					cout << "edgeIndex: " << edgeIndex << endl;
					edgeQuantity = length( wire->points[edgeIndex].pos - wire->points[edgeIndex-1].pos );
				}
			}
		}
		else
		{
			if( edgeQuantity > momentum )
			{
				edgeQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				int numPoints = wire->numPoints;
				momentum = momentum - edgeQuantity;
				edgeIndex--;
				if( edgeIndex == -1 )
				{
					wire->DeactivateWireCharge( this );
					return;
				}
				else if( edgeIndex == 0 )
				{
					//cout << "zero" << endl;
					edgeQuantity = length( wire->points[numPoints - 1].pos - wire->retractPlayerPos );
					//fuseQuantity = length( points[fusePointIndex].pos - anchor.pos );
				}
				else
				{
					//cout << "other: " << fusePointIndex << endl;
					//fuseQuantity = length( points[fusePointIndex].pos - points[fusePointIndex-1].pos );
					edgeQuantity = length( wire->points[numPoints-1 - edgeIndex].pos 
					- wire->points[(numPoints-1) - (edgeIndex-1)].pos );
					//cout << "length: " << fuseQuantity << endl;
				}
			}
		}
	}

	if( edgeIndex == 0 )
	{
		V2d dir = normalize( wire->points[0].pos- wire->anchor.pos );
		position = wire->anchor.pos + dir * edgeQuantity;
	}
	else
	{
		V2d dir = normalize( wire->points[edgeIndex].pos - wire->points[edgeIndex-1].pos );
		position = wire->anchor.pos + dir * edgeQuantity;
	}
	//position = 
}

void Wire::WireCharge::UpdatePrePhysics()
{
}

void Wire::WireCharge::HitEnemy()
{
	Reset();
	wire->DeactivateWireCharge( this );
}

void Wire::CreateWireCharge()
{
	WireCharge *charge = new WireCharge( this, numTotalCharges );
	++numTotalCharges;
	if( inactiveChargeList == NULL )
	{
		inactiveChargeList = charge;
	}
	else
	{
		charge->next = inactiveChargeList;
		inactiveChargeList->prev = charge;
		inactiveChargeList = charge;
	}
}

void Wire::ActivateWireCharge( int index )
{
	WireCharge *charge = GetWireCharge();
	if( charge != NULL )
	{
		if( right )
		{
			//cout << "activate!: " << index << endl;
			charge->edgeIndex = index;
			if( index > 0 )
			{
				
				charge->edgeQuantity = length(points[index].pos - points[index-1].pos);
				cout << "setting quant: " << charge->edgeQuantity << endl;
			}
			else
			{
				charge->edgeQuantity = length( points[0].pos - anchor.pos );
				cout << "setting quantb: " << charge->edgeQuantity << endl;
			}
		
			//charge->edgeQuantity = 0;
			charge->position = points[index].pos;
			charge->action = WireCharge::Action::RETRACTING;

			if( activeChargeList == NULL )
			{
				activeChargeList = charge;
				charge->next = NULL;
				charge->prev = NULL;
			}
			else
			{
				charge->next = activeChargeList;
				activeChargeList->prev = charge;
				activeChargeList = charge;
			}
		}
		else
		{
		}
	}
}

void Wire::DeactivateWireCharge( WireCharge *charge )
{
	cout << "removing charge from list!" << endl;
	assert( activeChargeList != NULL );

	if( charge->prev == NULL && charge->next == NULL )
	{
		assert( activeChargeList == charge );
		activeChargeList = NULL;
	}
	else
	{
		if( charge->prev != NULL )
		{
			charge->prev->next = charge->next;
		}

		if( charge->next != NULL )
		{
			charge->next->prev = charge->prev;
		}

		charge->prev = NULL;
		charge->next = NULL;
	}
}

Wire::WireCharge *Wire::GetWireCharge()
{
	if( inactiveChargeList == NULL )
		return NULL;
	else
	{
		if( inactiveChargeList->next == NULL )
		{
			Wire::WireCharge * temp = inactiveChargeList;
			inactiveChargeList = NULL;


			return temp;
		}
		else
		{
			Wire::WireCharge * temp = inactiveChargeList;


			inactiveChargeList = temp->next;
			temp->next->prev = NULL;

			inactiveChargeList->next = NULL;

			return temp;
		}
	}
}

void Wire::WireCharge::UpdateSprite()
{
	if( action == INACTIVE )
	{
		ClearSprite();
		//cout << "updating inactive sprite" << endl;
	}
	else
	{
		//cout << "wire charge sprite update" << endl;
		VertexArray &va = wire->chargeVA;
		IntRect subRect = wire->ts_wireCharge->GetSubRect( 0 );//frame / animFactor );
		//if( owner->player->position.x < position.x )
		//{
		//	subRect.left += subRect.width;
		//	subRect.width = -subRect.width;
		//}
		va[vaIndex*4+0].texCoords = Vector2f( subRect.left, 
			subRect.top );
		va[vaIndex*4+1].texCoords = Vector2f( subRect.left 
			+ subRect.width, 
			subRect.top );
		va[vaIndex*4+2].texCoords = Vector2f( subRect.left 
			+ subRect.width, 
			subRect.top + subRect.height );
		va[vaIndex*4+3].texCoords = Vector2f( subRect.left, 
			subRect.top + subRect.height );

		//va[vaIndex*4+0].color = Color::Red;
		//va[vaIndex*4+1].color = Color::Red;
		//va[vaIndex*4+2].color = Color::Red;
		//va[vaIndex*4+3].color = Color::Red;

		Vector2f p( position.x, position.y );

		Vector2f spriteSize = Vector2f( subRect.width / 2, subRect.height / 2 );
		//spriteSize *= 10.f;
		va[vaIndex*4+0].position = p + Vector2f( -spriteSize.x, -spriteSize.y );
		va[vaIndex*4+1].position = p + Vector2f( spriteSize.x, -spriteSize.y );
		va[vaIndex*4+2].position = p + Vector2f( spriteSize.x, spriteSize.y );
		va[vaIndex*4+3].position = p + Vector2f( -spriteSize.x, spriteSize.y );
	}
}

void Wire::WireCharge::ClearSprite()
{
	VertexArray &va = wire->chargeVA;
	va[vaIndex*4+0].position = Vector2f( 0, 0 );
	va[vaIndex*4+1].position = Vector2f( 0, 0 );
	va[vaIndex*4+2].position = Vector2f( 0, 0 );
	va[vaIndex*4+3].position = Vector2f( 0, 0 );
}

void Wire::UpdateChargesPhysics()
{
	if( state != RETRACTING )
		return;

	WireCharge *curr = activeChargeList;
	while( curr != NULL )
	{
		curr->UpdatePhysics();
		curr = curr->next;
	}
}

void Wire::UpdateChargesPrePhysics()
{
	WireCharge *curr = activeChargeList;
	while( curr != NULL )
	{
		curr->UpdatePrePhysics();
		curr = curr->next;
	}
}

void Wire::UpdateChargesPostPhysics()
{
	WireCharge *curr = activeChargeList;
	while( curr != NULL )
	{
		curr->UpdatePrePhysics();
		curr = curr->next;
	}
}

void Wire::UpdateChargesSprites()
{
	WireCharge *curr = activeChargeList;
	while( curr != NULL )
	{
		//cout << "update sprite---!!: " << endl;
		curr->UpdateSprite();
		curr = curr->next;
	}
}