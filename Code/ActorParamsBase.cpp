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
	if (groundInfo != NULL && groundInfo->ground != NULL)
	{
		return groundInfo->ground->GetEdge(groundInfo->edgeStart->index);
	}

	return NULL;
}

int ActorParams::GetWorld()
{
	return type->info.world;
}


ActorParams::ActorParams(ActorType *at)
	:ISelectable(ISelectable::ACTOR), boundingQuad(sf::Quads, 4),
	hasMonitor(false), group(NULL), type(at)
{
	groundInfo = NULL;
	lines = NULL;
	enemyLevel = 1;
	loop = false;
	for (int i = 0; i < 4; ++i)
		boundingQuad[i].color = Color(0, 255, 0, 100);
}

ActorParams::~ActorParams()
{
	if (lines != NULL)
		delete lines;

	if (groundInfo != NULL)
	{
		delete groundInfo;
	}
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
		for (; it != globalPath.end(); ++it)
		{

			Vector2i temp((*it).x - position.x, (*it).y - position.y);
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
	globalPath.push_back(position);
	for (list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it)
	{
		globalPath.push_back(position + (*it));
	}
	return globalPath;
}

void ActorParams::PlaceAerial(sf::Vector2i &pos)
{
	position = pos;
	image = type->GetSprite(false);
	image.setPosition(position.x, position.y);

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
	is >> position.x;
	is >> position.y;

	PlaceAerial(position);
}

void ActorParams::LoadGlobalPath(ifstream &is)
{
	list<Vector2i> globalPath;
	int pathLength;
	is >> pathLength;

	globalPath.push_back(Vector2i(position.x, position.y));

	for (int i = 0; i < pathLength; ++i)
	{
		int localX, localY;
		is >> localX;
		is >> localY;
		globalPath.push_back(Vector2i(position.x + localX, position.y + localY));
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
		if (groundInfo != NULL && groundInfo->ground != NULL)
			return true;
	}
	else if (type->CanBeRailGrounded())
	{
		if (groundInfo != NULL && groundInfo->railGround != NULL)
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
		target->draw(image);
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
		cs.setPosition(position.x, position.y);

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
		if (groundInfo != NULL)
		{
			int edgeIndex = groundInfo->GetEdgeIndex();
			if (groundInfo->ground != NULL)
			{
				of << ground << " " << groundInfo->ground->writeIndex;
			}
			else
			{
				assert(groundInfo->railGround != NULL);
				of << ground << " " << groundInfo->railGround->writeIndex;

			}
			of << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
		}
		else
		{
			of << air << " " << position.x << " " << position.y << endl;
		}
	}
	else if (canGrounded)
	{
		assert(groundInfo != NULL);

		int edgeIndex = groundInfo->GetEdgeIndex();

		if (groundInfo->ground != NULL)
		{
			of << groundInfo->ground->writeIndex;
		}
		else
		{
			assert(groundInfo->railGround != NULL);
			of << groundInfo->railGround->writeIndex;
		}

		of << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
	}
	else if (canAerial)
	{
		of << position.x << " " << position.y << endl;
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

GroundInfo::GroundInfo()
	:edgeStart(NULL), groundQuantity(-1), ground(NULL), railGround(NULL)
{

}

V2d GroundInfo::GetPosition()
{
	TerrainPoint *next = ground->GetNextPoint(edgeStart->index);

	V2d start = V2d(edgeStart->pos);
	V2d end = V2d(next->pos);

	V2d dir = normalize(end - start);
	return start + dir * groundQuantity;
}

int GroundInfo::GetEdgeIndex()
{
	return edgeStart->index;
}

TerrainPoint *GroundInfo::GetNextPoint()
{
	if (ground != NULL)
	{
		return ground->GetNextPoint(edgeStart->GetIndex());
	}
	else if (railGround != NULL)
	{
		return railGround->GetNextPoint(edgeStart->GetIndex());
	}
	else
	{
		return NULL;
	}
}

void GroundInfo::AddActor(ActorPtr a)
{
	if (ground != NULL)
	{
		ground->enemies[edgeStart].push_back(a);
		ground->UpdateBounds();
	}
	else if (railGround != NULL)
	{
		railGround->enemies[edgeStart].push_back(a);
		railGround->UpdateBounds();
	}
	else
	{
		assert(0);
	}
}

void GroundInfo::RemoveActor(ActorPtr a)
{
	if (ground != NULL)
	{
		ground->enemies[edgeStart].remove(a);
	}
	else if (railGround != NULL)
	{
		railGround->enemies[edgeStart].remove(a);
	}
	else
	{
		assert(0);
	}
}

void ActorParams::SetBoundingQuad()
{
	//float note
	int width = type->info.size.x;
	int height = type->info.size.y;

	if (type->CanBeGrounded() && groundInfo != NULL)
	{

		TerrainPoint *next = groundInfo->GetNextPoint();

		V2d v0((*groundInfo->edgeStart).pos.x, (*groundInfo->edgeStart).pos.y);
		V2d v1(next->pos.x, next->pos.y);
		V2d along = normalize(v1 - v0);
		V2d other(along.y, -along.x);


		V2d startGround = v0 + along * groundInfo->groundQuantity;
		V2d leftGround = startGround - along * (width / 2.0) + other * 1.0;
		V2d rightGround = startGround + along * (width / 2.0) + other * 1.0;
		V2d leftAir = leftGround + other * (double)(height - 1);
		V2d rightAir = rightGround + other * (double)(height - 1);

		boundingQuad[0].position = Vector2f(leftGround.x, leftGround.y);
		boundingQuad[1].position = Vector2f(leftAir.x, leftAir.y);
		boundingQuad[2].position = Vector2f(rightAir.x, rightAir.y);
		boundingQuad[3].position = Vector2f(rightGround.x, rightGround.y);

		V2d pos = (leftGround + leftAir + rightAir + rightGround) / 4.0;
		position = Vector2i(pos.x, pos.y);
	}
	else if (type->CanBeRailGrounded() && groundInfo != NULL && groundInfo->railGround != NULL)
	{
		TerrainPoint *curr = groundInfo->edgeStart;
		TerrainPoint *next = groundInfo->GetNextPoint();
		//assert(next != NULL);

		V2d currP(curr->pos);
		V2d nextP(next->pos);
		V2d dir = normalize(nextP - currP);
		V2d pos = currP + dir * groundInfo->groundQuantity;

		position = Vector2i(pos);

		boundingQuad[0].position = Vector2f(position.x - width / 2, position.y - height / 2);
		boundingQuad[1].position = Vector2f(position.x + width / 2, position.y - height / 2);
		boundingQuad[2].position = Vector2f(position.x + width / 2, position.y + height / 2);
		boundingQuad[3].position = Vector2f(position.x - width / 2, position.y + height / 2);
	}
	else
	{
		//patroller doesnt need a box because its not physical with the environment
		boundingQuad[0].position = Vector2f(position.x - width / 2, position.y - height / 2);
		boundingQuad[1].position = Vector2f(position.x + width / 2, position.y - height / 2);
		boundingQuad[2].position = Vector2f(position.x + width / 2, position.y + height / 2);
		boundingQuad[3].position = Vector2f(position.x - width / 2, position.y + height / 2);
	}

	UpdateExtraVisuals();
}

void ActorParams::UpdateGroundedSprite()
{
	TerrainPoint *edge = groundInfo->edgeStart;
	TerrainPoint *next = groundInfo->GetNextPoint();

	V2d pr(edge->pos.x, edge->pos.y);
	V2d cu(next->pos.x, next->pos.y);


	//this shouldn't remain here. i need more detailed checking.
	double groundLength = length(pr - cu);
	int width = type->info.size.x;

	if (groundInfo->groundQuantity + width / 2 > groundLength)
	{
		groundInfo->groundQuantity = groundLength - width / 2;
	}
	else if (groundInfo->groundQuantity - width / 2 < 0)
	{
		groundInfo->groundQuantity = width / 2;
	}

	V2d dir = normalize(cu - pr);
	V2d newPoint = pr + dir * groundInfo->groundQuantity;

	//V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
	//								(groundInfo->groundQuantity / length( cu - pr ) ) );

	image.setPosition(newPoint.x, newPoint.y);

	if (groundInfo->railGround == NULL)
	{
		double angle = atan2((cu - pr).y, (cu - pr).x) / PI * 180;
		image.setRotation(angle);
	}
}

void ActorParams::AnchorToRail(TerrainRail *rail,
	int edgeIndex, double quantity)
{
	assert(groundInfo == NULL);

	groundInfo = new GroundInfo;

	groundInfo->railGround = rail;

	groundInfo->groundQuantity = quantity;

	image = type->GetSprite(false);// .setTexture(*type->ts_image->texture);	

	Vector2i point;

	TerrainPoint *curr, *next;

	int numP = rail->GetNumPoints();

	for (int i = 0; i < numP - 1; ++i)
	{
		curr = rail->GetPoint(i);
		next = rail->GetNextPoint(i);

		if (edgeIndex == i)
		{
			V2d pr(curr->pos.x, curr->pos.y);
			V2d cu(next->pos.x, next->pos.y);

			V2d newPoint(pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length(cu - pr)), pr.y + (cu.y - pr.y) *
				(groundInfo->groundQuantity / length(cu - pr)));

			double angle = atan2((cu - pr).y, (cu - pr).x) / PI * 180;

			groundInfo->edgeStart = curr;


			UpdateGroundedSprite();
			SetBoundingQuad();

			break;
		}
	}
}

void ActorParams::AnchorToGround(PolyPtr poly, int edgeIndex, double quantity)
{
	assert(groundInfo == NULL);
	/*if( groundInfo != NULL )
	{
	cout << "unanchor" << endl;

	//UnAnchor( actor );
	cout << "end unanchor" << endl;

	//delete groundInfo;
	//groundInfo = NULL;
	}*/



	groundInfo = new GroundInfo;

	groundInfo->ground = poly;

	groundInfo->groundQuantity = quantity;


	image = type->GetSprite(true);// .setTexture(*type->ts_image->texture);	

	Vector2i point;

	TerrainPoint *curr, *next;

	//int numP = poly->GetNumPoints();

	curr = poly->GetPoint(edgeIndex);

	next = poly->GetNextPoint(edgeIndex);
	//prev = poly->GetPrevPoint(edgeIndex);

	V2d ne(next->pos.x, next->pos.y);
	V2d cu(curr->pos.x, curr->pos.y);

	Edge *currEdge = poly->GetEdge(edgeIndex);

	V2d newPoint = currEdge->GetPoint(groundInfo->groundQuantity);

	/*V2d newPoint(cu.x + (ne.x - cu.x) * (groundInfo->groundQuantity / length(ne - cu)), cu.y + (ne.y - cu.y) *
		(groundInfo->groundQuantity / length(cu - pr)));*/

	double angle = atan2((ne - cu).y, (ne - cu).x) / PI * 180;

	groundInfo->edgeStart = curr;


	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::AnchorToGround(GroundInfo &gi)
{
	groundInfo = new GroundInfo;
	*groundInfo = gi;

	image = type->GetSprite(true);

	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::AnchorToRail(GroundInfo &gi)
{
	groundInfo = new GroundInfo;
	*groundInfo = gi;

	image = type->GetSprite(false);

	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::UnAnchor()
{
	assert(groundInfo != NULL);

	if (groundInfo != NULL)
	{
		position = Vector2i(image.getPosition().x, image.getPosition().y);

		image.setOrigin(image.getLocalBounds().width / 2, image.getLocalBounds().height / 2);
		image.setRotation(0);

		if (groundInfo->ground != NULL)
		{
			groundInfo->ground->enemies[groundInfo->edgeStart].remove(this);
		}
		else if (groundInfo->railGround != NULL)
		{
			groundInfo->railGround->enemies[groundInfo->edgeStart].remove(this);
		}

		delete groundInfo;
		groundInfo = NULL;

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
	if (groundInfo == NULL)
	{
		position.x += delta.x;
		position.y += delta.y;
		SetBoundingQuad();

		image.setPosition(position.x, position.y);
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

	if (groundInfo != NULL)
	{
		groundInfo->RemoveActor(this);
	}
}

void ActorParams::Activate()
{
	active = true;
	cout << "addding to group of size: " << group->actors.size() << endl;
	group->actors.push_back(this);

	if (groundInfo != NULL)
	{
		groundInfo->AddActor(this);
	}
}
