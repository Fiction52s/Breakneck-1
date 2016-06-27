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
		bool hasWires ): smallOrbVA( sf::Quads, 6 * 4 ), basePos( 128 - 32, 64 * 6 + 270 )
{
	
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
		starPositions[i - 5] = new Vector2i[numStars[(OrbColor)i]];
	}

	for( int i = 0; i < OrbColor::Count; ++i )
	{
		stringstream ss;
		ss << "charge" << i << "a_64x64.png";
		ts_charges[i] = owner->GetTileset( ss.str(), 64, 64 );
	}
	
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
				break;

			}
			--powerIndex;
		}

		if( powerIndex < 0 )
		{
				int f = tealCounter;//5 - tealCounter;
				//12 - 17
				sf::IntRect ir = ts_smallOrbs->GetSubRect( f );

				orbColors[i] = (OrbColor)tealCounter;
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
	}

	OrbColor c = orbColors[activeOrb];
	int cc = (int)c;
	largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( cc ) );
	
	largeOrb.setPosition( basePos + Vector2f( 32, 32 ) - Vector2f( 0, 64 * activeOrb) );
	/*smallOrbVA[0].position = Vector2f( 0, 0 );
	smallOrbVA[1].position = Vector2f( 64, 0 );
	smallOrbVA[2].position = Vector2f( 64, 64 );
	smallOrbVA[3].position = Vector2f( 0, 64 );

	smallOrbVA[0].texCoords = Vector2f( 0, 0 );
	smallOrbVA[1].texCoords = Vector2f( 64, 0 );
	smallOrbVA[2].texCoords = Vector2f( 64, 64 );
	smallOrbVA[3].texCoords = Vector2f( 0, 64 );*/

}

typedef Vector2i V2i;
void PowerOrbs::SetStarPositions( OrbColor oc )
{
	int index = (int)oc - 5;
	int nStars = numStars[oc];
	Vector2i *s = starPositions[index];
	switch( index )
	{
	case 0:
		s[0] = V2i( 48, 59 );
		s[1] = V2i( 79, 59 );
		s[2] = V2i( 112, 59 );
		s[3] = V2i( 143, 59 );
		s[4] = V2i( 48, 96 );
		s[5] = V2i( 79,  96 );
		s[6] = V2i( 112, 96 );
		s[7] = V2i( 143, 96 );
		s[8] = V2i( 48, 133 );
		s[9] = V2i( 79, 133 );
		s[10] = V2i( 112, 133 );
		s[11] = V2i( 143, 133 );
		break;
	case 1:
		s[0] = V2i( 95, 35 );
		s[1] = V2i( 112, 61 );
		s[2] = V2i( 127, 84 );
		s[3] = V2i( 141, 106 );
		s[4] = V2i( 154, 127 );
		s[5] = V2i( 126,  127 );
		s[6] = V2i( 95, 127 );
		s[7] = V2i( 65, 127 );
		s[8] = V2i( 37, 127 );
		s[9] = V2i( 50, 106 );
		s[10] = V2i( 64, 84 );
		s[11] = V2i( 79, 61 );
		break;
	case 2:
		s[0] =  V2i( 95 , 32 );
		s[1] =  V2i( 118 , 53 );
		s[2] =  V2i( 139 , 75 );
		s[3] =  V2i( 159 , 96 );
		s[4] =  V2i( 139 , 116 );
		s[5] =  V2i( 118, 138 );
		s[6] =  V2i( 95, 159 );
		s[7] =  V2i( 74, 138 );
		s[8] =  V2i( 52, 116 );
		s[9] =  V2i( 32, 96 );
		s[10] = V2i( 52 , 75 );
		s[11] = V2i( 74 , 53 );
		break;
	case 3:
		s[0] =  V2i( 95 , 34 );
		s[1] =  V2i( 125 , 54 );
		s[2] =  V2i( 146 , 68 );
		s[3] =  V2i( 146 , 95 );
		s[4] =  V2i( 146 , 128 );
		s[5] =  V2i( 125 , 142 );
		s[6] =  V2i( 95 , 162 );
		s[7] =  V2i( 66 , 142 );
		s[8] =  V2i( 45 , 128 );
		s[9] =  V2i( 45 , 95 );
		s[10] = V2i( 45 , 68 );
		s[11] = V2i( 66 , 54 );
		break;
	case 4:
		s[0] =  V2i( 95 , 34 );
		s[1] =  V2i( 146 , 68 );
		s[2] =  V2i( 146 , 128 );
		s[3] =  V2i( 95 , 162 );
		s[4] =  V2i( 45 , 128 );
		s[5] =  V2i( 45 , 68 );
		s[6] =  V2i( 95 , 67 );
		s[7] =  V2i( 120 , 83 );
		s[8] =  V2i( 120, 113 );
		s[9] =  V2i( 95 , 129 );
		s[10] = V2i( 71 , 113 );
		s[11] = V2i( 71 , 83 );
		break;
	case 5:
		s[0] =  V2i( 95 , 34 );
		s[1] =  V2i( 125 , 58 );
		s[2] =  V2i( 153 , 81 );
		s[3] =  V2i( 141 , 15  );
		s[4] =  V2i( 128 , 150 );
		s[5] =  V2i( 95 , 150 );
		s[6] =  V2i( 63 , 150 );
		s[7] =  V2i( 50 , 115 );
		s[8] =  V2i( 38, 81 );
		s[9] =  V2i( 66 , 58 );
		s[10] = V2i( 95 , 74 );
		s[11] = V2i( 95 , 126 );
		break;
	case 6:
		s[0] =  V2i( 95  , 34 );
		s[1] =  V2i( 155, 60 );
		s[2] =  V2i( 96 , 86 );
		s[3] =  V2i( 36 , 60 );
		s[4] =  V2i( 155 , 78 );
		s[5] =  V2i( 155 , 141 );
		s[6] =  V2i( 106 , 164 );
		s[7] =  V2i( 106 , 102 );
		s[8] =  V2i( 36 , 78 );
		s[9] =  V2i( 85 , 102 );
		s[10] = V2i( 85 , 64 );
		s[11] = V2i( 36 , 141 );
		break;
	}
}

void PowerOrbs::Draw( sf::RenderTarget *target )
{
	target->draw( smallOrbVA, ts_smallOrbs->texture );
	target->draw( largeOrb );
}