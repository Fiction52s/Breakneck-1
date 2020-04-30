#include "EditorDecorInfo.h"
#include "Physics.h"
#include "EditSession.h"
#include "EditorRail.h"
#include "QuadTree.h"
#include "TransformTools.h"

using namespace std;
using namespace sf;

EditorDecorInfo::EditorDecorInfo(const std::string &dName,
	Tileset *p_ts, int p_tile, int lay, sf::Vector2f &centerPos, float rot, sf::Vector2f &sc)
	:ISelectable(ISelectable::ISelectableType::IMAGE)
{
	decorName = dName;
	ts = p_ts;
	tile = p_tile;
	layer = lay;
	dMode = D_NORMAL;

	center = centerPos;
	rotation = rot;
	scale = sc;

	tileSize = Vector2f(ts->tileWidth, ts->tileHeight);

	ts->SetQuadSubRect(quad, tile);
	UpdateQuad();
}

IntRect EditorDecorInfo::GetAABB()
{
	float left = quad[0].position.x;
	float right = quad[0].position.x;
	float top = quad[0].position.y;
	float bot = quad[0].position.y;

	for (int i = 1; i < 4; ++i)
	{
		left = min(left, quad[i].position.x);
		right = min(right, quad[i].position.x);
		top = min(top, quad[i].position.y);
		bot = min(bot, quad[i].position.y);
	}

	return IntRect(left, top, right - left, bot - top);
}

EditorDecorInfo::EditorDecorInfo(EditorDecorInfo &edi)
	:ISelectable(ISelectable::ISelectableType::IMAGE)
{
	//spr = edi.spr;
	//currScale = spr.getScale() - Vector2f( 1.f, 1.f);
	//currRotate = spr.getRotation();
	//spr.setScale(Vector2f(currScale.x + 1.f, currScale.y + 1.f));
	//spr.setRotation(currRotate);

	/*dMode = edi.dMode;
	
	layer = edi.layer;
	decorName = edi.decorName;
	tile = edi.tile;
	myList = edi.myList;*/
}

void EditorDecorInfo::StartTransformation()
{
	//origSpr = spr;
	dMode = D_TRANSFORM;
}

void EditorDecorInfo::CancelTransformation()
{
	//spr = origSpr;
	dMode = D_NORMAL;
	//currScale = origSpr.getScale() - Vector2f( 1, 1 );
	//currRotate = origSpr.getRotation();
}

DecorPtr EditorDecorInfo::CompleteTransformation()
{
	//if (renderMode == RENDERMODE_TRANSFORM)
	//{
	//	SetRenderMode(RENDERMODE_NORMAL);

	//	PolyPtr newPoly(new TerrainPolygon);

	//	int numP = GetNumPoints();
	//	TerrainPoint *curr;
	//	Vector2i temp;

	//	newPoly->Reserve(numP);
	//	for (int i = 0; i < numP; ++i)
	//	{
	//		temp.x = round(lines[i * 2].position.x);
	//		temp.y = round(lines[i * 2].position.y);
	//		newPoly->AddPoint(temp, false);
	//	}

	//	UpdateLinePositions();

	//	newPoly->Finalize();

	//	//check for validity here

	//	//not even going to use the same polygon here, this is just for testing. 
	//	//what will really happen is that I create a copy, adding in the rounded points from my lines.
	//	//that way I can adjust and test for correctness just like i usually would, and then just
	//	//do a replacebrush action
	//	//maybe test for correctness?

	//	//SoftReset();
	//	//Finalize();

	//	return newPoly;
	//}
	
	dMode = D_NORMAL;

	DecorPtr newDec = new EditorDecorInfo(*this);

	newDec->selected = false;
	//spr = origSpr;

	return newDec;
}

void EditorDecorInfo::UpdateTransformation(TransformTools *tr)
{
	Transform t;
	t.rotate(tr->rotation);
	t.scale(tr->scale);
}


bool EditorDecorInfo::CompareDecorInfoLayer(EditorDecorInfo &di0, EditorDecorInfo &di1)
{
	return di0.layer < di1.layer;
}

bool EditorDecorInfo::ContainsPoint(sf::Vector2f test)
{
	return QuadContainsPoint(quad, test);
}

bool EditorDecorInfo::Intersects(sf::IntRect rect)
{
	return GetAABB().intersects(rect);
}

void EditorDecorInfo::UpdateQuad()
{
	Transform t;
	t.rotate(rotation);
	t.scale(scale);
	
	int halfWidth = tileSize.x / 2;
	int halfHeight = tileSize.y / 2;
	Vector2f topLeft(-halfWidth, -halfHeight);
	Vector2f topRight(halfWidth, -halfHeight);
	Vector2f botRight(halfWidth, halfHeight);
	Vector2f botLeft(-halfWidth, halfHeight);

	quad[0].position = t.transformPoint(topLeft) + center;
	quad[1].position = t.transformPoint(topRight) + center;
	quad[2].position = t.transformPoint(botRight) + center;
	quad[3].position = t.transformPoint(botLeft) + center;
}

void EditorDecorInfo::Move(sf::Vector2i delta)
{
	center += Vector2f(delta);
	UpdateQuad();
}

void EditorDecorInfo::MoveTo(sf::Vector2i &pos)
{
	center.x = pos.x;
	center.y = pos.y;
	UpdateQuad();
}

void EditorDecorInfo::BrushDraw(sf::RenderTarget *target,
	bool valid)
{
	target->draw(quad, 4, sf::Quads, ts->texture);
}

void EditorDecorInfo::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads, ts->texture);
	if (selected && dMode == D_NORMAL )
	{
		sf::RectangleShape rs;
		rs.setFillColor(Color::Transparent);
		rs.setOutlineColor(Color::Green);
		rs.setOutlineThickness(3 * EditSession::zoomMultiple);
		//rs.setSize(size);
		rs.setSize(Vector2f( tileSize.x * scale.x, tileSize.y * scale.y ));
		rs.setOrigin(rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2);
		rs.setRotation(rotation);
		rs.setPosition(center);
		//rs.setOrigin( rs.get)
		//rs.setPosition(spr.getGlobalBounds().left, spr.getGlobalBounds().top);
		//rs.setSize(Vector2f(spr.getGlobalBounds().width, spr.getGlobalBounds().height));
		target->draw(rs);
	}
}

void EditorDecorInfo::Deactivate()
{
	//cout << "deactivating decor" << endl;

	active = false;
	myList->remove(this);
}

void EditorDecorInfo::Activate()
{
	//cout << "adding image" << endl;

	active = true;
	myList->push_back(this);
}

void EditorDecorInfo::SetSelected(bool select)
{
	selected = select;
}

void EditorDecorInfo::WriteFile(std::ofstream &of)
{
	of << decorName << endl;
	of << layer << endl;
	of << center.x << " " << center.y << endl;
	of << rotation << endl;
	of << scale.x << " " << scale.y << endl;
	of << tile << endl;
}