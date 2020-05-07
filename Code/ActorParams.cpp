//#include "EditSession.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Physics.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "Enemy_Shard.h"
#include "ActorParams.h"
#include "EditSession.h"
#include "EditorRail.h"
#include "Action.h"

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

sf::Font *PoiParams::font = NULL;





//remnove the postype thing. we have 2 bools for that already
PlayerParams::PlayerParams(ActorType *at, sf::Vector2i pos )
	:ActorParams(at)
{
	PlaceAerial(pos);
}

PlayerParams::PlayerParams(ActorType *at, ifstream &is )
	:ActorParams(at)
{
	LoadAerial(is);
}

bool PlayerParams::CanApply()
{
	EditSession *session = EditSession::GetSession();
	Vector2i intPos = GetIntPos();
	sf::IntRect me(intPos.x - image.getLocalBounds().width / 2, intPos.y - image.getLocalBounds().height / 2,
		image.getLocalBounds().width, image.getLocalBounds().height );
	for( list<PolyPtr>::iterator it = session->polygons.begin(); it != session->polygons.end(); ++it )
	{
		if( (*it)->Intersects( me ) )
		{
			session->CreateError(ERR_PLAYER_INTERSECTS_POLY);
			return false;
		}
	}
	return true;
}

void PlayerParams::Deactivate()
{
	//nothing
}

void PlayerParams::Activate()
{
	//nothing
}

ActorParams *PlayerParams::Copy()
{
	assert( false );
	return NULL;
}


PoiParams::PoiParams(ActorType *at, int level)
	:ActorParams(at)
{
	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setFillColor( Color::White );
	
	name = "----";


	PlaceAerial(Vector2i(0, 0));
	//PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

PoiParams::PoiParams(ActorType *at,
	std::ifstream &is)
	:ActorParams( at )
{
	EditSession *edit = EditSession::GetSession();

	int air = 0;
	int ground = 1;

	int posType;
	is >> posType;


	
	Vector2i pos;

	if (posType == air )
	{
		LoadAerial(is);

		string pname;
		is >> pname;

		nameText.setFont(*font);
		nameText.setCharacterSize(18);
		nameText.setFillColor(Color::White);
		

		name = pname;
	}
	else if (posType == ground)
	{
		LoadGrounded(is);

		string pname;
		is >> pname;

		nameText.setFont(*font);
		nameText.setCharacterSize(18);
		nameText.setFillColor(Color::White);

		name = pname;
	}
	else
	{
		assert(0);
	}
}

ActorParams *PoiParams::Copy()
{
	PoiParams *copy = new PoiParams( *this );
	return copy;
}

void PoiParams::WriteParamFile( std::ofstream &of )
{
	of << name << endl;
}

void PoiParams::SetParams()
{
	Panel *p = type->panel;

	name = p->textBoxes["name"]->text.getString().toAnsiString();

	nameText.setString( name );
}

void PoiParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( name );
}

void PoiParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	nameText.setString( name );
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2, 
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2 );
	Vector2f fPos = GetFloatPos();
	nameText.setPosition(fPos.x, fPos.y - 40 );

	target->draw( nameText );
}

KeyParams::KeyParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0, 0));
	
	numKeys = 3;
	zoneType = 0;
}

KeyParams::KeyParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	is >> numKeys;
	is >> zoneType;
}

void KeyParams::WriteParamFile( std::ofstream &of )
{
	of << numKeys << endl;
	of << zoneType << endl;
}

void KeyParams::SetParams()
{
	Panel *p = type->panel;

	hasMonitor = false;//p->checkBoxes["monitor"]->checked;

	stringstream ss;
	string numkeysString = p->textBoxes["numkeys"]->text.getString().toAnsiString();
	string zoneTypeString =p->textBoxes["zonetype"]->text.getString().toAnsiString();

	ss << numkeysString;

	int numK;
	ss >> numK;

	if( !ss.fail() )
	{
		numKeys = numK;
	}

	ss.clear();

	ss << zoneTypeString;

	int zt;
	ss >> zt;

	if (!ss.fail())
	{
		zoneType = zt;
	}
	
}

void KeyParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["numkeys"]->text.setString( boost::lexical_cast<string>( numKeys ) );

	p->textBoxes["zonetype"]->text.setString(boost::lexical_cast<string>(zoneType));
}

ActorParams *KeyParams::Copy()
{
	KeyParams *copy = new KeyParams( *this );
	return copy;
}

NexusParams::NexusParams(ActorType *at, int level)
	:ActorParams(at), nexusIndex( 0 )
{
	PlaceAerial(Vector2i(0, 0));
	//PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

NexusParams::NexusParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	is >> nexusIndex;
}

void NexusParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["nexusindex"]->text.setString( boost::lexical_cast<string>( nexusIndex ) );
}

void NexusParams::SetParams()
{
	Panel *p = type->panel;

	
	int index;

	stringstream ss;
	string s = p->textBoxes["nexusindex"]->text.getString().toAnsiString();
	ss << s;


	ss >> index;

	if( !ss.fail() )
	{
		nexusIndex = index;
	}
}

void NexusParams::WriteParamFile( ofstream &of )
{
	of << nexusIndex << endl;
}

ActorParams *NexusParams::Copy()
{
	NexusParams *copy = new NexusParams( *this );
	return copy;
}

GroundTriggerParams::GroundTriggerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	LoadBool(is, facingRight);

	is >> typeStr;
}

GroundTriggerParams::GroundTriggerParams(ActorType *at, int level)
	:ActorParams(at)
{
	//PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
	PlaceAerial(Vector2i(0, 0));
	typeStr = "NONE";
	facingRight = true;

	
}

void GroundTriggerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["triggertype"]->text.setString(typeStr);
	p->checkBoxes["facingright"]->checked = facingRight;
	
}

void GroundTriggerParams::SetParams()
{
	Panel *p = type->panel;

	string s = p->textBoxes["triggertype"]->text.getString().toAnsiString();

	typeStr = s;

	bool right = p->checkBoxes["facingright"]->checked;
}

void GroundTriggerParams::WriteParamFile(ofstream &of)
{
	WriteBool(of, facingRight);
	of << typeStr << endl;
}

ActorParams *GroundTriggerParams::Copy()
{
	GroundTriggerParams *copy = new GroundTriggerParams(*this);
	return copy;
}

ShipPickupParams::ShipPickupParams(ActorType *at, int level)
	:ActorParams(at), facingRight( true )
{
	PlaceAerial(Vector2i(0, 0));
	//PlaceGrounded( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

ShipPickupParams::ShipPickupParams(ActorType *at, ifstream &is )
	:ActorParams(at)
{
	LoadGrounded(is);
	LoadBool(is, facingRight);
}

void ShipPickupParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->checkBoxes["facingright"]->checked = facingRight;
	//p->textBoxes["facing"]->text.setString( boost::lexical_cast<string>( nexusIndex ) );
}

void ShipPickupParams::SetParams()
{
	Panel *p = type->panel;

	
	int index;


	bool right = p->checkBoxes["facingright"]->checked;

	facingRight = right;
}

void ShipPickupParams::WriteParamFile( ofstream &of )
{
	WriteBool(of, facingRight);
}

ActorParams *ShipPickupParams::Copy()
{
	ShipPickupParams *copy = new ShipPickupParams( *this );
	return copy;
}

ShardParams::ShardParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0,0));
	SetShard(0, 0, 0);
}

void ShardParams::SetShard(int w, int li)
{
	SetShard(w, li % 11, li / 11);
}

void ShardParams::SetShard(int w, int realX, int realY)
{
	world = w;
	sX = realX;
	sY = realY;
	EditSession *session = EditSession::GetSession();
	localIndex = realX + realY * 11;
	Tileset *ts = session->ts_shards[world];
	image.setTexture(*ts->texture);
	image.setTextureRect(ts->GetSubRect(localIndex));
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);

	Panel *p = type->panel;
	GridSelector *gs = p->gridSelectors["shardselector"];
	gs->selectedX = sX;
	gs->selectedY = sY + world * 2;

	shardStr = gs->names[gs->selectedX][gs->selectedY];
}

int ShardParams::GetTotalIndex()
{
	return world * 22 + localIndex;
}

ShardParams::ShardParams(ActorType *at,ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	int w;
	is >> w;

	int li;
	is >> li;

	SetShard(w, li);
}

void ShardParams::SetShardFromStr()
{
	/*ShardType st = Shard::GetShardType(shardStr);
	int sti = st;
	int rem = sti % 21;

	SetShard(sti / 21, rem % 3, rem / 3);*/
}

void ShardParams::WriteParamFile( std::ofstream &of )
{
	of << world << " " << localIndex << endl;
}

void ShardParams::SetParams()
{
	Panel *p = type->panel;

	shardStr = p->labels["shardtype"]->getString();
}

void ShardParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->labels["shardtype"]->setString( shardStr );

	GridSelector *gs = p->gridSelectors["shardselector"];
	gs->selectedX = sX;
	gs->selectedY = sY + world * 2;
}

ActorParams *ShardParams::Copy()
{
	ShardParams *copy = new ShardParams( *this );
	return copy;
}

BlockerParams::BlockerParams(ActorType *at,ifstream &is)
	:ActorParams(at)
{
	lines = NULL;
	LoadAerial(is);
	
	LoadGlobalPath(is);

	int ibType;
	is >> ibType;
	bType = ibType;

	LoadBool(is, armored);

	is >> spacing;

	LoadEnemyLevel(is);
}

BlockerParams::BlockerParams(ActorType *at, int level)
	:ActorParams(at)
{
	lines = NULL;
	PlaceAerial(Vector2i(0,0));
	armored = false;

	bType = 0;

	spacing = 0;
}

void BlockerParams::SetParams()
{
	Panel *p = type->panel;

	armored = p->checkBoxes["armored"]->checked;

	
	stringstream ss;

	string typeStr = p->textBoxes["btype"]->text.getString().toAnsiString();

	int lev;
	string levelStr = p->textBoxes["level"]->text.getString().toAnsiString();
	ss << levelStr;

	ss >> lev;

	if (!ss.fail() && lev > 0 && lev <= type->info.numLevels)
	{
		enemyLevel = lev;
	}

	
	ss << typeStr;

	int t_type;
	ss >> t_type;

	if (!ss.fail())
	{
		bType = t_type;
	}

	hasMonitor = false;

	string spacingStr = p->textBoxes["spacing"]->text.getString().toAnsiString();

	
	ss << spacingStr;

	int t_spacing;
	ss >> t_spacing;

	if (!ss.fail())
	{
		spacing = t_spacing;
	}

	myEnemy->UpdateFromParams(this, 0);
	//hasMonitor = p->checkBoxes["monitor"]->checked;
	//try
	//{
	//	speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
	//}
	//catch(boost::bad_lexical_cast &)
	//{
	//	//error
	//}
}

void BlockerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	p->textBoxes["btype"]->text.setString(boost::lexical_cast<string>(bType));
	p->checkBoxes["armored"]->checked = armored;
	p->textBoxes["spacing"]->text.setString(boost::lexical_cast<string>(spacing));

	p->textBoxes["level"]->text.setString(boost::lexical_cast<string>(enemyLevel));

	EditSession *edit = EditSession::GetSession();
	MakeGlobalPath(edit->patrolPath);
	//p->checkBoxes["monitor"]->checked = hasMonitor;
}

void BlockerParams::Draw(sf::RenderTarget *target)
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



		//if (loop)
		//{

		//	//draw the line between the first and last
		//	sf::Vertex vertices[2] =
		//	{
		//		sf::Vertex(li[localPathSize].position, Color::Magenta),
		//		sf::Vertex(li[0].position, Color::White)
		//	};

		//	target->draw(vertices, 2, sf::Lines);
		//}


		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= fPos;
		}
	}

	ActorParams::Draw(target);
}

void BlockerParams::WriteParamFile(ofstream &of)
{
	WritePath(of);

	//WriteLoop(of);

	of << bType << "\n";

	WriteBool(of, armored);
	
	of << spacing << endl;

	WriteLevel(of);
}

ActorParams *BlockerParams::Copy()
{
	BlockerParams *bp = new BlockerParams(*this);
	return bp;
}


AirTriggerParams::AirTriggerParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0,0));
	triggerRect.setFillColor(Color(200, 200, 200, 150));

	rectWidth = 50;
	rectHeight = 50;
	SetRect(rectWidth, rectHeight, GetIntPos());

	trigType = "none";//"..no.shard..";
}

AirTriggerParams::AirTriggerParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> trigType;

	is >> rectWidth;

	is >> rectHeight;

	triggerRect.setFillColor(Color(200, 200, 200, 150));

	SetRect(rectWidth, rectHeight, GetIntPos());
}

void AirTriggerParams::SetRect(int width, int height, Vector2i &center)
{
	triggerRect.setSize(Vector2f(width, height));
	triggerRect.setOrigin(triggerRect.getLocalBounds().width / 2,
		triggerRect.getLocalBounds().height / 2);
	SetPosition(center);
	Vector2f fPos = GetFloatPos();
	triggerRect.setPosition(fPos);
	image.setPosition(fPos);
	SetBoundingQuad();

	rectWidth = width;
	rectHeight = height;
}

void AirTriggerParams::WriteParamFile(std::ofstream &of)
{
	of << trigType << endl;
	of << rectWidth << endl;
	of << rectHeight << endl;
}

void AirTriggerParams::SetParams()
{
	Panel *p = type->panel;

	trigType = p->textBoxes["triggertype"]->text.getString();
}

void AirTriggerParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["triggertype"]->text.setString(trigType);
}

ActorParams *AirTriggerParams::Copy()
{
	AirTriggerParams *copy = new AirTriggerParams(*this);
	return copy;
}

void AirTriggerParams::Draw(RenderTarget *target)
{
	ActorParams::Draw(target);

	nameText.setString(trigType);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);

	Vector2f fPos = GetFloatPos();
	nameText.setPosition(fPos.x, fPos.y - 40);

	target->draw(nameText);

	triggerRect.setPosition(fPos);
	target->draw(triggerRect);
	
}

FlowerPodParams::FlowerPodParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	is >> typeStr;
}

FlowerPodParams::FlowerPodParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0, 0));
	//PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);

	typeStr = "NONE";
	facingRight = true;
}

void FlowerPodParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString("test");
	if (group != NULL)
		p->textBoxes["group"]->text.setString(group->name);
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["podtype"]->text.setString(typeStr);
}

void FlowerPodParams::SetParams()
{
	Panel *p = type->panel;

	string s = p->textBoxes["podtype"]->text.getString().toAnsiString();

	typeStr = s;

	//bool right = p->checkBoxes["facingright"]->checked;
}

void FlowerPodParams::WriteParamFile(ofstream &of)
{
	//of << (int)facingRight << endl;
	of << typeStr << endl;
}

ActorParams *FlowerPodParams::Copy()
{
	FlowerPodParams *copy = new FlowerPodParams(*this);
	return copy;
}

template<typename X> ActorParams *MakeParams(
	ActorType *type)
//PolyPtrtp, int edgeIndex,
//double quant, sf::Vector2i worldPos)
{
	EditSession *edit = EditSession::GetSession();
	if (type->canBeGrounded)
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			return new X(edit->enemyEdgePolygon,
				edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
		}
	}
	else if( type->canBeAerial )
	{
		return new X(sf::Vector2i(edit->worldPos));
	}

	return NULL;
}

BasicGroundEnemyParams::BasicGroundEnemyParams(ActorType *at, int level)
	:ActorParams(at)
{
	enemyLevel = level;
	PlaceAerial(Vector2i(0, 0));
	//PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

BasicGroundEnemyParams::BasicGroundEnemyParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadGrounded(is);

	ParamsInfo &pi = at->info;

	if (pi.writeMonitor)
	{
		LoadMonitor(is);
	}

	if (pi.writeLevel)
	{
		LoadEnemyLevel(is);
	}
}

void BasicGroundEnemyParams::WriteParamFile(std::ofstream &of)
{
	ParamsInfo &pi = type->info;

	if (pi.writeMonitor)
	{
		WriteMonitor(of);
	}

	if (pi.writeLevel)
	{
		WriteLevel(of);
	}

	WriteSpecialParams(of);
}

ActorParams *BasicGroundEnemyParams::Copy()
{
	BasicGroundEnemyParams *copy = new BasicGroundEnemyParams(*this);
	return copy;
}

BasicRailEnemyParams::BasicRailEnemyParams(ActorType *at, int level)
	:ActorParams(at)
{
	enemyLevel = level;
	PlaceAerial(Vector2i(0, 0));
	//PlaceRailed(p_rail, p_edgeIndex, p_edgeQuantity);
}

BasicRailEnemyParams::BasicRailEnemyParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadRailed(is);

	ParamsInfo &pi = at->info;

	if (pi.writeMonitor)
	{
		LoadMonitor(is);
	}

	if (pi.writeLevel)
	{
		LoadEnemyLevel(is);
	}
}

void BasicRailEnemyParams::WriteParamFile(std::ofstream &of)
{
	ParamsInfo &pi = type->info;

	if (pi.writeMonitor)
	{
		WriteMonitor(of);
	}

	if (pi.writeLevel)
	{
		WriteLevel(of);
	}

	WriteSpecialParams(of);
}

ActorParams *BasicRailEnemyParams::Copy()
{
	BasicRailEnemyParams *copy = new BasicRailEnemyParams(*this);
	return copy;
}

BasicAirEnemyParams::BasicAirEnemyParams(ActorType *at, int level)
	:ActorParams(at)
{
	enemyLevel = level;
	PlaceAerial(Vector2i(0,0));
}

BasicAirEnemyParams::BasicAirEnemyParams(ActorType *at, ifstream &is)
	: ActorParams(at)
{
	LoadAerial(is);

	ParamsInfo &pi = at->info;
	if (pi.writeMonitor)
	{
		LoadMonitor(is);
	}
	
	if (pi.writePath)
	{
		LoadGlobalPath(is);
	}

	if (pi.writeLoop)
	{
		LoadBool(is, loop);
	}

	if (pi.writeLevel)
	{
		LoadEnemyLevel(is);
	}
}

void BasicAirEnemyParams::WriteParamFile(std::ofstream &of)
{
	ParamsInfo &pi = type->info;
	if (pi.writeMonitor)
	{
		WriteMonitor(of);
	}

	if (pi.writePath)
	{
		WritePath(of);
	}

	if (pi.writeLoop)
	{
		WriteLoop(of);
	}

	if (pi.writeLevel)
	{
		WriteLevel(of);
	}

	WriteSpecialParams(of);
}

void BasicAirEnemyParams::SetPanelInfo()
{
	//SetBasicPanelInfo();

	Panel *p = type->panel;

	ParamsInfo &pi = type->info;
	if (pi.writeLevel)
	{
		p->textBoxes["level"]->text.setString(boost::lexical_cast<string>(enemyLevel));
	}

	if (pi.writeMonitor)
	{
		p->checkBoxes["monitor"]->checked = hasMonitor;
	}
	
	if (pi.writePath)
	{
		EditSession *edit = EditSession::GetSession();
		MakeGlobalPath(edit->patrolPath);
	}

	if (pi.writeLoop)
	{
		p->checkBoxes["loop"]->checked = loop;
	}

	SetSpecialPanelInfo();
}

void BasicAirEnemyParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;

	ParamsInfo &pi = type->info;

	if (pi.writeLevel)
	{
		int level;
		string s = p->textBoxes["level"]->text.getString().toAnsiString();
		ss << s;

		ss >> level;

		if (!ss.fail() && level > 0 && level <= pi.numLevels)
		{
			enemyLevel = level;
		}
	}
	
	if (pi.writeMonitor)
	{
		hasMonitor = p->checkBoxes["monitor"]->checked;
	}

	SetSpecialParams();
}

ActorParams *BasicAirEnemyParams::Copy()
{
	BasicAirEnemyParams *copy = new BasicAirEnemyParams(*this);
	return copy;
}

void BasicAirEnemyParams::Draw(sf::RenderTarget *target)
{
	if (type->info.writePath)
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
				li[i].position -= fPos;
			}
		}
	}

	ActorParams::Draw(target);
}

JugglerParams::JugglerParams(ActorType *at, int level)
	:BasicAirEnemyParams(at, level )
{
	//enemyLevel = level;
	//PlaceAerial(pos);

	numJuggles = 100;
}

JugglerParams::JugglerParams(ActorType *at, ifstream &is)
	: BasicAirEnemyParams(at, is)
{
	is >> numJuggles;
}

void JugglerParams::WriteSpecialParams(std::ofstream &of)
{
	of << numJuggles << endl;
}

void JugglerParams::SetSpecialPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["numjuggles"]->text.setString(boost::lexical_cast<string>(numJuggles));
}

void JugglerParams::SetSpecialParams()
{
	Panel *p = type->panel;

	stringstream ss;

	string numJuggleStr = p->textBoxes["numjuggles"]->text.getString().toAnsiString();
	ss << numJuggleStr;
	
	int nJuggles;
	ss >> nJuggles;

	if (!ss.fail())
	{
		numJuggles = nJuggles;
	}
}

ActorParams *JugglerParams::Copy()
{
	JugglerParams *copy = new JugglerParams(*this);
	return copy;
}

GroundedJugglerParams::GroundedJugglerParams(ActorType *at, int level)
	:BasicGroundEnemyParams(at, level)
{
	//enemyLevel = level;

	PlaceAerial(Vector2i(0,0));

	numJuggles = 100;
}

GroundedJugglerParams::GroundedJugglerParams(ActorType *at, ifstream &is)
	: BasicGroundEnemyParams(at, is)
{
	is >> numJuggles;
}

void GroundedJugglerParams::WriteSpecialParams(std::ofstream &of)
{
	of << numJuggles << endl;
}

void GroundedJugglerParams::SetSpecialPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["numjuggles"]->text.setString(boost::lexical_cast<string>(numJuggles));
}

void GroundedJugglerParams::SetSpecialParams()
{
	Panel *p = type->panel;

	stringstream ss;

	string numJuggleStr = p->textBoxes["numjuggles"]->text.getString().toAnsiString();
	ss << numJuggleStr;

	int nJuggles;
	ss >> nJuggles;

	if (!ss.fail())
	{
		numJuggles = nJuggles;
	}
}

ActorParams *GroundedJugglerParams::Copy()
{
	GroundedJugglerParams *copy = new GroundedJugglerParams(*this);
	return copy;
}

XBarrierParams::XBarrierParams(ActorType *at, int level)
	:ActorParams(at)
{
	Init();
	PlaceAerial(Vector2i(0,0));
	name = "----";
	hasEdge = false;
}

XBarrierParams::XBarrierParams(ActorType *at,
	ifstream &is)
	: ActorParams(at)
{
	Init();
	LoadAerial(is);

	is >> name;

	LoadBool(is, hasEdge);
}

void XBarrierParams::Init()
{
	EditSession *session = EditSession::GetSession();
	nameText.setFont(session->arial);
	nameText.setCharacterSize(40);
	nameText.setFillColor(Color::White);

	line[0].color = Color::Red;
	line[1].color = Color::Red;
}

void XBarrierParams::WriteParamFile(std::ofstream &of)
{
	of << name << endl;

	WriteBool(of, hasEdge);
}

void XBarrierParams::SetParams()
{
	Panel *p = type->panel;
	name = p->textBoxes["name"]->text.getString().toAnsiString();

	hasEdge = p->checkBoxes["hasedge"]->checked;
}

void XBarrierParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString(name);

	p->checkBoxes["hasedge"]->checked = hasEdge;
}

ActorParams *XBarrierParams::Copy()
{
	XBarrierParams *copy = new XBarrierParams(*this);
	return copy;
}

void XBarrierParams::Draw(sf::RenderTarget *target)
{
	ActorParams::Draw(target);

	Vector2f fPos = GetFloatPos();

	line[0].position.x = fPos.x;
	line[1].position.x = fPos.x;
	line[0].position.y = fPos.y - 1000;
	line[1].position.y = fPos.y + 1000;

	target->draw(line, 2, sf::Lines);

	nameText.setString(name);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);
	nameText.setPosition(fPos.x, fPos.y - 100);

	target->draw(nameText);
}

const float CameraShotParams::CAMWIDTH = 960.f;
const float CameraShotParams::CAMHEIGHT = 540.f;

CameraShotParams::CameraShotParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0,0));
	Init();

	SetZoom(1);

	camName = "----";
}

CameraShotParams::CameraShotParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	is >> camName;

	float z;
	is >> z;

	Init();

	SetZoom(z);
}

void CameraShotParams::Init()
{
	camRect.setFillColor(Color(255, 255,255, 0));
	camRect.setOutlineColor(Color(255, 255, 255));
	camRect.setOutlineThickness(5);

	EditSession *session = EditSession::GetSession();
	nameText.setFont(session->arial);
	nameText.setCharacterSize(40);
	nameText.setFillColor(Color::White);

	zoomText.setFont(session->arial);
	zoomText.setCharacterSize(40);
	zoomText.setFillColor(Color::White);
}

void CameraShotParams::SetZoom(float z)
{
	zoom = z;
	int width = CAMWIDTH * zoom;
	int height = CAMHEIGHT * zoom;

	camRect.setSize(Vector2f(width, height));
	camRect.setOrigin(camRect.getLocalBounds().width / 2,
		camRect.getLocalBounds().height / 2);

	Vector2f fPos = GetFloatPos();
	camRect.setPosition(fPos);
	image.setPosition(fPos);
	SetBoundingQuad();

	zoomText.setString( "x" + to_string(zoom));
}

void CameraShotParams::SetZoom(sf::Vector2i &testPoint)
{

	Vector2f fPos = GetFloatPos();
	float xDist = abs(testPoint.x - fPos.x);
	float yDist = abs(testPoint.y - fPos.y);
	float xProp = xDist / CAMWIDTH;
	float yProp = yDist / CAMHEIGHT;

	float maxZoom = max(xProp, yProp) * 2;
	SetZoom(maxZoom);
}

void CameraShotParams::WriteParamFile(std::ofstream &of)
{
	of << camName << endl;
	of << zoom << endl;
}

void CameraShotParams::SetParams()
{
	Panel *p = type->panel;

	camName = p->textBoxes["name"]->text.getString();
}

void CameraShotParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString(camName);
	//if (group != NULL)
	//	p->textBoxes["group"]->text.setString(group->name);
	//p->textBoxes["name"]->text.setString(camName);
	//p->textBoxes["zoom"]->text.setString(to_string(zoom));
}

ActorParams *CameraShotParams::Copy()
{
	CameraShotParams *copy = new CameraShotParams(*this);
	return copy;
}

void CameraShotParams::Draw(RenderTarget *target)
{
	Vector2f fPos = GetFloatPos();
	camRect.setPosition(fPos);
	target->draw(camRect);

	ActorParams::Draw(target);

	nameText.setString(camName);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);
	nameText.setPosition(fPos.x, fPos.y - 100);

	target->draw(nameText);

	zoomText.setOrigin(zoomText.getLocalBounds().left + zoomText.getLocalBounds().width / 2,
		zoomText.getLocalBounds().top + zoomText.getLocalBounds().height / 2);
	zoomText.setPosition(fPos.x, fPos.y - 200);

	target->draw(zoomText);
}

ExtraSceneParams::ExtraSceneParams(ActorType *at, int level)
	:ActorParams(at)
{
	Init();
	PlaceAerial(Vector2i(0,0));
	name = "----";
	extraSceneType = 0;
}

ExtraSceneParams::ExtraSceneParams(ActorType *at,
	ifstream &is)
	: ActorParams(at)
{
	Init();
	LoadAerial(is);

	is >> name;

	is >> extraSceneType;
}

void ExtraSceneParams::Init()
{
	EditSession *session = EditSession::GetSession();
	nameText.setFont(session->arial);
	nameText.setCharacterSize(40);
	nameText.setFillColor(Color::White);
}

void ExtraSceneParams::WriteParamFile(std::ofstream &of)
{
	of << name << endl;
	of << extraSceneType << endl;
}

void ExtraSceneParams::SetParams()
{
	Panel *p = type->panel;
	name = p->textBoxes["name"]->text.getString().toAnsiString();

	stringstream ss;
	string sceneTypeStr = p->textBoxes["scenetype"]->text.getString().toAnsiString();
	ss << sceneTypeStr;

	int sType;
	ss >> sType;

	if (!ss.fail())
	{
		extraSceneType = sType;
	}
}

void ExtraSceneParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString(name);
	p->textBoxes["scenetype"]->text.setString(to_string(extraSceneType));
}

ActorParams *ExtraSceneParams::Copy()
{
	ExtraSceneParams *copy = new ExtraSceneParams(*this);
	return copy;
}

void ExtraSceneParams::Draw(sf::RenderTarget *target)
{
	ActorParams::Draw(target);

	nameText.setString(name);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);

	Vector2f fPos = GetFloatPos();
	nameText.setPosition(fPos.x, fPos.y - 100);

	target->draw(nameText);
}