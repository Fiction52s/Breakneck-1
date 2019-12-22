#include "EditorRail.h"
#include "EditorTerrain.h"
#include "Physics.h"
#include "EditSession.h"
#include "ActorParams.h"
#include "poly2tri/poly2tri.h"

using namespace std;
using namespace sf;

TerrainRail::TerrainRail()
	:ISelectable(ISelectable::RAIL)
{
	Init();
}

TerrainRail::TerrainRail(TerrainRail &r)
	:ISelectable( ISelectable::RAIL )
{
	Init();

	r.CopyPoints(pointStart, pointEnd);
	numPoints = r.numPoints;
}

void TerrainRail::Init()
{
	pointStart = NULL;
	pointEnd = NULL;
	finalized = false;
	numPoints = 0;
	lines = NULL;
	movingPointMode = false;
	selected = false;
	railRadius = 10;

	requirePower = false;
	accelerate = false;
	level = 1;
}

TerrainRail::~TerrainRail()
{
	if (lines != NULL)
		delete[] lines;

	ClearPoints();
}

void TerrainRail::SwitchDirection()
{
	TerrainPoint *curr = pointEnd;

	TerrainPoint *newNext;
	TerrainPoint *newPrev = NULL;

	while (curr != NULL)
	{
		newNext = curr->prev;
		curr->prev = newPrev;
		curr->next = newNext;

		newPrev = curr;
		curr = newNext;
	}
}

bool TerrainRail::CanApply()
{
	return true;
}

int TerrainRail::GetPointIndex(TerrainPoint *p)
{
	int i = 0;
	TerrainPoint *curr = pointStart;
	while (curr != NULL)
	{
		if (curr == p)
		{
			return i;
		}
		curr = curr->next;
		++i;
	}

	assert(0);

	return -1;
}

TerrainPoint *TerrainRail::GetPointAtIndex(int index)
{
	int i = 0;
	TerrainPoint *curr = pointStart;
	while (curr != NULL)
	{
		if (i == index)
			return curr;

		curr = curr->next;
		++i;
	}

	assert(0);

	return NULL;
}

bool TerrainRail::CanAdd()
{
	return false;
}

void TerrainRail::Deactivate(EditSession *edit, SelectPtr select)
{
	cout << "deactivating rail" << endl;
	RailPtr rail = boost::dynamic_pointer_cast<TerrainRail>(select);

	edit->rails.remove(rail);

	//remove enemies
	for (EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it)
	{
		list<ActorPtr> params = (*it).second;
		for (list<ActorPtr>::iterator pit = params.begin(); pit != params.end(); ++pit)
		{
			SelectPtr ptr = boost::dynamic_pointer_cast<ISelectable>((*pit));
			//(*pit)->Deactivate( edit, ptr );
		}
	}
}

void TerrainRail::Activate(EditSession *edit, SelectPtr select)
{
	RailPtr rail = boost::dynamic_pointer_cast<TerrainRail>(select);

	edit->rails.push_back(rail);

	//add in enemies
	for (EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it)
	{
		list<ActorPtr> params = (*it).second;
		for (list<ActorPtr>::iterator pit = params.begin(); pit != params.end(); ++pit)
		{
			SelectPtr ptr = boost::dynamic_pointer_cast<ISelectable>((*pit));
			//(*pit)->Activate( edit, ptr );
		}
	}
}

void TerrainRail::WriteFile(std::ofstream &of)
{
	int iRequirePower = (int)requirePower;
	int iAccelerate = (int)accelerate;

	of << iRequirePower << endl;
	of << iAccelerate << endl;
	of << level << endl;

	of << numPoints << endl;

	for (TerrainPoint *pcurr = pointStart; pcurr != NULL; pcurr = pcurr->next)
	{
		of << pcurr->pos.x << " " << pcurr->pos.y << endl;
	}
}

void TerrainRail::AlignExtremes(double primLimit)
{
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		TerrainPoint *prev;
		if (curr == pointStart)
		{
			prev = pointEnd;
		}
		else
		{
			prev = curr->prev;
		}

		TerrainPoint *next;
		if (curr == pointEnd)
		{
			next = pointStart;
		}
		else
		{
			next = curr->next;
		}

		V2d prevExtreme(0, 0);
		V2d nextExtreme(0, 0);
		Vector2i prevVec = curr->pos - prev->pos;
		Vector2i nextVec = curr->pos - next->pos;
		V2d prevNormVec = normalize(V2d(prevVec.x, prevVec.y));
		V2d nextNormVec = normalize(V2d(nextVec.x, nextVec.y));

		if (prevNormVec.x > primLimit)
			prevExtreme.x = 1;
		else if (prevNormVec.x < -primLimit)
			prevExtreme.x = -1;
		if (prevNormVec.y > primLimit)
			prevExtreme.y = 1;
		else if (prevNormVec.y < -primLimit)
			prevExtreme.y = -1;

		if (nextNormVec.x > primLimit)
			nextExtreme.x = 1;
		else if (nextNormVec.x < -primLimit)
			nextExtreme.x = -1;
		if (nextNormVec.y > primLimit)
			nextExtreme.y = 1;
		else if (nextNormVec.y < -primLimit)
			nextExtreme.y = -1;

		if (finalized)
		{
			if (!curr->selected)
			{
				continue;
			}

			bool prevValid = true, nextValid = true;
			if (nextNormVec.x == 0 || nextNormVec.y == 0)
			{
				nextValid = false;
			}

			if (prevNormVec.x == 0 || prevNormVec.y == 0)
			{
				prevValid = false;
			}

			if (prevValid && nextValid)
			{
				if (prevExtreme.x != 0)
				{
					if (nextExtreme.x != 0)
					{
						double sum = curr->pos.y + prev->pos.y + next->pos.y;
						int avg = round(sum / 3.0);
						prev->pos.y = avg;
						curr->pos.y = avg;
						next->pos.y = avg;
					}
					else if (nextExtreme.y != 0)
					{
						curr->pos.y = prev->pos.y;
						curr->pos.x = next->pos.x;
					}
					else
					{
						curr->pos.y = prev->pos.y;
					}
				}
				else if (prevExtreme.y != 0)
				{
					if (nextExtreme.y != 0)
					{
						double sum = curr->pos.x + prev->pos.x + next->pos.x;
						int avg = round(sum / 3.0);
						prev->pos.x = avg;
						curr->pos.x = avg;
						next->pos.x = avg;
					}
					else if (nextExtreme.x != 0)
					{
						curr->pos.x = prev->pos.x;
						curr->pos.y = next->pos.y;
					}
					else
					{
						curr->pos.x = prev->pos.x;
					}
				}
			}
			else if (prevValid)
			{
				if (prevExtreme.y != 0)
				{
					double sum = curr->pos.x + prev->pos.x;
					int avg = round(sum / 2.0);
					curr->pos.x = avg;
					prev->pos.x = avg;
				}
				else if (prevExtreme.x != 0)
				{
					double sum = curr->pos.y + prev->pos.y;
					int avg = round(sum / 2.0);
					curr->pos.y = avg;
					prev->pos.y = avg;
				}
			}
			else if (nextValid)
			{
				if (nextExtreme.y != 0)
				{
					double sum = curr->pos.x + next->pos.x;
					int avg = round(sum / 2.0);
					curr->pos.x = avg;
					next->pos.x = avg;
				}
				else if (nextExtreme.x != 0)
				{
					double sum = curr->pos.y + next->pos.y;
					int avg = round(sum / 2.0);
					curr->pos.y = avg;
					next->pos.y = avg;
				}
			}
			else
			{
				continue;
			}
		}
		else
		{
			if (nextNormVec.x == 0 || nextNormVec.y == 0)
			{
				continue;
			}

			if (nextExtreme.x != 0)
			{
				double sum = curr->pos.y + next->pos.y;
				int avg = round(sum / 2.0);
				curr->pos.y = avg;
				next->pos.y = avg;
			}

			if (nextExtreme.y != 0)
			{
				double sum = curr->pos.x + next->pos.x;
				int avg = round(sum / 2.0);
				curr->pos.x = avg;
				next->pos.x = avg;
			}
		}
	}
}

void TerrainRail::Move(SelectPtr me, Vector2i move)
{
	assert(finalized);

	TerrainPoint *curr = pointStart;
	while (curr != NULL)
	{
		TerrainPoint *temp = curr->next;
		curr->pos += move;
		curr = temp;
	}

	for (int i = 0; i < numPoints; ++i)
	{
		//lines
		lines[i * 2].position += Vector2f(move.x, move.y);
		lines[i * 2 + 1].position += Vector2f(move.x, move.y);
	}

	for (EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it)
	{
		list<ActorPtr> &actorList = (*it).second;
		for (list<ActorPtr>::iterator ait = actorList.begin(); ait != actorList.end(); ++ait)
		{
			(*ait)->UpdateGroundedSprite();
			(*ait)->SetBoundingQuad();
		}
	}

	UpdateBounds();
	return;
}

void TerrainRail::UpdateBounds()
{
	TerrainPoint *curr = pointStart;
	left = curr->pos.x;
	right = curr->pos.x;
	top = curr->pos.y;
	bottom = curr->pos.y;
	curr = curr->next;
	while (curr != NULL)
	{
		left = min(curr->pos.x, left);
		right = max(curr->pos.x, right);
		top = min(curr->pos.y, top);
		bottom = max(curr->pos.y, bottom);
		curr = curr->next;
	}

	for (EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it)
	{
		list<ActorPtr> &en = (*it).second;
		for (list<ActorPtr>::iterator ait = en.begin(); ait != en.end(); ++ait)
		{
			sf::VertexArray & bq = (*ait)->boundingQuad;
			for (int i = 0; i < 4; ++i)
			{
				int x = bq[i].position.x;
				int y = bq[i].position.y;
				if (x < left)
				{
					left = x;
				}
				if (x > right)
				{
					right = x;
				}
				if (y < top)
				{
					top = y;
				}
				if (y > bottom)
				{
					bottom = y;
				}
			}
		}
	}
}

void TerrainRail::Finalize()
{
	finalized = true;
	numLineVerts = (numPoints - 1) * 2;
	lines = new sf::Vertex[numLineVerts];

	UpdateLines();

	UpdateBounds();
}

void TerrainRail::RemoveSelectedPoints()
{
	//SoftReset();
	//for (TerrainPoint *curr = pointStart; curr != NULL; )
	//{
	//	if (curr->selected)
	//	{
	//		TerrainPoint *temp = curr->next;
	//		RemovePoint(curr);
	//		curr = temp;
	//	}
	//	else
	//	{
	//		curr = curr->next;
	//	}
	//}
	////Reset();


	//Finalize();
	//SetSelected(false);
}

void TerrainRail::SetSelected(bool select)
{
	selected = select;

	if (selected)
	{
		for (int i = 0; i < numLineVerts; ++i)
		{
			lines[i].color = Color::White;
		}
	}
	else
	{
		for (int i = 0; i < numLineVerts; ++i)
		{
			lines[i].color = Color::Red;
		}

		for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
		{
			curr->selected = false;
		}
	}
}

bool TerrainRail::ContainsPoint(Vector2f test)
{
	int pointCount = numPoints;

	int i, j;
	bool c = false;

	TerrainPoint *curr = pointStart;
	TerrainPoint *next;

	//double zoomMultiple = EditSession::GetSession()->zoomMultiple;
	while( curr != pointEnd )
	{
		next = curr->next;
		if (IsEdgeTouchingCircle(V2d(curr->pos), V2d(next->pos), V2d(test), railRadius))
		{
			return true;
		}
		curr = curr->next;
	}

	return false;
}

void TerrainRail::UpdateLineColor(sf::Vertex *li, TerrainPoint *p, int index)
{
	TerrainPoint *next = p->next;
	if (next == NULL)
	{
		assert(false);
	}

	//Vector2f diff = Vector2f(next->pos - p->pos);//p1 - p0;
	//V2d dir = normalize(V2d(diff));
	//V2d norm = V2d(dir.y, -dir.x);

	Color edgeColor;
	edgeColor = Color::Red;

	lines[index].color = edgeColor;
	lines[index + 1].color = edgeColor;
}

void TerrainRail::UpdateLines()
{
	if (numPoints > 0)
	{
		int i = 0;
		TerrainPoint *curr = pointStart;
		TerrainPoint *next;
		while (curr != pointEnd)
		{
			next = curr->next;

			UpdateLineColor(lines, curr, i);
			lines[i].position = sf::Vector2f(curr->pos.x, curr->pos.y);
			lines[i+1].position = sf::Vector2f(next->pos.x, next->pos.y);
			i += 2;
			curr = curr->next;
		}
	}
}

void TerrainRail::AddPoint(TerrainPoint* tp)
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

void TerrainRail::InsertPoint(TerrainPoint *tp, TerrainPoint *prevPoint)
{
	tp->next = prevPoint->next;
	if (tp->next != NULL)
	{
		tp->next->prev = tp;
	}
	else
	{
		pointEnd = tp;
	}

	prevPoint->next = tp;
	tp->prev = prevPoint;
	++numPoints;
}

void TerrainRail::RemovePoint(TerrainPoint *tp)
{
	assert(pointStart != NULL);

	if (tp->prev != NULL)
	{
		tp->prev->next = tp->next;
	}
	if (tp->next != NULL)
	{
		tp->next->prev = tp->prev;
	}

	if (tp == pointStart)
	{
		pointStart = tp->next;
	}
	if (tp == pointEnd)
	{
		pointEnd = tp->prev;
	}
	--numPoints;
}

TerrainPoint * TerrainRail::HasPointPos(Vector2i &pos)
{
	TerrainPoint *curr = pointStart;
	while (curr != NULL)
	{
		if (curr->pos == pos)
		{
			return curr;
		}
		curr = curr->next;
	}

	return NULL;
}

void TerrainRail::Reset()
{
	ClearPoints();
	SoftReset();
}

void TerrainRail::SoftReset()
{
	if (lines != NULL)
		delete[] lines;

	lines = NULL;
	finalized = false;
}

void TerrainRail::ClearPoints()
{
	TerrainPoint *curr = pointStart;
	while (curr != NULL)
	{
		TerrainPoint *temp = curr->next;
		delete curr;
		curr = temp;
	}

	pointStart = NULL;
	pointEnd = NULL;
	numPoints = 0;
}

void TerrainRail::SetParams(Panel *p)
{
	requirePower = p->checkBoxes["requirepower"]->checked;
	accelerate = p->checkBoxes["accelerate"]->checked;
	string levelStr = p->textBoxes["level"]->text.getString().toAnsiString();

	stringstream ss;
	ss << levelStr;

	int lev;
	ss >> lev;

	if (!ss.fail() && lev > 0 && lev <= MAX_RAIL_LEVEL)
	{
		level = lev;
	}
}

void TerrainRail::UpdatePanel(Panel *p)
{
	p->checkBoxes["requirepower"]->checked = requirePower;
	p->checkBoxes["accelerate"]->checked = accelerate;
	p->textBoxes["level"]->text.setString(to_string(level));
}

//0 means a window came up and they canceled. -1 means no enemies were in danger on that polygon, 1 means that you confirmed to delete the enemies
int TerrainRail::IsRemovePointsOkayEnemies(EditSession *edit)
{
	for (EnemyMap::iterator mapIt = enemies.begin(); mapIt != enemies.end(); ++mapIt)
	{
		for (list<ActorPtr>::iterator it = (*mapIt).second.begin(); it != (*mapIt).second.end(); ++it)
		{
			TerrainPoint *edgeEnd = (*it)->groundInfo->edgeStart->next;
			if (edgeEnd == NULL)
				edgeEnd = (*it)->groundInfo->ground->pointStart;

			if ((*it)->type->CanBeGrounded() &&
				((*(*it)->groundInfo->edgeStart).selected || edgeEnd->selected))
			{
				bool removeSelectedActors = edit->ConfirmationPop("1 or more enemies will be removed by deleting these points.");

				if (removeSelectedActors)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
	}

	return -1;
}

void TerrainRail::MoveSelectedPoints(Vector2i move)
{
	movingPointMode = true;

	int ind = 0;
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		if (curr->selected)
		{
			curr->pos += move;
		}
		UpdateLineColor(lines, curr, ind);
		ind += 2;
	}
}

TerrainRail *TerrainRail::Copy()
{
	TerrainRail *newRail = new TerrainRail(*this);//, true);
	newRail->Finalize();
	return newRail;
}

void TerrainRail::CopyPoints(TerrainRail *rail)
{
	TerrainPoint *start = new TerrainPoint(rail->pointStart->pos, false);
	pointStart = start;

	TerrainPoint *prev = pointStart;
	TerrainPoint *it = rail->pointStart->next;
	TerrainPoint *newPoint;

	for (; it != NULL; it = it->next)
	{
		newPoint = new TerrainPoint(it->pos, false);
		prev->next = newPoint;
		newPoint->prev = prev;
		prev = newPoint;
	}

	pointEnd = prev;
	numPoints = rail->numPoints;
}

void TerrainRail::CopyPoints(TerrainPoint *&start, TerrainPoint *&end)
{
	TerrainPoint *copyCurr = NULL;
	TerrainPoint *copyPrev = NULL;
	TerrainPoint *prev = pointEnd;
	int numNewPoints = 0;
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		copyCurr = new TerrainPoint(curr->pos, false);

		numNewPoints++;

		if (curr == pointStart)
		{
			start = copyCurr;
		}
		else if (curr == pointEnd)
		{
			end = copyCurr;
		}

		if (copyPrev != NULL)
		{
			copyCurr->prev = copyPrev;
			copyPrev->next = copyCurr;
		}
		copyPrev = copyCurr;
	}
}

void TerrainRail::CopyOtherPoints(TerrainPoint *&start,
	TerrainPoint *&end)
{
	for (TerrainPoint *curr = start; curr != NULL; curr = curr->next)
	{
		AddPoint(new TerrainPoint(curr->pos, false));
	}
}

TerrainPoint *TerrainRail::GetClosePoint(double radius, V2d &wPos)
{
	if (wPos.x <= right + radius && wPos.x >= left - radius
		&& wPos.y <= bottom + radius && wPos.y >= top - radius)
	{
		for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
		{
			if (length(wPos - V2d(curr->pos.x, curr->pos.y)) <= radius)
			{
				return curr;
			}
		}
	}
	return NULL;
}

bool TerrainRail::IsPoint(sf::Vector2i &p)
{
	for (TerrainPoint *tp = pointStart; tp != NULL; tp = tp->next)
	{
		if (tp->pos == p)
		{
			return true;
		}
	}
	return false;
}

sf::Rect<int> TerrainRail::TempAABB()
{
	assert(numPoints > 1);
	TerrainPoint *curr = pointStart;
	int l = curr->pos.x;
	int r = curr->pos.x;
	int t = curr->pos.y;
	int b = curr->pos.y;
	curr = curr->next;
	for (; curr != NULL; curr = curr->next)
	{
		l = min(curr->pos.x, l);
		r = max(curr->pos.x, r);
		t = min(curr->pos.y, t);
		b = max(curr->pos.y, b);
	}

	return Rect<int>(l, t, r - l, b - t);
}

bool TerrainRail::Intersects(sf::IntRect rect)
{
	TerrainPoint *curr = pointStart;
	TerrainPoint *next;

	LineIntersection li;

	V2d rPoints[4];
	rPoints[0] = V2d(rect.left, rect.top);
	rPoints[1] = V2d(rect.left + rect.width, rect.top);
	rPoints[2] = V2d(rect.left + rect.width, rect.top + rect.height);
	rPoints[3] = V2d(rect.left, rect.top + rect.height);
	
	int i, j;
	V2d currPos, nextPos;
	while (curr != NULL)
	{
		if (rect.contains(curr->pos))
		{
			return true;
		}

		if (curr == pointEnd)
			return false;

		next = curr->next;
		currPos = V2d(curr->pos);
		nextPos = V2d(next->pos);
		
		for (i = 0; i < 4; ++i)
		{
			j = i - 1;
			if (j < 0)
				j = 3;

			li = SegmentIntersect(currPos, nextPos, rPoints[j], rPoints[i]);
			if (!li.parallel) //error or fail
			{
				return true;
			}
		}
		curr = curr->next;
	}

	return false;
}

void TerrainRail::BrushDraw(sf::RenderTarget *target, bool valid)
{
	target->draw(lines, numPoints * 2, sf::Lines);
}

void TerrainRail::Draw( double zoomMultiple, bool showPoints, sf::RenderTarget *target)
{
	if (movingPointMode)
	{
		int i = 0;
		TerrainPoint *curr = pointStart;
		TerrainPoint *next;
		while (curr != pointEnd)
		{
			next = curr->next;
			/*if (curr->selected)
			{
				cout << "currPos: " << curr->pos.x << ", " << curr->pos.y << endl;
			}*/

			lines[i * 2].position = Vector2f(curr->pos.x, curr->pos.y);
			lines[i * 2 + 1].position = Vector2f(next->pos.x, next->pos.y);

			curr = next;
			++i;
		}

		target->draw(lines, numLineVerts, sf::Lines);
		return;
	}

	target->draw(lines, numLineVerts, sf::Lines);

	if (showPoints)
	{
		CircleShape cs;
		cs.setRadius(8 * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setFillColor(Color::Magenta);

		CircleShape csSel;
		csSel.setRadius(8 * zoomMultiple);
		csSel.setOrigin(csSel.getLocalBounds().width / 2, csSel.getLocalBounds().height / 2);
		csSel.setFillColor(Color::Green);

		for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
		{
			if (curr->selected)
			{
				csSel.setPosition(curr->pos.x, curr->pos.y);
				target->draw(csSel);
			}
			else
			{
				cs.setPosition(curr->pos.x, curr->pos.y);
				target->draw(cs);
			}


		}
	}
}