#include "EditorRail.h"
#include "Physics.h"
#include "EditSession.h"
#include "ActorParams.h"
#include "TransformTools.h"
#include "Action.h"
#include "CircleGroup.h"
#include "Enemy_Blocker.h"

#include "EnemyChain.h"

using namespace std;
using namespace sf;

const std::string TerrainRail::railTypeNames[] = { 
"Terrain",
"Floor",
"Ceiling",
"Bounce",
"ScorpionOnly",
"Grind",
"Phase",
"InversePhase",
"Accelerate",
"Fade",
"Locked",
"Anti-Timeslow",
"WireOnly",
"WireBlocking",
"Hit" };

TerrainRail::TerrainRail()
	:ISelectable(ISelectable::RAIL)
{
	Init();
}

TerrainRail::TerrainRail(TerrainRail &r)
	:ISelectable( ISelectable::RAIL )
{
	Init();

	rType = r.rType;

	CopyPointsFromRail(&r);
}

int TerrainRail::GetNumPoints()
{
	return pointVector.size();
}

Edge *TerrainRail::GetEdge(int index)
{
	int numEdges = (GetNumPoints() - 1) * 2;
	if (index >= 0 && index < numEdges)
	{
		return &edges[index];
	}
	else
	{
		return NULL;
	}
}

void TerrainRail::CalcEdgeAABBs()
{
	int numP = GetNumPoints();
	int numEdges = (numP - 1) * 2;
	for (int i = 0; i < numEdges; ++i)
	{
		edges[i].CalcAABB();
	}
}

void TerrainRail::SetupEdges()
{
	int numP = GetNumPoints();
	int numEdgesPerSide = numP - 1;
	edges.resize(numEdgesPerSide * 2);
	TerrainPoint *curr, *next;
	for (int i = 0; i < numEdgesPerSide; ++i)
	{
		curr = GetPoint(i);
		next = GetPoint(i + 1);

		edges[i].v0 = V2d(curr->pos);
		edges[i].v1 = V2d(next->pos);
		edges[i].rail = this;
	}

	for (int i = 0; i < numEdgesPerSide; ++i)
	{
		if (i > 0)
		{
			edges[i].edge0 = &edges[i - 1];
		}
		else
		{
			edges[i].edge0 = NULL;
		}

		if (i < numP - 2)
		{
			edges[i].edge1 = &edges[i + 1];
		}
		else
		{
			edges[i].edge1 = NULL;
		}
	}

	//underside
	int eIndex, pIndex;
	for (int i = 0; i < numEdgesPerSide; ++i)
	{
		eIndex = i + numEdgesPerSide;
		pIndex = numEdgesPerSide - i;
		curr = GetPoint(pIndex);
		next = GetPoint(pIndex - 1);

		edges[eIndex].v0 = V2d(curr->pos);
		edges[eIndex].v1 = V2d(next->pos);
		edges[eIndex].rail = this;
	}

	for (int i = 0; i < numEdgesPerSide; ++i)
	{
		eIndex = i + numEdgesPerSide;
		if (i > 0)
		{
			edges[eIndex].edge0 = &edges[eIndex - 1];
		}
		else
		{
			edges[eIndex].edge0 = NULL;
		}

		if (i < numEdgesPerSide - 1)
		{
			edges[eIndex].edge1 = &edges[eIndex + 1];
		}
		else
		{
			edges[eIndex].edge1 = NULL;
		}
	}
}


bool TerrainRail::PointsTooCloseToEachOther(double radius)
{
	int numP = GetNumPoints();
	TerrainPoint *currI, *currJ;
	for (int i = 0; i < numP; ++i)
	{
		currI = GetPoint(i);
		for (int j = 0; j < numP; ++j)
		{
			if (i == j)
				continue;

			currJ = GetPoint(j);
			V2d a(currI->pos.x, currI->pos.y);
			V2d b(currJ->pos.x, currJ->pos.y);
			if (length(a - b) < radius)
			{
				return true;
			}
		}
	}

	return false;
}

bool TerrainRail::IsInternallyValid()
{
	if (PointsTooCloseToEachOther(1) )
	{
		return false;
	}
	return true;
}

void TerrainRail::AddEdgesToQuadTree(QuadTree *tree)
{
	CalcEdgeAABBs();
	int numP = GetNumPoints();
	int numEdges = (numP - 1) * 2;
	Edge *e;
	for (int i = 0; i < numEdges; ++i)
	{
		e = GetEdge(i);
		e->edgeIndex = i;
		tree->Insert(e);
	}
}

void TerrainRail::SetRailType(int r)
{
	rType = r;
}

bool TerrainRail::IsEdgeActive(Edge *e)
{
	switch (rType)
	{
	case FLOORANDCEILING:
	case PHASE:
	case FADE:
	case INVERSEPHASE:
		return true;
	case FLOOR:
		if (e->Normal().y <= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	case CEILING:
		if (e->Normal().y >= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	case BOUNCE:
	case SCORPIONONLY:
		return true;
	}

	return false;
}

bool TerrainRail::IsTerrainType()
{
	return rType == FLOORANDCEILING || rType == FLOOR || rType == CEILING
		|| rType == BOUNCE || rType == SCORPIONONLY || rType == PHASE 
		|| rType == INVERSEPHASE || rType == FADE;
}

void TerrainRail::Init()
{	
	sess = Session::GetSession();
	railIndex = -1;
	ts_rail = sess->GetSizedTileset("Env/rails_128x64.png");
	queryNext = NULL;
	quadHalfWidth = 6;
	renderMode = RENDERMODE_NORMAL;
	finalized = false;
	lines = NULL;
	coloredQuads = NULL;
	coloredNodeCircles = NULL;
	texturedQuads = NULL;
	numTexturedQuads = 0;
	selected = false;
	railRadius = 20;
	
	rType = FLOORANDCEILING;
	

	enemyParams = NULL;
	enemyChain = NULL;

	ResetState();
}

TerrainRail::~TerrainRail()
{
	SoftReset();

	ClearPoints();

	if (enemyParams != NULL)
	{
		delete enemyParams;
	}
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
	if( index >= 0 && index < GetNumPoints() )
	{
		return &pointVector[index];
	}
	else
	{
		return NULL;
	}
}

int TerrainRail::GetRailIndexFromString(const std::string &s)
{
	if (s == "FLOORANDCEILING")
	{
		return FLOORANDCEILING;
	}
	else if (s == "FLOOR")
	{
		return FLOOR;
	}
	else if (s == "CEILING")
	{
		return CEILING;
	}
	else if (s == "BOUNCE")
	{
		return BOUNCE;
	}
	else if (s == "SCORPIONONLY")
	{
		return SCORPIONONLY;
	}
	else if (s == "GRIND")
	{
		return GRIND;
	}
	else if (s == "PHASE")
	{
		return PHASE;
	}
	else if (s == "INVERSEPHASE")
	{
		return INVERSEPHASE;
	}
	else if (s == "ACCELERATE")
	{
		return ACCELERATE;
	}
	else if (s == "FADE")
	{
		return FADE;
	}
	else if (s == "LOCKED")
	{
		return LOCKED;
	}
	else if (s == "ANTITIMESLOW")
	{
		return ANTITIMESLOW;
	}
	else if (s == "WIREONLY")
	{
		return WIREONLY;
	}
	else if (s == "WIREBLOCKING")
	{
		return WIREBLOCKING;
	}
	else if (s == "HIT")
	{
		return HIT;
	}
	else
	{
		cout << "rail string was not matched: " << s << endl;
		assert(0);
		return -1;
	}
}

bool TerrainRail::CanAdd()
{
	return false;
}

void TerrainRail::Deactivate()
{
	active = false;
	EditSession *sess = EditSession::GetSession();
	
	cout << "deactivating rail :" << sess->rails.size() << endl;
	sess->rails.remove(this);

}

void TerrainRail::Activate()
{
	active = true;
	EditSession *sess = EditSession::GetSession();

	cout << "activating rail :" << sess->rails.size() << endl;
	sess->rails.push_back(this);

}

bool TerrainRail::IsEnemyType()
{
	return rType >= RailType::BLOCKER;
}

void TerrainRail::WriteFile(std::ofstream &of)
{
	of << rType << "\n";

	if (IsEnemyType())
	{
		enemyParams->WriteFile(of);
	}

	int numP = GetNumPoints();
	of << numP << "\n";

	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		of << curr->pos.x << " " << curr->pos.y << "\n";
	}
}

void TerrainRail::Reserve(int numP)
{
	pointVector.reserve(numP);
}

void TerrainRail::Load(std::ifstream &is)
{
	is >> rType;

	if (IsEnemyType())
	{
		is >> enemyTypeName;

		//ActorType *at = sess->types[typeString];


		//enemyParams = sess->types[typeString]->defaultParamsVec[0]->Copy();
		//enemyParams->group = sess->groups["--"];

	//SetChainPath();

	//enemyParams->CreateMyEnemy();
	//enemyChain = (EnemyChain*)enemyParams->myEnemy;

		//ActorPtr a(NULL);
		//at->LoadEnemy(is, a);
		//defaultParamsVec[0]->Copy();
		//enemyParams->group = sess->groups["--"];
	}

	//a little messy but it works for these. Shouldn't need to do anything more special
	//this allows for special parameters for each enemy to be saved, such as spacing between
	//flies/blockers
	if (rType == RailType::BLOCKER )
	{
		enemyParams = new BlockerParams(sess->types[enemyTypeName], is);
	}
	else if (rType == RailType::FLY)
	{
		enemyParams = new FlyParams(sess->types["healthfly"], is);
	}

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

	Finalize();
}

bool TerrainRail::RequiresPowerToGrind()
{
	return false; //for now
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

	Vector2f moveF(move);
	V2d moveD(move);

	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		curr->pos += move;

		if (i < numP - 1)
		{
			lines[i * 2].position += moveF;
			lines[i * 2 + 1].position += moveF;

			coloredQuads[i * 4].position += moveF;
			coloredQuads[i * 4 + 1].position += moveF;
			coloredQuads[i * 4 + 2].position += moveF;
			coloredQuads[i * 4 + 3].position += moveF;

			edges[i].v0 += moveD;
			edges[i].v1 += moveD;
			edges[i + (numP - 1)].v0 += moveD;
			edges[i + (numP - 1)].v1 += moveD;
		}

		coloredNodeCircles->SetPosition(i, coloredNodeCircles->GetPosition(i) + moveF);
	}

	for (int i = 0; i < numTexturedQuads; ++i)
	{
		texturedQuads[i * 4 + 0].position += moveF;
		texturedQuads[i * 4 + 1].position += moveF;
		texturedQuads[i * 4 + 2].position += moveF;
		texturedQuads[i * 4 + 3].position += moveF;
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

	if (enemyParams != NULL)
	{
		enemyParams->Move(move);
	}

	/*if (blockerParams->myEnemy != NULL)
	{
		blockerParams->myEnemy->UpdateOnEditPlacement();
		blockerParams->myEnemy->UpdateFromEditParams(0);
	}*/

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

	double padding = 30; //to allow flat rails to have a bounding box for queries
	//has to do w/ drawing too.

	aabb.left = left - padding;
	aabb.top = top - padding;
	aabb.width = (right - left) + padding * 2;
	aabb.height = (bottom - top) + padding * 2;
}

sf::Color TerrainRail::GetRailColor()
{
	/*switch (rType)
	{
	case NORMAL:
		return Color::Red;
	case LOCKED:
		return Color( 104, 255, 0 );
	case ANTITIMESLOW:
		return Color(0, 211, 255);
	case WIREONLY:
		return Color(238, 83, 169);
	case WIREBLOCKING:
		return Color::White;
	case FLOORANDCEILING:
		return Color(155, 173, 183);
	case FLOOR:
		return Color(223, 113, 38);
	case CEILING:
		return Color(55, 148, 110);
	case BOUNCE:
		return Color(251, 242, 54);
	case SCORPIONONLY:
		return Color(138, 111, 48);
	}*/

	return Color::White;
}

void TerrainRail::UpdateTexturedQuads()
{
	double startQuant = 0;
	double endQuant = 0;
	int startIndex = 0;
	V2d startCenter;
	V2d endCenter;
	V2d eNorm;
	IntRect subRect = ts_rail->GetSubRect(rType);
	Edge *curr, *next;
	int numQuadsForEdge;

	int numSmoothQuads = 3;
	int numP = GetNumPoints();

	double railTileWidth = ts_rail->tileWidth;
	double railTileHeight = ts_rail->tileHeight;

	double out = ts_rail->tileHeight / 2.0;
	double in = ts_rail->tileHeight / 2.0;

	/*for (int i = 0; i < numEdgesPerSide; ++i)
	{
		curr = GetPoint(i);
		next = GetPoint(i + 1);

		edges[i].v0 = V2d(curr->pos);
		edges[i].v1 = V2d(next->pos);
		edges[i].rail = this;
	}*/

	for (int i = 0; i < numP - 1; ++i)
	{
		curr = GetEdge(i);
		next = curr->GetNextEdge();

		eNorm = curr->Normal();
		//next = curr->GetNextEdge();

		numQuadsForEdge = ceil(curr->GetLength() / railTileWidth);

		startQuant = 0;

		for (int j = 0; j < numQuadsForEdge; ++j)
		{
			startCenter = curr->GetPosition(startQuant);

			if (j == numQuadsForEdge - 1)
			{
				endQuant = curr->GetLength();
				endCenter = curr->v1;
			}
			else
			{
				endQuant = startQuant + railTileWidth;
				endCenter = curr->GetPosition(endQuant);
			}

			texturedQuads[startIndex + j * 4 + 0].position =
				Vector2f(startCenter + eNorm * railTileHeight / 2.0);
			texturedQuads[startIndex + j * 4 + 1].position =
				Vector2f(endCenter + eNorm * railTileHeight / 2.0);
			texturedQuads[startIndex + j * 4 + 2].position =
				Vector2f(endCenter - eNorm * railTileHeight / 2.0);
			texturedQuads[startIndex + j * 4 + 3].position =
				Vector2f(startCenter - eNorm * railTileHeight / 2.0);

			texturedQuads[startIndex + j * 4 + 0].texCoords =
				Vector2f(subRect.left, subRect.top);
			texturedQuads[startIndex + j * 4 + 1].texCoords =
				Vector2f(subRect.left + (endQuant - startQuant), subRect.top);
			texturedQuads[startIndex + j * 4 + 2].texCoords =
				Vector2f(subRect.left + (endQuant - startQuant), subRect.top + subRect.height);
			texturedQuads[startIndex + j * 4 + 3].texCoords =
				Vector2f(subRect.left, subRect.top + subRect.height);


			startQuant += railTileWidth;
		}

		if (next != NULL && next->Normal() != curr->Normal())
		{
			V2d nextNorm = next->Normal();

			V2d centerPoint = curr->v1;

			double c = cross(next->Along(), curr->Along());

			int quadIndex = startIndex + numQuadsForEdge * 4;

			V2d startVec, endVec;

			if (c < 0)
			{
				startVec = eNorm * out;
				endVec = nextNorm * out;
			}
			else
			{
				startVec = -nextNorm * out;
				endVec = -eNorm * out;
			}

			V2d nextVec;
			double angleDiff = GetVectorAngleDiffCW(startVec, endVec);
			for (int k = 0; k < numSmoothQuads; ++k)
			{
				nextVec = startVec;
				RotateCW(nextVec, angleDiff / numSmoothQuads);
				texturedQuads[quadIndex + k * 4 + 0].position = Vector2f(curr->v1 + startVec);
				texturedQuads[quadIndex + k * 4 + 1].position = Vector2f(curr->v1 + nextVec);
				texturedQuads[quadIndex + k * 4 + 2].position = Vector2f(centerPoint);
				texturedQuads[quadIndex + k * 4 + 3].position = Vector2f(centerPoint);

				float outerWidth = length(
					texturedQuads[quadIndex + k * 4 + 1].position -
					texturedQuads[quadIndex + k * 4 + 0].position);
				texturedQuads[quadIndex + k * 4 + 0].texCoords = Vector2f(subRect.left, subRect.top);
				texturedQuads[quadIndex + k * 4 + 1].texCoords = Vector2f(subRect.left + outerWidth, subRect.top);
				texturedQuads[quadIndex + k * 4 + 2].texCoords = Vector2f(subRect.left + outerWidth / 2, subRect.top + out);
				texturedQuads[quadIndex + k * 4 + 3].texCoords = Vector2f(subRect.left + outerWidth / 2, subRect.top + out);
				startVec = nextVec;
			}

			numQuadsForEdge += numSmoothQuads;

		}

		startIndex += numQuadsForEdge * 4;		
	}
}

void TerrainRail::Finalize()
{
	finalized = true;
	int numP = GetNumPoints();
	numLineVerts = (numP - 1) * 2;
	numColoredQuads = (numP - 1);
	lines = new sf::Vertex[numLineVerts];

	coloredQuads = new sf::Vertex[numColoredQuads * 4];


	coloredNodeCircles = new CircleGroup(numP, quadHalfWidth, GetRailColor(), 12);
	coloredNodeCircles->ShowAll();

	SetupEdges();

	Edge *curr, *next;
	numTexturedQuads = 0;

	int numQuadsForEdge;

	double railTileWidth = ts_rail->tileWidth;
	double railTileHeight = ts_rail->tileHeight;

	int numSmoothQuads = 3;

	for (int i = 0; i < numP - 1; ++i)
	{
		curr = GetEdge(i);
		next = curr->GetNextEdge();

		

		numTexturedQuads += ceil(curr->GetLength() / railTileWidth);

		if ( next != NULL && next->Normal() != curr->Normal())
		{
			numTexturedQuads += numSmoothQuads;
		}
	}

	texturedQuads = new Vertex[numTexturedQuads * 4];

	double startQuant = 0;
	double endQuant = 0;
	int startIndex = 0;
	V2d startCenter;
	V2d endCenter;
	V2d eNorm;
	IntRect subRect = ts_rail->GetSubRect(rType);

	double out = ts_rail->tileHeight / 2.0;
	double in = ts_rail->tileHeight / 2.0;
	for (int i = 0; i < numP - 1; ++i)
	{
		curr = GetEdge(i);
		next = curr->GetNextEdge();
		eNorm = curr->Normal();
		//next = curr->GetNextEdge();

		numQuadsForEdge = ceil(curr->GetLength() / railTileWidth);

		startQuant = 0;

		for (int j = 0; j < numQuadsForEdge; ++j)
		{
			startCenter = curr->GetPosition(startQuant);

			if (j == numQuadsForEdge - 1)
			{
				endQuant = curr->GetLength();
				endCenter = curr->v1;
			}
			else
			{
				endQuant = startQuant + railTileWidth;
				endCenter = curr->GetPosition(endQuant);
			}

			texturedQuads[startIndex + j * 4 + 0].position =
				Vector2f(startCenter + eNorm * railTileHeight / 2.0);
			texturedQuads[startIndex + j * 4 + 1].position =
				Vector2f(endCenter + eNorm * railTileHeight / 2.0);
			texturedQuads[startIndex + j * 4 + 2].position =
				Vector2f(endCenter - eNorm * railTileHeight / 2.0);
			texturedQuads[startIndex + j * 4 + 3].position =
				Vector2f(startCenter - eNorm * railTileHeight / 2.0);
			
			texturedQuads[startIndex + j * 4 + 0].texCoords =
				Vector2f(subRect.left, subRect.top);
			texturedQuads[startIndex + j * 4 + 1].texCoords =
				Vector2f(subRect.left + (endQuant - startQuant), subRect.top);
			texturedQuads[startIndex + j * 4 + 2].texCoords =
				Vector2f(subRect.left + (endQuant - startQuant), subRect.top + subRect.height);
			texturedQuads[startIndex + j * 4 + 3].texCoords =
				Vector2f(subRect.left, subRect.top + subRect.height);
			

			startQuant += railTileWidth;
		}

		if (next != NULL && next->Normal() != curr->Normal())
		{
			V2d nextNorm = next->Normal();

			V2d centerPoint = curr->v1;

			double c = cross(next->Along(), curr->Along());

			int quadIndex = startIndex + numQuadsForEdge * 4;

			V2d startVec, endVec;

			if (c < 0)
			{
				startVec = eNorm * out;
				endVec = nextNorm * out;
			}
			else
			{
				startVec = -nextNorm * out;
				endVec = -eNorm * out;
			}

			V2d nextVec;
			double angleDiff = GetVectorAngleDiffCW(startVec, endVec);
			for (int k = 0; k < numSmoothQuads; ++k)
			{
				nextVec = startVec;
				RotateCW(nextVec, angleDiff / numSmoothQuads);
				texturedQuads[quadIndex + k * 4 + 0].position = Vector2f(curr->v1 + startVec);
				texturedQuads[quadIndex + k * 4 + 1].position = Vector2f(curr->v1 + nextVec);
				texturedQuads[quadIndex + k * 4 + 2].position = Vector2f(centerPoint);
				texturedQuads[quadIndex + k * 4 + 3].position = Vector2f(centerPoint);

				float outerWidth = length(
					texturedQuads[quadIndex + k * 4 + 1].position -
					texturedQuads[quadIndex + k * 4 + 0].position);
				texturedQuads[quadIndex + k * 4 + 0].texCoords = Vector2f(subRect.left, subRect.top);
				texturedQuads[quadIndex + k * 4 + 1].texCoords = Vector2f(subRect.left + outerWidth, subRect.top);
				texturedQuads[quadIndex + k * 4 + 2].texCoords = Vector2f(subRect.left + outerWidth / 2, subRect.top + out);
				texturedQuads[quadIndex + k * 4 + 3].texCoords = Vector2f(subRect.left + outerWidth / 2, subRect.top + out);
				startVec = nextVec;
			}

			numQuadsForEdge += numSmoothQuads;

		}

		startIndex += numQuadsForEdge * 4;
	}

	UpdateLines();

	UpdateBounds();

	TryCreateEnemyChain();
}

int TerrainRail::GetRailType()
{
	return rType; 
}

const std::string &TerrainRail::GetRailTypeName()
{
	return railTypeNames[rType];
}

void TerrainRail::AddEnemyChainToWorldTrees()
{
	if (enemyChain != NULL)
	{
		enemyChain->AddToWorldTrees();
	}
}

void TerrainRail::SetChainPath()
{
	if (enemyParams != NULL)
	{
		enemyParams->SetPosition(V2d(GetPoint(0)->pos));
		enemyParams->SetPath(this);
		enemyParams->SetBoundingQuad();
	}
}

void TerrainRail::TryCreateEnemyChain()
{
	if (!IsEnemyType())
	{
		return;
	}

	/*string typeString;
	if (rType == BLOCKER)
	{
		typeString = "blocker";

	}
	else if (rType == FLY)
	{
		typeString = "healthfly";
	}
	else
	{
		return;
	}*/

	if (enemyParams != NULL)
	{
		//delete enemyParams;
		//enemyParams = NULL;
	}
	else
	{
		enemyParams = sess->types[enemyTypeName]->defaultParamsVec[0]->Copy();
		enemyParams->group = sess->groups["--"];
	}

	SetChainPath();

	if (enemyParams->myEnemy == NULL)
	{
		enemyParams->CreateMyEnemy();
		enemyChain = (EnemyChain*)enemyParams->myEnemy;
	}
}

int TerrainRail::GetNumSelectedPoints()
{
	int numP = GetNumPoints();
	int numSelected = 0;
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		if (curr->selected)
		{
			++numSelected;
		}
	}

	return numSelected;
}

void TerrainRail::FillSelectedIndexBuffer(
	std::vector<int> &selectedIndexes)
{
	int numP = GetNumPoints();
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		if (curr->selected)
		{
			selectedIndexes.push_back(i);
		}
	}
}

void TerrainRail::AddEnemiesToBrush(Brush *b)
{
	for (auto mit = enemies.begin(); mit != enemies.end(); ++mit)
	{
		for (auto eit = (*mit).second.begin(); eit != (*mit).second.end(); ++eit)
		{
			b->AddObject((*eit));
		}
	}
}

void TerrainRail::CreateNewRailsWithSelectedPointsRemoved( list<RailPtr> &rails )
{
	int numP = GetNumPoints();
	int numDeletePoints = GetNumSelectedPoints();

	vector<int> selectedIndexes;
	selectedIndexes.reserve(numDeletePoints);
	FillSelectedIndexBuffer(selectedIndexes);

	sort(selectedIndexes.begin(), selectedIndexes.end());

	int startIndex = 0;

	int indexMarker = 0;
	int currSelectedIndex = selectedIndexes[indexMarker];

	int diff;
	while (startIndex < numP - 1)
	{
		diff = currSelectedIndex - startIndex;
		if( diff >= 2 )
		{
			RailPtr newRail = new TerrainRail;
			newRail->Reserve(diff);
			for (int i = startIndex; i < currSelectedIndex; ++i)
			{
				newRail->AddPoint(GetPoint(i)->pos, false);
			}
			newRail->rType = rType;
			rails.push_back(newRail);
		}
		startIndex = currSelectedIndex + 1;
		++indexMarker;

		if (indexMarker > numDeletePoints)
		{
			break;
		}
		else if (indexMarker == numDeletePoints)
		{
			currSelectedIndex = numP;
		}
		else
		{
			currSelectedIndex = selectedIndexes[indexMarker];
		}
	}
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
			lines[i].color = GetRailColor();
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
		next = GetPoint(i+1);
		if (IsEdgeTouchingCircle(V2d(curr->pos), V2d(next->pos), V2d(test), rad))
		{
			return true;
		}
	}

	return false;
}

void TerrainRail::UpdateLineColor(sf::Vertex *li, int i, int index)
{
	//TerrainPoint *curr, *next;

	//Vector2f diff;

	//int nextIndex = index + 1;

	//if (renderMode == RENDERMODE_TRANSFORM)
	//{
	//	Vector2f roundedNextPos(round(lines[nextIndex].position.x), round(lines[nextIndex].position.y));
	//	Vector2f roundedPos(round(lines[index].position.x), round(lines[index].position.y));

	//	diff = roundedNextPos - roundedPos;

	//}
	//else
	//{
	//	curr = GetPoint(i);
	//	next = GetPoint(i+1);

	//	diff = Vector2f(next->pos - curr->pos);
	//}

	//V2d dir = normalize(V2d(diff));
	//V2d norm = V2d(dir.y, -dir.x);

	////static EdgeAngleType GetEdgeAngleType(Edge * e);
	////static EdgeAngleType GetEdgeAngleType(V2d &normal);

	//TerrainPolygon::EdgeAngleType eat = TerrainPolygon::GetEdgeAngleType(norm);

	//Color edgeColor;
	//switch (eat)
	//{
	//case TerrainPolygon::EDGE_FLAT:
	//	edgeColor = Color::Red;
	//	break;
	//case TerrainPolygon::EDGE_SLOPED:
	//	edgeColor = Color::Green;
	//	break;
	//case TerrainPolygon::EDGE_STEEPSLOPE:
	//	edgeColor = Color::White;
	//	break;
	//case TerrainPolygon::EDGE_WALL:
	//	edgeColor = Color::Magenta;
	//	break;
	//case TerrainPolygon::EDGE_STEEPCEILING:
	//	edgeColor = Color::Yellow;
	//	break;
	//case TerrainPolygon::EDGE_SLOPEDCEILING:
	//	edgeColor = Color::Cyan;
	//	break;
	//case TerrainPolygon::EDGE_FLATCEILING:
	//	edgeColor = Color::Red;
	//	break;
	//}

	//lines[index].color = edgeColor;
	//lines[nextIndex].color = edgeColor;

	Color edgeColor = GetRailColor();

	lines[index].color = edgeColor;
	lines[index + 1].color = edgeColor;
}

void TerrainRail::UpdateColoredQuads()
{
	int numP = GetNumPoints();
	assert(numP > 1);
	
	Edge *edge;
	V2d along;
	V2d other;
	
	double dQuadHalfWidth = quadHalfWidth;

	Color quadColor = GetRailColor();

	for (int i = 0; i < numP - 1; ++i)
	{
		edge = GetEdge(i);

		along = edge->Along();//normalize(Vector2f(next->pos - curr->pos));
		other = edge->Normal();

		coloredQuads[i * 4].position = Vector2f(edge->v0 + other * dQuadHalfWidth);
		coloredQuads[i * 4 + 1].position = Vector2f(edge->v1 + other * dQuadHalfWidth);
		coloredQuads[i * 4 + 2].position = Vector2f(edge->v1 - other * dQuadHalfWidth);
		coloredQuads[i * 4 + 3].position = Vector2f(edge->v0 - other * dQuadHalfWidth);

		coloredQuads[i * 4].color = quadColor;
		coloredQuads[i * 4 + 1].color = quadColor;
		coloredQuads[i * 4 + 2].color = quadColor;
		coloredQuads[i * 4 + 3].color = quadColor;
	}

	for (int i = 0; i < numP; ++i)
	{
		coloredNodeCircles->SetPosition(i, Vector2f(GetPoint(i)->pos));
		coloredNodeCircles->SetColor(i, GetRailColor());
	}
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
			next = GetPoint(i+1);

			UpdateLineColor(lines, i, index);

			lines[index].position = sf::Vector2f(curr->pos.x, curr->pos.y);
			lines[index + 1].position = sf::Vector2f(next->pos.x, next->pos.y);
			index += 2;
		}
	}

	UpdateColoredQuads();
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

	if (coloredQuads != NULL)
		delete[] coloredQuads;

	if (coloredNodeCircles != NULL)
		delete coloredNodeCircles;

	if (texturedQuads != NULL)
	{
		delete[] texturedQuads;
	}

	lines = NULL;
	finalized = false;
}

void TerrainRail::ClearPoints()
{
	pointVector.clear();
}

void TerrainRail::SetParams(Panel *p)
{

}

void TerrainRail::UpdatePanel(Panel *p)
{
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

void TerrainRail::BackupEnemyPositions()
{
	enemyPosBackups.clear();
	int totalEnemies = 0;
	for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); ++enemyIt)
	{
		auto &aList = (*enemyIt).second;
		if (!aList.empty())
		{
			for (auto it = aList.begin(); it != aList.end(); ++it)
			{
				totalEnemies++;
				//backupEnemyPosInfos[(*it)] = ((*it)->posInfo);
			}
		}
	}

	enemyPosBackups.reserve(totalEnemies);
	for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); ++enemyIt)
	{
		auto &aList = (*enemyIt).second;
		if (!aList.empty())
		{
			for (auto it = aList.begin(); it != aList.end(); ++it)
			{
				enemyPosBackups.push_back(make_pair((*it), (*it)->posInfo));
			}
		}
	}
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

		next = GetPoint(i+1);

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

void TerrainRail::BrushSave(std::ofstream &of)
{
	of << ISelectable::RAIL << "\n";

	WriteFile(of);
}

void TerrainRail::BrushDraw(sf::RenderTarget *target, bool valid)
{
	Draw(target);
	//target->draw(coloredQuads, numColoredQuads * 4, sf::Quads);
	//coloredNodeCircles->Draw(target);

	//target->draw(lines, numLineVerts, sf::Lines);
}

void TerrainRail::MovePoint(int index, sf::Vector2i &delta)
{
	SetPointPos(index, GetPoint(index)->pos + delta);
}

ActorPtr TerrainRail::GetClosestEnemy(int index, double &minQuant)
{
	ActorPtr minActor = NULL;
	double currMinQuant;
	if (enemies.find(GetPoint(index)) != enemies.end())
	{
		list<ActorPtr> &actorList = enemies[GetPoint(index)];
		for (auto it = actorList.begin(); it != actorList.end(); ++it)
		{
			currMinQuant = (*it)->posInfo.groundQuantity;// -(*it)->GetSize().x / 2;
			if (it == actorList.begin())
			{
				minQuant = currMinQuant;
				minActor = (*it);
			}
			else
			{
				if (currMinQuant < minQuant)
				{
					minQuant = currMinQuant;
					minActor = (*it);
				}
			}

		}
	}

	return minActor;
}

ActorPtr TerrainRail::GetFurthestEnemy(int index, double &maxQuant)
{
	ActorPtr maxActor = NULL;
	double currMaxQuant;
	if (enemies.find(GetPoint(index)) != enemies.end())
	{
		list<ActorPtr> &actorList = enemies[GetPoint(index)];
		for (auto it = actorList.begin(); it != actorList.end(); ++it)
		{
			currMaxQuant = (*it)->posInfo.groundQuantity;// +(*it)->GetSize().x / 2;
			if (it == actorList.begin())
			{
				maxQuant = currMaxQuant;
				maxActor = (*it);
			}
			else
			{
				if (currMaxQuant > maxQuant)
				{
					maxQuant = currMaxQuant;
					maxActor = (*it);
				}
			}

		}
	}

	return maxActor;
}

void TerrainRail::SetPointPos(int index, sf::Vector2i &p)
{
	TerrainPoint *curr = GetPoint(index);

	curr->pos = p;

	auto currIt = enemies.find(curr);
	if (currIt != enemies.end())
	{
		list<ActorPtr> &currList = (*currIt).second;
		for (auto it = currList.begin(); it != currList.end(); ++it)
		{
			(*it)->UpdateGroundedSprite();
			(*it)->SetBoundingQuad();
		}
	}

	TerrainPoint *prev = GetPoint(index - 1);
	if (prev != NULL)
	{
		currIt = enemies.find(prev);
		if (currIt != enemies.end())
		{
			list<ActorPtr> &currList = (*currIt).second;
			for (auto it = currList.begin(); it != currList.end(); ++it)
			{
				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}
	}
	

	Edge *edge = GetEdge(index);
	int numEdges = (GetNumPoints() - 1) * 2;
	int oppIndex = (numEdges - 1) - index;
	Edge *oppEdge = GetEdge(oppIndex);
	//Edge *invEdge = //GetEdge(index + (GetNumPoints() - 1));
	
	V2d dPos(curr->pos);

	if (edge != NULL)
	{
		edge->v0 = dPos;
	}

	Edge *prevEdge = GetEdge(index - 1);
	if (prevEdge != NULL)
	{
		prevEdge->v1 = dPos;
	}

	if (oppEdge != NULL)
	{
		oppEdge->v1 = dPos;
	}

	Edge *oppNext = GetEdge(oppIndex + 1);
	if (oppNext != NULL)
	{
		oppNext->v0 = dPos;
	}
}

void TerrainRail::UpdateEnemyChain()
{
	if (enemyParams != NULL)
	{
		SetChainPath();
		enemyChain->UpdateParams(enemyParams);
	}
}

void TerrainRail::SetRailToActorType(ActorParams *ap)
{
	assert(ap->myEnemy != NULL);
	int t = -1;

	switch( ap->myEnemy->type )
	{
	case EnemyType::EN_BLOCKERCHAIN:
		t = RailType::BLOCKER;
		enemyTypeName = ap->myEnemy->editParams->GetTypeName();
		break;
	case EnemyType::EN_FLYCHAIN:
		t = RailType::FLY;
		enemyTypeName = ap->myEnemy->editParams->GetTypeName();
		break;
	}

	assert(t != -1);

	SetRailType(t);
}

void TerrainRail::CancelTransformation()
{
	UpdateLines();
	
	/*for (auto it = myFlies.begin(); it != myFlies.end(); ++it)
	{
		(*it)->SetPosition((*it)->preTransformPos);
	}*/

	//triBackups.clear();
}

RailPtr TerrainRail::CompleteTransformation(TransformTools *tr)
{
	if (renderMode == RENDERMODE_TRANSFORM)
	{
		SetRenderMode(TerrainRail::RENDERMODE_NORMAL);
		RailPtr newRail(new TerrainRail);

		int numP = GetNumPoints();
		TerrainPoint *curr;
		Vector2i temp;

		newRail->Reserve(numP);
		int posInd;
		for (int i = 0; i < numP; ++i)
		{
			posInd = i * 2;
			if (i == numP - 1)
				posInd--;
			temp.x = round(lines[posInd].position.x);
			temp.y = round(lines[posInd].position.y);
			newRail->AddPoint(temp, false);
		}

		//newPoly->SetMaterialType(terrainWorldType, terrainVariation);

		UpdateLines();
		//UpdateLinePositions();

		//newRail->SetFlyTransform(this, tr);

		newRail->SetRailType(rType);

		newRail->Finalize();

		return newRail;
	}
	

	//check for validity here

	//not even going to use the same polygon here, this is just for testing. 
	//what will really happen is that I create a copy, adding in the rounded points from my lines.
	//that way I can adjust and test for correctness just like i usually would, and then just
	//do a replacebrush action
	//maybe test for correctness?

	//SoftReset();
	//Finalize();

	

	return NULL;
	/*}*/
}

void TerrainRail::UpdateTransformation(TransformTools *tr)
{
	UpdateLines();

	int numP = GetNumPoints();
	Vector2f fDiff;
	Vector2f fCurr;

	Transform t;
	t.rotate(tr->rotation);
	t.scale(tr->scale);

	int prevIndex;

	Vector2f center = tr->origCenter;
	Vector2f trCenter = tr->GetCenter();

	TerrainPoint *curr, *next;

	int posInd;
	for (int i = 0; i < numP; ++i)
	{
		posInd = i * 2;
		if (i == numP - 1)
		{
			posInd--;
		}
		fCurr = lines[posInd].position;

		fDiff = fCurr - center;
		fDiff = t.transformPoint(fDiff);

		fCurr = fDiff + trCenter;

		lines[posInd].position = fCurr;

		if( i > 0 && i < numP - 1 )
		{
			lines[posInd - 1].position = fCurr;
		}
	}

	Vector2f along, other;
	Vector2f start, end;
	for (int i = 0; i < numColoredQuads; ++i)
	{
		start = lines[i * 2].position;
		end = lines[i * 2 + 1].position;

		along = normalize(end - start);
		other = Vector2f(along.y, -along.x);

		coloredQuads[i * 4].position = start + other * quadHalfWidth;
		coloredQuads[i * 4 + 1].position = end + other * quadHalfWidth;
		coloredQuads[i * 4 + 2].position = end - other * quadHalfWidth;
		coloredQuads[i * 4 + 3].position = start - other * quadHalfWidth;

		coloredNodeCircles->SetPosition(i, start);

		if (i == numColoredQuads - 1)
		{
			coloredNodeCircles->SetPosition(i+1, end);
		}
	}

	//UpdateTexturedQuads();
}

void TerrainRail::ResetState()
{
	state = IDLE;
	frame = 0;
}

void TerrainRail::FadeOut()
{
	if (state == IDLE)
	{
		state = FADINGOUT;
		frame = 0;
	}
}

bool TerrainRail::IsActive()
{
	return state == IDLE || state == FADINGOUT;
}

void TerrainRail::UpdateState()
{
	if (state == IDLE || state == INACTIVE)
	{
		return;
	}
	else
	{
		if (state == FADINGOUT && frame == 60)
		{
			if (rType == FADE)
			{
				state = TEMPORARILYINACTIVE;
			}
			else
			{
				state = INACTIVE;
			}
			frame = 0;
		}
		else if (state == FADINGIN && frame == 60)
		{
			state = IDLE;
			frame = 0;
		}
		else if (state == TEMPORARILYINACTIVE && frame == 120)
		{
			state = FADINGIN;
			frame = 0;
		}
	}

	++frame;
}

void TerrainRail::StoreEnemyPositions(std::vector<std::pair<ActorPtr, PositionInfo>>&b)
{
	int totalEnemies = 0;
	for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); ++enemyIt)
	{
		auto &aList = (*enemyIt).second;
		if (!aList.empty())
		{
			for (auto it = aList.begin(); it != aList.end(); ++it)
			{
				totalEnemies++;
			}
		}
	}

	b.reserve(b.size() + totalEnemies);
	for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); ++enemyIt)
	{
		auto &aList = (*enemyIt).second;
		if (!aList.empty())
		{
			for (auto it = aList.begin(); it != aList.end(); ++it)
			{
				b.push_back(make_pair((*it), (*it)->posInfo));
			}
		}
	}
}

void TerrainRail::SetRenderMode(RenderMode rm)
{
	renderMode = rm;
}

void TerrainRail::Draw( RenderTarget *target )
{
	Draw(1.0, false, target);
}

void TerrainRail::Draw( double zoomMultiple, bool showPoints, sf::RenderTarget *target)
{
	int numP = GetNumPoints();

	if (!IsActive())
	{
		target->draw(lines, numLineVerts, sf::Lines);
		return;
	}

	if (renderMode == RENDERMODE_NORMAL)
	{


		switch (rType)
		{
		case FLOORANDCEILING:
		case FLOOR:
		case CEILING:
		case BOUNCE:
		case SCORPIONONLY:
		case GRIND:
		case ACCELERATE:
		case FADE:
		case LOCKED:
		case PHASE:
		case INVERSEPHASE:
		case ANTITIMESLOW:
		case WIREONLY:
		case WIREBLOCKING:
		case HIT:
		{
			/*target->draw(coloredQuads, numColoredQuads * 4, sf::Quads);
			coloredNodeCircles->Draw(target);*/
			target->draw(texturedQuads, numTexturedQuads * 4,
				sf::Quads, ts_rail->texture);
			break;
		}
		case FLY:
		case BLOCKER:
		{
			enemyParams->Draw(target);
			//blockerParams->Draw(target);
			break;
		}
		}
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

void TerrainRail::HandleQuery(QuadTreeCollider * qtc)
{
	qtc->HandleEntrant(this);
}

bool TerrainRail::IsTouchingBox(const sf::Rect<double> &r)
{
	return r.intersects(aabb);
}