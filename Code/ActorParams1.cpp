#include "EditSession.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Physics.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
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


PatrollerParams::PatrollerParams(ActorType *at, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop )
	:ActorParams(at)
{	
	lines = NULL;
	PlaceAerial(pos);

	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;
}

PatrollerParams::PatrollerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	lines = NULL;

	LoadAerial(is);

	LoadMonitor(is);

	LoadGlobalPath(is);

	LoadBool(is, loop);

	is >> speed;

	
}

PatrollerParams::PatrollerParams(ActorType *at,
	sf::Vector2i &pos )
	:ActorParams(at )
{	
	lines = NULL;
	PlaceAerial(pos);

	loop = false;
	speed = 10;
}

void PatrollerParams::SetParams()
{
	Panel *p = type->panel;

	bool loop = p->checkBoxes["loop"]->checked;
	

	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	stringstream ss;
	ss << speedStr;

	int t_speed; 
	ss >> t_speed;

	if( !ss.fail() )
	{
		speed = t_speed;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
	//try
	//{
	//	speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
	//}
	//catch(boost::bad_lexical_cast &)
	//{
	//	//error
	//}
}

void PatrollerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );
	p->checkBoxes["loop"]->checked = loop;
	p->checkBoxes["monitor"]->checked = hasMonitor;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
}

void PatrollerParams::Draw( sf::RenderTarget *target )
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
	//target->draw( image );

	//DrawBoundar
}

void PatrollerParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);

	WritePath(of);
	WriteLoop(of);

	of << speed << endl;
}

ActorParams *PatrollerParams::Copy()
{
	PatrollerParams *copy = new PatrollerParams( *this );
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

CrawlerParams::CrawlerParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity,
	int level )
	:ActorParams(at)
{
	enemyLevel = level;
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

CrawlerParams::CrawlerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	LoadMonitor(is);

	LoadEnemyLevel(is);
}

CrawlerParams::CrawlerParams(ActorType *at,
		TerrainPolygon *p_edgePolygon,
		int p_edgeIndex, double p_edgeQuantity )
		:ActorParams( at)
{
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

ActorParams *CrawlerParams::Copy()
{
	CrawlerParams *copy = new CrawlerParams( *this );
	return copy;
}





BossCrawlerParams::BossCrawlerParams(ActorType *at, TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( at)
{
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

BossCrawlerParams::BossCrawlerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
}

ActorParams *BossCrawlerParams::Copy()
{
	BossCrawlerParams *copy = new BossCrawlerParams( *this );
	return copy;
}




BasicTurretParams::BasicTurretParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams(at )
{
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

BasicTurretParams::BasicTurretParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadMonitor(is);
}

void BasicTurretParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);
	WriteLevel(of);
}

void BasicTurretParams::SetParams()
{
	/*Panel *p = type->panel;
	stringstream ss;
	string bulletSpeedString = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
	string framesWaitString = p->textBoxes["waitframes"]->text.getString().toAnsiString();
	ss << bulletSpeedString;
			
	int fWait;
	ss >> fWait;

	if( !ss.fail() )
	{
		framesWait = fWait;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;*/
}

void BasicTurretParams::SetPanelInfo()
{
	/*Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( bulletSpeed ) );
	p->textBoxes["waitframes"]->text.setString( boost::lexical_cast<string>( framesWait ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;*/
}

ActorParams *BasicTurretParams::Copy()
{
	BasicTurretParams *copy = new BasicTurretParams( *this );
	return copy;
}




FootTrapParams::FootTrapParams(ActorType *at, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( at)	
{
	PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

FootTrapParams::FootTrapParams(ActorType *at, ifstream &is )
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadMonitor(is);
}

void FootTrapParams::SetParams()
{
	Panel *p = type->panel;
	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void FootTrapParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

void FootTrapParams::WriteParamFile( ofstream &of )
{
	WriteMonitor(of);
}

ActorParams *FootTrapParams::Copy()
{
	FootTrapParams *copy = new FootTrapParams( *this );
	return copy;
}

AirdasherParams::AirdasherParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	PlaceAerial(pos);
}

AirdasherParams::AirdasherParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
}

void AirdasherParams::WriteParamFile(std::ofstream &of)
{
	WriteMonitor(of);
}

void AirdasherParams::SetParams()
{
	Panel *p = type->panel;
	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void AirdasherParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *AirdasherParams::Copy()
{
	AirdasherParams *copy = new AirdasherParams(*this);
	return copy;
}

BoosterParams::BoosterParams(ActorType *at, sf::Vector2i &pos, int p_strength)
	:ActorParams(at), strength(p_strength)
{
	PlaceAerial(pos);
}


BoosterParams::BoosterParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> strength;
}

BoosterParams::BoosterParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	PlaceAerial(pos);

	strength = 10;
}

void BoosterParams::WriteParamFile(std::ofstream &of)
{
	of << strength << "\n";
}

void BoosterParams::SetParams()
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

void BoosterParams::SetPanelInfo()
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

ActorParams *BoosterParams::Copy()
{
	BoosterParams *copy = new BoosterParams(*this);
	return copy;
}

SpringParams::SpringParams(ActorType *at, sf::Vector2i &pos, std::list<sf::Vector2i> &globalPath,
	int p_moveFrames)
	:ActorParams(at), moveFrames(p_moveFrames)
{
	PlaceAerial(pos);

	lines = NULL;

	SetPath(globalPath);
}

SpringParams::SpringParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> moveFrames;

	lines = NULL;

	Vector2i other;
	is >> other.x;
	is >> other.y;

	list<Vector2i> globalPath;
	globalPath.push_back(Vector2i(position.x, position.y));
	globalPath.push_back(position + other);
	SetPath(globalPath);

	
}

SpringParams::SpringParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	PlaceAerial(pos);

	moveFrames = 60;

	lines = NULL;


}

void SpringParams::WriteParamFile(std::ofstream &of)
{
	of << moveFrames << "\n";
	of << localPath.front().x << " " << localPath.front().y << endl;
}

void SpringParams::SetPath(std::list<sf::Vector2i> &globalPath)
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

void SpringParams::SetParams()
{
	Panel *p = type->panel;

	string moveFrameStr = p->textBoxes["moveframes"]->text.getString().toAnsiString();

	stringstream ss;
	ss << moveFrameStr;

	int t_moveFrames;
	ss >> t_moveFrames;

	if (!ss.fail())
	{
		moveFrames = t_moveFrames;
	}


	hasMonitor = false;

}

void SpringParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
	{
		p->textBoxes["group"]->text.setString(group->name);
	}

	p->textBoxes["moveframes"]->text.setString((boost::lexical_cast<string>(moveFrames)));

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}
ActorParams *SpringParams::Copy()
{
	SpringParams *copy = new SpringParams(*this);
	return copy;
}

void SpringParams::Draw(sf::RenderTarget *target)
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


ComboerParams::ComboerParams(ActorType *at, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop)
	:ActorParams(at)
{
	lines = NULL;
	PlaceAerial(pos);
	SetPath(globalPath);

	loop = p_loop;
	speed = p_speed;

}

ComboerParams::ComboerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	lines = NULL;

	LoadAerial(is);

	LoadMonitor(is);

	LoadGlobalPath(is);

	LoadBool(is, loop);

	is >> speed;
}

ComboerParams::ComboerParams(ActorType *at,
	sf::Vector2i &pos)
	:ActorParams(at)
{
	lines = NULL;

	PlaceAerial(pos);
	loop = false;
	speed = 10;
}

void ComboerParams::SetParams()
{
	Panel *p = type->panel;

	bool loop = p->checkBoxes["loop"]->checked;


	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	stringstream ss;
	ss << speedStr;

	int t_speed;
	ss >> t_speed;

	if (!ss.fail())
	{
		speed = t_speed;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
	//try
	//{
	//	speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
	//}
	//catch(boost::bad_lexical_cast &)
	//{
	//	//error
	//}
}

void ComboerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	p->textBoxes["speed"]->text.setString(boost::lexical_cast<string>(speed));
	p->checkBoxes["loop"]->checked = loop;
	p->checkBoxes["monitor"]->checked = hasMonitor;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath = GetGlobalPath();
}


void ComboerParams::Draw(sf::RenderTarget *target)
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



		if (loop)
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White)
			};

			target->draw(vertices, 2, sf::Lines);
		}


		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= Vector2f(position.x, position.y);
		}
	}

	ActorParams::Draw(target);
	//target->draw( image );

	//DrawBoundar
}

void ComboerParams::WriteParamFile(ofstream &of)
{
	WriteMonitor(of);

	WritePath(of);
	WriteLoop(of);

	of << speed << endl;
}

ActorParams *ComboerParams::Copy()
{
	ComboerParams *copy = new ComboerParams(*this);
	if (copy->lines != NULL)
	{
		int numVertices = copy->lines->getVertexCount();

		VertexArray &oldli = *copy->lines;
		copy->lines = new VertexArray(sf::LinesStrip, numVertices);
		VertexArray &li = *copy->lines;


		for (int i = 0; i < numVertices; ++i)
		{
			li[i] = oldli[i];
		}
	}
	return copy;
}