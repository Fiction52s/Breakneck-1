#include "EditorRail.h"
#include "Physics.h"
#include "EditSession.h"
#include "ActorParams.h"

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

	CopyPointsFromRail(&r);
	//numPoints = r.numPoints;
}

int TerrainRail::GetNumPoints()
{
	return pointVector.size();
}

void TerrainRail::Init()
{
	finalized = false;
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
	std::reverse(pointVector.begin(), pointVector.end());
}

bool TerrainRail::CanApply()
{
	return true;
}

TerrainPoint *TerrainRail::GetPoint(int index)
{
	assert(index >= 0 && index < pointVector.size());
	return &pointVector[index];
}

TerrainPoint *TerrainRail::GetNextPoint(int index)
{
	assert(index >= 0 && index < pointVector.size());
	if (index == GetNumPoints() - 1)
	{
		return NULL;
	}
	else
	{
		return &pointVector[index + 1];
	}
}

TerrainPoint *TerrainRail::GetPrevPoint(int index)
{
	assert(index >= 0 && index < pointVector.size());
	if (index == 0)
	{
		return NULL;
	}
	else
	{
		return &pointVector[index - 1];
	}
}


bool TerrainRail::CanAdd()
{
	return false;
}

void TerrainRail::Deactivate()
{
	EditSession *sess = EditSession::GetSession();
	cout << "deactivating rail" << endl;

	sess->rails.remove(this);
}

void TerrainRail::Activate()
{
	EditSession *sess = EditSession::GetSession();

	sess->rails.push_back(this);
}

void TerrainRail::WriteFile(std::ofstream &of)
{
	int iRequirePower = (int)requirePower;
	int iAccelerate = (int)accelerate;

	of << iRequirePower << endl;
	of << iAccelerate << endl;
	of << level << endl;

	int numP = GetNumPoints();
	of << numP << endl;

	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		of << curr->pos.x << " " << curr->pos.y << endl;
	}
}

void TerrainRail::Reserve(int numP)
{
	pointVector.reserve(numP);
}

void TerrainRail::Load(std::ifstream &is)
{
	int power;
	is >> power;

	int accel;
	is >> accel;

	int lev;
	is >> lev;

	int numRailPoints;
	is >> numRailPoints;

	Reserve(numRailPoints);
	for (int j = 0; j < numRailPoints; ++j)
	{
		int x, y;
		is >> x;
		is >> y;
		AddPoint(Vector2i(x, y), false);
	}

	requirePower = power;
	accelerate = accel;
	level = lev;

	Finalize();
}

bool TerrainRail::AlignExtremes()
{
	vector<PointMoveInfo> emptyLockPoints;
	return AlignExtremes(emptyLockPoints);
}

bool TerrainRail::AlignExtremes(std::vector<PointMoveInfo> &lockPoints)
{
	return false;
	/*double primLimit = EditSession::PRIMARY_LIMIT;
	bool adjustedAtAll = false;

	bool checkPoint;
	bool adjusted = true;

	int lockPointIndex = 0;
	assert(lockPoints.empty() || lockPoints.size() == GetNumPoints());

	EditSession *sess = EditSession::GetSession();

	bool lockPointsEmpty = lockPoints.empty();


	int numP = GetNumPoints();

	while (adjusted)
	{
		adjusted = false;
		lockPointIndex = 0;
		int result;
		bool isPointLocked;

		for (int i = 0; i < numP; ++i, lockPointIndex++)
		{
			isPointLocked = !lockPointsEmpty && lockPoints[lockPointIndex].moveIntent;
			result = FixNearPrimary(i, isPointLocked);

			if (result > 0)
			{
				adjusted = true;
				adjustedAtAll = true;
			}
		}
	}

	return adjustedAtAll;*/
}

void TerrainRail::Move(Vector2i move)
{
	assert(finalized);

	int numP = GetNumPoints();

	TerrainPoint *curr;

	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		curr->pos += move;

		lines[i * 2].position += Vector2f(move.x, move.y);
		lines[i * 2 + 1].position += Vector2f(move.x, move.y);
	}

	for (auto it = enemies.begin(); it != enemies.end(); ++it)
	{
		list<ActorPtr> &actorList = (*it).second;
		for (auto ait = actorList.begin(); ait != actorList.end(); ++ait)
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
	TerrainPoint *curr = GetPoint(0);
	left = curr->pos.x;
	right = curr->pos.x;
	top = curr->pos.y;
	bottom = curr->pos.y;
	
	int numP = GetNumPoints();
	for (int i = 1; i < numP; ++i)
	{
		curr = GetPoint(i);
		left = min(curr->pos.x, left);
		right = max(curr->pos.x, right);
		top = min(curr->pos.y, top);
		bottom = max(curr->pos.y, bottom);
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
	numLineVerts = (GetNumPoints() - 1) * 2;
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

		int numP = GetNumPoints();
		TerrainPoint *curr;
		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);
			curr->selected = false;
		}
	}
}

bool TerrainRail::ContainsPoint(Vector2f test)
{
	return ContainsPoint(test, railRadius);
}

bool TerrainRail::ContainsPoint(sf::Vector2f test, double rad)
{
	int numP = GetNumPoints();

	TerrainPoint *curr, *next;
	for (int i = 0; i < numP-1; ++i)
	{
		curr = GetPoint(i);
		next = GetNextPoint(i);
		if (IsEdgeTouchingCircle(V2d(curr->pos), V2d(next->pos), V2d(test), rad))
		{
			return true;
		}
	}

	return false;
}

void TerrainRail::UpdateLineColor(sf::Vertex *li, int i, int index)
{
	/*TerrainPoint *next = p->next;
	if (next == NULL)
	{
		assert(false);
	}*/

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
	int numP = GetNumPoints();
	if (numP > 0)
	{
		int index = 0;
		TerrainPoint *curr, *next;

		for (int i = 0; i < numP-1; ++i)
		{
			curr = GetPoint(i);
			next = GetNextPoint(i);

			UpdateLineColor(lines, i, index);

			lines[index].position = sf::Vector2f(curr->pos.x, curr->pos.y);
			lines[index + 1].position = sf::Vector2f(next->pos.x, next->pos.y);
			index += 2;
		}
	}
}

TerrainPoint * TerrainRail::AddPoint(sf::Vector2i &p, bool sel)
{
	pointVector.push_back(TerrainPoint(p, sel));
	TerrainPoint *end = GetEndPoint();
	end->index = GetNumPoints() - 1;
	return end;
}

TerrainPoint *TerrainRail::GetEndPoint()
{
	return &pointVector.back();
}

void TerrainRail::RemoveLastPoint()
{
	if (pointVector.empty())
		return;

	TerrainPoint *end = GetEndPoint();

	if (finalized || enemies[end].size() != 0)
	{
		assert(0); //cant remove points when things are attached!
	}
	else
	{
		pointVector.pop_back();
	}
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
	pointVector.clear();
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
	//TerrainPoint *next;
	//for (EnemyMap::iterator mapIt = enemies.begin(); mapIt != enemies.end(); ++mapIt)
	//{
	//	for (list<ActorPtr>::iterator it = (*mapIt).second.begin(); it != (*mapIt).second.end(); ++it)
	//	{
	//		next = (*it)->groundInfo->ground->GetNextPoint((*it)->groundInfo->edgeStart->index);
	//		//if (edgeEnd == NULL)
	//		//	edgeEnd = (*it)->groundInfo->ground->pointStart;

	//		if ((*it)->type->CanBeGrounded() &&
	//			((*(*it)->groundInfo->edgeStart).selected || edgeEnd->selected))
	//		{
	//			bool removeSelectedActors = edit->ConfirmationPop("1 or more enemies will be removed by deleting these points.");

	//			if (removeSelectedActors)
	//			{
	//				return 1;
	//			}
	//			else
	//			{
	//				return 0;
	//			}
	//		}
	//	}
	//}

	return -1;
}

TerrainRail *TerrainRail::Copy()
{
	TerrainRail *newRail = new TerrainRail(*this);//, true);
	newRail->Finalize();
	return newRail;
}

void TerrainRail::CopyPointsFromRail(TerrainRail *rail)
{
	int numP = rail->GetNumPoints();
	Reserve(numP);
	TerrainPoint *rCurr;
	for (int i = 0; i < numP; ++i)
	{
		rCurr = rail->GetPoint(i);
		AddPoint(rCurr->pos, false);
	}
}

void TerrainRail::CopyPointsFromPoly(PolyPtr tp)
{
	int numP = tp->GetNumPoints();
	Reserve(numP);
	TerrainPoint *polyCurr;
	for (int i = 0; i < numP; ++i)
	{
		polyCurr = tp->GetPoint(i);
		AddPoint(polyCurr->pos, false);
	}
}

TerrainPoint *TerrainRail::GetClosePoint(double radius, V2d &wPos)
{
	if (wPos.x <= right + radius && wPos.x >= left - radius
		&& wPos.y <= bottom + radius && wPos.y >= top - radius)
	{
		int numP = GetNumPoints();
		TerrainPoint *curr;
		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);
			if (length(wPos - V2d(curr->pos.x, curr->pos.y)) <= radius)
			{
				return curr;
			}
		}
	}
	return NULL;
}

bool TerrainRail::Intersects(sf::IntRect rect)
{
	LineIntersection li;

	V2d rPoints[4];
	rPoints[0] = V2d(rect.left, rect.top);
	rPoints[1] = V2d(rect.left + rect.width, rect.top);
	rPoints[2] = V2d(rect.left + rect.width, rect.top + rect.height);
	rPoints[3] = V2d(rect.left, rect.top + rect.height);
	
	int j, k;

	TerrainPoint *curr, *next;
	V2d currPos, nextPos;
	int numP = GetNumPoints();
	for (int i = 0; i < numP-1; ++i)
	{
		curr = GetPoint(i);
		if (rect.contains(curr->pos))
		{
			return true;
		}

		next = GetNextPoint(i);

		if (rect.contains(next->pos))
		{
			return true;
		}

		currPos = V2d(curr->pos);
		nextPos = V2d(next->pos);

		for (k = 0; k < 4; ++k)
		{
			j = k - 1;
			if (j < 0)
				j = 3;

			li = SegmentIntersect(currPos, nextPos, rPoints[j], rPoints[k]);
			if (!li.parallel) //error or fail
			{
				return true;
			}
		}
	}

	return false;
}

void TerrainRail::BrushDraw(sf::RenderTarget *target, bool valid)
{
	target->draw(lines, GetNumPoints() * 2, sf::Lines);
}

void TerrainRail::Draw( double zoomMultiple, bool showPoints, sf::RenderTarget *target)
{
	int numP = GetNumPoints();
	if (movingPointMode)
	{
		int index = 0;
		TerrainPoint *curr, *next;

		
		for (int i = 0; i < numP - 1; ++i)
		{
			curr = GetPoint(i);
			next = GetNextPoint(i);

			lines[i * 2].position = Vector2f(curr->pos.x, curr->pos.y);
			lines[i * 2 + 1].position = Vector2f(next->pos.x, next->pos.y);
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


		TerrainPoint *curr;
		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);
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