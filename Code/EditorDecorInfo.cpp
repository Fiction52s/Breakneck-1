#include "EditorDecorInfo.h"
#include "Physics.h"
#include "EditSession.h"
#include "EditorRail.h"
#include "QuadTree.h"
#include "TransformTools.h"

using namespace std;
using namespace sf;

EditorDecorInfo::EditorDecorInfo(sf::Sprite &s, int lay,
	const std::string &dName, int p_tile)
	:ISelectable(ISelectable::ISelectableType::IMAGE)
{
	spr = s;
	layer = lay;
	decorName = dName;
	tile = p_tile;
	dMode = D_NORMAL;
	currScale = Vector2f(0.f,0.f);
	currRotate = 0;
}

void EditorDecorInfo::StartTransformation()
{
	origSpr = spr;
	dMode = D_TRANSFORM;
}

void EditorDecorInfo::CancelTransformation()
{
	spr = origSpr;
	dMode = D_NORMAL;
	currScale = origSpr.getScale() - Vector2f( 1, 1 );
	currRotate = origSpr.getRotation();
	//UpdateLinePositions();
	//UpdateLineColors();
	//SetRenderMode(RENDERMODE_NORMAL);
	//triBackups.clear();
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
	spr = origSpr;

	return newDec;
}

void EditorDecorInfo::UpdateTransformation(TransformTools *tr)
{
	Transform t;
	t.rotate(tr->rotation);
	t.scale(tr->scale);

	spr.setRotation(tr->rotation + currRotate);
	spr.setScale(tr->scale + currScale);
	spr.setPosition(tr->GetCenter());
	//spr.setRotation(tr->rotation);
	//spr.setScale(scale);
}


bool EditorDecorInfo::CompareDecorInfoLayer(EditorDecorInfo &di0, EditorDecorInfo &di1)
{
	return di0.layer < di1.layer;
}

bool EditorDecorInfo::ContainsPoint(sf::Vector2f test)
{
	return spr.getGlobalBounds().contains(test);
	//sf::Transform trans = spr.getTransform();
	//FloatRect fr = spr.getLocalBounds();
	//Vector2f points[4];
	//points[0] = trans * Vector2f(fr.left, fr.top);
	//points[1] = trans * Vector2f(fr.left + fr.width, fr.top);
	//points[2] = trans * Vector2f(fr.left + fr.width, fr.top + fr.height);
	//points[3] = trans * Vector2f(fr.left, fr.top + fr.height);

	//bool result = QuadContainsPoint(V2d(points[0]),
	//	V2d(points[1]),
	//	V2d(points[2]),
	//	V2d(points[3]), V2d(test.x, test.y));

	////cout << "result: " << result << endl;
	//return result;
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

void EditorDecorInfo::MoveTo(sf::Vector2i &pos)
{
	spr.setPosition(pos.x, pos.y);
}

void EditorDecorInfo::BrushDraw(sf::RenderTarget *target,
	bool valid)
{
	target->draw(spr);
}

void EditorDecorInfo::Draw(sf::RenderTarget *target)
{
	target->draw(spr);
	if (selected && dMode == D_NORMAL )
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
	of << spr.getPosition().x << " " << spr.getPosition().y << endl;
	of << spr.getRotation() << endl;
	of << spr.getScale().x << " " << spr.getScale().y << endl;
	of << tile << endl;
}