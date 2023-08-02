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

SpecterParams::SpecterParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at )
{
	PlaceAerial(pos);
}

SpecterParams::SpecterParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
}

void SpecterParams::WriteParamFile(std::ofstream &of )
{
	WriteMonitor(of);
}

void SpecterParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void SpecterParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *SpecterParams::Copy()
{
	SpecterParams *copy = new SpecterParams( *this );
	return copy;
}


CopycatParams::CopycatParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams( at)
{
	PlaceAerial(pos);
}

CopycatParams::CopycatParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
}

void CopycatParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);
}

void CopycatParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void CopycatParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->checkBoxes["monitor"]->checked = hasMonitor;
}

ActorParams *CopycatParams::Copy()
{
	CopycatParams *copy = new CopycatParams( *this );
	return copy;
}

GorillaParams::GorillaParams(ActorType *at,
		sf::Vector2i &pos, int p_wallWidth,
		int p_followFrames )
		:ActorParams( at)
{
	wallWidth = p_wallWidth;
	followFrames = p_followFrames;
	PlaceAerial(pos);
}

GorillaParams::GorillaParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
	is >> wallWidth;
	is >> followFrames;
}

GorillaParams::GorillaParams(ActorType *at, sf::Vector2i &pos)
	:ActorParams(at)
{
	wallWidth = 400;
	followFrames = 60;
	PlaceAerial(pos);
}

void GorillaParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);
	of << wallWidth << endl;
	of << followFrames << endl;
}

void GorillaParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string wallWidthStr = p->textBoxes["wallwidth"]->text.getString().toAnsiString();
	string followFramesStr = p->textBoxes["followframes"]->text.getString().toAnsiString();

	hasMonitor = p->checkBoxes["monitor"]->checked;
	
	ss << wallWidthStr;

	int t_wallWidth;
	ss >> t_wallWidth;

	if( !ss.fail() )
	{
		wallWidth = t_wallWidth;
	}

	int t_followFrames;
	ss >> t_followFrames;

	if( !ss.fail() )
	{
		followFrames = t_followFrames;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void GorillaParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["wallwidth"]->SetString( boost::lexical_cast<string>( wallWidth ) );
	p->textBoxes["followframes"]->SetString( boost::lexical_cast<string>( followFrames ) );
	

	p->checkBoxes["monitor"]->checked = hasMonitor;
}
ActorParams *GorillaParams::Copy()
{
	GorillaParams *copy = new GorillaParams( *this );
	return copy;
}

NarwhalParams::NarwhalParams(ActorType *at,
		sf::Vector2i &pos, sf::Vector2i & p_dest,
		int p_moveFrames )
		:ActorParams( at)
{
	dest = p_dest;
	moveFrames = p_moveFrames;
	PlaceAerial(pos);
}

NarwhalParams::NarwhalParams(ActorType *at,ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	LoadMonitor(is);
	is >> dest.x;
	is >> dest.y;
	is >> moveFrames;
}

NarwhalParams::NarwhalParams(ActorType *at, sf::Vector2i &pos )
	:ActorParams(at )
{
	dest = pos + Vector2i( 500, 0 );
	moveFrames = 60;
	PlaceAerial(pos);
}

void NarwhalParams::SetPath( std::list<sf::Vector2i> &globalPath )
{
	if( globalPath.size() > 1 )
	{
		dest = globalPath.back();
	}
}

void NarwhalParams::Draw( RenderTarget *target )
{
	ActorParams::Draw( target );

	//if( position != dest )
	//{
	sf::Vertex line[] = { 
		sf::Vertex( GetFloatPos(), Color::Green ),
		sf::Vertex( Vector2f( dest.x, dest.y ), Color::Green ) };
	target->draw( line, 2, sf::Lines );
	//}
}

void NarwhalParams::WriteParamFile( std::ofstream &of )
{
	WriteMonitor(of);
	of << dest.x << " " << dest.y << endl;
	
	of << moveFrames << endl;
}

void NarwhalParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string moveFramesStr = p->textBoxes["moveframes"]->text.getString().toAnsiString();
	

	hasMonitor = p->checkBoxes["monitor"]->checked;
	
	ss << moveFramesStr;

	int t_moveFrames;
	ss >> t_moveFrames;

	if( !ss.fail() )
	{
		moveFrames = t_moveFrames;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}

void NarwhalParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["moveframes"]->SetString( boost::lexical_cast<string>( moveFrames ) );

	p->checkBoxes["monitor"]->checked = hasMonitor;

	EditSession *edit = EditSession::GetSession();
	edit->patrolPath.clear();
	edit->patrolPath.push_back(GetIntPos());
	edit->patrolPath.push_back(dest);
}

ActorParams *NarwhalParams::Copy()
{
	NarwhalParams *copy = new NarwhalParams( *this );
	return copy;
}

BossSkeletonParams::BossSkeletonParams(ActorType *at, Vector2i &pos )
	:ActorParams(at )
{
	PlaceAerial(pos);
}

BossSkeletonParams::BossSkeletonParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
}

ActorParams *BossSkeletonParams::Copy()
{
	BossSkeletonParams *copy = new BossSkeletonParams( *this );
	return copy;
}

SwingLauncherParams::SwingLauncherParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> speed;
	int cw;
	is >> cw;
	clockwise = cw;

	lines = NULL;

	Vector2i other;
	is >> other.x;
	is >> other.y;

	vector<Vector2i> globalPath;
	globalPath.reserve(2);
	Vector2i intPos = GetIntPos();
	globalPath.push_back(intPos);
	globalPath.push_back(intPos + other);
	SetPath(globalPath);
}

SwingLauncherParams::SwingLauncherParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0, 0));

	if (at->panel != NULL) //create for log
	{
		speed = at->panel->sliders["speed"]->defaultValue;
	}

	clockwise = true;

	lines = NULL;
}

void SwingLauncherParams::WriteParamFile(std::ofstream &of)
{
	of << speed << "\n";
	int cw = clockwise;
	of << cw << "\n";
	of << localPath.front().x << " " << localPath.front().y << endl;
}

void SwingLauncherParams::SetPath(std::vector<sf::Vector2i> &globalPath)
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

void SwingLauncherParams::SetParams()
{
	Panel *p = type->panel;

	speed = p->sliders["speed"]->GetCurrValue();
	clockwise = p->checkBoxes["clockwise"]->checked;

	hasMonitor = false;

}

void SwingLauncherParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->sliders["speed"]->SetCurrValue(speed);
	p->checkBoxes["clockwise"]->checked = clockwise;

	//EditSession *edit = EditSession::GetSession();
	//MakeGlobalPath(edit->patrolPath);
}
ActorParams *SwingLauncherParams::Copy()
{
	SwingLauncherParams *copy = new SwingLauncherParams(*this);
	copy->DeepCopyPathLines();
	return copy;
}

void SwingLauncherParams::OnCreate()
{
	EditSession *edit = EditSession::GetSession();
	edit->SetDirectionButton(this);
}

void SwingLauncherParams::DrawWhileSettingPath(sf::Vector2i mousePos, sf::RenderTarget *target )
{
	Vector2f diff = Vector2f(mousePos) - posInfo.GetPositionF();
	float len = length(diff);
	sf::CircleShape cs;
	cs.setFillColor(Color::Transparent);
	cs.setRadius(len);
	cs.setOutlineColor(Color::Red);
	cs.setOutlineThickness(10);
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
	Vector2f circleCenter = posInfo.GetPositionF() + diff;
	cs.setPosition(circleCenter);
	target->draw(cs);
}

void SwingLauncherParams::Draw(sf::RenderTarget *target)
{
	int localPathSize = localPath.size();

	if (localPathSize > 0)
	{
		VertexArray &li = *lines;

		Vector2f fPos = GetFloatPos();
		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position += fPos;
		}


		target->draw(li);

		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= fPos;
		}

		Vector2f diff = Vector2f(localPath[0]);
		float len = length(diff);
		sf::CircleShape cs;
		cs.setFillColor(Color::Transparent);
		cs.setRadius(len);
		cs.setOutlineColor(Color::Red);
		cs.setOutlineThickness(10);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		Vector2f circleCenter = posInfo.GetPositionF() + diff;
		cs.setPosition(circleCenter);
		target->draw(cs);
	}

	

	ActorParams::Draw(target);
}