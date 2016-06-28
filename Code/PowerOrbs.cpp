#include "PowerOrbs.h"
#include "GameSession.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace sf;

PowerOrbs::PowerOrbs( GameSession *owner, bool hasAirDash,
		bool hasGravReverse,
		bool hasBounce,
		bool hasGrind,
		bool hasTimeSlow,
		bool hasWires ): smallOrbVA( sf::Quads, 6 * 4 ), basePos( 96, 64 * 6 + 270 )
{
	testBlah = 0;
	for( int i = 0; i < 7; ++i )
	{
		starVA[i] = new Vertex[12 * 4];
	}
	activeStars = 12;
	starState = 0;
	starFrame = 0;
	activeOrb = 5;


	activeOrb = 5;
	numStars[TEAL0] = 16;
	numStars[TEAL1] = 16;
	numStars[TEAL2] = 16;
	numStars[TEAL3] = 16;
	numStars[TEAL4] = 16;
	numStars[TEAL5] = 16;
	numStars[BLUE] = 16;
	numStars[GREEN] = 16;
	numStars[YELLOW] = 16;
	numStars[ORANGE] = 16;
	numStars[RED] = 16;
	numStars[MAGENTA] = 16;
	
	ts_largeOrbs = owner->GetTileset( "lifeL_192x192.png", 192, 192 );
	ts_smallOrbs = owner->GetTileset( "lifeS_64x64.png", 64, 64 );

	largeOrb.setTexture( *ts_largeOrbs->texture );
	largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( activeOrb ) );
	largeOrb.setOrigin( largeOrb.getLocalBounds().width / 2, largeOrb.getLocalBounds().height / 2 );

	for( int i = 5; i < 11; ++i )
	{
		starPositions[i - 5] = new Vector2f[numStars[(OrbColor)i]];
	}

	ts_charge = owner->GetTileset( "charge_64x64.png", 64, 64 );
	
	bool hasPower[6] = { hasAirDash, hasGravReverse, hasBounce, hasGrind, hasTimeSlow,
		hasWires };
	int powerIndex = 5;
	int tealCounter = 0;

	//Vector2f bPos( basePos.x, basePos.y );
	//cout << "setting position small orb" << endl;
	for( int i = 0; i < 6; ++i )
	{
		smallOrbVA[i*4+0].position = basePos + Vector2f( 0, 0 ) - Vector2f( 0, 64 * i );
		smallOrbVA[i*4+1].position = basePos + Vector2f( 64, 0 ) - Vector2f( 0, 64 * i );
		smallOrbVA[i*4+2].position = basePos + Vector2f( 64, 64 ) - Vector2f( 0, 64 * i );
		smallOrbVA[i*4+3].position = basePos + Vector2f( 0, 64 ) - Vector2f( 0, 64 * i );
	}

	bool end = false;
	for( int i = 5; i >= 0; --i )
	{
		//start at bottom
		while( powerIndex >= 0 )
		{
			if( hasPower[powerIndex] )
			{

				int f = 12 + powerIndex;
				//12 - 17
				sf::IntRect ir = ts_smallOrbs->GetSubRect( f );

				orbColors[i] = (OrbColor)(powerIndex + 6);

				smallOrbVA[i*4+0].texCoords = Vector2f( ir.left, ir.top );
				smallOrbVA[i*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
				smallOrbVA[i*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
				smallOrbVA[i*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );

				--powerIndex;

				if( i == 0 )
				{
					end = true;
				}
				break;

			}
			--powerIndex;
		}

		if( end )
			break;

		if( powerIndex < 0 )
		{
			cout << "teal coloring!!" << tealCounter << endl;
				int f = 5 - tealCounter;//5 - tealCounter;
				//12 - 17
				sf::IntRect ir = ts_smallOrbs->GetSubRect( f );

				orbColors[i] = (OrbColor)(5-tealCounter);
				smallOrbVA[i*4+0].texCoords = Vector2f( ir.left, ir.top );
				smallOrbVA[i*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
				smallOrbVA[i*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
				smallOrbVA[i*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );
				++tealCounter;
		}

	}

	for( int i = 0; i < 6; ++i )
	{
		OrbColor col = orbColors[i];
		cout << "i: " << i << ", col: " << (int)col << endl;
	}

	OrbColor c = orbColors[activeOrb];
	int cc = (int)c;
	largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( cc ) );
	
	largeOrb.setPosition( basePos + Vector2f( 32, 32 ) - Vector2f( 0, 64 * activeOrb) );

	for( int i = 0; i < 6; ++i )
	{
		SetStarPositions( i, orbColors[i] );
	}

	UpdateStarVA();
	
	/*smallOrbVA[0].position = Vector2f( 0, 0 );
	smallOrbVA[1].position = Vector2f( 64, 0 );
	smallOrbVA[2].position = Vector2f( 64, 64 );
	smallOrbVA[3].position = Vector2f( 0, 64 );

	smallOrbVA[0].texCoords = Vector2f( 0, 0 );
	smallOrbVA[1].texCoords = Vector2f( 64, 0 );
	smallOrbVA[2].texCoords = Vector2f( 64, 64 );
	smallOrbVA[3].texCoords = Vector2f( 0, 64 );*/

}

typedef Vector2f V2f;
void PowerOrbs::SetStarPositions( int index, OrbColor oc )
{
	//int nStars = numStars[oc];
	cout<< "index: " << index << endl;
	Vector2f *s = starPositions[index];
	switch( oc )
	{
	case TEAL0:
	case TEAL1:
	case TEAL2:
	case TEAL3:
	case TEAL4:
	case TEAL5:
		s[0] = V2f( 48, 59 );
		s[1] = V2f( 79, 59 );
		s[2] = V2f( 112, 59 );
		s[3] = V2f( 143, 59 );
		s[4] = V2f( 48, 96 );
		s[5] = V2f( 79,  96 );
		s[6] = V2f( 112, 96 );
		s[7] = V2f( 143, 96 );
		s[8] = V2f( 48, 133 );
		s[9] = V2f( 79, 133 );
		s[10] = V2f( 112, 133 );
		s[11] = V2f( 143, 133 );
		break;
	case BLUE:
		s[0] = V2f( 95, 35 );
		s[1] = V2f( 112, 61 );
		s[2] = V2f( 127, 84 );
		s[3] = V2f( 141, 106 );
		s[4] = V2f( 154, 127 );
		s[5] = V2f( 126,  127 );
		s[6] = V2f( 95, 127 );
		s[7] = V2f( 65, 127 );
		s[8] = V2f( 37, 127 );
		s[9] = V2f( 50, 106 );
		s[10] = V2f( 64, 84 );
		s[11] = V2f( 79, 61 );
		break;
	case GREEN:
		s[0] =  V2f( 95 , 32 );
		s[1] =  V2f( 118 , 53 );
		s[2] =  V2f( 139 , 75 );
		s[3] =  V2f( 159 , 96 );
		s[4] =  V2f( 139 , 116 );
		s[5] =  V2f( 118, 138 );
		s[6] =  V2f( 95, 159 );
		s[7] =  V2f( 74, 138 );
		s[8] =  V2f( 52, 116 );
		s[9] =  V2f( 32, 96 );
		s[10] = V2f( 52 , 75 );
		s[11] = V2f( 74 , 53 );
		break;
	case YELLOW:
		s[0] =  V2f( 95 , 34 );
		s[1] =  V2f( 125 , 54 );
		s[2] =  V2f( 146 , 68 );
		s[3] =  V2f( 146 , 95 );
		s[4] =  V2f( 146 , 128 );
		s[5] =  V2f( 125 , 142 );
		s[6] =  V2f( 95 , 162 );
		s[7] =  V2f( 66 , 142 );
		s[8] =  V2f( 45 , 128 );
		s[9] =  V2f( 45 , 95 );
		s[10] = V2f( 45 , 68 );
		s[11] = V2f( 66 , 54 );
		break;
	case ORANGE:
		s[0] =  V2f( 95 , 34 );
		s[1] =  V2f( 146 , 68 );
		s[2] =  V2f( 146 , 128 );
		s[3] =  V2f( 95 , 162 );
		s[4] =  V2f( 45 , 128 );
		s[5] =  V2f( 45 , 68 );
		s[6] =  V2f( 95 , 67 );
		s[7] =  V2f( 120 , 83 );
		s[8] =  V2f( 120, 113 );
		s[9] =  V2f( 95 , 129 );
		s[10] = V2f( 71 , 113 );
		s[11] = V2f( 71 , 83 );
		break;
	case RED:
		s[0] =  V2f( 95 , 34 );
		s[1] =  V2f( 125 , 58 );
		s[2] =  V2f( 153 , 81 );
		s[3] =  V2f( 141 , 115 );
		s[4] =  V2f( 128 , 150 );
		s[5] =  V2f( 95 , 150 );
		s[6] =  V2f( 63 , 150 );
		s[7] =  V2f( 50 , 115 );
		s[8] =  V2f( 38, 81 );
		s[9] =  V2f( 66 , 58 );
		s[10] = V2f( 95 , 74 );
		s[11] = V2f( 95 , 126 );
		break;
	case MAGENTA:
		s[0] =  V2f( 95  , 34 );
		s[1] =  V2f( 155, 60 );
		s[2] =  V2f( 96 , 86 );
		s[3] =  V2f( 36 , 60 );
		s[4] =  V2f( 155 , 78 );
		s[5] =  V2f( 155 , 141 );
		s[6] =  V2f( 106 , 164 );
		s[7] =  V2f( 106 , 102 );
		s[8] =  V2f( 36 , 78 );
		s[9] =  V2f( 85 , 102 );
		s[10] = V2f( 85 , 64 );
		s[11] = V2f( 36 , 141 );
		break;
	}

	Vector2f b = basePos - Vector2f( 96, 64 * index + 96 );
	Vertex * va = starVA[index];
	for( int i = 0; i < 12; ++i )
	{
		va[i*4+0].position = b + s[i] + Vector2f( 0, 0 );
		va[i*4+1].position = b + s[i] + Vector2f( 64, 0 );
		va[i*4+2].position = b + s[i] + Vector2f( 64, 64 );
		va[i*4+3].position = b + s[i] + Vector2f( 0, 64 );
	}
}

void PowerOrbs::Reset()
{
}

void PowerOrbs::Draw( sf::RenderTarget *target )
{
	target->draw( smallOrbVA, ts_smallOrbs->texture );
	target->draw( largeOrb );
	sf::RenderStates rs;
	rs.texture = ts_charge->texture;
	target->draw( starVA[activeOrb], activeStars *4, sf::Quads, rs );
}

void PowerOrbs::UpdateStarVA()
{
	Vertex *va = starVA[activeOrb];

	int animFactor = 3;
	int colorType;
	if( orbColors[activeOrb] <= TEAL5 )
	{
		colorType = 0;
	}
	else
	{
		colorType = (int)orbColors[activeOrb] - 5;
	}
	IntRect ir = ts_charge->GetSubRect( colorType * 9 + starFrame / animFactor );
	for( int i = 0; i < activeStars; ++i )
	{
		va[i*4+0].texCoords = Vector2f( ir.left, ir.top );
		va[i*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
		va[i*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
		va[i*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );
		/*va[i*4+0].color = Color::Red;
		va[i*4+1].color = Color::Red;
		va[i*4+2].color = Color::Red;
		va[i*4+3].color = Color::Red;*/
		/*va[i*4+1].position = b + s[i] + Vector2f( 64, 0 );
		va[i*4+2].position = b + s[i] + Vector2f( 64, 64 );
		va[i*4+3].position = b + s[i] + Vector2f( 0, 64 );*/
	}

	++starFrame;
	if( starFrame == animFactor * 8 )
	{
		starFrame = 0;
	}

	++testBlah;
	if( testBlah == 180 )
	{
		testBlah = 0;
		--activeOrb;
		if( activeOrb < 0 )
			activeOrb = 5;

		OrbColor c = orbColors[activeOrb];
		int cc = (int)c;
		largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( cc ) );
	
		largeOrb.setPosition( basePos + Vector2f( 32, 32 ) - Vector2f( 0, 64 * activeOrb) );
	}
	/*for( int i = activeStars; i < 12; ++i )
	{
		va[i*4+3].position = Vector2f( 0, 0 );
	}*/

	//instead of 12, use numstars for the color
	
}