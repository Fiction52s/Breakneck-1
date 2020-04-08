#include "EditSession.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Physics.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "Boss.h"
#include "ActorParams.h"

using namespace std;
using namespace sf;

#define cout std::cout

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )



BatParams::BatParams(ActorType *at, sf::Vector2i pos, list<Vector2i> &globalPath, int p_bulletSpeed, int p_framesBetweenNodes, bool p_loop )
	:ActorParams(at )
{	
	lines = NULL;
	PlaceAerial(pos);
	SetPath( globalPath );

	framesBetweenNodes = p_framesBetweenNodes; 
	//nodeDistance = p_nodeDistance;
	bulletSpeed = p_bulletSpeed;

	loop = p_loop;	
}

BatParams::BatParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	lines = NULL;

	LoadAerial(is);
	LoadMonitor(is);
	LoadGlobalPath(is);

	LoadBool(is, loop);

	is >> bulletSpeed;
	is >> framesBetweenNodes;

	
}

BatParams::BatParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at)
{	
	lines = NULL;
	PlaceAerial(pos);
	loop = false;
	//speed = 5;
	framesBetweenNodes = 60;
	//nodeDistance = 100;
	bulletSpeed = 10;

}

void BatParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		
		VertexArray &li = *lines;
	
	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position += Vector2f( position.x, position.y );
		}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	ActorParams::Draw( target );
}

void BatParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);

	WritePath(of);
	WriteLoop(of);

	of << bulletSpeed << endl;
	of << framesBetweenNodes << endl;
}

void BatParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string bulletSpeedStr = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
	//string nodeDistanceStr = p->textBoxes["nodedistance"]->text.getString().toAnsiString();
	string betweenStr = p->textBoxes["framesbetweennodes"]->text.getString().toAnsiString();
	bool t_loop = p->checkBoxes["loop"]->checked;

	hasMonitor = p->checkBoxes["monitor"]->checked;
	
	ss << bulletSpeedStr;

	int t_bulletSpeed;
	ss >> t_bulletSpeed;

	if( !ss.fail() )
	{
		bulletSpeed = t_bulletSpeed;
	}

	/*ss.clear();

	ss << nodeDistanceStr;

	int t_nodeDistance;
	ss >> t_nodeDistance;

	if( !ss.fail() )
	{
		nodeDistance = t_nodeDistance;
	}*/

	ss.clear();

	ss << betweenStr;

	int t_framesBetweenNodes;
	ss >> t_framesBetweenNodes;

	if( !ss.fail() )
	{
		framesBetweenNodes = t_framesBetweenNodes;
	}

	ss.clear();

	loop = t_loop;
}

void BatParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( bulletSpeed ) );
	//p->textBoxes["nodedistance"]->text.setString( boost::lexical_cast<string>( nodeDistance ) );
	p->textBoxes["framesbetweennodes"]->text.setString( boost::lexical_cast<string>( framesBetweenNodes ) );
	p->checkBoxes["loop"]->checked = loop;
	p->checkBoxes["monitor"]->checked = false;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
	//EditSession::SetMonitorGrid( monitorType, p->gridSelectors["monitortype"] );
}

ActorParams *BatParams::Copy()
{
	BatParams *copy = new BatParams( *this );
	if( copy->lines != NULL )
	{
		int numVertices = copy->lines->getVertexCount();

		VertexArray &oldli = *copy->lines;
		copy->lines = new VertexArray( sf::LinesStrip, numVertices );
		VertexArray &li = *copy->lines;
		

		for( int i = 0; i < numVertices; ++i )
		{
			li[i] = oldli[i];
		}
	}
	return copy;
}




StagBeetleParams::StagBeetleParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, bool p_clockwise, float p_speed )
	:ActorParams( at)
{
	clockwise = p_clockwise;
	speed = p_speed;

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

StagBeetleParams::StagBeetleParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadMonitor(is);

	string cwStr;
	is >> cwStr;

	if (cwStr == "+clockwise")
		clockwise = true;
	else if (cwStr == "-clockwise")
		clockwise = false;
	else
	{
		assert(false && "boolean problem");
	}

	is >> speed;
}

StagBeetleParams::StagBeetleParams(ActorType *at,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at ), clockwise( true ), speed( 0 )
{
	
	speed = 10;
	clockwise = true;

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );	
}

void StagBeetleParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);
	if( clockwise )
		of << "+clockwise" << endl;
	else
		of << "-clockwise" << endl;
	
	of.precision( 5 );
	of << fixed << speed << endl;
}

void StagBeetleParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;

	bool t_clockwise = p->checkBoxes["clockwise"]->checked;
	double t_speed;

	stringstream ss;
	string s = p->textBoxes["speed"]->text.getString().toAnsiString();
	ss << s;

	ss >> t_speed;

	if( !ss.fail() )
	{
		speed = t_speed;
	}

	//ss.clear();

	clockwise = t_clockwise;

	
}

void StagBeetleParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *StagBeetleParams::Copy()
{
	StagBeetleParams *copy = new StagBeetleParams( *this );
	return copy;
}

PoisonFrogParams::PoisonFrogParams(ActorType *at, TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, double p_edgeQuantity)//, bool p_clockwise, float p_speed )
	:ActorParams(at), pathQuads( sf::Quads, 4 * 50 )
{
	gravFactor = 30;
	jumpWaitFrames = 60;
	jumpStrength = Vector2i( 5, 12 );

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	UpdatePath();
}

PoisonFrogParams::PoisonFrogParams(ActorType *at, TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, double p_edgeQuantity, int p_gravFactor, sf::Vector2i &p_jumpStrength,
	int p_jumpWaitFrames )
	:ActorParams( at), pathQuads( sf::Quads, 4 * 50 )
{
	gravFactor = p_gravFactor;
	jumpStrength = p_jumpStrength;
	jumpWaitFrames = p_jumpWaitFrames;

	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );	

	UpdatePath();
}

PoisonFrogParams::PoisonFrogParams(ActorType *at, ifstream &is)
	:ActorParams(at), pathQuads(sf::Quads, 4 * 50)
{
	LoadGrounded(is);
	LoadMonitor(is);

	is >> gravFactor;
	is >> jumpStrength.x;
	is >> jumpStrength.y;
	is >> jumpWaitFrames;

	UpdatePath();
}

void PoisonFrogParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);
	of << gravFactor << endl;
	of << jumpStrength.x << " " << jumpStrength.y << endl;
	of << jumpWaitFrames << endl;
	/*if( clockwise )
		of << "+clockwise" << endl;
	else
		of << "-clockwise" << endl;
	
	of.precision( 5 );
	of << fixed << speed << endl;*/
}

void PoisonFrogParams::SetParams()
{
	Panel *p = type->panel;

	//bool clockwise = p->checkBoxes["clockwise"]->checked;
	//double speed;
	hasMonitor = p->checkBoxes["monitor"]->checked;

	stringstream ss;
	string xStrengthStr = p->textBoxes["xstrength"]->text.getString().toAnsiString();
	string yStrengthStr = p->textBoxes["ystrength"]->text.getString().toAnsiString();
	string jumpWaitFramesStr = p->textBoxes["jumpwaitframes"]->text.getString().toAnsiString();
	string gravityFactorStr = p->textBoxes["gravfactor"]->text.getString().toAnsiString();
	
	ss << xStrengthStr;

	int t_xStrength;
	ss >> t_xStrength;

	if( !ss.fail() )
	{
		jumpStrength.x = t_xStrength;
	}

	ss.clear();

	ss << yStrengthStr;

	int t_yStrength;
	ss >> t_yStrength;

	if( !ss.fail() )
	{
		jumpStrength.y = t_yStrength;
	}

	ss.clear();

	ss << jumpWaitFramesStr;

	int t_jumpWaitFrames;
	ss >> t_jumpWaitFrames;

	if( !ss.fail() )
	{
		jumpWaitFrames = t_jumpWaitFrames;
	}

	ss.clear();

	ss << gravityFactorStr;

	int t_gravFactor;
	ss >> t_gravFactor;

	if( !ss.fail() )
	{
		gravFactor = t_gravFactor;
	}

	UpdatePath();
}

void PoisonFrogParams::SetPanelInfo()
{
	Panel *p = type->panel;
	
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["jumpwaitframes"]->text.setString( boost::lexical_cast<string>( jumpWaitFrames ) ); 
	p->textBoxes["xstrength"]->text.setString( boost::lexical_cast<string>( jumpStrength.x ) ); 
	p->textBoxes["ystrength"]->text.setString( boost::lexical_cast<string>( jumpStrength.y ) ); 
	p->textBoxes["gravfactor"]->text.setString( boost::lexical_cast<string>( gravFactor ) ); 
	p->checkBoxes["monitor"]->checked = hasMonitor;
}

void PoisonFrogParams::UpdatePath()
{
	int totalQuads = 50;

	if( groundInfo == NULL )
	{
		for( int i = 0; i < totalQuads; ++i )
		{
			pathQuads[i*4+0].position = Vector2f( 0,0 );
			pathQuads[i*4+1].position = Vector2f( 0,0 );
			pathQuads[i*4+2].position = Vector2f( 0,0 );
			pathQuads[i*4+3].position = Vector2f( 0,0 );
		}
		return;
	}

	Color pathColor( 0, 255, 0 );
	
	int squareRad = 4;// * EditSession::zoomMultiple;
	Vector2f pos( position.x, position.y );

	Vector2f gravity( 0, gravFactor / 64.f );

	V2d fireDir;
	TerrainPoint *curr = groundInfo->edgeStart;
	TerrainPoint *next = groundInfo->GetNextPoint();

	V2d e( next->pos.x - curr->pos.x, next->pos.y - curr->pos.y );
	e = normalize( e );
	e = V2d( e.y, -e.x );

	

	//negative x jumpstrength means you face left
	Vector2f vel = Vector2f( jumpStrength.x, -jumpStrength.y );
	
	if( e.y > 0 )
	{
		gravity.y = -gravity.y;
		vel.y = -vel.y;
	}
	
	for( int i = 0; i < totalQuads; ++i )
	{
		//cout << "i: " << i << endl;
		pathQuads[i*4+0].position = Vector2f( pos.x - squareRad,
			pos.y - squareRad );
		pathQuads[i*4+1].position = Vector2f( pos.x + squareRad,
			pos.y - squareRad );
		pathQuads[i*4+2].position = Vector2f( pos.x + squareRad,
			pos.y + squareRad );
		pathQuads[i*4+3].position = Vector2f( pos.x - squareRad,
			pos.y + squareRad );

		pathQuads[i*4+0].color = pathColor;
		pathQuads[i*4+1].color = pathColor;
		pathQuads[i*4+2].color = pathColor;
		pathQuads[i*4+3].color = pathColor;

		pos += vel;
		vel += gravity;
	}
}

void PoisonFrogParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	target->draw( pathQuads );
}

void PoisonFrogParams::UpdateExtraVisuals()
{
	UpdatePath();
}

ActorParams *PoisonFrogParams::Copy()
{
	PoisonFrogParams *copy = new PoisonFrogParams( *this );
	return copy;
}




CurveTurretParams::CurveTurretParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, double p_bulletSpeed, int p_framesWait,
	sf::Vector2i p_gravFactor, bool relative )
	:ActorParams(at ), bulletPathQuads( sf::Quads, 100 * 4 )
{
	bulletSpeed = p_bulletSpeed;
	framesWait = p_framesWait;
	gravFactor = p_gravFactor;
	relativeGrav = relative;

	
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

CurveTurretParams::CurveTurretParams(ActorType *at, ifstream &is)
	:ActorParams(at), bulletPathQuads(sf::Quads, 100 * 4)
{
	LoadGrounded(is);
	LoadMonitor(is);

	is >> bulletSpeed;
	is >> framesWait;
	is >> gravFactor.x;
	is >> gravFactor.y;

	relativeGrav = false;
	string relativeGravStr;
	is >> relativeGravStr;
	if (relativeGravStr == "+relative")
	{
		relativeGrav = true;
	}
}

CurveTurretParams::CurveTurretParams(ActorType *at,
		TerrainPolygon *p_edgePolygon,
		int p_edgeIndex, double p_edgeQuantity )
		:ActorParams(at ), bulletPathQuads( sf::Quads, 100 * 4 )
{	
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	gravFactor = Vector2i( 0, 0 );
	framesWait = 60;
	relativeGrav = true;
	bulletSpeed = 1;	
}

void CurveTurretParams::UpdateExtraVisuals()
{
	UpdateBulletCurve();
}

void CurveTurretParams::WriteParamFile( ofstream &of )
{
	//cout << "write curve turret params. this: " << (int)this << endl;
	WriteMonitor(of);
	of << bulletSpeed << endl;
	of << framesWait << endl;
	of << gravFactor.x << endl;
	of << gravFactor.y << endl;

	if( relativeGrav )
	{
		of << "+relative" << endl;
	}
	else
	{
		of << "-relative" << endl;
	}
	
}

void CurveTurretParams::UpdateBulletCurve()
{
	int totalQuads = 100;

	double factorGrav = 256;

	if( groundInfo == NULL )
	{
		for( int i = 0; i < totalQuads; ++i )
		{
			bulletPathQuads[i*4+0].position = Vector2f( 0,0 );
			bulletPathQuads[i*4+1].position = Vector2f( 0,0 );
			bulletPathQuads[i*4+2].position = Vector2f( 0,0 );
			bulletPathQuads[i*4+3].position = Vector2f( 0,0 );
		}
		return;
	}

	Color pathColor( 255, 0, 0 );
	int squareRad = 4;// * EditSession::zoomMultiple;
	Vector2f pos( position.x, position.y );
	V2d fireDir;

	
	TerrainPoint *curr = groundInfo->edgeStart;
	TerrainPoint *next = groundInfo->GetNextPoint();

	V2d e( next->pos.x - curr->pos.x, next->pos.y - curr->pos.y );
	V2d groundDir = normalize( e );
	e = V2d( groundDir.y, -groundDir.x );

	Vector2f bulletVel = Vector2f( e.x, e.y ) * bulletSpeed;
	
	for( int i = 0; i < totalQuads; ++i )
	{
		//cout << "i: " << i << endl;
		bulletPathQuads[i*4+0].position = Vector2f( pos.x - squareRad,
			pos.y - squareRad );
		bulletPathQuads[i*4+1].position = Vector2f( pos.x + squareRad,
			pos.y - squareRad );
		bulletPathQuads[i*4+2].position = Vector2f( pos.x + squareRad,
			pos.y + squareRad );
		bulletPathQuads[i*4+3].position = Vector2f( pos.x - squareRad,
			pos.y + squareRad );

		bulletPathQuads[i*4+0].color = pathColor;
		bulletPathQuads[i*4+1].color = pathColor;
		bulletPathQuads[i*4+2].color = pathColor;
		bulletPathQuads[i*4+3].color = pathColor;

		pos += bulletVel;

		V2d trueGrav( 0, 0 );//( gravFactor.x, gravFactor.y );

		if( relativeGrav )
		{
			trueGrav += groundDir * ( gravFactor.x / factorGrav );
			trueGrav += e * (-gravFactor.y / factorGrav );
		}
		else
		{
			trueGrav = V2d( gravFactor.x / factorGrav, 
				gravFactor.y / factorGrav );
		}
		
		//trueGrav.x = e.y * gravFactor.x;
		//trueGrav.y = e.x * gravFactor.y;

		bulletVel += Vector2f( trueGrav.x, trueGrav.y );
	}
}

void CurveTurretParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;

	stringstream ss;
	string bulletSpeedString = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
	string framesWaitString = p->textBoxes["waitframes"]->text.getString().toAnsiString();
	string xGravString = p->textBoxes["xgravfactor"]->text.getString().toAnsiString();
	string yGravString = p->textBoxes["ygravfactor"]->text.getString().toAnsiString();
	
	relativeGrav = p->checkBoxes["relativegrav"]->checked;

	ss << bulletSpeedString;

	double t_bulletSpeed;
	ss >> t_bulletSpeed;

	if( !ss.fail() )
	{
		bulletSpeed = t_bulletSpeed;
		//assert( false );
	}

	ss.clear();

	ss << framesWaitString;

	int t_framesWait;
	ss >> t_framesWait;

	if( !ss.fail() )
	{
		framesWait = t_framesWait;
	}

	ss.clear();
			
	int t_xGravFactor;
	ss << xGravString;

	ss >> t_xGravFactor;

	if( !ss.fail() )
	{
		gravFactor.x = t_xGravFactor;
	}

	ss.clear();

	int t_yGravFactor;
	ss << yGravString;

	ss >> t_yGravFactor;

	if( !ss.fail() )
	{
		gravFactor.y = t_yGravFactor;
	}

	UpdateBulletCurve();
	//also set up visuals
}

void CurveTurretParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( bulletSpeed ) );
	p->textBoxes["waitframes"]->text.setString( boost::lexical_cast<string>( framesWait ) );
	p->textBoxes["xgravfactor"]->text.setString( boost::lexical_cast<string>( gravFactor.x ) );
	p->textBoxes["ygravfactor"]->text.setString( boost::lexical_cast<string>( gravFactor.y ) );
	p->checkBoxes["relativegrav"]->checked = relativeGrav;
	p->checkBoxes["monitor"]->checked = hasMonitor;
}

void CurveTurretParams::Draw( RenderTarget *target )
{
	ActorParams::Draw( target );

	target->draw( bulletPathQuads );
}

ActorParams *CurveTurretParams::Copy()
{
	CurveTurretParams *copy = new CurveTurretParams( *this );
	return copy;
}

BossBirdParams::BossBirdParams(ActorType *at, Vector2i &pos )
	:ActorParams( at), debugLines( sf::Lines, 4 * 2 )
{
	PlaceAerial(pos);

	width = Boss_Bird::GRID_SIZE_X * 160;
	height = Boss_Bird::GRID_SIZE_Y * 80;
}

BossBirdParams::BossBirdParams(ActorType *at, ifstream &is)
	:ActorParams(at), debugLines(sf::Lines, 4 * 2)
{
	LoadAerial(is);

	width = Boss_Bird::GRID_SIZE_X * 160;
	height = Boss_Bird::GRID_SIZE_Y * 80;
}


ActorParams *BossBirdParams::Copy()
{
	BossBirdParams *copy = new BossBirdParams( *this );
	return copy;
}

void BossBirdParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	//target->draw( debugLines );
}

void BossBirdParams::CreateFormation()
{
	//depreciated
	sf::Vector2f center( position.x, position.y );
	sf::Vector2f origin( center.x - width / 2.f, center.y - height / 2.f );

	debugLines[0].position = origin;
	debugLines[1].position = Vector2f( origin.x + width, origin.y );

	debugLines[2].position = Vector2f( origin.x + width, origin.y );
	debugLines[3].position = Vector2f( origin.x + width, origin.y + height );

	debugLines[4].position = Vector2f( origin.x + width, origin.y + height );
	debugLines[5].position = Vector2f( origin.x, origin.y + height );

	debugLines[6].position = Vector2f( origin.x, origin.y + height );
	debugLines[7].position = origin;

	for( int i = 0; i < 4 * 2; ++i )
	{
		debugLines[i].color = Color::Green;
	}
}

GravityFallerParams::GravityFallerParams(ActorType *at, TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, double p_edgeQuantity)//, bool p_clockwise, float p_speed )
	:ActorParams(at)
{
	variation = 0;

	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

GravityFallerParams::GravityFallerParams(ActorType *at, TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, double p_edgeQuantity, int var )
	:ActorParams(at)
{
	variation = var;

	PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

GravityFallerParams::GravityFallerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadMonitor(is);

	is >> variation;
}


void GravityFallerParams::WriteParamFile(ofstream &of)
{
	WriteMonitor(of);
	of << variation << endl;
	/*if( clockwise )
	of << "+clockwise" << endl;
	else
	of << "-clockwise" << endl;

	of.precision( 5 );
	of << fixed << speed << endl;*/
}

void GravityFallerParams::SetParams()
{
	Panel *p = type->panel;

	//bool clockwise = p->checkBoxes["clockwise"]->checked;
	//double speed;
	hasMonitor = p->checkBoxes["monitor"]->checked;

	stringstream ss;
	string varStr = p->textBoxes["var"]->text.getString().toAnsiString();

	ss << varStr;

	int t_variation;
	ss >> t_variation;

	if (!ss.fail())
	{
		variation = t_variation;
	}
}

void GravityFallerParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	p->textBoxes["var"]->text.setString(boost::lexical_cast<string>(variation));
	p->checkBoxes["monitor"]->checked = hasMonitor;
}

void GravityFallerParams::Draw(sf::RenderTarget *target)
{
	ActorParams::Draw(target);
}

ActorParams *GravityFallerParams::Copy()
{
	GravityFallerParams *copy = new GravityFallerParams(*this);
	return copy;
}

GravitySpringParams::GravitySpringParams(ActorType *at, sf::Vector2i &pos, std::list<sf::Vector2i> &globalPath,
	int p_speed)
	:ActorParams(at), speed( p_speed )
{
	PlaceAerial(pos);

	lines = NULL;

	SetPath(globalPath);
}

GravitySpringParams::GravitySpringParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> speed;

	Vector2i other;
	is >> other.x;
	is >> other.y;

	lines = NULL;

	list<Vector2i> globalPath;
	globalPath.push_back(Vector2i(position.x, position.y));
	globalPath.push_back(position + other);
	SetPath(globalPath);
}

GravitySpringParams::GravitySpringParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	PlaceAerial(pos);

	speed = 30;

	lines = NULL;
}

void GravitySpringParams::WriteParamFile(std::ofstream &of)
{
	of << speed << "\n";
	if (localPath.size() == 0)
	{
		of << 0 << " " << 0 << endl;
	}
	else
	{
		of << localPath.front().x << " " << localPath.front().y << endl;
	}
	
}

void GravitySpringParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	ActorParams::SetPath(globalPath);
	if (globalPath.size() > 1)
	{
		VertexArray &li = *lines;
		Vector2f diff = li[1].position - li[0].position;
		float f = GetVectorAngleCW(diff);
		float rot = f / PI * 180.f + 90;
		image.setRotation(rot);
	}
}

void GravitySpringParams::SetParams()
{
	Panel *p = type->panel;

	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	stringstream ss;
	ss << speedStr;

	int t_speed;
	ss >> t_speed;

	if (!ss.fail())
	{
		speed = t_speed;
	}

	hasMonitor = false;
}

void GravitySpringParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->textBoxes["speed"]->text.setString((boost::lexical_cast<string>(speed)));

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
}

ActorParams *GravitySpringParams::Copy()
{
	GravitySpringParams *copy = new GravitySpringParams(*this);
	return copy;
}

void GravitySpringParams::Draw(sf::RenderTarget *target)
{
	int localPathSize = localPath.size();

	if (localPathSize > 0)
	{
		VertexArray &li = *lines;


		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position += Vector2f(position.x, position.y);
		}


		target->draw(li);

		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= Vector2f(position.x, position.y);
		}
	}

	ActorParams::Draw(target);
}

GravityModifierParams::GravityModifierParams(ActorType *at, sf::Vector2i &pos, int p_strength)
	:ActorParams(at), strength(p_strength)
{
	PlaceAerial(pos);
}


GravityModifierParams::GravityModifierParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> strength;
}

GravityModifierParams::GravityModifierParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	PlaceAerial(pos);

	strength = 50;;
}

void GravityModifierParams::WriteParamFile(std::ofstream &of)
{
	of << strength << "\n";
}

void GravityModifierParams::SetParams()
{
	Panel *p = type->panel;

	string strengthStr = p->textBoxes["strength"]->text.getString().toAnsiString();

	stringstream ss;
	ss << strengthStr;

	int t_strength;
	ss >> t_strength;

	if (!ss.fail())
	{
		strength = t_strength;
	}
	//hasMonitor = p->checkBoxes["monitor"]->checked;
}

void GravityModifierParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->textBoxes["strength"]->text.setString(boost::lexical_cast<string>(strength));
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *GravityModifierParams::Copy()
{
	GravityModifierParams *copy = new GravityModifierParams(*this);
	return copy;
}
