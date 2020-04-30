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

void EditorDecorInfo::StartTransformation()
{
	dMode = D_TRANSFORM;
}

void EditorDecorInfo::CancelTransformation()
{
	dMode = D_NORMAL;
}

DecorPtr EditorDecorInfo::CompleteTransformation(TransformTools *tr)
{	
	dMode = D_NORMAL;

	sf::Transform t;
	t.rotate(tr->rotation + rotation);
	t.scale(tr->scale + (scale - Vector2f(1, 1)));

	Vector2f newCenter = tr->GetCenter() + t.transformPoint(transformOffset);

	DecorPtr newDec = new EditorDecorInfo( decorName, ts, tile, layer,
		newCenter, tr->rotation + rotation, tr->scale + (scale - Vector2f( 1, 1)) );
	newDec->myList = myList;
	newDec->selected = false;

	return newDec;
}

void EditorDecorInfo::UpdateTransformation(TransformTools *tr)
{
	Transform t;
	t.rotate(tr->rotation + rotation);
	t.scale(tr->scale + ( scale - Vector2f( 1,1)));

	Vector2f tCenter = tr->GetCenter() + t.transformPoint( transformOffset );

	int halfWidth = tileSize.x / 2;
	int halfHeight = tileSize.y / 2;
	Vector2f topLeft(-halfWidth, -halfHeight);
	Vector2f topRight(halfWidth, -halfHeight);
	Vector2f botRight(halfWidth, halfHeight);
	Vector2f botLeft(-halfWidth, halfHeight);

	quad[0].position = t.transformPoint(topLeft) + tCenter;
	quad[1].position = t.transformPoint(topRight) + tCenter;
	quad[2].position = t.transformPoint(botRight) + tCenter;
	quad[3].position = t.transformPoint(botLeft) + tCenter;
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
	return isQuadTouchingQuad(V2d(quad[0].position), V2d(quad[1].position),
		V2d(quad[2].position), V2d(quad[3].position), V2d(rect.left, rect.top),
		V2d(rect.left + rect.width, rect.top), V2d(rect.left + rect.width, rect.top + rect.height),
		V2d(rect.left, rect.top + rect.height));
	//return GetAABB().intersects(rect);
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
	//if (dMode == D_TRANSFORM)
	//{
	//	//target->draw(transQuad, 4, sf::Quads, ts->texture);
	//	return;
	//}
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