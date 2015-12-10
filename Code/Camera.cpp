#include "Camera.h"
#include "VectorMath.h"
#include <iostream>
#include "GameSession.h"

using namespace std;

#define V2d sf::Vector2<double>

Camera::Camera()
{
	offset.x = 0;
	offset.y = 0;

	pos.x = 0;
	pos.y = 0;

	zoomFactor = 1;
	zoomOutRate = 1;
	zoomInRate = .1;
	offsetRate = 3;
	maxZoom = 3.5;

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
	
}

void Camera::Update( Actor *player )
{
	GameSession *owner = player->owner;
	V2d ideal;// = player->position;
	//pos.x = player->position.x;
	//pos.y = player->position.y;
	V2d playerPos = player->position;
	bool desp = player->desperationMode || player->action == Actor::DEATH;
	if( desp )
	{
		offset.x = 0;
		offset.y = 0;
		pos.x = playerPos.x;
		pos.y = playerPos.y;
		zoomLevel = 0;
		zoomFactor = 1;
		return;
		//return;
	}

	if( owner->pauseFrames > 0 )
	{
		if( owner->pauseFrames % 2 == 0 )
		{
			offset.y -= 3;
		}
		else
		{
			offset.y += 3;
		}
		
		pos.x = playerPos.x + offset.x;
		pos.y = playerPos.y + offset.y;
	}

	if( desp )
		return;
	
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

		pVel = normalize( player->ground->v1 - player->ground->v0 ) * player->groundSpeed;
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
		

		

	//zoomFactor = abs(pVel.x) / 40.0 + 1;//length( pVel ) / 40 + 1;
	//zoomFactor = 2;
	//zoomFactor = 1;

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
	if( zDiff > 0 )
	{
		zoomFactor += zDiff / 20.0/*35.0*/ / player->slowMultiple;
	}
	else if( zDiff < 0 )
	{
	//	zoomFactor += zDiff / 350.0 / player->slowMultiple;
		zoomFactor += zDiff / 250.0 / player->slowMultiple;
	}

	

	if( zoomFactor < 1 )
		zoomFactor = 1;
	else if( zoomFactor > maxZoom )
		zoomFactor = maxZoom;

	//cout << "zoomFactor: " << zoomFactor << endl;
	pos.x = playerPos.x;
	pos.y = playerPos.y;
	
	double offX = pVel.x;
	double offXMax = 5;
	if( offX > offXMax  )
	{
		offX = offXMax ;
	}
	else if( offX < -offXMax  )
	{
		offX = -offXMax;
	}
	offset.x += offX;//pVel.x * 1.001;
	//offset.y += pVel.y * .3;

	double offY = pVel.y;
	double offYMax = 3;
	if( offY > offYMax  )
	{
		offY = offYMax ;
	}
	else if( offY < -offYMax  )
	{
		offY = -offYMax;
	}

	if( pVel.y > 0 )
		offset.y += offY;
		//offset.y += pVel.y;
	
	else if( pVel.y < 0 && player->ground != NULL )
	{
		offset.y += offY;//pVel.y;
	}
	else if( pVel.y < 0 )
	{
		if( offY < -1 )
			offY = -1;
		offset.y += offY; 
	}

	if( pVel.x == 0 )
	{
		offset.x += (-offset.x) / 40;
	}
	if( pVel.y == 0 )
	{
		offset.y += (-offset.y) / 20;
	}
	//cout << "pVel.y: " << pVel.y << endl;
	//cout << "offset.y << " << offset.y << " add: " << pVel.y * 1.0000001  << "what: " << pVel.y << endl;
	if( offset.x < -150 * zoomFactor )
		offset.x = -150 * zoomFactor;
	else if( offset.x > 150 * zoomFactor )
		offset.x = 150 * zoomFactor;

	if( offset.y < -50 * zoomFactor )
		offset.y = -50 * zoomFactor;
	else if( offset.y > 150 * zoomFactor )
		offset.y = 150 * zoomFactor;

	pos.x += offset.x;
	pos.y += offset.y;


	/*if(pVel.x > 0 )
	{
		pos.x += left;
	}
	else if( pVel.x < 0 )
	{
		pos.x -= bottom;
	}*/

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


	//zoomFactor = 1;
	
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

float Camera::GetZoom()
{
	return zoomLevel + zoomFactor;
}