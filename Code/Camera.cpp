#include "Camera.h"
#include "VectorMath.h"
#include <iostream>
#include "GameSession.h"
#include "Movement.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>


Camera::Camera()
{
	manual = false;
	bossCrawler = false;
	rumbling = false;

	offset.x = 0;
	offset.y = 0;

	pos.x = 0;
	pos.y = 0;

	zoomFactor = 1;
	zoomOutRate = 1;
	zoomInRate = .1;
	offsetRate = 3;
	maxZoom = 2.25;//3.5;

	zoomLevel1 = 0;
	zoomLevel2 = .5;
	zoomLevel3 = 1.75;

	minZoom = 1;
	maxOffset.x = 100 * 5;//10;
	maxOffset.y = 100 * 5;//10;

	left = 300;
	right = -300;
	top = 150;
	bottom = -150;

	zoomLevel = 0;
	
	easeOutFrame = 0;
	easingOut = false;

	currMove = NULL;

	rX = 0;
	rY = 0;
}

void Camera::EaseOutOfManual( int frames )
{
	SetManual( false );
	//manual = false;
	easingOut = true;
	easeOutCount = frames;
	easeOutFrame = 0;
	manualPos = pos;
}

void Camera::UpdateReal( Actor *player )
{
	GameSession *owner = player->owner;
	V2d playerPos = player->position;

	numActive = 0;
	//double numActive = 0;
	V2d diffSum( 0, 0 );
	double distanceFactor = .8;
	V2d vPos = V2d( pos.x, pos.y );

	double minX = pos.x - 480;// * zoomFactor;//playerPos.x - 480;
	double maxX = pos.x + 480;// * zoomFactor;//playerPos.x + 480;
	double minY = pos.y - 270;// * zoomFactor;//playerPos.y - 270;
	double maxY = pos.y + 270;// * zoomFactor;//playerPos.y + 270;

//	double numActive = 0;
	//double xLimit = 

	//double maxDistance = 0;
	Enemy *curr = owner->activeEnemyList;
	while( curr != NULL )
	{
		//bool alwaysExclude = (curr->type == Enemy::BASICEFFECT || length( playerPos - curr->position ) > 900
		//	|| curr->dead);
		//bool extra = (curr->type == Enemy::GHOST || curr->type == Enemy::GORILLA );
		//have stuff here for relative movers so 
		//bool sometimesExclude = curr->type == Enemy::GHOST
		if( length( playerPos - curr->position ) > 900 || !curr->affectCameraZoom )
		{
			curr = curr->next;
			continue;
		}

		


		++numActive;
		double len = length( curr->position - vPos );

	//	if( len > maxDistance )
	//		maxDistance = len;
		V2d dir = normalize( curr->position - vPos );
		
		if( curr->position.x < minX )
			minX = curr->position.x;
		else if( curr->position.x > maxX )
			maxX = curr->position.x;

		if( curr->position.y < minY )
			minY = curr->position.y;
		else if( curr->position.y > maxY )
			maxY = curr->position.y;
		
		//diffSum += dir * len;
		//cout << "len: " << len << endl;
		//cout << "pos: " << curr->position.x << ", " << curr->position.y << ", " << endl;

		//double tw = maxX - minX;
		//double th = maxY - minY;
		double tw = max( abs( maxX - playerPos.x ), abs( minX - playerPos.x ) ) * 2.0;
		double th = max( abs( maxY - playerPos.y ), abs( minY - playerPos.y ) ) * 2.0;

		double w= tw/ 960.0;
		double h= th/ 540.0;

		//cout << "ydiff: " << curr->position.y - player->position.y << endl;
		//cout << "distance seen: " << (th / 2.0) << endl;

		//cout << "test ratio: " << max( w, h ) << endl;
	
		curr = curr->next;
	}


	if( numActive > 0 )
	{
		//cout << "num active: " << numActive << endl;
		if( framesActive < 60 )
			framesActive++;
		
	}
	else
	{
		framesActive--;
		if( framesActive < 0 )
			framesActive = 0;
	}

	/*double add = (double)framesActive / 60 * 125;
	maxX += add;
	minX -= add;
	maxY += add;
	minY -= add;*/

	//cout << "num enemies: " << numActive << endl;

	sf::Vector2f center( (minX + maxX) / 2, (minY + maxY) / 2 );
	double width = maxX - minX;
	double height = maxY - minY;

	double wRatio = width / 960.0;
	double hRatio = height / 540.0;

	double tw = max( abs( maxX - playerPos.x ), abs( minX - playerPos.x ) ) * 2;
	double th = max( abs( maxY - playerPos.y ), abs( minY - playerPos.y ) ) * 2;

	double w= tw/ 960.0;
	double h= th/ 540.0;

	//cout << "ydiff: " << curr->position.y - player->position.y << endl;
	//cout << "distance seen: " << (th / 2.0) << endl;

	
	//cout << "test ratio: " << max( w, h ) << endl;

	//double ratio = max( wRatio, hRatio );
	double ratio = max( w, h );
	
	ratio = max( ratio, 1.0 );

	//offset.x = center.x - playerPos.x;
	//offset.y = center.y - playerPos.y;

	//cout << "wRatio: " << wRatio << ", ratio: " << ratio << endl;

	//zoomLevel = 0;
	//zoomFactor = ratio;

	testOffset.x = center.x - playerPos.x;//center.x;
	testOffset.y = center.y - playerPos.y;//center.y;
	testZoom = ratio;
	//pos.x = center.x;
	//pos.y = center.y;
	//pos.x = playerPos.x + offset

	if( numActive > 0 )
	{
		diffSum = diffSum / (double)numActive;
	}

	if( numActive > 0 )
	{
	//	offX = diffSum.x;
	}
	if( numActive > 0 )
	{
	//	offY = diffSum.y;
	}

	//pos.x = playerPos.x;
	//pos.y = playerPos.y;
	//zoomFactor = 1;
}

void Camera::SetRumble( int xFactor, int yFactor, int duration )
{
	assert( duration > 0 );

	rumbleLength = duration;
	rumbleX = xFactor;
	rumbleY = yFactor;
	rumbling = true;
	rumbleFrame = 0;
}

//make sure to rumble off when you reset the level
void Camera::UpdateRumble()
{
	if( !rumbling )
		return;

	if( rumbleFrame > 0 )
	{
		pos.x -= rX;
		pos.y -= rY;
	}
	
	if( rumbleFrame == rumbleLength )
	{
		rumbling = false;
		return;
	}

	int f = ( rumbleFrame % 3 ) - 1;
	
	rX = f * rumbleX;
	rY = f * rumbleY;

	//pos += sf::Vector2f( rX, rY );

	++rumbleFrame;
	
}

void Camera::Set( sf::Vector2f &p, float zFactor, int zLevel )
{
	assert( manual );
	pos.x = p.x;
	pos.y = p.y;

	UpdateRumble();

	zoomFactor = zFactor;
	zoomLevel = zLevel;
	
}

void Camera::Update( Actor *player )
{
	if( manual || player->action == Actor::SPAWNWAIT || player->action == Actor::INTRO
		|| player->action == Actor::EXIT )
	{
		if( manual && currMove != NULL )
		{
			for( int i = 0; i < NUM_STEPS; ++i )
			{
				currMove->Update();
			}
			

			if( relativeMoveSeq )
			{
				pos = sequenceStartPos + Vector2f( currMove->position.x, currMove->position.y );
				//cout << "
			}
			else
			{
				pos = Vector2f( currMove->position.x, currMove->position.y );
			}

			if( currMove->currMovement == NULL )
				currMove = NULL;

			
		}
		UpdateRumble();
	//	UpdateRumble();
		return;
	}

	if( bossCrawler )
	{
		return;
	}

	

	if( player->currInput.LDown() )
	{
	//	maxOffset.y = 900;
	}
	else
	{
	//	maxOffset.y = 500;
	}
	GameSession *owner = player->owner;
	V2d ideal;// = player->position;
	//pos.x = player->position.x;
	//pos.y = player->position.y;
	V2d playerPos = player->position;


	if( player->action == Actor::RIDESHIP || player->action == Actor::SKYDIVE )
	{
		zoomFactor = 1;
		pos.x = playerPos.x;
		pos.y = playerPos.y;
		return;
	}
	//bool desp = player->desperationMode || player->action == Actor::DEATH;
	//if( desp )
	//{
	//	offset.x = 0;
	//	offset.y = 0;
	//	pos.x = playerPos.x;
	//	pos.y = playerPos.y;
	//	zoomLevel = 0;
	//	zoomFactor = 1;
	//	return;
	//	//return;
	//}

	if( owner->pauseFrames > 0 )
	{
		if( owner->pauseFrames % 2 == 0 )
		{
		//	offset.y -= 3;
		}
		else
		{
		//	offset.y += 3;
		}
		
		pos.x = playerPos.x + offset.x;
		pos.y = playerPos.y + offset.y;
	}

	//if( desp )
	//	return;
	
	ControllerState & con = player->currInput;
	ControllerState & prevcon = player->prevInput;

	if( con.PUp() && !prevcon.PUp() )
	{
		
		zoomLevel -= .5;
		if( zoomLevel < 0 )
			zoomLevel = 0;
	}
	else if( con.PDown() && !prevcon.PDown() )
	{
		zoomLevel += .5;
		if( zoomLevel > 3 )
			zoomLevel = 3;
	}


	V2d pVel;
	
	double cap = 30;
	if( player->grindEdge != NULL )
	{
		//cout << "here!" << endl;
		V2d grindDir = normalize( player->grindEdge->v1 - player->grindEdge->v0 );
		V2d otherDir = grindDir;
		double oTemp = otherDir.x;
		otherDir.x = otherDir.y;
		otherDir.y = -oTemp;
		
		if( player->framesGrinding < cap )
		{
			
			playerPos += otherDir * player->normalHeight * (1 - player->framesGrinding / cap );
			//cout << "height on: " << playerPos.y << endl;
		}
		pVel = grindDir * player->grindSpeed;
		//cout << "grindspeed: " << player->grindSpeed << endl;
		
		if( player->reversed )
		{
			pVel = -pVel;
		}
	}
	else if( player->ground != NULL )
	{
		double cap2 = cap;
		if( player->framesGrinding < cap )
		{
			//cout << "bad trigger" << endl;
			player->framesNotGrinding = cap - player->framesGrinding;
			//cap2 = player->framesGrinding;
		}
		
		if( player->framesNotGrinding <= cap2 )
		{	
			V2d otherDir;
			if( player->ground != NULL )
			{
				otherDir = -player->ground->Normal();
				
				V2d offset = otherDir * player->normalHeight * (1 - player->framesNotGrinding / cap2);
				playerPos += offset;
				//cout << "height off: " << playerPos.y << endl;
				//cout << "offset: " << offset.x << ", " << offset.y << ", framesNotGrinding: " << player->framesNotGrinding << endl;
			}
		}

		if( player->action != Actor::JUMPSQUAT )
		{
			pVel = normalize( player->ground->v1 - player->ground->v0 ) * player->groundSpeed;
		}
		else
		{
			pVel = normalize( player->ground->v1 - player->ground->v0 ) * player->storedGroundSpeed;
		}
		
		if( player->reversed )
		{
			pVel = -pVel;
		}
	}
	else
	{
		pVel = player->velocity;
	}

	pVel.x /= player->slowMultiple;
	pVel.y /= player->slowMultiple;
		

	if( pVel.y > 0 )
	{
		if( framesFalling < 60 )
			framesFalling++;
	}
	else 
	{
		if( framesFalling > 0 )
			framesFalling--;
	}

	float temp;
	V2d f;
	double kk = 18.0;
	if( player->ground != NULL )
	{
		temp = abs(player->groundSpeed) / kk;
		f = normalize(player->ground->v1 - player->ground->v0 ) * player->groundSpeed * 10.0;
		//if( abs(temp - zoomFactor) > 1 )
		if( player->reversed )
			f = -f;
		
	}
	else
	{
		temp = length( player->velocity ) / kk;
		//temp = abs( player->velocity.x ) / 20.0;
		//temp = zoomFactor;
		f = player->velocity * 10.0;
	}

	double zDiff = temp - zoomFactor;
	//double zDiff = testZoom - zoomFactor;
	if( zDiff > 0 )
	{
		zoomFactor += zDiff / 100.0/*35.0*/ / player->slowMultiple;
	}
	else if( zDiff < 0 )
	{
		//if( numActive == 0 )
	//	zoomFactor += zDiff / 350.0 / player->slowMultiple;
		double oldZoom = zoomFactor;
		bool more = oldZoom > testZoom;
		if( numActive == 0 || more )
		{
			zoomFactor += zDiff / 350.0 / player->slowMultiple;
			if( more )
			{
				if( zoomFactor < testZoom )
					zoomFactor = testZoom;
			}
		}
		//cout << "zooming in!" << endl;
		/*if( numActive > 0 && oldZoom > testZoom && zoomFactor < testZoom )
		{
			cout << "stopping zoom in: " << zoomFactor << endl;
			zoomFactor = testZoom;		
		}*/
	}
	
	
	//cout << "zdiff: " << zDiff << ", zoomfactor: " << zoomFactor << ", test: " << testZoom << endl;
	//zoomFactor = ( zoomFactor + testZoom ) / 2.0;
	//zoomFactor = testZoom;


	if( zoomFactor < 1 )
		zoomFactor = 1;
	else if( zoomFactor > maxZoom )
		zoomFactor = maxZoom;

	//cout << "zoomFactor: " << zoomFactor << endl;
	pos.x = playerPos.x;
	pos.y = playerPos.y;
	

	//double offX = pVel.x * .7;
	//double offXMax = 2;

	double offX = pVel.x * .8;//1.0;
	double offXMax = 10;

	//offX = (offX + testOffset.x) / 2.0;

	if( offX > offXMax  )
	{
		offX = offXMax ;
	}
	else if( offX < -offXMax  )
	{
		offX = -offXMax;
	}
	offset.x += offX;//pVel.x * 1.001;
	//offset.x = ( offset.x + testOffset.x ) / 2.0;
	//offset.x = testOffset.x;
	//offset.y += pVel.y * .3;

	double offY = pVel.y * .8;//1.0;
	double offYMax = 10;
	
	//offY = (offY + testOffset.y) / 2.0;

	if( offY > offYMax  )
	{
		offY = offYMax ;
	}
	else if( offY < -offYMax  )
	{
		offY = -offYMax;
	}

	//if( pVel.y > 0 )
	//	offset.y += offY;
	//	//offset.y += pVel.y;
	//
	//else if( pVel.y < 0 && player->ground != NULL )
	//{
	//	offset.y += offY;//pVel.y;
	//}
	//else if( pVel.y < 0 )
	//{
	//	if( offY < -1 )
	//		offY = -1;
	//	offset.y += offY; 
	//}

	offset.y += offY;
	//offset.y = ( offset.y + testOffset.y ) / 2.0;
	//offset.y = testOffset.y;

	if( pVel.x == 0 )
	{
		offset.x += (-offset.x) / 40;
	}
	if( pVel.y == 0 )
	{	
		offset.y += (-offset.y) / 20;
	}
	

	int xLimit = 250;
	if( offset.x < -xLimit * zoomFactor )
		offset.x = -xLimit * zoomFactor;
	else if( offset.x > xLimit * zoomFactor )
		offset.x = xLimit * zoomFactor;

	//25
	if( offset.y < -75 * zoomFactor )
		offset.y = -75 * zoomFactor;
	else if( offset.y > 125 * zoomFactor )
		offset.y = 125 * zoomFactor;

	pos.x += offset.x;
	pos.y += offset.y;

	//is >> leftBounds;
	//	is >> topBounds;
	//	is >> boundsWidth;
	//	is >> boundsHeight;

	float halfw = 960 / 2;
	float halfh = 540 / 2;

	float xChangePos = 0, xChangeNeg = 0, yChangePos = 0, yChangeNeg = 0;

	float left = pos.x - halfw * zoomFactor;

	float tdiff = owner->leftBounds - left;
	if( tdiff > 0 )
	{
		xChangePos = tdiff;
	}
	float right = pos.x + halfw * zoomFactor;

	tdiff =  right - (owner->leftBounds + owner->boundsWidth);
	if( tdiff > 0 )
	{
		xChangeNeg = -tdiff;
	}
	float top = pos.y - halfh * zoomFactor;
	tdiff = owner->topBounds - top;
	if( tdiff > 0 )
	{
		yChangePos = tdiff;
	}

	float bot = pos.y + halfh * zoomFactor;
	tdiff = bot - (owner->topBounds + owner->boundsHeight);
	if( tdiff > 0 )
	{
		yChangeNeg = -tdiff;
	}


	
	Barrier *leftb = NULL, *rightb = NULL, *topb = NULL, *botb = NULL;
	for(list<Barrier*>::iterator it = owner->barriers.begin(); it != owner->barriers.end(); ++it )
	{
		if( (*it)->triggered )
		{
			continue;
		}

		
		if( (*it)->x )
		{
			if( (*it)->positiveOpen )
			{
				float diff = (*it)->pos - left;
				if( diff > 0 )
				{
					if( diff > xChangePos )
					{
						xChangePos = diff;
						leftb = (*it);
					}
					//pos.x += diff;
					//cout << "moving right" << endl;
					//offset.x += (*it)->pos - left;
				}
			}
			else
			{
				float diff =  right - (*it)->pos;
				if( diff > 0 )
				{
					//cout << "moving left: " << diff << endl;
					//pos.x -= right - (*it)->pos;
					//offset.x -= diff;
					if( -diff < xChangeNeg )
					{
						xChangeNeg = -diff;
						rightb = (*it);
					}
					//pos.x -= diff;
				}
			}
		}
		else
		{
			if( (*it)->positiveOpen )
			{
				float diff = (*it)->pos - top;
				if( diff > 0 )
				{
					if( diff > yChangePos )
					{
						yChangePos = diff;
						topb = (*it);
					}
					//offset.y += (*it)->pos - top;
					//pos.y += diff;
				}
			}
			else
			{
				float diff = bot - (*it)->pos;
				if( diff > 0 )
				{
					if( -diff < yChangeNeg )
					{
						yChangeNeg = -diff;
						botb = (*it);
					}
					//pos.y -= diff;
					//offset.y -= bot - (*it)->pos;
				}
			}
		}
	}

	Vector2f barrierOffset;
	float barrierZoom = -1;
	if( xChangePos != 0 && xChangeNeg != 0 )
	{
		float diff = rightb->pos - leftb->pos;//xChangePos - xChangeNeg;
		barrierZoom = diff / 960.f + .5f;
		//cout << "diff: " << diff << endl;
		//cout << "bzoom: " << barrierZoom << endl;
		pos.x = (rightb->pos + leftb->pos ) / 2.f;
		//barrierOffset.x = ( xChangePos + xChangeNeg ) / 2.f;
	}
	else if( xChangePos != 0 )
	{
		barrierOffset.x = xChangePos;
	}
	else if( xChangeNeg != 0 )
	{
		barrierOffset.x = xChangeNeg;
	}

	if( yChangePos != 0 && yChangeNeg != 0 )
	{
		//barrierOffset.y = ( yChangePos + yChangeNeg ) / 2.f;
	}
	else if( yChangePos != 0 )
	{
		barrierOffset.y = yChangePos;
	}
	else if( yChangeNeg != 0 )
	{
		barrierOffset.y = yChangeNeg;
	}

	pos += barrierOffset;




	UpdateReal( player );


	double blah = .01;
	
	if( numActive > 0 )
	{
		//cout << "testZoom: " << testZoom << endl;
		if( testZoom > zoomFactor )
		{
			//cout << "old zoomfactor : " << zoomFactor << " new: ";
			zoomFactor += blah;
			if( zoomFactor > testZoom )
				zoomFactor = testZoom;
			//cout << zoomFactor << endl;
		}
		/*else if( testZoom < zoomFactor )
		{
			zoomFactor -= blah;
			if( zoomFactor < testZoom )
				zoomFactor = testZoom;
		}*/

		//zoomFactor = zoomFactor * .5 + testZoom * .5;
	}

	if( barrierZoom > 0)
	{
		zoomFactor = barrierZoom;
	}


	if( zoomFactor < 1 )
		zoomFactor = 1;
	else if( zoomFactor > maxZoom )
		zoomFactor = maxZoom;
	//cout << "zf: " << zoomFactor << endl;
	//cout << "zoomfactor: " << zoomFactor << endl;
	//zoomFactor = testZoom;
	if( pVel.y > 0 )
	{
		ideal.y += top * zoomFactor;
	}
	else if( pVel.y < 0 )
	{
		ideal.y += bottom * zoomFactor;
	}

	if( pVel.x > 0 )
	{
		ideal.x += left * zoomFactor;
	}
	else if( pVel.x < 0 )
	{
		ideal.x += right * zoomFactor;
	}

	double camWidth = 960 * GetZoom();
	double camHeight = 540 * GetZoom();

	double rightExtra = (pos.x + camWidth / 2) - ( owner->leftBounds + owner->boundsWidth );
	double leftExtra = owner->leftBounds - ( pos.x - camWidth / 2 );
	double botExtra = (pos.y + camHeight / 2) - (owner->topBounds + owner->boundsHeight );
	double topExtra = owner->topBounds - (pos.y - camHeight / 2 );

	if( rightExtra > 0 )
	{
		//cout << "moving left" << endl;
		//pos.x -= rightExtra;
	}
	else if( leftExtra > 0 )
	{
		//cout << "moving right" << endl;
		//pos.x += leftExtra;
	}
	
	if( botExtra > 0 )
	{
		//cout << "moving up" << endl;
		//pos.y -= botExtra;
	}
	else if( topExtra > 0 )
	{
		//cout << "moving down" << endl;
		//pos.y += topExtra;
	}	

	if( easingOut )
	{
		//startManualPos.x = player->position.x;
		//startManualPos.y = player->position.y;
		//cout << "easing frame: " << easeOutFrame << endl;
		double r = (double)easeOutFrame / easeOutCount;
		sf::Vector2<double> p;
		p = r * sf::Vector2<double>(pos.x, pos.y) + (1.0-r) * sf::Vector2<double>( manualPos.x, manualPos.y );
		pos = Vector2f( p.x, p.y );
		zoomFactor = r * zoomFactor + (1-r) * manualZoom;
		
		++easeOutFrame;
		if( easeOutFrame == easeOutCount )
		{
			easingOut = false;
			
		}
		//return;
	}

	UpdateRumble();




	//offset.y += (double)framesFalling / 60 * 100.0;


	//cout << "offset: " << offset.x << ", " << offset.y << endl;
	/*double numActive = 0;
	Enemy *curr = owner->activeEnemyList;
	while( curr != NULL )
	{
		++numActive;
		curr = curr->next;
	}
	cout << "num active: " << numActive << endl;*/
}

void Camera::UpdateVS( Actor *a, Actor *a2 )
{

	float targetZoomFactor = zoomFactor;

	V2d center = (a->position + a2->position) / 2.0;

	Vector2f res( 1920 / 2, 1080 / 2 );
	double distx = abs( a->position.x - a2->position.x );
	double disty = abs( a->position.y - a2->position.y );

	double zx = distx / res.x;
	double zy = disty / res.y;
	
	double maxDist = max( zx, zy );

	if( zx == maxDist )
	{
		targetZoomFactor = zx;// / 2.0;

		double adistx = distx + max( 200.f * targetZoomFactor, 200.f );
		double azx = adistx / res.x;
		targetZoomFactor = azx;
	}
	else if( zy == maxDist )
	{
		targetZoomFactor = zy;// / 2.0;

		double adisty = disty + max( 200.f * targetZoomFactor, 200.f );
		double azy = adisty / res.y;
		targetZoomFactor = azy;
	}
	
	

	//double adistx = distx + 200;
	//double adisty = disty + 200;

	
	//double azy = adisty / res.y;
	
	

	
	//zoomFactor += 200 / res.x;

	double minZoom = .6;//1.0;
	double maxZoom = 8;
	if( targetZoomFactor < minZoom )
		targetZoomFactor = minZoom;
	else if( targetZoomFactor > maxZoom )
		targetZoomFactor = maxZoom;

	float rz = .25;
	zoomFactor = targetZoomFactor * rz + zoomFactor * (1-rz);
	
	float r = .25;
	pos.x = center.x * r + pos.x * (1-r) ;
	pos.y = center.y * r + pos.y * (1-r);

	//cout << "zoomfactor: " << zoomFactor << endl;
}

void Camera::Update2( Actor *player )
{
	ControllerState & con = player->currInput;
	ControllerState & prevcon = player->prevInput;

	if( con.PUp() && !prevcon.PUp() )
	{
		if( zoomLevel == zoomLevel1 )
		{
			zoomLevel = zoomLevel3;
		}
		else if( zoomLevel == zoomLevel2 )
			zoomLevel = zoomLevel1;
		else if( zoomLevel = zoomLevel3 )
			zoomLevel = zoomLevel2;
	}
	else if( con.PDown() && !prevcon.PDown() )
	{
		if( zoomLevel == zoomLevel1 )
		{
			zoomLevel = zoomLevel2;
		}
		else if( zoomLevel == zoomLevel2 )
			zoomLevel = zoomLevel3;
		else if( zoomLevel = zoomLevel3 )
			zoomLevel = zoomLevel1;
	}

	float temp;
	V2d f;
	if( player->ground != NULL )
	{
		temp = abs(player->groundSpeed) / 20.0;
		f = normalize(player->ground->v1 - player->ground->v0 ) * player->groundSpeed * 10.0;
		//if( abs(temp - zoomFactor) > 1 )
		if( player->reversed )
			f = -f;
		
	}
	else
	{
		temp = length( player->velocity ) / 20.0;
		//temp = abs( player->velocity.x ) / 20.0;
		//temp = zoomFactor;
		f = player->velocity * 10.0;
	}

	double zDiff = temp - zoomFactor;
	zoomFactor += zDiff / 30.0 / player->slowMultiple;

	if( zoomFactor < 1 )
		zoomFactor = 1;
	else if( zoomFactor > maxZoom )
		zoomFactor = maxZoom;

	/*if( abs( f.x - offset.x ) <= offsetRate )
	{
		offset.x = f.x;
		offset.y = f.y;
	}
	else if( f.x > offset.x )
	{
		offset.x += offsetRate;
	}
	else if( f.x < offset.x )
	{
		offset.x -= offsetRate;
	}

	if( abs( f.y - offset.y ) <= offsetRate )
	{
		offset.x = f.x;
		offset.y = f.y;
	}

	if( offset.x < -maxOffset.x )
		offset.x = -maxOffset.x;
	else if( offset.x > maxOffset.x )
		offset.x = maxOffset.x;


	if( abs( f.y - offset.y ) <= offsetRate )
	{
		offset.x = f.x;
		offset.y = f.y;
	}
	else if( f.y > offset.y )
	{
		offset.y += offsetRate;
	}
	else if( f.y < offset.y )
	{
		offset.y -= offsetRate;
	}

	if( abs( f.y - offset.y ) <= offsetRate )
	{
		offset.x = f.x;
		offset.y = f.y;
	}

	if( offset.y < -maxOffset.y )
		offset.y = -maxOffset.y;
	else if( offset.y > maxOffset.y )
		offset.y = maxOffset.y;*/

	sf::Vector2f diff =sf::Vector2f( f.x, f.y ) - offset;
	if( length( V2d( diff.x, diff.y ) ) < 90 )
	{
		V2d dn = normalize( V2d( diff.x, diff.y ) ) * 90.0;

	
		//if( length( V2d(offset.x,offset.y) ) > length( V2d( offset.x,offset.y) + V2d(diff.x,diff.y) / 90.0 / (double)player->slowMultiple * (double)zoomFactor ) )
		{
		
			diff.x = dn.x;
			diff.y = dn.y;
		}
		
		
	}
	sf::Vector2f tOffset =  diff / 90.f / (float)player->slowMultiple * zoomFactor;
	//offset.x += tOffset.x;
	//offset.y += tOffset.y;
	V2d to = (V2d( offset.x, offset.y) * ( 60.0 * player->slowMultiple - 1 ) + f) / ( 60.0 * player->slowMultiple);
	offset.x = to.x;
	offset.y = to.y;


	
	//zoom out should be different than zoom in
	//offset += diff / 90.f / (float)player->slowMultiple * zoomFactor;

	if( abs( offset.x ) < 1 )
		offset.x = 0;
	if( abs( offset.y ) < 1 )
		offset.y = 0;

	if( f.x > 0 )
	{
	//	offset.x += offsetRate;
	}
	else if( f.x < 0 )
	{
		//offset.x -= offsetRate;
	}
	

	pos.x = player->position.x;// - player->offsetX;
	pos.y = player->position.y;

	//offset.x = 
	//offset.y = floor( offset.y + .5f );
	//cout << "zoom: " << zoomFactor << endl;
	
	//pos.x += floor( offset.x + .5f );
	//pos.y += floor( offset.y + .5f );
	
	pos.x += offset.x * 2;
	//if( offset.y < 0 )
		//offset.y = 0;
	pos.y += offset.y;
//	cout << "offset: " << offset.x << ", " << offset.y << endl;
//	zoomFactor = 1;
}

void Camera::SetManual( bool man )
{
	manual = man;
	currMove = NULL;
	if( man )
	{ 
		startManualPos = pos;
		startManualZoom = zoomFactor;
	}
	else
	{
		manualPos = pos;
		manualZoom = zoomFactor;
	}
}

float Camera::GetZoom()
{
	return zoomLevel + zoomFactor;
}

sf::Vector2f Camera::GetPos()
{
	return pos + Vector2f( rX, rY );
}

void Camera::SetMovementSeq( MovementSequence *move, bool relative )
{
	currMove = move;
	relativeMoveSeq = relative;
	sequenceStartPos = pos;
	sequenceStartZoom = GetZoom();
}

