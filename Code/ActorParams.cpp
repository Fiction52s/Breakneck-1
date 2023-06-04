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
#include "ShardMenu.h"
#include "LogMenu.h"
#include "PlayerSkinShader.h"

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
PlayerParams::PlayerParams(ActorType *at, sf::Vector2i pos, int p_playerIndex)
	:ActorParams(at)
{
	playerIndex = p_playerIndex;

	/*sf::Text *t = new sf::Text(text, arial, characterHeight);
	t->setPosition(autoStart.x + labelPos.x, autoStart.y + labelPos.y);
	t->setFillColor(Color::Black);
	auto lb = t->getLocalBounds();
	t->setOrigin(lb.left, lb.top);

	auto bounds = t->getLocalBounds();

	AddAutoSpaceX(bounds.width + labelPos.x);
	AddAutoSpaceY(bounds.height + labelPos.y);*/

	EditSession *edit = EditSession::GetSession();
	numberText.setFont(edit->arial);
	numberText.setCharacterSize(40);

	Color c = Color::White;
	switch (playerIndex)
	{
	case 0:
		c = Color::Green;
		break;
	case 1:
		c = Color::Red;
		break;
	case 2:
		c = Color::Cyan;
		break;
	case 3:
		c = Color::Magenta;
		break;
	}

	numberText.setFillColor(c);
	numberText.setOutlineColor(Color::Black);
	numberText.setOutlineThickness(2);
	numberText.setString(to_string(playerIndex+1));
	auto &bounds = numberText.getLocalBounds();
	numberText.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height);

	skinShader = new PlayerSkinShader("player");
	skinShader->SetSkin(playerIndex);
	PlaceAerial(pos);
}

//PlayerParams::PlayerParams(ActorType *at, ifstream &is)
//	: ActorParams(at)
//{
//	skinShader = new PlayerSkinShader("player");
//	LoadAerial(is);
//}

PlayerParams::~PlayerParams()
{
	delete skinShader;
}

bool PlayerParams::CanApply()
{
	EditSession *edit = EditSession::GetSession();
	Vector2i intPos = GetIntPos();
	sf::IntRect me(intPos.x - image.getLocalBounds().width / 2, intPos.y - image.getLocalBounds().height / 2,
		image.getLocalBounds().width, image.getLocalBounds().height);
	for (list<PolyPtr>::iterator it = edit->polygons.begin(); it != edit->polygons.end(); ++it)
	{
		if ((*it)->Intersects(me))
		{
			edit->CreateError(ERR_PLAYER_INTERSECTS_POLY);
			return false;
		}
	}
	return true;
}

//void PlayerParams::Deactivate()
//{
//	//nothing
//}
//
//void PlayerParams::Activate()
//{
//	//nothing
//}

void PlayerParams::SetParams()
{

}


void PlayerParams::SetPanelInfo()
{

}

ActorParams *PlayerParams::Copy()
{
	assert(false);
	return NULL;
}

void PlayerParams::Draw(sf::RenderTarget *target)
{
	Vector2f viewCenter = target->getView().getCenter();
	Vector2f viewSize = target->getView().getSize();
	//if (image.getGlobalBounds().intersects(FloatRect(viewCenter.x - viewSize.x / 2, viewCenter.y - viewSize.y / 2,
	//	viewSize.x, viewSize.y)))
	if (GetAABB().intersects(FloatRect(viewCenter.x - viewSize.x / 2, viewCenter.y - viewSize.y / 2,
		viewSize.x, viewSize.y)))
	{
		numberText.setPosition(Vector2f(GetPosition()) + Vector2f(0, -40));


		target->draw(image, &(skinShader->pShader));
		target->draw(numberText);

		/*DrawMonitor(target);

		if (myEnemy != NULL)
			myEnemy->Draw(target);
		else
		{
			target->draw(image);
		}*/

		DrawBoundary(target);

		DrawQuad(target);

	}
}


PoiParams::PoiParams(ActorType *at, int level)
	:ActorParams(at)
{
	nameText.setFont(*font);
	nameText.setCharacterSize(18);
	nameText.setFillColor(Color::White);

	name = type->GetSelectedSpecialDropStr();


	PlaceAerial(Vector2i(0, 0));
	//PlaceGrounded(p_edgePolygon, p_edgeIndex, p_edgeQuantity);
}

PoiParams::PoiParams(ActorType *at,
	std::ifstream &is)
	:ActorParams(at)
{
	Session *sess = Session::GetSession();

	int air = 0;
	int ground = 1;

	int posType;
	is >> posType;



	Vector2i pos;

	if (posType == air)
	{
		LoadAerial(is);

		string pname;
		is >> pname;

		nameText.setFont(*font);
		nameText.setCharacterSize(18);
		nameText.setFillColor(Color::White);


		name = pname;
		if ( sess->IsSessTypeEdit() && type->GetSpecialOptionsIndex(name) < 0)
		{
			assert(0);
		}
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
		if ( sess->IsSessTypeEdit() && type->GetSpecialOptionsIndex(name) < 0)
		{
			assert(0);
		}
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

	name = type->GetSelectedSpecialDropStr();

	nameText.setString( name );
}

void PoiParams::SetPanelInfo()
{
	Panel *p = type->panel;

	type->SetSpecialDropIndex(type->GetSpecialOptionsIndex(name));
	//p->textBoxes["name"]->text.setString( name );
}

void PoiParams::Draw( sf::RenderTarget *target )
{
	EditSession *edit = EditSession::GetSession();
	if (edit != NULL)
	{
		if (!edit->editModeUI->IsLayerShowing(LAYER_SEQUENCE)
			&& GetTypeName() == "poi" )
		{
			return;
		}
	}

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

	p->textBoxes["numkeys"]->SetString( boost::lexical_cast<string>( numKeys ) );

	p->textBoxes["zonetype"]->SetString(boost::lexical_cast<string>(zoneType));
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

	p->textBoxes["nexusindex"]->SetString( boost::lexical_cast<string>( nexusIndex ) );
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
	p->textBoxes["name"]->SetString("test");
	//p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["triggertype"]->SetString(typeStr);
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

TutorialObjectParams::TutorialObjectParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	int numLines = 0;
	is >> numLines;
	is.get();

	string readStr;
	stringstream ss;
	for (int i = 0; i < numLines; ++i)
	{
		getline(is, readStr);
		ss << readStr;

		if (i < numLines - 1)
		{
			ss << "\n";
		}
	}

	tutStr = ss.str();
}

TutorialObjectParams::TutorialObjectParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0, 0));
	tutStr = "NONE";
}

void TutorialObjectParams::SetPanelInfo()
{
	Panel *p = type->panel;
	//p->textBoxes["name"]->text.setString("test");
	p->textBoxes["tutstr"]->SetString(tutStr);
	p->textBoxes["tutstr"]->SetCursorIndex(0);

}

void TutorialObjectParams::SetParams()
{
	Panel *p = type->panel;

	string s = p->textBoxes["tutstr"]->text.getString().toAnsiString();

	tutStr = s;
}

void TutorialObjectParams::WriteParamFile(ofstream &of)
{
	int numLines = 1;
	for (int i = 0; i < tutStr.size(); ++i)
	{
		if (tutStr.at(i) == '\n')
		{
			++numLines;
		}
	}
	of << numLines << endl;
	of << tutStr << endl;
}

ActorParams *TutorialObjectParams::Copy()
{
	TutorialObjectParams *copy = new TutorialObjectParams(*this);
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

ZonePropertiesParams::ZonePropertiesParams(ActorType *at, int level)
	:ActorParams(at), zoneType( 0 ), drainFactor( 1.f )
{
	PlaceAerial(Vector2i(0, 0));
}

ZonePropertiesParams::ZonePropertiesParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);
	is >> zoneType;
	is >> drainFactor;
}

void ZonePropertiesParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->dropdowns["zonetype"]->SetSelectedIndex(zoneType);
	p->sliders["drainfactor"]->SetCurrValueF(drainFactor);
}

void ZonePropertiesParams::SetParams()
{
	Panel *p = type->panel;

	zoneType = p->dropdowns["zonetype"]->selectedIndex;
	drainFactor = p->sliders["drainfactor"]->GetCurrValueF();
}

void ZonePropertiesParams::WriteParamFile(ofstream &of)
{
	of << zoneType << "\n";
	of << drainFactor << "\n";
}

ActorParams *ZonePropertiesParams::Copy()
{
	ZonePropertiesParams *copy = new ZonePropertiesParams(*this);
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
	shInfo.world = w;
	sX = realX;
	sY = realY;
	shInfo.localIndex = realX + realY * 11;

	if (myEnemy != NULL)
	{
		myEnemy->UpdateParamsSettings();
	}

	EditSession *edit= EditSession::GetSession();

	if (edit != NULL)
	{
		nameText.setFont(edit->arial);
		nameText.setCharacterSize(30);
		nameText.setString(edit->shardMenu->GetShardName(shInfo.world,
			shInfo.localIndex));
		nameText.setOrigin(nameText.getLocalBounds().left +
			nameText.getLocalBounds().width / 2, 0);
	}

	
	/*EditSession *session = EditSession::GetSession();
	
	Tileset *ts = session->ts_shards[world];
	image.setTexture(*ts->texture);
	image.setTextureRect(ts->GetSubRect(localIndex));
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);

	Panel *p = type->panel;
	GridSelector *gs = p->gridSelectors["shardselector"];
	gs->selectedX = sX;
	gs->selectedY = sY + world * 2;

	shardStr = gs->names[gs->selectedX][gs->selectedY];*/
}

int ShardParams::GetTotalIndex()
{
	return shInfo.world * 22 + shInfo.localIndex;
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

void ShardParams::WriteParamFile( std::ofstream &of )
{
	of << shInfo.world << " " << shInfo.localIndex << endl;
}

void ShardParams::SetParams()
{
	Panel *p = type->panel;

	//shardStr = p->labels["shardtype"]->getString();

	if (myEnemy != NULL)
	{
		myEnemy->UpdateParamsSettings();
	}
}

void ShardParams::SetPanelInfo()
{
	Panel *p = type->panel;

	//p->labels["shardtype"]->setString( shardStr );

	//GridSelector *gs = p->gridSelectors["shardselector"];
	//gs->selectedX = sX;
	//gs->selectedY = sY + world * 2;
}

void ShardParams::Draw(sf::RenderTarget *target)
{
	ActorParams::Draw(target);

	Vector2f fPos = GetFloatPos();

	sf::FloatRect fr = myEnemy->GetAABB();

	nameText.setPosition(fPos.x, fr.top - 35);//fPos.y - 40);

	target->draw(nameText);
}

ActorParams *ShardParams::Copy()
{
	ShardParams *copy = new ShardParams( *this );
	return copy;
}

LogParams::LogParams(ActorType *at, int level)
	:ActorParams(at)
{
	PlaceAerial(Vector2i(0, 0));
	SetLog(0, 0);
}

void LogParams::SetLog(int w, int li)
{
	EditSession *edit = EditSession::GetSession();

	lInfo.world = w;
	lInfo.localIndex = li;

	if (edit != NULL)
	{
		nameText.setFont(edit->arial);
		nameText.setCharacterSize(30);
		nameText.setString(edit->logMenu->GetLogName(lInfo.world, lInfo.localIndex));
		nameText.setOrigin(nameText.getLocalBounds().left +
			nameText.getLocalBounds().width / 2, 0);
	}

	if (myEnemy != NULL)
	{
		myEnemy->UpdateParamsSettings();
	}

	//SetLog(w, li % 11, li / 11);
}

void LogParams::SetLog(int w, int realX, int realY)
{
	lInfo.world = w;
	sX = realX;
	sY = realY;
	lInfo.localIndex = realX + realY * 11;

	if (myEnemy != NULL)
	{
		myEnemy->UpdateParamsSettings();
	}

	EditSession *edit = EditSession::GetSession();

	if (edit != NULL)
	{
		nameText.setFont(edit->arial);
		nameText.setCharacterSize(30);
		nameText.setString(edit->logMenu->GetLogName(lInfo.world, lInfo.localIndex));
		nameText.setOrigin(nameText.getLocalBounds().left +
			nameText.getLocalBounds().width / 2, 0);
	}


	/*EditSession *session = EditSession::GetSession();

	Tileset *ts = session->ts_shards[world];
	image.setTexture(*ts->texture);
	image.setTextureRect(ts->GetSubRect(localIndex));
	image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);

	Panel *p = type->panel;
	GridSelector *gs = p->gridSelectors["shardselector"];
	gs->selectedX = sX;
	gs->selectedY = sY + world * 2;

	shardStr = gs->names[gs->selectedX][gs->selectedY];*/
}

int LogParams::GetTotalIndex()
{
	return 0;//shInfo.world * 22 + shInfo.localIndex;
}

LogParams::LogParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	int w;
	is >> w;

	int li;
	is >> li;

	SetLog(w, li);
}

void LogParams::WriteParamFile(std::ofstream &of)
{
	of << lInfo.world << " " << lInfo.localIndex << endl;
}

void LogParams::SetParams()
{
	Panel *p = type->panel;

	//shardStr = p->labels["shardtype"]->getString();

	if (myEnemy != NULL)
	{
		myEnemy->UpdateParamsSettings();
	}
}

void LogParams::SetPanelInfo()
{
	Panel *p = type->panel;

	//p->labels["shardtype"]->setString( shardStr );

	//GridSelector *gs = p->gridSelectors["shardselector"];
	//gs->selectedX = sX;
	//gs->selectedY = sY + world * 2;
}

void LogParams::Draw(sf::RenderTarget *target)
{
	ActorParams::Draw(target);

	Vector2f fPos = GetFloatPos();

	sf::FloatRect fr = myEnemy->GetAABB();

	nameText.setPosition(fPos.x, fr.top - 35);//fPos.y - 40);

	target->draw(nameText);
}

ActorParams *LogParams::Copy()
{
	LogParams *copy = new LogParams(*this);
	return copy;
}

BlockerParams::BlockerParams(ActorType *at,ifstream &is)
	:ActorParams(at)
{
	lines = NULL;
	LoadAerial(is);
	
	//LoadGlobalPath(is);

	int ibType;
	is >> ibType;
	bType = ibType;

	LoadBool(is, fill);

	is >> spacing;

	LoadEnemyLevel(is);	
}

BlockerParams::BlockerParams(ActorType *at, int level)
	:ActorParams(at)
{
	lines = NULL;
	PlaceAerial(Vector2i(0,0));
	fill = true;

	bType = 0;

	spacing = 80;
}

void BlockerParams::OnCreate()
{
	EditSession *edit = EditSession::GetSession();
	edit->CreateChainButton(this);
}

void BlockerParams::SetParams()
{
	Panel *p = type->panel;

	fill = p->checkBoxes["fill"]->checked;

	bType = p->dropdowns["btype"]->selectedIndex;

	spacing = p->sliders["spacing"]->GetCurrValue();

	if (myEnemy != NULL)
	{
		myEnemy->UpdateParamsSettings();
	}
}

void BlockerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->dropdowns["btype"]->SetSelectedIndex(bType);
	p->checkBoxes["fill"]->checked = fill;
	p->sliders["spacing"]->SetCurrValue(spacing);

	EditSession *edit = EditSession::GetSession();
	MakeGlobalPath(edit->patrolPath);
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

		for (int i = 0; i < localPathSize + 1; ++i)
		{
			li[i].position -= fPos;
		}
	}

	ActorParams::Draw(target);
}

void BlockerParams::WriteParamFile(ofstream &of)
{
	//WritePath(of);

	//WriteLoop(of);

	of << bType << "\n";

	WriteBool(of, fill);
	
	of << spacing << endl;

	WriteLevel(of);
}

ActorParams *BlockerParams::Copy()
{
	BlockerParams *bp = new BlockerParams(*this);
	bp->lines = NULL;
	bp->myEnemy = NULL;
	bp->SetSelected(false);
	return bp;
}

FlyParams::FlyParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	lines = NULL;
	LoadAerial(is);

	//LoadGlobalPath(is);

	int ifType;
	is >> ifType;
	fType = ifType;

	is >> spacing;

	LoadEnemyLevel(is);
}

FlyParams::FlyParams(ActorType *at, int level)
	:ActorParams(at)
{
	lines = NULL;
	PlaceAerial(Vector2i(0, 0));

	fType = 0;

	spacing = 60;
}

void FlyParams::SetParams()
{
	Panel *p = type->panel;
	fill = p->checkBoxes["fill"]->checked;

	fType = p->dropdowns["ftype"]->selectedIndex;

	spacing = p->sliders["spacing"]->GetCurrValue();

	if (myEnemy != NULL)
	{
		myEnemy->UpdateParamsSettings();
	}
}

void FlyParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->dropdowns["ftype"]->SetSelectedIndex(fType);
	p->checkBoxes["fill"]->checked = fill;
	p->sliders["spacing"]->SetCurrValue(spacing);

	EditSession *edit = EditSession::GetSession();
	MakeGlobalPath(edit->patrolPath);
}

void FlyParams::Draw(sf::RenderTarget *target)
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
	}

	ActorParams::Draw(target);
}

void FlyParams::WriteParamFile(ofstream &of)
{
	//WritePath(of);

	of << fType << "\n";

	of << spacing << endl;

	WriteLevel(of);
}

ActorParams *FlyParams::Copy()
{
	FlyParams *fp = new FlyParams(*this);
	fp->lines = NULL;
	fp->myEnemy = NULL;
	fp->SetSelected(false);
	return fp;
}


void FlyParams::OnCreate()
{
	EditSession *edit = EditSession::GetSession();
	edit->CreateChainButton(this);
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

	p->textBoxes["triggertype"]->SetString(trigType);
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
	/*if (pi.writeLevel)
	{
		p->textBoxes["level"]->text.setString(boost::lexical_cast<string>(enemyLevel));
	}*/

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

	/*if (pi.writeLevel)
	{
		int level;
		string s = p->textBoxes["level"]->text.getString().toAnsiString();
		ss << s;

		ss >> level;

		if (!ss.fail() && level > 0 && level <= pi.numLevels)
		{
			enemyLevel = level;
		}
	}*/
	
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

	numJuggles = 0;
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
	type->panel->sliders["numJuggles"]->SetCurrValue(numJuggles);
}

void JugglerParams::SetSpecialParams()
{
	numJuggles = type->panel->sliders["numJuggles"]->GetCurrValue();
}

ActorParams *JugglerParams::Copy()
{
	JugglerParams *copy = new JugglerParams(*this);
	return copy;
}

GrindJugglerParams::GrindJugglerParams(ActorType *at, int level)
	:BasicAirEnemyParams(at, level)
{
	//enemyLevel = level;
	//PlaceAerial(pos);

	numKills = 0;
	clockwise = true;
}

GrindJugglerParams::GrindJugglerParams(ActorType *at, ifstream &is)
	: BasicAirEnemyParams(at, is)
{
	is >> numKills;
	int cw;
	is >> cw;
	clockwise = cw;
}

void GrindJugglerParams::WriteSpecialParams(std::ofstream &of)
{
	of << numKills << endl;
	int cw = clockwise;
	of << cw << endl;
}

void GrindJugglerParams::SetSpecialPanelInfo()
{
	type->panel->sliders["numKills"]->SetCurrValue(numKills);
	type->panel->checkBoxes["clockwise"]->checked = clockwise;
}

void GrindJugglerParams::SetSpecialParams()
{
	numKills = type->panel->sliders["numKills"]->GetCurrValue();
	clockwise = type->panel->checkBoxes["clockwise"]->checked;
}

ActorParams *GrindJugglerParams::Copy()
{
	GrindJugglerParams *copy = new GrindJugglerParams(*this);
	return copy;
}

GroundedGrindJugglerParams::GroundedGrindJugglerParams(ActorType *at, int level)
	:BasicGroundEnemyParams(at, level)
{
	//enemyLevel = level;

	PlaceAerial(Vector2i(0,0));

	numKills = 0;
	clockwise = true;
}

GroundedGrindJugglerParams::GroundedGrindJugglerParams(ActorType *at, ifstream &is)
	: BasicGroundEnemyParams(at, is)
{
	is >> numKills;
	int cw;
	is >> cw;
	clockwise = cw;
}

void GroundedGrindJugglerParams::WriteSpecialParams(std::ofstream &of)
{
	of << numKills << endl;
	int cw = clockwise;
	of << cw << endl;
}

void GroundedGrindJugglerParams::SetSpecialPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["numKills"]->SetString(boost::lexical_cast<string>(numKills));
}

void GroundedGrindJugglerParams::SetSpecialParams()
{
	Panel *p = type->panel;

	stringstream ss;

	string numKillsStr = p->textBoxes["numKills"]->text.getString().toAnsiString();
	ss << numKillsStr;

	int nKills;
	ss >> nKills;

	if (!ss.fail())
	{
		numKills = nKills;
	}
}

ActorParams *GroundedGrindJugglerParams::Copy()
{
	GroundedGrindJugglerParams *copy = new GroundedGrindJugglerParams(*this);
	return copy;
}

XBarrierParams::XBarrierParams(ActorType *at, int level)
	:ActorParams(at)
{
	Init();
	PlaceAerial(Vector2i(0,0));
	nameIndex = type->GetSelectedSpecialDropIndex();

	SetText(type->GetSelectedSpecialDropStr());

	hasEdge = false;
}

void XBarrierParams::SetText(const std::string &n)
{
	nameText.setString(n);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);
}

const std::string &XBarrierParams::GetName()
{
	if (nameIndex >= 0)
	{
		return type->GetSpecialDropStr(nameIndex);
	}
	else
	{
		return name;
	}
}

XBarrierParams::XBarrierParams(ActorType *at,
	ifstream &is)
	: ActorParams(at)
{
	Init();
	LoadAerial(is);

	string n;
	is >> n;
	
	nameIndex = type->GetSpecialOptionsIndex(n);
	
	if (nameIndex >= 0)
	{
		Init();
		SetText(n);
	}
	else
	{
		name = n;
	}

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
	of << GetName() << endl;

	WriteBool(of, hasEdge);
}

void XBarrierParams::SetParams()
{
	Panel *p = type->panel;
	nameIndex = type->GetSelectedSpecialDropIndex();
	SetText(type->GetSelectedSpecialDropStr());

	hasEdge = p->checkBoxes["hasedge"]->checked;
}

void XBarrierParams::SetPanelInfo()
{
	Panel *p = type->panel;

	type->SetSpecialDropIndex(nameIndex);

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

	
	nameText.setPosition(fPos.x, fPos.y - 100);

	target->draw(nameText);
}

GroundedWarperParams::GroundedWarperParams(ActorType *at, int level)
	:ActorParams(at)
{
	Init();
	PlaceAerial(Vector2i(0, 0));
	nameIndex = type->GetSelectedSpecialDropIndex();

	startActivated = true;
	SetText(type->GetSelectedSpecialDropStr());
}

void GroundedWarperParams::SetText(const std::string &n)
{
	nameText.setString(n);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);
}

const std::string &GroundedWarperParams::GetName()
{
	if (nameIndex >= 0)
	{
		return type->GetSpecialDropStr(nameIndex);
	}
	else
	{
		return name;
	}
}

GroundedWarperParams::GroundedWarperParams(ActorType *at,
	ifstream &is)
	: ActorParams(at)
{
	Init();
	LoadGrounded(is);

	string n;
	is >> n;

	LoadBool(is, startActivated);

	nameIndex = type->GetSpecialOptionsIndex(n);

	if (nameIndex >= 0)
	{
		Init();
		SetText(n);
	}
	else
	{
		name = n;
	}

	
}

void GroundedWarperParams::Init()
{
	EditSession *session = EditSession::GetSession();
	nameText.setFont(session->arial);
	nameText.setCharacterSize(40);
	nameText.setFillColor(Color::White);
}

void GroundedWarperParams::WriteParamFile(std::ofstream &of)
{
	of << GetName() << endl;
	WriteBool(of, startActivated);
}

void GroundedWarperParams::SetParams()
{
	Panel *p = type->panel;
	nameIndex = type->GetSelectedSpecialDropIndex();
	SetText(type->GetSelectedSpecialDropStr());

	startActivated = p->checkBoxes["startactivated"]->checked;

	
}

void GroundedWarperParams::SetPanelInfo()
{
	Panel *p = type->panel;

	type->SetSpecialDropIndex(nameIndex);

	p->checkBoxes["startactivated"]->checked = startActivated;
}

ActorParams *GroundedWarperParams::Copy()
{
	GroundedWarperParams *copy = new GroundedWarperParams(*this);
	return copy;
}

void GroundedWarperParams::Draw(sf::RenderTarget *target)
{
	ActorParams::Draw(target);

	Vector2f fPos = GetFloatPos();

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

	nameIndex = type->GetSelectedSpecialDropIndex();
	cout << "test here" << endl;
	if (nameIndex >= 0)
	{
		SetText(type->GetSelectedSpecialDropStr());
	}
	cout << "test over" << endl;
}

CameraShotParams::CameraShotParams(ActorType *at, ifstream &is)
	:ActorParams(at)
{
	LoadAerial(is);

	

	string n;
	is >> n;
	
	nameIndex = type->GetSpecialOptionsIndex(n);
	//assert(nameIndex >= 0);
	if (nameIndex >= 0)
	{
		Init();
		SetText(n);
	}
	else
	{
		name = n;
	}
	
	float z;
	is >> z;

	SetZoom(z);
}

const std::string &CameraShotParams::GetName()
{
	if (nameIndex >= 0)
	{
		return type->specialTypeOptions[nameIndex];
	}
	else
	{
		return name;
	}
}

void CameraShotParams::SetText(const std::string &n)
{
	nameText.setString(n);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);
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
	if (z < .25)
		return;
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

void CameraShotParams::SetZoom(sf::Vector2i &testPoint, bool ctrl )
{

	Vector2f fPos = GetFloatPos();
	float xDist = abs(testPoint.x - fPos.x);
	float yDist = abs(testPoint.y - fPos.y);
	float xProp = xDist / CAMWIDTH;
	float yProp = yDist / CAMHEIGHT;

	float test = .25f;

	float maxZoom = max(xProp, yProp) * 2;

	if (ctrl)
	{
		float mult = floor(maxZoom / test);
		maxZoom = mult * test;
	}

	SetZoom(maxZoom);
}

void CameraShotParams::WriteParamFile(std::ofstream &of)
{
	of << GetName() << endl;
	of << zoom << endl;
}

void CameraShotParams::SetParams()
{
	Panel *p = type->panel;

	nameIndex = type->GetSelectedSpecialDropIndex();
	SetText(type->GetSelectedSpecialDropStr());
}

void CameraShotParams::SetPanelInfo()
{
	Panel *p = type->panel;

	type->SetSpecialDropIndex(nameIndex);
}

ActorParams *CameraShotParams::Copy()
{
	CameraShotParams *copy = new CameraShotParams(*this);
	return copy;
}

void CameraShotParams::Draw(RenderTarget *target)
{
	EditSession *edit = EditSession::GetSession();
	if (edit != NULL)
	{
		if (!edit->editModeUI->IsLayerShowing(LAYER_SEQUENCE))
		{
			return;
		}
	}

	Vector2f fPos = GetFloatPos();
	camRect.setPosition(fPos);
	target->draw(camRect);

	ActorParams::Draw(target);

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
	nameIndex = type->GetSelectedSpecialDropIndex();
	SetText(type->GetSelectedSpecialDropStr());
	extraSceneType = 0;
}

ExtraSceneParams::ExtraSceneParams(ActorType *at,
	ifstream &is)
	: ActorParams(at)
{
	Init();
	LoadAerial(is);

	string n;
	is >> n;

	nameIndex = type->GetSpecialOptionsIndex(n);
	SetText(n);
	assert(nameIndex >= 0);
	

	is >> extraSceneType;
}

const std::string &ExtraSceneParams::GetName()
{
	return type->GetSpecialDropStr(nameIndex);
}

void ExtraSceneParams::SetText(const std::string &n)
{
	nameText.setString(n);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);
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
	of << GetName() << endl;
	of << extraSceneType << endl;
}

void ExtraSceneParams::SetParams()
{
	Panel *p = type->panel;
	nameIndex = type->GetSelectedSpecialDropIndex();
	SetText(type->GetSelectedSpecialDropStr());
	extraSceneType = p->dropdowns["scenetype"]->selectedIndex;
}

void ExtraSceneParams::SetPanelInfo()
{
	Panel *p = type->panel;
	type->SetSpecialDropIndex(nameIndex);
	p->dropdowns["scenetype"]->SetSelectedIndex(extraSceneType);
}

ActorParams *ExtraSceneParams::Copy()
{
	ExtraSceneParams *copy = new ExtraSceneParams(*this);
	return copy;
}

void ExtraSceneParams::Draw(sf::RenderTarget *target)
{
	ActorParams::Draw(target);
	Vector2f fPos = GetFloatPos();
	nameText.setPosition(fPos.x, fPos.y - 100);

	target->draw(nameText);
}