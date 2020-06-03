#include "PowerOrbs.h"
#include "GameSession.h"
#include <sstream>
#include <iostream>
#include "Actor.h"
#include "MainMenu.h"
#include "MovingGeo.h"

using namespace std;
using namespace sf;

//
FillRingSection::FillRingSection(TilesetManager &tm,
	const sf::Color &active, const sf::Color &remove,
	const sf::Color &empty,
	int p_rType, int p_maxPower,
	float p_startAngle)
	:maxPower(p_maxPower)
{
	activeColor = ColorGL( active );
	removeColor = ColorGL(remove);
	emptyColor = ColorGL(empty);
	

	//ringType = (RingType)p_rType;
	if (!ringShader.loadFromFile("Resources/Shader/ring_shader.frag", sf::Shader::Fragment))
	{
		cout << "ring shader not loading correctly!" << endl;
		assert(false);
	}
	ringShader.setUniform("u_startAngle", p_startAngle);
	ringShader.setUniform("u_filledRange", float( 1.6 * PI) ); //just for testing. should be //1.f);
	ringShader.setUniform("u_activeColor", activeColor);
	ringShader.setUniform("u_removeColor", removeColor);
	ringShader.setUniform("u_emptyColor", emptyColor);
	
	stringstream ss;
	//ss << "powerring" << (int)rType << "_200x200.png";
	ss << "HUD/powerring" << p_rType << "_100x100.png";
	//ss << "powerring_200x200.png";
	ts_ring = tm.GetTileset(ss.str(), 100, 100);
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

	ResetFull();
	
}

void FillRingSection::ResetEmpty()
{
	currPower = 0;
	Update();
}

void FillRingSection::ResetFull()
{
	currPower = maxPower;
	Update();
}

int FillRingSection::GetDivsActive()
{
	return 0;
	//double f = (currPower/(double)maxPower) * numDivisions;
}

int FillRingSection::GetDivsRemoved()
{
	return 0;
	//double f = (removedPower / (double)maxPower) * numDivisions;
}

int FillRingSection::GetDivsEmpty()
{
	return 0;
	//double f = (currPower / (double)maxPower) * numDivisions;
}

void FillRingSection::Update()
{
	ringShader.setUniform("u_filledRange", float(2 * PI) * (float)currPower / maxPower);
}

void FillRingSection::UpdateSprite()
{
	
}

int FillRingSection::Drain(int dmg)
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

int FillRingSection::Fill(int power)
{
	currPower += power;
	if (currPower >= maxPower)
	{
		int ret = (currPower - maxPower);//-currPower - 1;
		currPower = maxPower;
		return ret;
	}
	else
		return 0;
}

void FillRingSection::Draw(sf::RenderTarget *target)
{
	target->draw(ringSprite, &ringShader);
}

//you are trying to figure out a simple way to represent this so that you don't have to touch up all the 
//vertices each frame. you should only change the color if the color needs to be changed


void FillRingSection::SetupSection( sf::Vector2f &centerPos)
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

bool FillRingSection::IsFull()
{
	return (currPower == maxPower);
}

bool FillRingSection::IsEmpty()
{
	return (currPower == 0);
}

void FillRing::SetPosition(sf::Vector2f &pos)
{
	centerPos = pos;
	for (int i = 0; i < numRings; ++i)
	{
		rings[i]->SetupSection(centerPos);
	}
}

FillRing::FillRing( Vector2f &pos, int p_numRings, FillRingSection **p_rings)
	:numRings( p_numRings ), rings( p_rings ), centerPos( pos )
{

	//centerPos = Vector2f( 90, 230 );

	numTotalVertices = 0;
	int radiusSum = 50; //internal start radius

	rings = new FillRingSection*[numRings];

	for (int i = 0; i < numRings; ++i)
	{
		rings[i] = p_rings[i];
		rings[i]->SetupSection(centerPos);
	}
	ResetFull();
	/*for (int i = 0; i < numRings; ++i)
	{
		rings[i]->SetupSection(centerPos, radiusSum);
		rings[i]->Reset();
		radiusSum += rings[i]->width;
		numTotalVertices += rings[i]->maxPower * 4;
	}*/
	//CreateRing();
}

FillRing::~FillRing()
{
	for (int i = 0; i < numRings; ++i)
	{
		delete rings[i];
	}
	delete[] rings;
}


bool FillRing::IsFull()
{
	for (int i = 0; i < numRings; ++i)
	{
		if (!rings[i]->IsFull())
			return false;
	}
	return true;
}

bool FillRing::IsEmpty()
{
	for (int i = 0; i < numRings; ++i)
	{
		if (!rings[i]->IsEmpty())
			return false;
	}
	return true;
}

void FillRing::ResetEmpty()
{
	currRing = 0;
	for (int i = 0; i < numRings; ++i)
	{
		rings[i]->ResetEmpty();
	}
}

void FillRing::ResetFull()
{
	currRing = numRings - 1;
	for (int i = 0; i < numRings; ++i)
	{
		rings[i]->ResetFull();
	}
}


void FillRing::Update()
{
	for (int i = 0; i < numRings; ++i)
	{
		rings[i]->Update();
	}
	
}

void FillRing::CreateRing()
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

void FillRing::Draw( sf::RenderTarget *target )
{
	rings[currRing]->Draw(target);
	/*for (int i = 0; i < numRings; ++i)
	{
		rings[i]->Draw(target);
	}*/
	//target->draw( *ringVA );
	//target->draw( *middleVA );
	//target->draw( scorpTest );
	//target->draw( keyTest );
}

int FillRing::Fill( int fill )
{
	//cout << "fill: " << fill << endl;
	int res = rings[currRing]->Fill(fill);
	if (res > 0 && currRing < numRings - 1)
	{
		++currRing;
		return Fill(res);	
	}

	return res;
}

float FillRing::GetCurrPortionOfTotal()
{
	return ((float)rings[currRing]->currPower) / rings[currRing]->maxPower;
}

int FillRing::Drain(int drain)
{
	int res = rings[currRing]->Drain(drain);
	if (res > 0 && currRing > 0)
	{
		--currRing;
		return Drain(res);
	}

	return res;
}

DesperationOrb::DesperationOrb(TilesetManager &tm, Vector2f &pos )
{
	ts_orb = tm.GetTileset("HUD/powerring0_100x100.png", 100, 100);
	orbSpr.setTexture(*ts_orb->texture);
	orbSpr.setOrigin(orbSpr.getLocalBounds().width / 2, orbSpr.getLocalBounds().height / 2);
	orbSpr.setPosition(pos);
}

void DesperationOrb::SetPosition(sf::Vector2f &pos)
{
	orbSpr.setPosition(pos);
}

sf::Vector2f DesperationOrb::GetPosition()
{
	return orbSpr.getPosition();
}

void DesperationOrb::Draw(sf::RenderTarget *target)
{
	target->draw(orbSpr);
}

KinRing::KinRing( Actor *actor )
{

	TilesetManager &tm = *actor->owner;
	float totalHealth = actor->totalHealth;
	FillRingSection *blah[] = {
		//new FillRingSection(tm, Color::Black, sf::Color::Black, sf::Color::Black,0, 300, 0),
		new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,1, totalHealth/3, -PI / 2.0),
		new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,1, totalHealth/3, -PI / 2.0),
		new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,1, totalHealth/3, -PI / 2.0),
		//new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,1, totalHealth / 3, 0),
		//new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,2, totalHealth / 3, 0),
		//new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,3, totalHealth / 3, 0)
	};

	//Vector2f powerRingPos(80, 220);
	Vector2f powerRingPos(0, 0);
	powerRing = new PowerRing(powerRingPos, sizeof(blah) / sizeof(FillRingSection*), blah);
	despOrb = new DesperationOrb(tm, powerRingPos);

	statusCircle.setRadius(30);
	//statusCircle.setOutlineColor(Color::Black);
	//statusCircle.setOutlineThickness(3);
	statusCircle.setFillColor(Color::Green);
	statusCircle.setOrigin(statusCircle.getLocalBounds().width / 2,
		statusCircle.getLocalBounds().height / 2);
}

sf::Vector2f KinRing::GetCenter()
{
	return despOrb->GetPosition();
}

void KinRing::SetCenter(sf::Vector2f &pos)
{
	powerRing->SetPosition(pos);
	despOrb->SetPosition(pos);
	statusCircle.setPosition(pos);
}

void KinRing::Reset()
{
	powerRing->ResetFull();
}

KinRing::~KinRing()
{
	delete powerRing;
	delete despOrb;
}

void KinRing::Update()
{
	powerRing->Update();
	//float portion = powerRing->GetCurrPortionOfTotal();
	//Color c = Color::Red;
	//Color c1 = Color::Green;
	Color green(30, 155, 67);
	Color yellow(228, 223, 5);
	Color red(211, 10, 15);

	if (powerRing->currRing == 2)
	{
		statusCircle.setFillColor(green);
	}
	else if (powerRing->currRing == 1)
	{
		statusCircle.setFillColor(yellow);
	}
	else
	{
		statusCircle.setFillColor(red);
	}

	/*if (portion > .5)
	{
		
	}
	else if (portion > .3)
	{
		
	}
	else if (portion >= 0)
	{
		
	}*/
}

void KinRing::Draw(RenderTarget *target)
{
	target->draw(statusCircle);
	powerRing->Draw(target);
	despOrb->Draw(target);
}