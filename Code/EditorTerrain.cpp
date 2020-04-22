#include "EditorTerrain.h"
#include "Physics.h"
#include "EditSession.h"
#include "EditorRail.h"
#include "QuadTree.h"

using namespace std;
using namespace sf;

TerrainPoint *PointMoveInfo::GetPolyPoint()
{
	return poly->GetPoint(pointIndex);
}

TerrainPoint *PointMoveInfo::GetRailPoint()
{
	return rail->GetPoint(pointIndex);
}

bool EditorDecorInfo::CompareDecorInfoLayer( EditorDecorInfo &di0, EditorDecorInfo &di1)
{
	return di0.layer < di1.layer;
}

bool EditorDecorInfo::ContainsPoint(sf::Vector2f test)
{
	sf::Transform trans = spr.getTransform();
	FloatRect fr = spr.getLocalBounds();
	Vector2f points[4];
	points[0] = trans * Vector2f(fr.left, fr.top);
	points[1] = trans * Vector2f(fr.left + fr.width, fr.top);
	points[2] = trans * Vector2f(fr.left + fr.width, fr.top + fr.height);
	points[3] = trans * Vector2f(fr.left, fr.top + fr.height);

	bool result = QuadContainsPoint(V2d(points[0]),
		V2d(points[1]),
		V2d(points[2]),
		V2d(points[3]), V2d(test.x, test.y));

	//cout << "result: " << result << endl;
	return result;
}

bool EditorDecorInfo::Intersects(sf::IntRect rect)
{
	FloatRect fr(rect);
	return fr.intersects(spr.getGlobalBounds());
}

void EditorDecorInfo::Move(sf::Vector2i delta)
{
	spr.setPosition(spr.getPosition().x + delta.x, spr.getPosition().y + delta.y);
}

void EditorDecorInfo::BrushDraw(sf::RenderTarget *target,
	bool valid)
{
	target->draw(spr);
}

void EditorDecorInfo::Draw(sf::RenderTarget *target)
{
	target->draw(spr);
	if (selected)
	{
		sf::RectangleShape rs;
		rs.setFillColor(Color::Transparent);
		rs.setOutlineColor(Color::Green);
		rs.setOutlineThickness(3 * EditSession::zoomMultiple);
		rs.setPosition(spr.getGlobalBounds().left, spr.getGlobalBounds().top);
		rs.setSize(Vector2f(spr.getGlobalBounds().width, spr.getGlobalBounds().height));
		target->draw(rs);
	}
}

void EditorDecorInfo::Deactivate()
{
	cout << "deactivating decor" << endl;

	active = false;
	myList->remove(this);
}

void EditorDecorInfo::Activate()
{
	cout << "adding image" << endl;

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
	of << spr.getPosition().x << " " << spr.getPosition().y << endl;
	of << spr.getRotation() << endl;
	of << spr.getScale().x << " " << spr.getScale().y << endl;
	of << tile << endl;
}