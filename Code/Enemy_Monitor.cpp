#include "Enemy.h"
#include <iostream>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

Monitor::Monitor( GameSession *owner )
	:Enemy( owner, Enemy::MONITOR )
{
}

void Monitor::HandleEntrant( QuadTreeEntrant *qte )
{
	//empty
}

void Monitor::UpdatePrePhysics()
{
	//empty
}

void Monitor::UpdatePhysics()
{
	pair<bool,bool> hitMe = PlayerHitMe();
	bool ihit = IHitPlayer();
	if( ihit || hitMe.first )
	{
		cout << "got the monitor!" << endl;
		//get rid of me
	}

	
	//empty
}

void Monitor::UpdatePostPhysics()
{
	//empty
}

void Monitor::Draw( sf::RenderTarget *target)
{
}

void Monitor::DrawMinimap( sf::RenderTarget *target )
{
}

bool Monitor::IHitPlayer()
{
}

void Monitor::UpdateHitboxes()
{
}

std::pair<bool,bool> Monitor::PlayerHitMe()
{
}

bool Monitor::PlayerSlowingMe()
{
}

void Monitor::DebugDraw(sf::RenderTarget *target)
{
}

void Monitor::SaveEnemyState()
{
}

void Monitor::LoadEnemyState()
{
}

void Monitor::Reset()
{
}

void Monitor::ResetEnemy()
{
}