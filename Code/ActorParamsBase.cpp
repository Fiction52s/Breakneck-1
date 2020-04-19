#include "ActorParamsBase.h"
#include "EditorActors.h"
#include <sstream>
#include "Physics.h"
#include "GUI.h"
#include "EditorTerrain.h"
#include "Session.h"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include "EditorRail.h"
#include "Enemy.h"

using namespace std;
using namespace sf;


Enemy *ActorParams::GenerateEnemy()
{
	if (type->info.enemyCreator != NULL)
	{
		return type->info.enemyCreator(this);
	}
	else
	{
		return NULL;
	}
}

Edge *ActorParams::GetGroundEdge()
{
	return posInfo.GetEdge();

	/*if (groundInfo != NULL && groundInfo->ground != NULL)
	{
		return groundInfo->ground->GetEdge(groundInfo->edgeStart->index);
	}

	return NULL;*/
}

int ActorParams::GetWorld()
{
	return type->info.world;
}


ActorParams::ActorParams(ActorType *at)
	:ISelectable(ISelectable::ACTOR), boundingQuad(sf::Quads, 4),
	hasMonitor(false), group(NULL), type(at)
{
	lines = NULL;
	myEnemy = NULL;
	enemyLevel = 1;
	loop = false;
	for (int i = 0; i < 4; ++i)
		boundingQuad[i].color = Color(0, 255, 0, 100);

	//CreateMyEnemy();
}

ActorParams::~ActorParams()
{
	if (lines != NULL)
		delete lines;
}

void ActorParams::CreateMyEnemy()
{
	myEnemy = GenerateEnemy();
}

void ActorParams::WriteLevel(std::ofstream &of)
{
	if (enemyLevel < 1)
		enemyLevel = 1;
	of << enemyLevel << endl;
}

bool ActorParams::SetLevel(int lev)
{
	if (lev >= 1 && lev <= type->info.numLevels)
	{
		enemyLevel = lev;
		return true;
	}
	else
	{
		return false;
	}
}

void ActorParams::WriteMonitor(ofstream  &of)
{
	WriteBool(of, hasMonitor);
}

void ActorParams::WriteParamFile(std::ofstream &of)
{
	WriteBasicParamFile(of);
}

void ActorParams::WriteBasicParamFile(std::ofstream &of)
{
	WriteMonitor(of);
	WriteLevel(of);
}

void ActorParams::SetParams()
{
	SetBasicParams();
}

void ActorParams::SetBasicParams()
{
	Panel *p = type->panel;

	int level;

	stringstream ss;
	string s = p->textBoxes["level"]->text.getString().toAnsiString();
	ss << s;

	ss >> level;

	if (!ss.fail() && level > 0 && level < type->info.numLevels)
	{
		enemyLevel = level;
	}

	hasMonitor = p->checkBoxes["monitor"]->checked;
}



void ActorParams::WritePath(std::ofstream &of)
{
	of << localPath.size() << endl;

	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		of << (*it).x << " " << (*it).y << endl;
	}
}

void ActorParams::WriteLoop(std::ofstream &of)
{
	WriteBool(of, loop);
}

void ActorParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	if (lines != NULL)
	{
		delete lines;
		lines = NULL;
	}

	localPath.clear();
	if (globalPath.size() > 1)
	{
		int numLines = globalPath.size();

		lines = new VertexArray(sf::LinesStrip, numLines);
		VertexArray &li = *lines;
		li[0].position = Vector2f(0, 0);
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;

		Vector2i myIntPos = GetIntPos();
		for (; it != globalPath.end(); ++it)
		{
			Vector2i temp((*it).x - myIntPos.x, (*it).y - myIntPos.y);
			localPath.push_back(temp);
			li[index].position = Vector2f(temp.x, temp.y);
			li[index].color = Color::Magenta;
			++index;
		}
	}
}

std::list<sf::Vector2i> ActorParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	Vector2i myIntPos = GetIntPos();
	globalPath.push_back(myIntPos);
	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		globalPath.push_back(myIntPos + (*it));
	}
	return globalPath;
}

void ActorParams::PlaceAerial(sf::Vector2i &pos)
{
	SetPosition(pos);
	image = type->GetSprite(false);
	image.setPosition(GetFloatPos());

	SetBoundingQuad();
}

void ActorParams::PlaceGrounded(PolyPtr tp,
	int edgeIndex, double quant)
{
	AnchorToGround(tp, edgeIndex, quant);
	SetBoundingQuad();
}

void ActorParams::PlaceRailed(TerrainRail *rail,
	int edgeIndex, double quant)
{
	AnchorToRail(rail, edgeIndex, quant);
	SetBoundingQuad();
}

void ActorParams::LoadGrounded(std::ifstream &is)
{
	int terrainIndex, edgeIndex;
	double edgeQuantity;

	is >> terrainIndex;
	is >> edgeIndex;
	is >> edgeQuantity;

	Session *sess = Session::GetSession();

	PolyPtr terrain = sess->GetPolygon(terrainIndex);
	PlaceGrounded(terrain, edgeIndex, edgeQuantity);
}

void ActorParams::LoadRailed(std::ifstream &is)
{
	int terrainIndex, edgeIndex;
	double edgeQuantity;

	is >> terrainIndex;
	is >> edgeIndex;
	is >> edgeQuantity;

	Session *sess = Session::GetSession();

	RailPtr rail = sess->GetRail(terrainIndex);
	PlaceRailed(rail, edgeIndex, edgeQuantity);
}

void ActorParams::LoadBool(ifstream &is, bool &b)
{
	int x;
	is >> x;
	b = x;
}

void ActorParams::WriteBool(ofstream &of, bool b)
{
	of << (int)b << "\n";
}

void ActorParams::LoadAerial(std::ifstream &is)
{
	Vector2i pos;
	is >> pos.x;
	is >> pos.y;

	PlaceAerial(pos);
}

void ActorParams::LoadGlobalPath(ifstream &is)
{
	list<Vector2i> globalPath;
	int pathLength;
	is >> pathLength;

	Vector2i myIntPos = GetIntPos();
	globalPath.push_back(myIntPos);

	for (int i = 0; i < pathLength; ++i)
	{
		int localX, localY;
		is >> localX;
		is >> localY;
		globalPath.push_back(Vector2i(myIntPos.x + localX, myIntPos.y + localY));
	}

	SetPath(globalPath);
}

void ActorParams::LoadMonitor(std::ifstream &is)
{
	int ihasMonitor;
	is >> ihasMonitor;
	hasMonitor = ihasMonitor;
}

void ActorParams::LoadEnemyLevel(std::ifstream &is)
{
	is >> enemyLevel;
	if (enemyLevel < 1)
		enemyLevel = 1;
}

void ActorParams::SetBasicPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["level"]->text.setString(boost::lexical_cast<string>(enemyLevel));
	p->checkBoxes["monitor"]->checked = hasMonitor;
}



void ActorParams::SetPanelInfo()
{
	SetBasicPanelInfo();
}

void ActorParams::SetSelected(bool select)
{
	cout << "------selected: " << select << endl;
	selected = select;
}

bool ActorParams::CanApply()
{
	if (type->CanBeGrounded())
	{
		if (posInfo.ground != NULL)
			return true;
	}
	else if (type->CanBeRailGrounded())
	{
		if (posInfo.railGround != NULL)
			return true;
	}
	else if (type->CanBeAerial())
	{
		return true;
	}

	return false;
}

bool ActorParams::CanAdd()
{
	assert(false);
	return false; //shouldn't get called anyway
}

void ActorParams::Draw(sf::RenderTarget *target)
{
	//cout << "Selected: " << selected << endl;


	//temporary checks might make it lag less?

	DrawLevel(target);

	DrawMonitor(target);

	Vector2f viewCenter = target->getView().getCenter();
	Vector2f viewSize = target->getView().getSize();
	if (image.getGlobalBounds().intersects(FloatRect(viewCenter.x - viewSize.x / 2, viewCenter.y - viewSize.y / 2,
		viewSize.x, viewSize.y)))
	{
		if (myEnemy != NULL)
			myEnemy->Draw(target);
		else
		{
			target->draw(image);
		}
		
	}

	DrawBoundary(target);
}

void ActorParams::DrawPreview(sf::RenderTarget *target)
{
}

void ActorParams::DrawMonitor(sf::RenderTarget *target)
{


	if (hasMonitor)
	{
		double w = image.getLocalBounds().width;
		double h = image.getLocalBounds().height;

		sf::CircleShape cs;
		cs.setRadius(max(w, h));

		cs.setFillColor(Color::White);

		cs.setOrigin(cs.getLocalBounds().width / 2,
			cs.getLocalBounds().height / 2);
		cs.setPosition(GetFloatPos());

		target->draw(cs);
	}



}

void ActorParams::WriteFile(ofstream &of)
{
	of << type->info.name << " ";

	bool canGrounded = type->CanBeGrounded() || type->CanBeRailGrounded();
	bool canAerial = type->CanBeAerial();

	int air = 0;
	int ground = 1;


	if (canGrounded && canAerial)
	{
		if ( !posInfo.IsAerial() )
		{
			int edgeIndex = posInfo.GetEdgeIndex();
			if (posInfo.ground != NULL)
			{
				of << ground << " " << posInfo.ground->writeIndex;
			}
			else
			{
				assert(posInfo.railGround != NULL);
				of << ground << " " << posInfo.railGround->writeIndex;

			}
			of << " " << edgeIndex << " " << posInfo.groundQuantity << endl;
		}
		else
		{
			Vector2i intPos = GetIntPos();
			of << air << " " << intPos.x << " " << intPos.y << endl;
		}
	}
	else if (canGrounded)
	{
		assert(!posInfo.IsAerial());

		int edgeIndex = posInfo.GetEdgeIndex();

		if (posInfo.ground != NULL)
		{
			of << posInfo.ground->writeIndex;
		}
		else
		{
			assert(posInfo.railGround != NULL);
			of << posInfo.railGround->writeIndex;
		}

		of << " " << edgeIndex << " " << posInfo.groundQuantity << endl;
	}
	else if (canAerial)
	{
		Vector2i intPos = GetIntPos();
		of << intPos.x << " " << intPos.y << endl;
	}
	else
	{
		assert(false);
	}

	WriteParamFile(of);
}

void ActorParams::DrawQuad(sf::RenderTarget *target)
{
	target->draw(boundingQuad);
}

PositionInfo::PositionInfo()
	:edgeIndex(-1), groundQuantity(-1), ground(NULL), railGround(NULL)
{

}

V2d PositionInfo::GetPosition()
{
	Edge *edge = GetEdge();
	if (edge != NULL)
	{
		return edge->GetPosition(groundQuantity);
	}
	else
	{
		return position;
	}
}

void PositionInfo::SetPosition(V2d &pos)
{
	position = pos;
}

int PositionInfo::GetEdgeIndex()
{
	return edgeIndex;
}

void PositionInfo::SetGround(PolyPtr p_ground, int p_edgeIndex, double quant)
{
	ground = p_ground;
	edgeIndex = p_edgeIndex;
	groundQuantity = quant;
}

void PositionInfo::SetRail(RailPtr p_rail, int p_edgeIndex, double quant)
{
	railGround = p_rail;
	edgeIndex = p_edgeIndex;
	groundQuantity = quant;
}

void PositionInfo::AddActor(ActorPtr a)
{
	if (ground != NULL)
	{
		ground->enemies[GetPoint()].push_back(a);
		ground->UpdateBounds();
	}
	else if (railGround != NULL)
	{
		railGround->enemies[GetPoint()].push_back(a);
		railGround->UpdateBounds();
	}
	else
	{
		//do nothing
		//assert(0);
	}
}

void PositionInfo::RemoveActor(ActorPtr a)
{
	if (ground != NULL)
	{
		ground->enemies[GetPoint()].remove(a);
	}
	else if (railGround != NULL)
	{
		railGround->enemies[GetPoint()].remove(a);
	}
	else
	{
		//do nothing
		//assert(0);
	}
}

Edge *PositionInfo::GetEdge()
{
	if (ground != NULL)
	{
		return ground->GetEdge(edgeIndex);
	}
	else if (railGround != NULL)
	{
		return NULL;
		//rails not complete yet
		//railGround->GetEdge(edgeIndex);
	}
	else
	{
		return NULL;
	}
}

void PositionInfo::SetAerial( V2d &pos )
{
	position = pos;
	ground = NULL;
	railGround = NULL;
}

bool PositionInfo::IsAerial()
{
	return (ground == NULL && railGround == NULL);
}

TerrainPoint *PositionInfo::GetPoint()
{
	if (ground != NULL)
	{
		return ground->GetPoint(edgeIndex);
	}
	else if(railGround != NULL )
	{
		return NULL;
	}
	else
	{
		return NULL;
	}
}

TerrainPoint *PositionInfo::GetNextPoint()
{
	if (ground != NULL)
	{
		return ground->GetNextPoint(edgeIndex);
	}
	else if (railGround != NULL)
	{
		return NULL;
	}
	else
	{
		return NULL;
	}
}

void ActorParams::SetPosition(V2d &pos)
{
	posInfo.SetPosition(pos);
}

Vector2i ActorParams::GetIntPos()
{
	V2d posD = GetPosition();
	return Vector2i(round(posD.x), round(posD.y));
}

Vector2f ActorParams::GetFloatPos()
{
	return Vector2f(GetPosition());
}

void ActorParams::SetPosition(Vector2i &pos)
{
	posInfo.SetPosition(V2d(pos));
}

void ActorParams::SetPosition(const sf::Vector2f &pos)
{
	posInfo.SetPosition(V2d(pos));
}

V2d ActorParams::GetPosition()
{
	return posInfo.GetPosition();
}

void ActorParams::SetBoundingQuad()
{
	//float note
	int width = type->info.size.x;
	int height = type->info.size.y;

	if (type->CanBeGrounded() && posInfo.ground != NULL )
	{
		Edge *edge = posInfo.GetEdge();
		V2d along = edge->Along();
		V2d other = edge->Normal();

		V2d startGround = posInfo.GetPosition();//v0 + along * groundInfo->groundQuantity;
		V2d leftGround = startGround - along * (width / 2.0) + other * 1.0;
		V2d rightGround = startGround + along * (width / 2.0) + other * 1.0;
		V2d leftAir = leftGround + other * (double)(height - 1);
		V2d rightAir = rightGround + other * (double)(height - 1);

		boundingQuad[0].position = Vector2f(leftGround.x, leftGround.y);
		boundingQuad[1].position = Vector2f(leftAir.x, leftAir.y);
		boundingQuad[2].position = Vector2f(rightAir.x, rightAir.y);
		boundingQuad[3].position = Vector2f(rightGround.x, rightGround.y);

		V2d pos = (leftGround + leftAir + rightAir + rightGround) / 4.0;
		SetPosition(pos);
	}
	else if (type->CanBeRailGrounded() && posInfo.railGround != NULL)
	{
		V2d pos = posInfo.GetPosition();

		SetPosition(pos);

		Vector2f fPos = GetFloatPos();
		boundingQuad[0].position = Vector2f(fPos.x - width / 2, fPos.y - height / 2);
		boundingQuad[1].position = Vector2f(fPos.x + width / 2, fPos.y - height / 2);
		boundingQuad[2].position = Vector2f(fPos.x + width / 2, fPos.y + height / 2);
		boundingQuad[3].position = Vector2f(fPos.x - width / 2, fPos.y + height / 2);
	}
	else
	{
		Vector2f fPos = GetFloatPos();
		//patroller doesnt need a box because its not physical with the environment
		boundingQuad[0].position = Vector2f(fPos.x - width / 2, fPos.y - height / 2);
		boundingQuad[1].position = Vector2f(fPos.x + width / 2, fPos.y - height / 2);
		boundingQuad[2].position = Vector2f(fPos.x + width / 2, fPos.y + height / 2);
		boundingQuad[3].position = Vector2f(fPos.x - width / 2, fPos.y + height / 2);
	}

	UpdateExtraVisuals();
}

void ActorParams::UpdateGroundedSprite()
{
	Edge *edge = posInfo.GetEdge();
	//this shouldn't remain here. i need more detailed checking.
	double groundLength = edge->GetLength();
	int width = type->info.size.x;

	if (posInfo.groundQuantity + width / 2 > groundLength)
	{
		posInfo.groundQuantity = groundLength - width / 2;
	}
	else if (posInfo.groundQuantity - width / 2 < 0)
	{
		posInfo.groundQuantity = width / 2;
	}

	V2d newPoint = posInfo.GetPosition();
	image.setPosition(newPoint.x, newPoint.y);

	if (posInfo.railGround == NULL)
	{
		image.setRotation(edge->GetNormalAngleDegrees());
	}
}

void ActorParams::AnchorToRail(TerrainRail *rail,
	int edgeIndex, double quantity)
{
	posInfo.SetRail(rail, edgeIndex, quantity);

	image = type->GetSprite(false);// .setTexture(*type->ts_image->texture);	

	Vector2i point;

	TerrainPoint *curr, *next;

	curr = rail->GetPoint(edgeIndex);
	next = rail->GetNextPoint(edgeIndex);

	V2d pr(curr->pos.x, curr->pos.y);
	V2d cu(next->pos.x, next->pos.y);

	V2d newPoint(pr.x + (cu.x - pr.x) * (quantity / length(cu - pr)), pr.y + (cu.y - pr.y) *
		quantity / length(cu - pr));

	double angle = atan2((cu - pr).y, (cu - pr).x) / PI * 180;

	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::AnchorToGround(PolyPtr poly, int edgeIndex, double quantity)
{
	assert(posInfo.ground == NULL);

	
	posInfo.SetGround(poly, edgeIndex, quantity);

	image = type->GetSprite(true);

	V2d newPoint = posInfo.GetPosition();

	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::AnchorToGround(PositionInfo &gi)
{
	posInfo = gi;

	image = type->GetSprite(true);

	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::AnchorToRail(PositionInfo &gi)
{
	posInfo = gi;

	image = type->GetSprite(false);

	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::UnAnchor()
{
	assert(posInfo.ground != NULL);
	if (posInfo.ground != NULL)
	{
		posInfo.SetAerial(V2d(image.getPosition()));

		image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
		image.setRotation(0);

		posInfo.RemoveActor(this);

		SetBoundingQuad();
	}
}

void ActorParams::DrawBoundary(sf::RenderTarget *target)
{
	if (selected)
	{
		sf::RectangleShape rs;
		rs.setFillColor(Color::Transparent);
		rs.setOutlineColor(Color::Green);
		//rs.setOutlineThickness(3 * EditSession::zoomMultiple);
		//fix soon
		rs.setPosition(image.getGlobalBounds().left, image.getGlobalBounds().top);
		rs.setSize(Vector2f(image.getGlobalBounds().width, image.getGlobalBounds().height));
		target->draw(rs);
		//cout << "selected draw" << endl;
	}
}

void ActorParams::DrawLevel(sf::RenderTarget *target)
{
	if (type->info.numLevels > 1 && enemyLevel > 1)
	{
		int extra = 10;
		sf::RectangleShape rs;
		rs.setFillColor(Color::Transparent);
		rs.setOutlineColor(Color(255, 0, 0, 100));
		//rs.setOutlineThickness(3 * EditSession::zoomMultiple);
		rs.setPosition(image.getGlobalBounds().left - extra, image.getGlobalBounds().top - extra);
		rs.setSize(Vector2f(image.getGlobalBounds().width + extra, image.getGlobalBounds().height + extra));
		target->draw(rs);
	}
	if (type->info.numLevels > 2 && enemyLevel > 2)
	{
		int extra = 20;
		sf::RectangleShape rs;
		rs.setFillColor(Color::Transparent);
		rs.setOutlineColor(Color(100, 100, 255, 100));
		//rs.setOutlineThickness(3 * EditSession::zoomMultiple);
		rs.setPosition(image.getGlobalBounds().left - extra, image.getGlobalBounds().top - extra);
		rs.setSize(Vector2f(image.getGlobalBounds().width + extra, image.getGlobalBounds().height + extra));
		target->draw(rs);
	}
}

bool ActorParams::ContainsPoint(sf::Vector2f test)
{
	sf::Transform trans = image.getTransform();


	/*V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );*/


	V2d A(boundingQuad[0].position.x, boundingQuad[0].position.y);
	V2d B(boundingQuad[1].position.x, boundingQuad[1].position.y);
	V2d C(boundingQuad[2].position.x, boundingQuad[2].position.y);
	V2d D(boundingQuad[3].position.x, boundingQuad[3].position.y);

	bool result = QuadContainsPoint(A, B, C, D, V2d(test.x, test.y));

	//cout << "result: " << result << endl;
	return result;
}

bool ActorParams::Intersects(sf::IntRect rect)
{
	V2d A(boundingQuad[0].position.x, boundingQuad[0].position.y);
	V2d B(boundingQuad[1].position.x, boundingQuad[1].position.y);
	V2d C(boundingQuad[2].position.x, boundingQuad[2].position.y);
	V2d D(boundingQuad[3].position.x, boundingQuad[3].position.y);

	V2d rA(rect.left, rect.top);
	V2d rB(rect.left + rect.width, rect.top);
	V2d rC(rect.left + rect.width, rect.top + rect.height);
	V2d rD(rect.left, rect.top + rect.height);
	if (isQuadTouchingQuad(A, B, C, D, rA, rB, rC, rD))
	{
		return true;
	}
	else
		return false;
}

bool ActorParams::IsPlacementOkay()
{
	return false;
}

void ActorParams::Move(sf::Vector2i delta)
{
	if (posInfo.IsAerial())
	{
		
		posInfo.position.x += delta.x;
		posInfo.position.y += delta.y;
		SetBoundingQuad();
		image.setPosition(GetFloatPos());
	}
}

void ActorParams::BrushDraw(sf::RenderTarget *target,
	bool valid)
{
	image.setColor(Color(255, 255, 255, 100));
	target->draw(image);
	image.setColor(Color::White);
}

void ActorParams::Deactivate()
{
	active = false;
	cout << "DEACTIVATING ACTOR PARAMS size from: " << group->actors.size() << endl;
	group->actors.remove(this);


	posInfo.RemoveActor(this);
}

void ActorParams::Activate()
{
	active = true;
	cout << "addding to group of size: " << group->actors.size() << endl;
	group->actors.push_back(this);

	posInfo.AddActor(this);
}
