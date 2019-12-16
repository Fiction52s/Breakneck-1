#include "EditorTerrain.h"
#include "Physics.h"
#include "EditSession.h"

using namespace std;
using namespace sf;

TerrainBrush::TerrainBrush(PolyPtr poly)
	:pointStart(NULL), pointEnd(NULL), lines(sf::Lines, poly->numPoints * 2), numPoints(0)
{
	//assert( poly->finalized );

	TerrainPoint *curr = poly->pointStart;
	left = curr->pos.x;
	right = curr->pos.x;
	top = curr->pos.y;
	bot = curr->pos.y;

	TerrainPoint *p = new TerrainPoint(*curr);
	p->gate = NULL;
	AddPoint(p);

	curr = curr->next;
	for (; curr != NULL; curr = curr->next)
	{
		if (curr->pos.x < left)
			left = curr->pos.x;
		else if (curr->pos.x > right)
			right = curr->pos.x;

		if (curr->pos.y < top)
			top = curr->pos.y;
		else if (curr->pos.y > bot)
			bot = curr->pos.y;

		TerrainPoint *tp = new TerrainPoint(*curr);
		tp->gate = NULL;
		AddPoint(tp);
	}
	UpdateLines();
	//centerPos = Vector2f( left + width / 2.f, top + height / 2.f );
}

TerrainBrush::TerrainBrush(TerrainBrush &brush)
	:pointStart(NULL), pointEnd(NULL), numPoints(0),
	lines(sf::Lines, brush.numPoints * 2)
{
	left = brush.left;
	right = brush.right;
	top = brush.top;
	bot = brush.bot;
	pointStart = NULL;
	pointEnd = NULL;



	for (TerrainPoint *tp = brush.pointStart; tp != NULL; tp = tp->next)
	{
		AddPoint(new TerrainPoint(*tp));
	}

	UpdateLines();
}

TerrainBrush::~TerrainBrush()
{
	TerrainPoint *curr = pointStart;
	while (curr != NULL)
	{
		TerrainPoint *temp = curr->next;
		delete curr;
		curr = temp;
	}
}

void TerrainBrush::UpdateLines()
{
	int index = 0;
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		TerrainPoint *prev;
		if (curr == pointStart)
			prev = pointEnd;
		else
			prev = curr->prev;

		lines[index * 2].position = Vector2f(prev->pos.x, prev->pos.y);
		lines[index * 2 + 1].position = Vector2f(curr->pos.x, curr->pos.y);

		++index;
	}
}

void TerrainBrush::Draw(sf::RenderTarget *target)
{
	target->draw(lines);

	CircleShape cs;
	cs.setRadius(5);
	cs.setFillColor(Color::Red);
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);

	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		cs.setPosition(curr->pos.x, curr->pos.y);
		target->draw(cs);
	}
}

void TerrainBrush::Move(Vector2i delta)
{
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		curr->pos.x += delta.x;
		curr->pos.y += delta.y;
	}
	UpdateLines();
	//centerPos.x += delta.x;
	//centerPos.y += delta.y;
}

void TerrainBrush::AddPoint(TerrainPoint *tp)
{
	if (pointStart == NULL)
	{
		pointStart = tp;
		pointEnd = tp;
		tp->prev = NULL;
		tp->next = NULL;
	}
	else
	{
		pointEnd->next = tp;
		tp->prev = pointEnd;
		pointEnd = tp;
		pointEnd->next = NULL;
	}

	++numPoints;
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

void EditorDecorInfo::Move(boost::shared_ptr<ISelectable> me,
	sf::Vector2i delta)
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

void EditorDecorInfo::Deactivate(EditSession *edit,
	boost::shared_ptr<ISelectable> select)
{
	cout << "deactivating decor" << endl;
	EditorDecorPtr dec = boost::dynamic_pointer_cast<EditorDecorInfo>(select);

	myList->remove(dec);
}

void EditorDecorInfo::Activate(EditSession *edit,
	boost::shared_ptr<ISelectable> select)
{
	cout << "adding image" << endl;
	EditorDecorPtr dec = boost::dynamic_pointer_cast<EditorDecorInfo>(select);

	myList->push_back(dec);
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