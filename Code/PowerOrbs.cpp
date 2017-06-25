#include "PowerOrbs.h"
#include "GameSession.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace sf;

PowerWheel::PowerWheel( GameSession *owner, bool hasAirDash,
		bool hasGravReverse,
		bool hasBounce,
		bool hasGrind,
		bool hasTimeSlow,
		bool hasWires ): smallOrbVA( sf::Quads, 6 * 2 * 4 ), //basePos( 96, 64 * 6 + 270 )
		basePos( 150, 450 ), partialSectionVA( sf::Triangles, 3 )
{
	origBasePos = basePos;
	mode = FILL;
	testBlah = 0;
	/*for( int i = 0; i < 7; ++i )
	{
		starVA[i] = new Vertex[12 * 4];
	}*/
	//activeStars = 12;
	//starState = 0;
	//starFrame = 0;
	

	swivelFrame = 0;
	swivelLength = 20;
	swivelLengthFill = 10;

	swivelingUp = false;
	swivelingDown = false;
	swivelStartAngle = 360.0 / 6.0;//0;
	
	orbHues[TEAL0] = Color( 0x00, 0xff, 0xff );
	orbHues[TEAL1] = Color( 0x00, 0xea, 0xff );
	orbHues[TEAL2] = Color( 0x0c, 0xcc, 0xff );
	orbHues[TEAL3] = Color( 0x00, 0xaa, 0xee );
	orbHues[TEAL4] = Color( 0x00, 0x99, 0xdd );
	orbHues[TEAL5] = Color( 0x00, 0x77, 0xcc );
	orbHues[BLUE] = Color( 0x33, 0x55, 0xff );
	orbHues[GREEN] = Color( 0x00, 0xcc, 0x44 );
	orbHues[YELLOW] = Color( 0xff, 0xee, 0x00 );
	orbHues[ORANGE] = Color( 0xff, 0xaa, 0x00 );
	orbHues[RED] = Color( 0xff, 0x22, 0x00 );
	orbHues[MAGENTA] = Color( 0xff, 0x00, 0xff );

	activeOrb = 0;

	numSections[TEAL0] =   15;
	numSections[TEAL1] =   15;
	numSections[TEAL2] =   15;
	numSections[TEAL3] =   15;
	numSections[TEAL4] =   15;
	numSections[TEAL5] =   15;
	numSections[BLUE] =    16;
	numSections[GREEN] =   18;
	numSections[YELLOW] =  20;
	numSections[ORANGE] =  21;
	numSections[RED] =     24;
	numSections[MAGENTA] = 25;


	ts_orbPointer = owner->GetTileset( "life_ring_192x128.png", 192, 128 );
	orbPointer.setTexture( *ts_orbPointer->texture );
	orbPointer.setTextureRect( ts_orbPointer->GetSubRect( 1 ) );
	orbPointer.setOrigin( orbPointer.getLocalBounds().width / 2,
		orbPointer.getLocalBounds().height + 48 );
	
	//ts_lifeStop = owner->GetTileset( "life_stop_64x64.png", 64, 64 );
	//lifeStop.setTexture( *ts_lifeStop->texture );
	
	//if (!lifeTextureShader.loadFromFile("lifetexture_shader.frag", sf::Shader::Fragment))
	////if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	//{
	//	cout << "life texture SHADER NOT LOADING CORRECTLY" << endl;
	//	assert( 0 && "life texture shader not loaded" );
	//}
	
	//ts_largeOrbs = owner->GetTileset( "lifeL_192x192.png", 192, 192 );
	//ts_smallOrbs = owner->GetTileset( "lifeL_192x192.png", 192, 192 );//owner->GetTileset( "lifeS_64x64.png", 64, 64 );

	ts_largeOrbs = owner->GetTileset( "lifeL_192x192.png", 192, 192 );
	ts_smallOrbs = owner->GetTileset( "lifeS_64x64.png", 64, 64 );
	ts_lifeTexture = owner->GetTileset( "life_texture_02_40x96.png", 40, 96 );

	/*ts_largeOrbs = owner->GetTileset( "bluekey.png", 50, 50 );
	ts_smallOrbs = owner->GetTileset( "bluekey.png", 50, 50 );*/

	largeOrb.setTexture( *ts_largeOrbs->texture );
	largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( activeOrb ) );
	largeOrb.setOrigin( largeOrb.getLocalBounds().width / 2, largeOrb.getLocalBounds().height / 2 );

	/*for( int i = 5; i < 11; ++i )
	{
		starPositions[i - 5] = new Vector2f[numStars[(OrbColor)i]];
	}*/

	//ts_charge = owner->GetTileset( "charge_64x64.png", 64, 64 );
	
	bool hasPower[6] = { hasAirDash, hasGravReverse, hasBounce, hasGrind, hasTimeSlow,
		hasWires };
	int powerIndex = 5;
	int tealCounter = 0;

	//Vector2f bPos( basePos.x, basePos.y );
	//cout << "setting position small orb" << endl;

	lifeTextureMultiple = 7;
	lifeTextureFrame = 0;
	

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
			//cout << "teal coloring!!" << tealCounter << endl;
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

		orbSectionVA[i] = CreateSectionVA( col, 96 );
		//cout << "i: " << i << ", col: " << (int)col << endl;
	}

	OrbColor c = orbColors[activeOrb];
	int cc = (int)c;
	largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( cc ) );
	
	//largeOrb.setPosition( basePos + Vector2f( 32, 32 ) ); //- Vector2f( 0, 64 * activeOrb) );

	
	UpdateSpritePositions();


	activeSection = 0;//numSections[orbColors[activeOrb]];
	activeLevel = 6;

	orbPointer.setRotation( swivelStartAngle );
	shifting = false;
	//SetVisibleSections( activeOrb, numSections[orbColors[activeOrb]], 2 );
	/*for( int i = 0; i < 6; ++i )
	{
		SetStarPositions( i, orbColors[i] );
	}*/

	//UpdateStarVA();
	
	/*smallOrbVA[0].position = Vector2f( 0, 0 );
	smallOrbVA[1].position = Vector2f( 64, 0 );
	smallOrbVA[2].position = Vector2f( 64, 64 );
	smallOrbVA[3].position = Vector2f( 0, 64 );

	smallOrbVA[0].texCoords = Vector2f( 0, 0 );
	smallOrbVA[1].texCoords = Vector2f( 64, 0 );
	smallOrbVA[2].texCoords = Vector2f( 64, 64 );
	smallOrbVA[3].texCoords = Vector2f( 0, 64 );*/
	//cout << "end of constructor" << endl;
}

void PowerWheel::UpdateSpritePositions()
{
	largeOrb.setPosition( basePos + Vector2f( 32, 32 ) );

	float radiusOffset = 128 + 10;
	Vector2f offset( 0, -radiusOffset );
	sf::Transform tr;

	Vector2f bPos = basePos + Vector2f( 32, 32 );
	Vector2f offsetArrows( 0, -radiusOffset + 10 );
	for( int i = 5; i >= 0; --i )
	{
		smallOrbVA[(i+6) * 4+0].position = bPos + Vector2f(-32,-32  ) + tr.transformPoint( offset );  
		smallOrbVA[(i+6) * 4+1].position = bPos + Vector2f( 32, -32 ) + tr.transformPoint( offset ); 
		smallOrbVA[(i+6) * 4+2].position = bPos + Vector2f( 32, 32  ) + tr.transformPoint( offset ); 
		smallOrbVA[(i+6) * 4+3].position = bPos + Vector2f( -32, 32 ) + tr.transformPoint( offset );
		tr.rotate( -360 / 6.0 );
	}

	sf::Transform tr1;
	tr1.rotate( 360 / 12.0 );
	for( int i = 0; i < 6; ++i )
	{
		smallOrbVA[i * 4+0].position = bPos + Vector2f(-32,-32  ) + tr1.transformPoint( offsetArrows );  
		smallOrbVA[i * 4+1].position = bPos + Vector2f( 32, -32 ) + tr1.transformPoint( offsetArrows ); 
		smallOrbVA[i * 4+2].position = bPos + Vector2f( 32, 32  ) + tr1.transformPoint( offsetArrows ); 
		smallOrbVA[i * 4+3].position = bPos + Vector2f( -32, 32 ) + tr1.transformPoint( offsetArrows );
		tr1.rotate( 360 / 6.0 );
	}

	orbPointer.setPosition( largeOrb.getPosition() );

	Vector2f radOffset( 0, -radius );
	sf::Transform trx;

	int sectionCount = numSections[orbColors[activeOrb]];
	//visSections - 1;
	trx.rotate( 360.0 / sectionCount * activeSection );

	VertexArray &va = partialSectionVA;

	va[0].position = bPos;
	va[1].position = bPos + trx.transformPoint( radOffset );

	trx.rotate( 360.0 / sectionCount );

	va[2].position = bPos + trx.transformPoint( radOffset );

	for( int i = 0; i < 6; ++i )
	{
	//	OrbColor col = orbColors[i];

	
		VertexArray &sva = *orbSectionVA[i];

		Vector2f offset( 0, -radius );
		sf::Transform tr;
		int sectionCount = numSections[orbColors[i]];
		//sva[0].position = trueBase;
		for( int i = 0; i < sectionCount; ++i )
		{
			sva[i*3+0].position = bPos;
			sva[i*3+1].position = bPos + tr.transformPoint( offset );
			tr.rotate( 360.0 / sectionCount );
			sva[i*3+2].position = bPos + tr.transformPoint( offset );
			//sva[i].position = trueBase + 
		}

	}
}

void PowerWheel::UpdateSmallOrbs()
{
	for( int i = 5; i >= 0; --i )
	{
		OrbColor oc = orbColors[i];


		int irIndex = oc;

		if( oc > TEAL5 )
		{
			irIndex += 6;
		}

		if( activeOrb < i )
		{
			irIndex += 6;
			//cout << "changed" << endl;
		}
		//cout << "activeOrb: " << activeOrb << ", i: " << i << ", irIndex: " << irIndex << endl;
		IntRect ir = ts_smallOrbs->GetSubRect( irIndex );
		
		
		smallOrbVA[(i+6)*4+0].texCoords = Vector2f( ir.left, ir.top );
		smallOrbVA[(i+6)*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
		smallOrbVA[(i+6)*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
		smallOrbVA[(i+6)*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );

		/*smallOrbVA[i*4+0].texCoords = Vector2f( ir.left, ir.top );
		smallOrbVA[i*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
		smallOrbVA[i*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
		smallOrbVA[i*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );*/
	}

	for( int i = 0; i < 6; ++i )
	{
		IntRect ir = ts_smallOrbs->GetSubRect( 24 + i );
		smallOrbVA[i*4+0].texCoords = Vector2f( ir.left, ir.top );
		smallOrbVA[i*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
		smallOrbVA[i*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
		smallOrbVA[i*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );
	}
}

typedef Vector2f V2f;

void PowerWheel::UpdateSections()
{
	if( mode == FILL )
	{
		Charge( 6 );
	}

	//cout << "orb: " << activeOrb << ", section: " << activeSection << ", level: " << activeLevel << endl;
	if( lifeTextureFrame == 4 * lifeTextureMultiple )
	{
		lifeTextureFrame = 0;
	}
	//cout << "update start: " << activeOrb << ", " << activeSection << ", " << activeLevel << endl;
	SetVisibleSections( activeOrb, activeSection, activeLevel );

	UpdateSmallOrbs();

	UpdateSwivel();

	++lifeTextureFrame;

	
}

void PowerWheel::UpdateSwivel()
{
	int sLength = swivelLength;
	if( mode == FILL )
	{
		sLength = swivelLengthFill;
	}

	if( swivelingUp )
	{
		float goalAngle = swivelStartAngle + 360.f / 6.f;

		if( swivelFrame == sLength + 1 )
		{
			swivelingUp = false;
			swivelStartAngle = goalAngle;
			orbPointer.setTextureRect( ts_orbPointer->GetSubRect( 1 ) );
			return;
		}
		
		CubicBezier bez(.45,.03,.36,.86 );

		float r = bez.GetValue( swivelFrame / (double)sLength );
		orbPointer.setRotation( swivelStartAngle * ( 1.f - r ) + goalAngle * r );
		orbPointer.setTextureRect( ts_orbPointer->GetSubRect( 0 ) );
		++swivelFrame;
	}
	else if( swivelingDown )
	{
		float goalAngle = swivelStartAngle - 360.f / 6.f;

		if( swivelFrame == sLength + 1 )
		{
			swivelingDown = false;
			swivelStartAngle = goalAngle;
			orbPointer.setTextureRect( ts_orbPointer->GetSubRect( 1 ) );
			return;
		}
		
		CubicBezier bez( .45,.03,.36,.86 );

		float r = bez.GetValue( swivelFrame / (double)sLength );
		orbPointer.setRotation( swivelStartAngle * ( 1.f - r ) + goalAngle * r );
		orbPointer.setTextureRect( ts_orbPointer->GetSubRect( 0 ) );
		++swivelFrame;
	}
}

void PowerWheel::Reset()
{
	shifting = false;
	basePos = origBasePos;
	mode = NORMAL;
	swivelingUp = false;
	swivelingDown = false;
	swivelStartAngle = 0;
	swivelFrame = 0;
	lifeTextureFrame = 0;
	activeOrb = 5;
	activeSection = numSections[orbColors[activeOrb]];
	activeLevel = 6;
	largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( orbColors[activeOrb] ) );
	orbPointer.setTextureRect( ts_orbPointer->GetSubRect( 1 ) );
	orbPointer.setRotation( 0 );

	UpdateSpritePositions();
}

void PowerWheel::Draw( sf::RenderTarget *target )
{
	sf::CircleShape test;
	test.setRadius( 16 );
	test.setOrigin( test.getLocalBounds().width / 2, test.getLocalBounds().height / 2 );
	test.setPosition( largeOrb.getPosition() );
	test.setFillColor( Color::Black );
	
	target->draw( smallOrbVA, ts_smallOrbs->texture );
	target->draw( largeOrb );
	target->draw( *orbSectionVA[activeOrb], ts_lifeTexture->texture );
	target->draw( partialSectionVA, ts_lifeTexture->texture );
	target->draw( test );
	target->draw( orbPointer );
//	target->draw( *orbMidSectionVA[activeOrb] );
//	target->draw( *orbSmallSectionVA[activeOrb] );
	//sf::RenderStates rs;
	//rs.texture = ts_charge->texture;
	//target->draw( starVA[activeOrb], activeStars *4, sf::Quads, rs );
}

void PowerWheel::Hide( bool hide, int frames )
{
	shifting = true;
	hiding = hide;
	hideFrame = 0;
	hideLength = frames;
}

void PowerWheel::UpdateHide()
{
	if( !shifting )
		return;

	++hideFrame;

	CubicBezier bez( 0, 0, 1, 1 );
	Vector2f hidePos = origBasePos + Vector2f( -500, 0 );
	float f = bez.GetValue( (double)hideFrame / hideLength );
	if( hiding )
	{
		basePos = origBasePos * (1.f - f ) + hidePos * f;
	}
	else
	{
		basePos = hidePos * (1.f - f ) + origBasePos * f;
	}
	
	UpdateSpritePositions();
	
	if( hideFrame > hideLength )
	{
		shifting = false;
	}
}

//level can be 1 2 3
void PowerWheel::SetVisibleSections( int orbIndex, int visSections,
	int currentLevel )
{
	//cout << "orbindex: " << orbIndex << ", vsSections: " << visSections << ", currlevel: " << currentLevel << endl;
	//cout << "set visible: " << orbIndex << ", " 
	//	<< visSections << ", " << currentLevel << endl;
	//assert( currentLevel > 0 );
	VertexArray &va = *orbSectionVA[orbIndex];

	//--visSections;
	int vSections = visSections;

	//if( currentLevel != 6 )
	//vSections--;

	//vSections--;

	
	Color bc = orbHues[orbColors[activeOrb]];
	//bc = Color::Red;
	IntRect ir = ts_lifeTexture->GetSubRect( 1 + lifeTextureFrame / lifeTextureMultiple );
	for( int i = 0; i < vSections - 1; ++i )
	{
		va[i*3+0].color = bc;//Color::Red;
		va[i*3+1].color = bc;//Color::Red;
		va[i*3+2].color = bc;//Color::Red;
		va[i*3+0].texCoords = Vector2f( ir.left + ir.width / 2, ir.top + ir.height ); //Vector2f( 0, 32 );
		va[i*3+1].texCoords = Vector2f( ir.left + ir.width, ir.top );//Vector2f( 16, 0 );
		va[i*3+2].texCoords = Vector2f( ir.left, ir.top );
	}
	
	int vertexCount = va.getVertexCount();
	int totalSections = vertexCount / 3;
	
	Color backCol = Color::White;

	//if( currentLevel == 6 )
	//	vSections++;
	//make a frozen texture for this and itll work!
	//Color bc = orbHues[orbColors[activeOrb]];//Color::Transparent;
	IntRect irBlank = ts_lifeTexture->GetSubRect( 0 );
	int capStart = vSections - 1;
	if( capStart < 0 )
		capStart = 0;
	for( int i = capStart; i < totalSections; ++i )
	{
		va[i*3+0].color = backCol;
		va[i*3+1].color = backCol;
		va[i*3+2].color = backCol;

		va[i*3+0].texCoords = Vector2f( irBlank.left + irBlank.width / 2, irBlank.top + irBlank.height );
		va[i*3+1].texCoords = Vector2f( irBlank.left + irBlank.width, irBlank.top );
		va[i*3+2].texCoords = Vector2f( irBlank.left, irBlank.top );

		//va[i*3+0].texCoords = Vector2f( ir.left + ir.width / 2, ir.top + ir.height ); //Vector2f( 0, 32 );
		//va[i*3+1].texCoords = Vector2f( ir.left + ir.width, ir.top );//Vector2f( 16, 0 );
		//va[i*3+2].texCoords = Vector2f( ir.left, ir.top );//Vector2f( 32, 32 );

		//va[i].color = Color::Blue;
	}

	if( currentLevel > 0 )
	{
		SetVisibleCurrentSection( orbIndex, visSections - 1, 32 + (currentLevel - 1) * (96-32) / 5 );
	}
	/*if( currentLevel == 1 )
	{
		SetVisibleCurrentSection( orbIndex, visSections - 1, 48 );
	}
	else if( currentLevel == 2 )
	{
		SetVisibleCurrentSection( orbIndex, visSections - 1, 72 );
	}*/
	else
	{
		for( int i = 0; i < 3; ++i )
		{
			partialSectionVA[i].position = Vector2f( 0, 0 );
		}
	}
}

void PowerWheel::SetVisibleCurrentSection( int orbIndex, int currentSection, float radius )
{
	//cout << "orbindex: " << orbIndex << ", " << currentSection << ", " << radius << endl;
	Vector2f trueBase = basePos + Vector2f( 32, 32 );
	int sectionCount = numSections[orbColors[orbIndex]];

	Vector2f offset( 0, -radius );
	sf::Transform tr;

	tr.rotate( 360.0 / sectionCount * currentSection );

	VertexArray &va = partialSectionVA;

	va[0].position = trueBase;
	va[1].position = trueBase + tr.transformPoint( offset );
	tr.rotate( 360.0 / sectionCount );
	va[2].position = trueBase + tr.transformPoint( offset );

	Color c = orbHues[orbColors[orbIndex]];

	va[0].color = c;
	va[1].color = c;
	va[2].color = c;

	IntRect ir = ts_lifeTexture->GetSubRect( 1 + lifeTextureFrame / lifeTextureMultiple );
	va[0].texCoords = Vector2f( ir.left + ir.width / 2, ir.top + ir.height ); //Vector2f( 0, 32 );
	va[1].texCoords = Vector2f( ir.left + ir.width, ir.top );//Vector2f( 16, 0 );
	va[2].texCoords = Vector2f( ir.left, ir.top );
}

VertexArray * PowerWheel::CreateSectionVA( OrbColor col, float p_radius )
{
	radius = p_radius;
	Vector2f trueBase = basePos + Vector2f( 32, 32 );
	int sectionCount = numSections[col];
	int numVertices = sectionCount * 6;//+ 1 + 1;
	VertexArray *sVA = new VertexArray( sf::PrimitiveType::Triangles, numVertices );
	VertexArray &sva = *sVA;

	for( int i = 0; i < numVertices; ++i )
	{
		sva[i].color = Color::Blue;
	}

	Vector2f offset( 0, -radius );
	sf::Transform tr;
	//sva[0].position = trueBase;
	for( int i = 0; i < sectionCount; ++i )
	{
		sva[i*3+0].position = trueBase;
		sva[i*3+1].position = trueBase + tr.transformPoint( offset );
		tr.rotate( 360.0 / sectionCount );
		sva[i*3+2].position = trueBase + tr.transformPoint( offset );
		//sva[i].position = trueBase + 
	}

	//Vector2f diff0 = sva[2].position - sva[1].position;
	//cout << "diff0: " << sqrt(( diff0.x * diff0.x ) + (diff0.y * diff0.y )) << endl;//diff0.x << ", " << diff0.y << endl;

	return sVA;
}

bool PowerWheel::Damage( int power )
{
	return true;
	/*if( mode == DESPERATION )
	{
		return true;
	}*/
	//3 is one section
	int remainder = power % 6;
	int mult = power / 6;

	//while( mult >= activeLevel )
	//{
	//	mult -= activeLvel;
	//	
	//	//survival mode
	//	if( activeOrb == 0 )
	//	{
	//		activeSection = 0;
	//		activeLevel = 0;
	//		Reset(); //temporary
	//		return false;
	//	}

	//	--activeOrb;
	//	activeSection = numSections[orbColors[activeOrb]];
	//	
	//	
	//}
	while( true )
	{
		int num;
		if( remainder > 0 )
		{
			num = remainder;
			remainder = 0;
		}
		else
		{
			if( mult == 0 )
			{
				return true;
			}
			else
			{
				mult--;
				num = 6;
			}
		}
		activeLevel -= num;
		if( activeLevel < 1 )
		{
			activeSection--;
			if( activeSection < 1 )
			{
				activeOrb--;
				if( activeOrb < 0 )
				{
					activeSection = 0;
					activeLevel = 0;
					activeOrb = 0;
					mode = DESPERATION;
					//Reset(); //tempoary. goes to survival mode
					return false;
					//return true;
				}
				OrbColor oc = orbColors[activeOrb];
				activeSection = numSections[oc];
				largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( oc ) );
				swivelingDown = true;
				swivelFrame = 0;
						//swivelStartAngle = -360.f / 6.f * ( 5 - ( activeOrb + 1 ) ); 
			//swiveling = true;
			//swivelFrame = 0;
			}
			activeLevel = 6 + activeLevel;
		}

	}

	
	return true;
}

bool PowerWheel::Use( int power )
{
	return true;
	return Damage( power );
}

void PowerWheel::Recover( int power )
{
}

void PowerWheel::OrbUp()
{
	activeOrb++;
	if( activeOrb == 6 )
	{
		activeOrb = 5;
		activeSection = numSections[orbColors[activeOrb]];
		activeLevel = 6;
		mode = NORMAL;
		return;
	}
	OrbColor oc = orbColors[activeOrb];
	activeSection = 1;
	largeOrb.setTextureRect( ts_largeOrbs->GetSubRect( oc ) );
	swivelingUp = true;
	swivelFrame = 0;
}

void PowerWheel::Charge( int power )
{
	//3 is one section
	int remainder = power % 6;
	int mult = power / 6;

	while( mult > 0 )
	{
		mult--;
		++activeSection;

		if( activeSection > numSections[orbColors[activeOrb]] )
		{
			OrbUp();
		}
	}

	activeLevel += remainder;
	if( activeLevel > 6 )
	{
		activeSection++;
		if( activeSection > numSections[orbColors[activeOrb]] )
		{
			OrbUp();
		}
		activeLevel = -6 + activeLevel;
	}

	
//	return true;
}

//
PowerRingSection::PowerRingSection( TilesetManager &tm, const sf::Color &active, const sf::Color &remove,
	const sf::Color &empty, int p_rType, int p_maxPower, float startAngle )
	:activeColor(active), removeColor(remove), emptyColor(empty),
	maxPower(p_maxPower)
{
	//ringType = (RingType)p_rType;
	if (!ringShader.loadFromFile("ring_shader.frag", sf::Shader::Fragment))
	{
		cout << "ring shader not loading correctly!" << endl;
		assert(false);
	}
	ringShader.setUniform("u_startAngle", startAngle);
	ringShader.setUniform("u_filledRange", float( 1.6 * PI) ); //just for testing. should be //1.f);
	ringShader.setUniform("u_activeColor", ColorGL(active));
	ringShader.setUniform("u_removeColor", ColorGL(remove));
	ringShader.setUniform("u_emptyColor", ColorGL(empty));
	
	stringstream ss;
	//ss << "powerring" << (int)rType << "_200x200.png";
	ss << "powerring" << p_rType << "_200x200.png";
	//ss << "powerring_200x200.png";
	ts_ring = tm.GetTileset(ss.str(), 200, 200);
	/*switch( rType )
	{
	case NORMAL:
		
		break;
	}*/
	ringShader.setUniform("u_ringTex", sf::Shader::CurrentTexture);
	//ringShader.setUniform("u_ringTex", ts_ring->texture);
	
	ringSprite.setTexture(*ts_ring->texture);
	//ringSprite.setTextureRect(ts_ring->GetSubRect(3));
	ringSprite.setOrigin(ringSprite.getLocalBounds().width / 2, ringSprite.getLocalBounds().height / 2);

	Reset();
	
}

void PowerRingSection::Reset()
{
	currPower = maxPower;
	Update();
}

int PowerRingSection::GetDivsActive()
{
	return 0;
	//double f = (currPower/(double)maxPower) * numDivisions;
}

int PowerRingSection::GetDivsRemoved()
{
	return 0;
	//double f = (removedPower / (double)maxPower) * numDivisions;
}

int PowerRingSection::GetDivsEmpty()
{
	return 0;
	//double f = (currPower / (double)maxPower) * numDivisions;
}

void PowerRingSection::Update()
{
	ringShader.setUniform("u_filledRange", float(2 * PI) * (float)currPower / maxPower);
}

void PowerRingSection::UpdateSprite()
{
	
}

int PowerRingSection::Drain(int dmg)
{
	prevPower = currPower;
	currPower -= dmg;
	if (currPower < 0)
	{
		int ret = -currPower;
		currPower = 0;
		return ret;
	}
	else
	{
		return 0;
	}
}

int PowerRingSection::Fill(int power)
{
	currPower += power;
	if (currPower > maxPower)
	{
		int ret = -currPower;
		currPower = maxPower;
		return ret;
	}
	else
		return 0;
}

void PowerRingSection::Draw(sf::RenderTarget *target)
{
	target->draw(ringSprite, &ringShader);
}

//you are trying to figure out a simple way to represent this so that you don't have to touch up all the 
//vertices each frame. you should only change the color if the color needs to be changed


void PowerRingSection::SetupSection( sf::Vector2f &centerPos)
{

	ringSprite.setPosition(centerPos);
	//int innerRadius = startRadius;
	//int outerRadius = startRadius + width;

	//double div = 360.0 / maxPower;

	//Vector2f innerOffset(0, -innerRadius);
	//Vector2f outerOffset(0, -outerRadius);

	////Vertex *circleVA = va;
	////Vertex *ringVA = (va + maxPower * 4);
	//sf::Transform t;

	//Vector2f innerPointCurr;
	//Vector2f outerPointCurr;
	//Vector2f innerPointPrev;
	//Vector2f outerPointPrev;
	//for (int i = 0; i < maxPower; ++i)
	//{
	//	innerPointPrev = t.transformPoint(innerOffset);
	//	outerPointPrev = t.transformPoint(outerOffset);
	//	
	//	t.rotate(div);

	//	innerPointCurr = t.transformPoint(innerOffset);
	//	outerPointCurr = t.transformPoint(outerOffset);

	//	va[i * 4 + 0].position = innerPointPrev + centerPos;
	//	va[i * 4 + 1].position = outerPointPrev + centerPos;
	//	va[i * 4 + 2].position = outerPointCurr + centerPos;
	//	va[i * 4 + 3].position = innerPointCurr + centerPos;

	//	t.rotate(div);
	//}
}

PowerRing::PowerRing( Vector2f &pos, int p_numRings, PowerRingSection **p_rings)
	:numRings( p_numRings ), rings( p_rings ), centerPos( pos )
{

	//centerPos = Vector2f( 90, 230 );

	numTotalVertices = 0;
	int radiusSum = 50; //internal start radius

	rings = new PowerRingSection*[numRings];

	for (int i = 0; i < numRings; ++i)
	{
		rings[i] = p_rings[i];
		rings[i]->SetupSection(centerPos);
	}
	Reset();
	/*for (int i = 0; i < numRings; ++i)
	{
		rings[i]->SetupSection(centerPos, radiusSum);
		rings[i]->Reset();
		radiusSum += rings[i]->width;
		numTotalVertices += rings[i]->maxPower * 4;
	}*/
	//CreateRing();
}

void PowerRing::Reset()
{
	currRing = numRings - 1;
	for (int i = 0; i < numRings; ++i)
	{
		rings[i]->Reset();
	}
}

PowerRing::~PowerRing()
{
	for (int i = 0; i < numRings; ++i)
	{
		delete rings[i];
	}
	delete[] rings;
}

void PowerRing::Update()
{
	rings[currRing]->Update();
}

void PowerRing::CreateRing()
{
	scorpTest.setFillColor( Color::Yellow );
	scorpTest.setRadius( 25 );
	scorpTest.setOrigin( scorpTest.getLocalBounds().width / 2, 
		scorpTest.getLocalBounds().height / 2 );
	scorpTest.setPosition( centerPos + Vector2f( 155, 0 ));

	keyTest.setFillColor( Color::Blue );
	keyTest.setRadius( 25 );
	keyTest.setOrigin( keyTest.getLocalBounds().width / 2, 
		keyTest.getLocalBounds().height / 2 );
	keyTest.setPosition( scorpTest.getPosition() + Vector2f( 40, -40 ) );
	float innerRadius = 40;
	float outerRadius = 64;
	int reps = 4;
	int numCirclePoints = 6 * reps;
	int numPoints = numCirclePoints * 2;
}

void PowerRing::Draw( sf::RenderTarget *target )
{
	for (int i = 0; i < numRings; ++i)
	{
		rings[i]->Draw(target);
	}
	//target->draw( *ringVA );
	//target->draw( *middleVA );
	//target->draw( scorpTest );
	//target->draw( keyTest );
}

int PowerRing::Fill( int fill )
{
	int res = rings[currRing]->Fill(fill);
	if (res > 0 && currRing < numRings - 1)
	{
		++currRing;
		return Fill(res);	
	}

	return res;
}

int PowerRing::Drain(int drain)
{
	int res = rings[currRing]->Drain(drain);
	if (res > 0 && currRing > 0)
	{
		--currRing;
		return Drain(res);
	}

	return res;
}