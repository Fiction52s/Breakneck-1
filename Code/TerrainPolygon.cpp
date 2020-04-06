//edit mode

#include "GUI.h"
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include "poly2tri/poly2tri.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "Physics.h"
#include "Action.h"
#include "TerrainRender.h"
#include <set>
#include "GameSession.h"
#include "ActorParams.h"
#include "Action.h"

using namespace std;
using namespace sf;

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

#define cout std::cout

TerrainPolygon::TerrainPolygon( sf::Texture *gt)
	:ISelectable( ISelectable::TERRAIN ), grassTex( gt )
{
	inverse = false;
	layer = 0;
	va = NULL;
	lines = NULL;
	selected = false;
	grassVA = NULL;
	isGrassShowing = false;
	finalized = false;
	numPoints = 0;
	pointStart = NULL;
	pointEnd = NULL;
	movingPointMode = false;
	terrainWorldType = MOUNTAIN;
	terrainVariation = 0;


	
	//tr = NULL;
	EditSession *session = EditSession::GetSession();
	if (session != NULL)
	{
		grassSize = session->grassSize;//64;//64;//22;
		grassSpacing = session->grassSpacing;//-20;//-5;
	}
	else
	{
		grassSize = 0;
		grassSpacing = 0;
	}
	

	pShader = &session->polyShaders[terrainWorldType * EditSession::MAX_TERRAINTEX_PER_WORLD + terrainVariation];
}

TerrainPolygon::TerrainPolygon(TerrainPolygon &poly, bool pointsOnly, bool storeSelectedPoints )
	:ISelectable(ISelectable::TERRAIN)
{
	layer = 0;
	inverse = poly.inverse;
	grassTex = poly.grassTex;
	terrainWorldType = poly.terrainWorldType;
	terrainVariation = poly.terrainVariation;
	//SetMaterialType( poly.terrainWorldType, poly.terrainVariation );
	if (pointsOnly)
	{
		va = NULL;
		lines = NULL;
		selected = false;
		grassVA = NULL;
		isGrassShowing = false;
		finalized = false;
		numPoints = 0;
		pointStart = NULL;
		pointEnd = NULL;
		movingPointMode = false;

		poly.CopyPoints(pointStart, pointEnd, storeSelectedPoints );
		numPoints = poly.numPoints;
	}
	else
	{

		assert(false && "havent implemented yet");
	}
}

TerrainPolygon::~TerrainPolygon()
{
	if (lines != NULL)
		delete[] lines;

	if (va != NULL)
		delete va;

	if (grassVA != NULL)
		delete grassVA;

	//DestroyEnemies();

	ClearPoints();
}

TerrainPoint * TerrainPolygon::GetLoopedPrev(TerrainPoint *p)
{
	TerrainPoint *prev = p->prev;;
	if (prev == NULL)
		prev = pointEnd;

	return prev;
}

TerrainPoint * TerrainPolygon::GetLoopedNext(TerrainPoint *p)
{
	TerrainPoint *next = p->next;;
	if (next == NULL)
		next = pointStart;

	return next;
}

void TerrainPolygon::WriteFile(std::ofstream & of)
{
	of << terrainWorldType << " " << terrainVariation << endl;

	of << numPoints << endl;

	for (TerrainPoint *pcurr = pointStart; pcurr != NULL; pcurr = pcurr->next)
	{
		of << pcurr->pos.x << " " << pcurr->pos.y << endl; // << " " << (int)(*it2).special << endl;
	}

	if (!IsSpecialPoly())
	{
		WriteGrass(of);
	}
}

void TerrainPolygon::WriteGrass(std::ofstream &of)
{
	int edgesWithSegments = 0;

	VertexArray &grassVa = *grassVA;

	int edgeIndex = 0;
	int i = 0;
	list<list<GrassSeg>> grassListList;
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if (temp == NULL)
		{
			next = pointStart->pos;
		}
		else
		{
			next = temp->pos;
		}

		V2d v0(curr->pos.x, curr->pos.y);
		V2d v1(next.x, next.y);

		bool rem;
		int num = GetNumGrass(curr, rem);//floor( remainder ) + 1;

		grassListList.push_back(list<GrassSeg>());

		list<GrassSeg> &grassList = grassListList.back();

		GrassSeg *gPtr = NULL;
		bool hasGrass = false;
		for (int j = 0; j < num; ++j)
		{
			//V2d pos = v0 + (v1 - v0) * ((double)(j )/ num);

			if (grassVa[i * 4].color.a == 255 || grassVa[i * 4].color.a == 254)
			{
				hasGrass = true;
				if (gPtr == NULL)//|| (j == num - 1 && rem ))
				{
					grassList.push_back(GrassSeg(edgeIndex, j, 0));
					gPtr = &grassList.back();
				}
				else
				{
					grassList.back().reps++;
				}
			}
			else
			{
				if (gPtr != NULL)
					gPtr = NULL;
			}

			++i;
		}

		if (hasGrass)
		{
			++edgesWithSegments;
		}

		++edgeIndex;

	}

	of << edgesWithSegments << endl;

	for (list<list<GrassSeg>>::iterator it = grassListList.begin(); it != grassListList.end(); ++it)
	{
		int numSegments = (*it).size();

		if (numSegments > 0)
		{
			int edgeIndex = (*it).front().edgeIndex;
			of << edgeIndex << " " << numSegments << endl;

			for (list<GrassSeg>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
			{
				of << (*it2).index << " " << (*it2).reps << endl;
			}
		}
	}
}

bool TerrainPolygon::SwitchPolygon( bool cw, TerrainPoint *rootPoint,
	TerrainPoint *otherEnd )
{
	if (cw)
	{
		TerrainPoint *prev = rootPoint->prev;
		if (prev == NULL)
			prev = pointEnd;

		TerrainPoint *next = rootPoint->next;
		if (next == NULL)
			next = pointStart;

		V2d rp(rootPoint->pos);

		V2d origDir = normalize( rp - V2d( prev->pos ) );
		V2d stayDir = normalize( rp - V2d( next->pos ) );
		V2d otherDir = normalize( rp - V2d( otherEnd->pos ) );

		double stay = GetVectorAngleDiffCW(origDir, stayDir);//GetClockwiseAngleDifference(origDir, stayDir);
		double other = GetVectorAngleDiffCW(origDir, otherDir);

		if (stay < other)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		assert(0);
		return false;
	}
}

bool TerrainPolygon::IntersectsGate(GateInfo *gi)
{
	IntRect tAABB(left, top, right - left, bottom - top);
	if (tAABB.intersects(gi->GetAABB()))
	{
		Vector2i gp0 = gi->point0->pos;
		Vector2i gp1 = gi->point1->pos;

		Vector2i myPos;
		Vector2i myPrevPos;

		for (TerrainPoint *my = pointStart; my != NULL; my = my->next)
		{
			TerrainPoint *myPrev;
			if (my == pointStart)
			{
				myPrev = pointEnd;
			}
			else
			{
				myPrev = my->prev;
			}

			myPos = my->pos;
			myPrevPos = myPrev->pos;

			if (gp0 == myPrevPos || gp1 == myPrevPos
				|| gp0 == myPos || gp1 == myPos)
			{
				continue;
			}

			LineIntersection li = EditSession::SegmentIntersect(myPrevPos, myPos, gp0, gp1);
			if (!li.parallel)
			{
				return true;
			}
		}
	}

	return false;
}

bool TerrainPolygon::IsInternallyValid()
{
	EditSession *sess = EditSession::GetSession();

	if (inverse)
	{
		if (IsClockwise())
			return false;
	}
	else
	{
		if (!IsClockwise())
			return false;
	}

	//points close to other points on myself
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		for (TerrainPoint *curr2 = pointStart; curr2 != NULL; curr2 = curr2->next)
		{
			if (curr->pos.x == curr2->pos.x && curr->pos.y == curr2->pos.y)
			{
				continue;
			}

			V2d a(curr->pos.x, curr->pos.y);
			V2d b(curr2->pos.x, curr2->pos.y);
			if (length(a - b) < sess->validityRadius)
			{
				//cout << "len: " << length( a - b ) << endl;
				return false;
			}
		}
	}

	//check for slivers that are at too extreme of an angle. tiny triangle type things
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		TerrainPoint *prev, *next;
		if (curr == pointStart)
		{
			prev = pointEnd;
		}
		else
		{
			prev = curr->prev;
		}

		TerrainPoint *temp = curr->next;
		if (temp == NULL)
		{
			next = pointStart;
		}
		else
		{
			next = curr->next;
		}

		//test for minimum angle difference between edges
		V2d pos(curr->pos.x, curr->pos.y);
		V2d prevPos(prev->pos.x, prev->pos.y);
		V2d nextPos(next->pos.x, next->pos.y);


		double ff = dot(normalize(prevPos - pos), normalize(nextPos - pos));
		if (ff > .99)
		{
			//cout << "ff: " << ff << endl;
			return false;
		}
	}


	//line intersection on myself
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

		for (TerrainPoint *curr2 = pointStart; curr2 != NULL; curr2 = curr2->next)
		{
			TerrainPoint *prev2;

			if (curr2 == pointStart)
			{
				prev2 = pointEnd;
			}
			else
			{
				prev2 = curr2->prev;
			}

			if (prev2 == prev || prev2 == curr || curr2 == prev || curr2 == curr)
			{
				continue;
			}

			LineIntersection li = EditSession::LimitSegmentIntersect(prev->pos, curr->pos, prev2->pos, curr2->pos);

			if (!li.parallel)
			{
				return false;
			}
		}
	}

	//for( std::map<std::string, ActorGroup*>::iterator it = edit->groups.begin(); it != edit->groups.end() && res2; ++it )
	for (EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it)
	{
		for (list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait)
		{
			//need to round these floats probably

			sf::VertexArray &bva = (*ait)->boundingQuad;
			if (sess->QuadPolygonIntersect(this, Vector2i(bva[0].position.x, bva[0].position.y),
				Vector2i(bva[1].position.x, bva[1].position.y), Vector2i(bva[2].position.x, bva[2].position.y),
				Vector2i(bva[3].position.x, bva[3].position.y)))
			{
				return false;
			}
			else
			{

			}
		}
	}


	TerrainPoint *prev;
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		if (curr->gate != NULL)
		{
			if (curr == pointStart)
			{
				prev = pointEnd;
			}
			else
			{
				prev = curr->prev;
			}

			Vector2i prevPos = prev->pos;
			Vector2i pos = curr->pos;

			LineIntersection li = EditSession::LimitSegmentIntersect(prevPos, pos, curr->gate->point0->pos, curr->gate->point1->pos);

			if (!li.parallel)
			{
				return false;
			}

		}
	}



	return true;
	
}

bool TerrainPolygon::CanApply()
{
	EditSession *session = EditSession::GetSession();

	auto & currPolyList = session->GetCorrectPolygonList(this);

	if (session->IsPolygonValid(this, NULL))
	{
		return true;
	}

	return false;
}

int TerrainPolygon::GetPointIndex(TerrainPoint *p)
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

TerrainPoint *TerrainPolygon::GetPointAtIndex(int index)
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

bool TerrainPolygon::CanAdd()
{
	return false;
}

void TerrainPolygon::Deactivate(EditSession *edit, SelectPtr select )
{
	cout << "deactivating polygon" << endl;
	PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( select );

	edit->GetCorrectPolygonList(this).remove(poly);

	if (inverse)
	{
		edit->inversePolygon.reset();
	}
	
	//remove enemies
	for( EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		list<ActorPtr> params = (*it).second;
		for( list<ActorPtr>::iterator pit = params.begin(); pit != params.end(); ++pit )
		{
			SelectPtr ptr = boost::dynamic_pointer_cast<ISelectable>( (*pit ) );
			//(*pit)->Deactivate( edit, ptr );
		}
	}

	//remove gates
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->gate != NULL )
		{
			if( curr->gate->edit != NULL )
			{
				curr->gate->Deactivate( edit, curr->gate );
				//curr->gate->edit = NULL;
				//edit->gates.remove( curr->gate );
			}
		}
	}
}

void TerrainPolygon::Activate( EditSession *edit, SelectPtr select )
{
	PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( select );

	edit->GetCorrectPolygonList(this).push_back(poly);
	
	if (inverse)
	{
		edit->inversePolygon = poly;
	}
	
	
	//add in enemies
	for( EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		list<ActorPtr> params = (*it).second;
		for( list<ActorPtr>::iterator pit = params.begin(); pit != params.end(); ++pit )
		{
			SelectPtr ptr = boost::dynamic_pointer_cast<ISelectable>( (*pit ) );
			//(*pit)->Activate( edit, ptr );
		}
	}

	//add in gates
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->gate != NULL )
		{
			if( curr->gate->edit == NULL )
			{
				curr->gate->Activate(edit, curr->gate);
			}
		}
	}
}

//return 0 on no fix, 1 on moved current point, and 2 on moved next point
int TerrainPolygon::FixNearPrimary(TerrainPoint* curr, bool currLocked)
{
	TerrainPoint *next = GetLoopedNext(curr);

	Vector2i extreme = GetExtreme(curr, next);

	if (extreme.x == 0 && extreme.y == 0)
		return 0;

	if (currLocked)
	{
		if (extreme.x != 0)
			next->pos.y = curr->pos.y;
		else
			next->pos.x = curr->pos.x;

		return 2;
	}

	if (extreme.x != 0)
		curr->pos.y = next->pos.y;
	else
		curr->pos.x = next->pos.x;

	return 1;
}

Vector2i TerrainPolygon::GetExtreme(TerrainPoint *p0,
	TerrainPoint *p1)
{
	Vector2i extreme(0, 0);

	Vector2i diff = p1->pos - p0->pos;

	if (diff.x == 0 || diff.y == 0)
		return extreme;

	double primLimit = EditSession::PRIMARY_LIMIT;
	V2d diffDir = normalize(V2d(diff));
	if (diffDir.x > primLimit)
		extreme.x = 1;
	else if (diffDir.x < -primLimit)
		extreme.x = -1;
	if (diffDir.y > primLimit)
		extreme.y = 1;
	else if (diffDir.y < -primLimit)
		extreme.y = -1;

	return extreme;
}

bool TerrainPolygon::AlignExtremes(std::vector<PointMoveInfo> &lockPoints)
{
	double primLimit = EditSession::PRIMARY_LIMIT;
	bool adjustedAtAll = false;
	TerrainPoint *prev;
	TerrainPoint *next;
	bool checkPoint;
	bool adjusted = true;

	int lockPointIndex = 0;
	assert(lockPoints.empty() || lockPoints.size() == numPoints);

	EditSession *sess = EditSession::GetSession();

	bool lockPointsEmpty = lockPoints.empty();
	while( adjusted)
	{
		//RemoveClusters(sess->validityRadius);

		adjusted = false;
		lockPointIndex = 0;
		int result;
		bool isPointLocked;
		for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next, lockPointIndex++)
		{
			isPointLocked = !lockPointsEmpty && lockPoints[lockPointIndex].moveIntent;
			result = FixNearPrimary(curr, isPointLocked);

			if (result > 0)
			{
				adjusted = true;
				adjustedAtAll = true;
			}
			
			/*if (curr->selected)
			{
				if (extreme.x != 0)
					next->pos.y = curr->pos.y;
				else
					next->pos.x = curr->pos.x;
				continue;
			}*/

			//Vector2i gateDir;
			//if (curr->HasPrimaryGate(gateDir))
			//{
			//	if ((extreme.x != 0 && gateDir.x != 0) || (extreme.y != 0 && gateDir.y != 0))
			//	{
			//		if (next->HasPrimaryGate(gateDir))
			//		{
			//			assert(0);
			//			//this is a special case that I can cover later
			//		}
			//		else
			//		{
			//			if (extreme.x != 0)
			//				next->pos.y = curr->pos.y;
			//			else
			//				next->pos.x = curr->pos.x;
			//			adjusted = true;
			//			continue;
			//		}
			//	}
			//}
		}
	}

	return adjustedAtAll;
}

bool TerrainPolygon::AlignExtremes()
{
	vector<PointMoveInfo> emptyLockPoints;
	return AlignExtremes(emptyLockPoints);
}

bool TerrainPolygon::RemoveClusters(double minDist)
{
	bool adjusted = false;

	TerrainPoint *curr, *next, *tempNext;
	double dist;
	V2d dir;
	V2d c, n;
	while (adjusted)
	{
		curr = pointStart;
		while (curr != NULL)
		{
			next = curr->next;
			tempNext = curr->next;
			if (next == NULL)
			{
				next = pointStart;
			}

			c = V2d(curr->pos);
			n = V2d(next->pos);
			dir = n - c;
			dist = length(dir);

			if (dist < minDist)
			{
				//normalize(dir);

				//choose old points


				//choose new points
				adjusted = true;
				//merge points
				RemovePoint(curr);
				next->pos = Vector2i((c + n) / 2.0);



				//move old points

				//move new points



				//move next further away
				//n += dir * ((minDist - dist) + .5);
				//next->pos = Vector2i(n.x, n.y);

				//merge points



				//delete point method
				//RemovePoint(curr);

				cout << "point too close!!!!!" << endl;
			}


			curr = tempNext;
		}
	}

	return adjusted;
}



void TerrainPolygon::DestroyEnemies()
{
	/*for( EnemyMap::iterator mapIt = enemies.begin(); mapIt != enemies.end(); ++mapIt )
	{
		for( list<ActorPtr>::iterator it = (*mapIt).second.begin(); it != (*mapIt).second.end(); ++it )
		{
			(*it)->group->actors.remove( (*it ) );
			delete (*it);
		}
	}
	enemies.clear();*/
}

void TerrainPolygon::Move( SelectPtr me, Vector2i move )
{
	assert( finalized );
	
	TerrainPoint *curr = pointStart;
	while( curr != NULL )
	{
		TerrainPoint *temp = curr->next;
		curr->pos += move;
		if( curr->gate != NULL )
		{
			curr->gate->UpdateLine();
			//cout << "updating line" << endl;
		}
		curr = temp;
	}

	for( int i = 0; i < numPoints; ++i )
	{
		//lines
		lines[i*2].position += Vector2f( move.x, move.y );
		lines[i*2+1].position += Vector2f( move.x, move.y );
	}

	for( int i = 0; i < vaSize; ++i )
	{
		VertexArray &vaa = *va;

		//triangles
		vaa[i].position += Vector2f( move.x, move.y );
		//vaa[i*3+1].position += Vector2f( move.x, move.y );
		//vaa[i*3+2].position += Vector2f( move.x, move.y );
	}

	for( int i = 0; i < numGrassTotal; ++i )
	{
		//quads
		VertexArray &gva = *grassVA;
		gva[i*4].position += Vector2f( move.x, move.y );
		gva[i*4+1].position += Vector2f( move.x, move.y );
		gva[i*4+2].position += Vector2f( move.x, move.y );
		gva[i*4+3].position += Vector2f( move.x, move.y );
	}

	for( EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		list<ActorPtr> &actorList = (*it).second;
		for( list<ActorPtr>::iterator ait = actorList.begin(); ait != actorList.end(); ++ait )
		{
			(*ait)->UpdateGroundedSprite();
			(*ait)->SetBoundingQuad();
		}
	}

	UpdateBounds();
	return;
}

void TerrainPolygon::SetLayer( int newLayer )
{
	//Color testColor( 0x75, 0x70, 0x90 );
	if( newLayer != layer )
	{
		layer = newLayer;
		if( newLayer == 0 )
		{
			VertexArray &v = *va;
			for( int i = 0; i < vaSize; ++i )
			{
				v[i].color = fillCol;
			}
		}
		else if( newLayer == 1 )
		{
			VertexArray &v = *va;
			for( int i = 0; i < vaSize; ++i )
			{
				//gotta alter this soon
				v[i].color = COLOR_BLUE;
			}
		}
	}
}


void TerrainPolygon::UpdateBounds()
{
	TerrainPoint *curr = pointStart;
	//PointList::iterator it = points.begin();
	left = curr->pos.x;
	right = curr->pos.x;
	top = curr->pos.y;
	bottom = curr->pos.y;
	curr = curr->next;
	while( curr != NULL )
	{
		left = min( curr->pos.x, left );
		right = max( curr->pos.x, right );
		top = min( curr->pos.y, top );
		bottom = max( curr->pos.y, bottom );
		curr = curr->next;
	}

	for( EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		list<ActorPtr> &en = (*it).second;
		for( list<ActorPtr>::iterator ait = en.begin(); ait != en.end(); ++ait )
		{
			sf::VertexArray & bq = (*ait)->boundingQuad;
			for( int i = 0; i < 4; ++i )
			{
				int x = bq[i].position.x;
				int y = bq[i].position.y;
				if( x < left )
				{
					left = x;
				}
				if( x > right )
				{
					right = x;
				}
				if( y < top )
				{
					top = y;
				}
				if( y > bottom )
				{
					//cout << "adjusting botton from: " << bottom << " to " << y << endl;
					bottom = y;
					
				}
					
			}

		}
	}
}

bool TerrainPolygon::IsSpecialPoly()
{
	return terrainWorldType > CORE;
}

void TerrainPolygon::UpdateMaterialType()
{
	EditSession *session = EditSession::GetSession();
	int texInd = terrainWorldType * session->MAX_TERRAINTEX_PER_WORLD + terrainVariation;
	pShader = &session->polyShaders[texInd];
	//pShader->setUniform("u_texture", session->terrainTextures[texInd]);

	Color sCol( 0x77, 0xBB, 0xDD );
	//factor in variation later
	//Color newColor;
	/*switch( world )
	{
	case 0:
		fillCol = Color::Blue;
		break;
	case 1:
		fillCol = Color::Green;
		break;
	case 2:
		fillCol = Color::Yellow;
		break;
	case 3:
		fillCol = Color( 100, 200, 200 );
		break;
	case 4:
		fillCol = Color::Red;
		break;
	case 5:
		fillCol = Color::Magenta;
		break;
	case 6:
		fillCol = Color::White;
		break;
	}*/
	fillCol = Color::White;
	selectCol = sCol;
	//selectCol = 

	int vCount = va->getVertexCount();
	VertexArray &v = *va;
	for( int i = 0; i < vCount; ++i )
	{	
		v[i].color = fillCol;
	}
}

void TerrainPolygon::SetMaterialType(int world, int variation)
{
	terrainWorldType = (TerrainPolygon::TerrainWorldType)world;
	terrainVariation = variation;

	if (finalized)
	{
		UpdateMaterialType();
	}
}

void TerrainPolygon::FinalizeInverse()
{
	inverse = true;
	finalized = true;
	isGrassShowing = false;
	//material = "mat";

	FixWindingInverse();

	UpdateBounds();

	int testExtra = inverseExtraBoxDist;
	vector<p2t::Point*> outerQuadPoints;

	sf::Rect<double> finalRect;
	finalRect.left = left - testExtra;
	finalRect.top = top - testExtra;
	finalRect.width = (right-left) +  testExtra * 2;
	finalRect.height = (bottom-top) + testExtra * 2;

	outerQuadPoints.push_back(new p2t::Point(finalRect.left, finalRect.top));
	outerQuadPoints.push_back(new p2t::Point(finalRect.left + finalRect.width, finalRect.top));
	outerQuadPoints.push_back(new p2t::Point(finalRect.left + finalRect.width, finalRect.top + finalRect.height));
	outerQuadPoints.push_back(new p2t::Point(finalRect.left, finalRect.top + finalRect.height));

	p2t::CDT * cdt = new p2t::CDT(outerQuadPoints);

	lines = new sf::Vertex[numPoints*2+1];
	
	
	//cout << "points size: " << points.size() << endl;

	vector<p2t::Point*> polyline;
	TerrainPoint *curr = pointStart;
	while( curr != NULL )
	{
		TerrainPoint *temp = curr->next;
		polyline.push_back( new p2t::Point(curr->pos.x, curr->pos.y ) );
		curr = temp;
	}

	cdt->AddHole(polyline);

	cdt->Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt->GetTriangles();

	vaSize = tris.size() * 3;
	va = new VertexArray(sf::Triangles, vaSize);

	VertexArray & v = *va;
	Color testColor(0x75, 0x70, 0x90);
	Color selectCol(0x77, 0xBB, 0xDD);

	if (selected)
	{
		testColor = selectCol;
	}
	for (int i = 0; i < tris.size(); ++i)
	{
		p2t::Point *p = tris[i]->GetPoint(0);
		p2t::Point *p1 = tris[i]->GetPoint(1);
		p2t::Point *p2 = tris[i]->GetPoint(2);
		v[i * 3] = Vertex(Vector2f(p->x, p->y), testColor);
		v[i * 3 + 1] = Vertex(Vector2f(p1->x, p1->y), testColor);
		v[i * 3 + 2] = Vertex(Vector2f(p2->x, p2->y), testColor);
	}

	SetMaterialType(terrainWorldType, terrainVariation);

	//assert( tris.size() * 3 == points.size() );
	delete cdt;

	for (auto it = outerQuadPoints.begin(); it != outerQuadPoints.end(); ++it)
	{
		delete (*it);
	}

	for (int i = 0; i < numPoints; ++i)
	{
		delete polyline[i];
		//	delete tris[i];
	}

	UpdateLines();
	

	UpdateBounds();
	

	SetupGrass();	
}

int TerrainPolygon::GetNumGrass(TerrainPoint *curr, bool &rem)
{
	Vector2i next;
	rem = false;
	TerrainPoint *temp = curr->next;
	if (temp == NULL)
	{
		next = pointStart->pos;
	}
	else
	{
		//++temp;
		next = temp->pos;
		//--temp;
	}

	V2d v0(curr->pos.x, curr->pos.y);
	V2d v1(next.x, next.y);

	double len = length(v1 - v0);
	len -= grassSize / 2 + grassSpacing;
	double reps = len / (grassSize + grassSpacing);
	double remainder = reps - floor(reps);
	if (remainder > 0)
	{
		reps += 1; //for the last one
		rem = true;
	}
	reps += 1;

	int num = reps;

	return num;
}

int TerrainPolygon::GetNumGrassTotal()
{
	int total = 0;
	int inds = 0;
	TerrainPoint *curr = pointStart;
	bool rem;
	for (curr = pointStart; curr != NULL; curr = curr->next)
	{
		total += GetNumGrass(curr, rem);
	}

	return total;
}

void TerrainPolygon::SetupGrass(TerrainPoint *curr, int &i )
{
	VertexArray &grassVa = *grassVA;
	
	Vector2i next;

	TerrainPoint *temp = curr->next;
	if (temp == NULL)
	{
		next = pointStart->pos;
	}
	else
	{
		next = temp->pos;
	}

	V2d v0(curr->pos.x, curr->pos.y);
	V2d v1(next.x, next.y);

	bool rem;
	int num = GetNumGrass(curr, rem);

	V2d along = normalize(v1 - v0);
	V2d realStart = v0 + along * (double)(grassSize + grassSpacing);
	
	for (int j = 0; j < num; ++j)
	{
		V2d posd = realStart + along * (double)((grassSize + grassSpacing) * (j - 1));//v0 + normalize(v1 - v0) * ((grassSize + grassSpacing) * (j-1) + );

		if (j == 0)
		{
			posd = v0;
		}
		else if (j == num - 1 && rem)
		{
			//V2d prev = ;//v0 + (v1 - v0) * ((double)(j-1) / num);
			posd = v1 + normalize(v0 - v1) * (grassSize / 2.0 + grassSpacing);//(v1 + prev) / 2.0;
		}

		Vector2f pos(posd.x, posd.y);


		Vector2f topLeft = pos + Vector2f(-grassSize / 2, -grassSize / 2);
		Vector2f topRight = pos + Vector2f(grassSize / 2, -grassSize / 2);
		Vector2f bottomLeft = pos + Vector2f(-grassSize / 2, grassSize / 2);
		Vector2f bottomRight = pos + Vector2f(grassSize / 2, grassSize / 2);

		//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
		grassVa[i * 4].color.a = 0;
		grassVa[i * 4].position = topLeft;
		grassVa[i * 4].texCoords = Vector2f(0, 0);

		//grassVa[i*4+1].color = Color::Blue;
		//borderVa[i*4+1].color.a = 10;
		grassVa[i * 4 + 1].color.a = 0;
		grassVa[i * 4 + 1].position = bottomLeft;
		grassVa[i * 4 + 1].texCoords = Vector2f(0, grassSize);

		//grassVa[i*4+2].color = Color::Blue;
		//borderVa[i*4+2].color.a = 10;
		grassVa[i * 4 + 2].color.a = 0;
		grassVa[i * 4 + 2].position = bottomRight;
		grassVa[i * 4 + 2].texCoords = Vector2f(grassSize, grassSize);

		//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
		//borderVa[i*4+3].color.a = 10;
		grassVa[i * 4 + 3].color.a = 0;
		grassVa[i * 4 + 3].position = topRight;
		grassVa[i * 4 + 3].texCoords = Vector2f(grassSize, 0);
		++i;
	}
}

void TerrainPolygon::Finalize()
{
	AlignExtremes();

	if (inverse)
	{
		FinalizeInverse();
		return;
	}
	finalized = true;
	isGrassShowing = false;
	//material = "mat";

	

	lines = new sf::Vertex[numPoints*2+1];
	
	FixWinding();
	//cout << "points size: " << points.size() << endl;

	vector<p2t::Point*> polyline;
	TerrainPoint *curr = pointStart;
	while( curr != NULL )
	{
		TerrainPoint *temp = curr->next;
		polyline.push_back( new p2t::Point(curr->pos.x, curr->pos.y ) );
		curr = temp;
	}

	if (polyline.size() == 1)
	{
		assert(0);
	}

	p2t::CDT * cdt = new p2t::CDT( polyline );
	
	cdt->Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt->GetTriangles();
	
	vaSize = tris.size() * 3;
	va = new VertexArray( sf::Triangles , vaSize );
	
	VertexArray & v = *va;
	Color testColor( 0x75, 0x70, 0x90 );
	Color selectCol( 0x77, 0xBB, 0xDD );

	if( selected )
	{
		testColor = selectCol;
	}
	for( int i = 0; i < tris.size(); ++i )
	{	
		p2t::Point *p = tris[i]->GetPoint( 0 );	
		p2t::Point *p1 = tris[i]->GetPoint( 1 );	
		p2t::Point *p2 = tris[i]->GetPoint( 2 );	
		v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
		v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
		v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );
	}

	SetMaterialType( terrainWorldType, terrainVariation );

	//assert( tris.size() * 3 == points.size() );
	delete cdt;
	for( int i = 0; i < numPoints; ++i )
	{
		delete polyline[i];
	//	delete tris[i];
	}

	UpdateLines();

	UpdateBounds();

	SetupGrass();
}

void TerrainPolygon::SetupGrass()
{
	numGrassTotal = GetNumGrassTotal();
	VertexArray *gva = new VertexArray(sf::Quads, numGrassTotal * 4);
	grassVA = gva;

	VertexArray &grassVa = *gva;

	int i = 0;
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		SetupGrass(curr, i);
	}
}

void TerrainPolygon::RemoveSelectedPoints()
{	
	SoftReset();
	for( TerrainPoint *curr = pointStart; curr != NULL; )
	{
		if( curr->selected )
		{
			TerrainPoint *temp = curr->next;
			RemovePoint( curr );
			curr = temp;
		}
		else
		{
			curr = curr->next;
		}
	}
	//Reset();


	Finalize();
	SetSelected( false );
}

void TerrainPolygon::SwitchGrass( V2d mousePos )
{
	
	VertexArray &grassVa = *grassVA;
	double radius = grassSize / 2 - 20;//+ grassSpacing / 2;
	int i = 0;

	V2d center;
	for (int i = 0; i < numGrassTotal; ++i)
	{
		center = V2d((grassVa[i * 4 + 0].position
			+ grassVa[i * 4 + 1].position
			+ grassVa[i * 4 + 2].position
			+ grassVa[i * 4 + 3].position) / 4.f);
		if (length(center - mousePos) <= radius)
		{
			if (grassVa[i * 4].color.a == 50)
			{
				grassVa[i * 4].color.a = 254;
				grassVa[i * 4 + 1].color.a = 254;
				grassVa[i * 4 + 2].color.a = 254;
				grassVa[i * 4 + 3].color.a = 254;
				//cout << "making full: " << i << endl;
			}
			else if (grassVa[i * 4].color.a == 255)
			{
				grassVa[i * 4].color.a = 49;
				grassVa[i * 4 + 1].color.a = 49;
				grassVa[i * 4 + 2].color.a = 49;
				grassVa[i * 4 + 3].color.a = 49;
				//cout << "making seethru: " << i << endl;
			}
			//break;
		}
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = pointStart->pos;
		}
		else
		{
			next = temp->pos;
		}

		V2d v0( curr->pos.x, curr->pos.y );
		V2d v1( next.x, next.y );

		bool rem;
		int num = GetNumGrass(curr, rem);

		for( int j = 0; j < num; ++j )
		{
			V2d pos = v0 + (v1- v0) * ((double)(j )/ num);
				
			if( length( pos - mousePos ) <= radius )
			{
				
			}
			++i;
		}
	}
}

void TerrainPolygon::UpdateGrass()
{
	VertexArray & grassVa = *grassVA;
	for( int i = 0; i < numGrassTotal; ++i )
	{
		if( grassVa[i*4].color.a == 49 )
		{
			grassVa[i*4].color.a = 50;
			grassVa[i*4+1].color.a = 50;
			grassVa[i*4+2].color.a = 50;
			grassVa[i*4+3].color.a = 50;
		}
		else if( grassVa[i*4].color.a == 254 )
		{
			grassVa[i*4].color.a = 255;
			grassVa[i*4+1].color.a = 255;
			grassVa[i*4+2].color.a = 255;
			grassVa[i*4+3].color.a = 255;
		}
	}
}

void TerrainPolygon::Draw( bool showPath, double zoomMultiple, RenderTarget *rt, bool showPoints, TerrainPoint *dontShow )
{
	if( movingPointMode )
	{

			int i = 0;
			TerrainPoint *curr = pointStart;
			while( curr != NULL )
			{
				lines[i*2].position = Vector2f( curr->pos.x, curr->pos.y );

				TerrainPoint *temp = curr->next;
				if( temp == NULL )
				{
					lines[i*2+1].position = Vector2f( pointStart->pos.x, pointStart->pos.y );
				}
				else
				{
					lines[i*2+1].position = Vector2f( temp->pos.x, temp->pos.y );
				}
				
				curr = temp;
				++i;
			}

			rt->draw( lines, numPoints * 2, sf::Lines );


			//lines
			
		
		return;
	}


	if( grassVA != NULL )
		rt->draw(*grassVA, grassTex );

	if( va != NULL )
		rt->draw( *va, pShader );

	//always do this now for awhile
	if( false )
	{
		if( !isGrassShowing )
		{
			for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
			{
				CircleShape cs;
				cs.setRadius( 8 * zoomMultiple );
				cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

				if( curr->selected )
					cs.setFillColor( Color::Red );
				else
					cs.setFillColor( Color::Green );

				cs.setPosition( curr->pos.x, curr->pos.y );
				rt->draw( cs );
			}
		}
		rt->draw( lines, numPoints * 2, sf::Lines );
	}

	rt->draw( lines, numPoints * 2, sf::Lines );

	if( showPoints )
	{
		CircleShape cs;
		cs.setRadius( 8 * zoomMultiple );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Magenta );

		CircleShape csSel;
		csSel.setRadius(8 * zoomMultiple);
		csSel.setOrigin(csSel.getLocalBounds().width / 2, csSel.getLocalBounds().height / 2);
		csSel.setFillColor(Color::Green);

		for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
		{
			if( curr == dontShow )
			{
				continue;
			}
			
			if (curr->selected)
			{
				csSel.setPosition(curr->pos.x, curr->pos.y);
				rt->draw(csSel);
			}
			else
			{
				cs.setPosition(curr->pos.x, curr->pos.y);
				rt->draw(cs);
			}
			
			
		}
	}

	Vector2i center( (right + left) / 2, (bottom + top) / 2 );

	if( showPath )
	{
		for( list<Vector2i>::iterator it = path.begin(); it != path.end(); ++it )
		{
			CircleShape cs;
			cs.setRadius( EditSession::POINT_SIZE * zoomMultiple );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

			cs.setFillColor( Color::Magenta );
			cs.setPosition( center.x + (*it).x, center.y + (*it).y );
			rt->draw( cs );
	
		}


		if( path.size() > 1 )
		{
			list<Vector2i>::iterator prev = path.begin();
			list<Vector2i>::iterator curr = path.begin();
			++curr;
			while( curr != path.end() )
			{
				sf::Vertex activePreview[2] =
				{
					sf::Vertex(sf::Vector2<float>(center.x + (*prev).x, center.y + (*prev).y), Color::White ),
					sf::Vertex(sf::Vector2<float>(center.x + (*curr).x, center.y + (*curr).y), Color::White )
				};
				rt->draw( activePreview, 2, sf::Lines );

				prev = curr;
				++curr;
			}
		
		}
	}
}

void TerrainPolygon::SetSelected( bool select )
{
	selected = select;
	
	if( selected )
	{
		for( int i = 0; i < vaSize; ++i )
		{
			VertexArray & v = *va;
			v[i].color = selectCol;
		}
	}
	else
	{
		//Color testColor( 0x75, 0x70, 0x90 );
		for( int i = 0; i < vaSize; ++i )
		{
			VertexArray & v = *va;
			v[i].color = fillCol;
		}

		for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
		{
			curr->selected = false;
		}
	}
}

bool TerrainPolygon::ContainsPoint( Vector2f test )
{
	int pointCount = numPoints;

	int i, j, c = 0;

	TerrainPoint *it = pointStart;
	TerrainPoint *jt = pointEnd;

	for( ; it != NULL; )
	{
		Vector2f point(it->pos.x, it->pos.y );
		Vector2f pointJ(jt->pos.x, jt->pos.y );
		if ( ((point.y > test.y ) != ( pointJ.y > test.y ) ) &&
			(test.x < (pointJ.x-point.x) * (test.y-point.y) / (pointJ.y-point.y) + point.x) )
				c = !c;
		jt = it;
		it = it->next;
	}


	//not sure if this will affect other stuff. i hope it wont. otherwise move this is out to the point
	//select area for adding to a selected brush instead of here
	if (inverse)
	{
		if (!c)
		{
			if (test.x >= left - inverseExtraBoxDist && test.x <= right + inverseExtraBoxDist
				&& test.y >= top - inverseExtraBoxDist && test.y <= bottom + inverseExtraBoxDist)
			{
				c = true;
			}
		}
		else
		{
			c = false;
		}
	}


	return c;
}

void TerrainPolygon::FixWinding()
{
    if (IsClockwise())
    {
		//cout << "not fixing" << endl;
    }
    else
    {
		for( TerrainPoint *curr = pointStart; curr != NULL; )
		{
			TerrainPoint *tt = curr->next;
			TerrainPoint *oldPrev = curr->prev;
			curr->prev = curr->next;
			curr->next = oldPrev;
			curr = tt;
		}
		TerrainPoint *tt = pointStart;
		pointStart = pointEnd;
		pointEnd = tt;

    }
}

void TerrainPolygon::UpdateLineColor( sf::Vertex *li, TerrainPoint *p, int index )
{
	TerrainPoint *next = p->next;
	if (next == NULL)
	{
		next = pointStart;
	}

	Vector2f diff = Vector2f( next->pos - p->pos );//p1 - p0;
	V2d dir = normalize(V2d(diff));
	V2d norm = V2d(dir.y, -dir.x);

	EdgeAngleType eat = GetEdgeAngleType(norm);

	Color edgeColor;
	switch (eat)
	{
	case EDGE_FLAT:
		edgeColor = Color::Red;
		break;
	case EDGE_SLOPED:
		edgeColor = Color::Green;
		break;
	case EDGE_STEEPSLOPE:
		edgeColor = Color::White;
		break;
	case EDGE_WALL:
		edgeColor = Color::Magenta;
		break;
	case EDGE_STEEPCEILING:
		edgeColor = Color::Yellow;
		break;
	case EDGE_SLOPEDCEILING:
		edgeColor = Color::Cyan;
		break;
	case EDGE_FLATCEILING:
		edgeColor = Color::Red;
		break;
	}

	lines[index].color = edgeColor;
	lines[index + 1].color = edgeColor;
}

void TerrainPolygon::FixWindingInverse()
{
	if( !IsClockwise() )
    {
		//cout << "not fixing" << endl;
    }
    else
    {
		for( TerrainPoint *curr = pointStart; curr != NULL; )
		{
			TerrainPoint *tt = curr->next;
			TerrainPoint *oldPrev = curr->prev;
			curr->prev = curr->next;
			curr->next = oldPrev;
			curr = tt;
		}
		TerrainPoint *tt = pointStart;
		pointStart = pointEnd;
		pointEnd = tt;
    }
}

void TerrainPolygon::UpdateLines()
{
	if (numPoints > 0)
	{
		int i = 0;
		TerrainPoint *curr = pointStart;
		lines[0].position = sf::Vector2f(curr->pos.x, curr->pos.y);
		UpdateLineColor(lines, curr, i);
		lines[2 * numPoints - 1].position = sf::Vector2f(curr->pos.x, curr->pos.y);
		curr = curr->next;
		++i;
		while (curr != NULL)
		{
			UpdateLineColor(lines, curr, i + 1);
			lines[i].position = sf::Vector2f(curr->pos.x, curr->pos.y);
			lines[++i].position = sf::Vector2f(curr->pos.x, curr->pos.y);
			++i;
			curr = curr->next;
		}
	}
}

void TerrainPolygon::AddPoint( TerrainPoint* tp)
{
	if( pointStart == NULL )
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

bool TerrainPolygon::PointsTooCloseToSegInProgress(sf::Vector2i point,
	int minDistance, bool finalPoint )
{
	V2d p(point.x, point.y);

	TerrainPoint *pcurr = pointStart;
	TerrainPoint *pnext = NULL;

	Vector2i endPos = pointEnd->pos;

	if (finalPoint)
	{
		pcurr = pcurr->next;
	}

	while (pcurr != NULL)
	{
		pnext = pcurr->next;

		if (pnext == NULL || pnext->next == NULL)
		{
			return false;
		}

		if (SegmentWithinDistanceOfPoint(endPos, point, pcurr->pos, minDistance))
		{
			return true;
		}

		pcurr = pnext;
	}
	return false;
}

bool TerrainPolygon::IsValidInProgressPoint(sf::Vector2i point)
{
	EditSession *sess = EditSession::GetSession();


	if (numPoints == 0)
		return true;

	if (numPoints >= 3 && IsCloseToFirstPoint(sess->GetZoomedPointSize(), V2d(point)) &&
		IsCompletionValid())
	{
		return true;
	}

	double minEdge = sess->GetZoomedMinEdgeLength();
	{
		bool pointTooClose = PointTooClose(point, minEdge, true);
		bool linesIntersect = LinesIntersectInProgress(point);
		if (pointTooClose || linesIntersect )
		{
			return false;
		}

		if (PointsTooCloseToSegInProgress(point, minEdge) )
		{
			return false;
		}

		return true;
	}

	return false;
}

LineIntersection TerrainPolygon::GetSegmentFirstIntersection(sf::Vector2i &a, sf::Vector2i &b,
	TerrainPoint *&outSegStart, TerrainPoint *&outSegEnd, bool ignoreStartPoint )
{
	LineIntersection li;

	V2d A(a.x, a.y);

	outSegStart = NULL;
	outSegEnd = NULL;

	TerrainPoint *prevP = NULL;
	TerrainPoint *other = NULL;
	TerrainPoint *otherPrev = NULL;

	TerrainPoint *min = NULL;
	Vector2i minIntersection;
	bool emptyInter = true;

	TerrainPoint *realStartPoint = NULL;
	TerrainPoint *currP = pointStart;
	TerrainPoint *nextP = NULL;

	V2d storedPos;

	for (; currP != NULL; currP = currP->next)
	{
		nextP = currP->next;
		if (nextP == NULL)
			nextP = pointStart;

		V2d A(a);
		V2d B(b);
		V2d dir = normalize(B - A);
		V2d CurrP = V2d(currP->pos);
		V2d NextP = V2d(nextP->pos);
		V2d otherDir = normalize( NextP - CurrP );

		//if it dives into a poly as soon as you hit it, ignore
		double c = cross(otherDir, dir);
		double c0 = cross(normalize(CurrP - A), dir );
		double c1 = cross(normalize(NextP - A), dir );


		if (c < 0 || ( c0 < 0 && c1 < 0 ) )
		{
			int ff = 0;
			//continue;
		}

		//if ( dir == otherDir || dir == -otherDir )
		//{
		//	//possible rounding error gonna happen
		//	continue;
		//}
		if (ignoreStartPoint)
		{
			li = EditSession::LimitSegmentIntersect(a, b, currP->pos, nextP->pos, true);
		}
		else
		{
			li = EditSession::SegmentIntersect(a, b, currP->pos, nextP->pos);
		}
		//li = EditSession::LimitSegmentIntersect(a, b, currP->pos, nextP->pos, true );
		
		//Vector2i lii(round(li.position.x), round(li.position.y));

		if (!li.parallel)
		{
			if (emptyInter)
			{
				emptyInter = false;
				storedPos = li.position;
				outSegStart = currP;
				outSegEnd = nextP;
			}
			else if (length(li.position - A) < length(storedPos - A))
			{
				storedPos = li.position;
				outSegStart = currP;
				outSegEnd = nextP;
			}
		}
	}

	if (!emptyInter)
	{
		li.position = storedPos;
		li.parallel = false;
	}

	return li;
}

TerrainPoint *TerrainPolygon::GetSamePoint(sf::Vector2i &p)
{
	for (TerrainPoint *tp = pointStart; tp != NULL; tp = tp->next)
	{
		if (tp->pos == p)
		{
			return tp;
		}
	}
	return NULL;
}

bool compareInter(DetailedInter &inter0, DetailedInter &inter1)
{
	TerrainPoint *start = inter0.inter.point;
	V2d startD = V2d(start->pos.x, start->pos.y);
	return length(inter0.inter.position - startD) < length(inter1.inter.position- startD);
}

int TerrainPolygon::GetIntersectionNumber(sf::Vector2i &a, sf::Vector2i &b, Inter &inter,
	TerrainPoint *&outSegStart, bool &outFirstPoint )
{
	outFirstPoint = false;
	outSegStart = NULL;

	TerrainPoint dummy(a, false);

	LineIntersection li;

	//V2d A(a.x, a.y);

	TerrainPoint *prevP = NULL;
	TerrainPoint *other = NULL;
	TerrainPoint *otherPrev = NULL;

	TerrainPoint *min = NULL;
	Vector2i minIntersection;
	bool emptyInter = true;

	TerrainPoint *realStartPoint = NULL;
	TerrainPoint *currP = pointStart;
	TerrainPoint *nextP = NULL;

	V2d storedPos;

	list<DetailedInter> inters;
	list<DetailedInter> sortedInters;

	for (; currP != NULL; currP = currP->next)
	{
		nextP = currP->next;
		if (nextP == NULL)
			nextP = pointStart;

		li = EditSession::SegmentIntersect(a, b, currP->pos, nextP->pos);
		//Vector2i lii(round(li.position.x), round(li.position.y));

		

		if (!li.parallel)
		{
			if (length(li.position - V2d(a)) == 0.0)
			{
				outFirstPoint = true;
			}
			inters.push_back(DetailedInter(&dummy, li.position, currP));
		}
	}

	inters.sort(compareInter);

	int interIndex = 0;
	for (auto it = inters.begin(); it != inters.end(); ++it )
	{
		if ((*it).inter.position == inter.position)
		{
			outSegStart = (*it).otherPoint;
			return interIndex;
		}
		++interIndex;
	}

	return -1;
}

sf::Vector2i TerrainPolygon::TrimSliverPos(sf::Vector2<double> &prevPos,
	sf::Vector2<double> &pos, sf::Vector2<double> &nextPos,
	double minAngle, bool cw )
{
	double halfMinAngle = minAngle / 2;
	double angle = PI / 2.0 - halfMinAngle;

	double lenA = length(pos - prevPos);
	double lenB = length(nextPos - pos);
	double minLen = min(lenA, lenB);

	V2d adjA = prevPos;
	V2d adjB = nextPos;

	adjA = pos + normalize(prevPos - pos) * minLen;
	adjB = pos + normalize(nextPos - pos) * minLen;

	V2d dA = normalize(adjB - adjA);
	V2d dB = -dA;
	double xx = length(dA);
	double yy = length(dB);

	if (cw)
	{
		RotateCCW(dA, angle);
		RotateCW(dB, angle);
	}
	else
	{
		RotateCW(dA, angle);
		RotateCCW(dB, angle);
	}
	

	V2d aStart = adjA;
	V2d bStart = adjB;

	V2d aEnd = aStart + dA * 100.0;
	V2d bEnd = bStart + dB * 100.0;

	LineIntersection li = lineIntersection(aStart, aEnd, bStart, bEnd);
	if (!li.parallel)
	{

		double lenA = length(aStart - li.position);
		double lenB = length(bStart - li.position);
		double lenDA = length(dA);
		double lenDB = length(dB);
		//might miss minAngle slightly
		return Vector2i( round(li.position.x), round(li.position.y));
	}
	else
	{
		if (aStart == aEnd && aStart == bStart && aStart == bEnd)
		{
			cout << "all points the same clipping" << endl;
			return Vector2i(aStart); //all points the same?
		}
		else
		{
			cout << "another case?" << endl;
			return Vector2i(aStart);
		}
		
		
		//assert(0);
		//return Vector2i(-5, -5);
	}
}

bool TerrainPolygon::TryToMakeInternallyValid()
{
	return false;
	//remove slivers
	//align extremes
	//fix clusters
}

bool TerrainPolygon::IsClustered(TerrainPoint*curr)
{
	bool adjusted = false;
	
	TerrainPoint *prev = GetLoopedPrev(curr);
	TerrainPoint *next = GetLoopedNext(curr);
	V2d c(curr->pos);
	V2d n(next->pos);
	V2d p(prev->pos);

	double minDist = EditSession::POINT_SIZE;
	if (length(n - c) < minDist || length( c - p ) < minDist )
	{
		return true;
	}

	return false;
}

bool TerrainPolygon::RemoveClusters(std::list<TerrainPoint*> &checkPoints)
{
	bool adjusted = false;
	TerrainPoint *p;
	for (auto it = checkPoints.begin(); it != checkPoints.end(); ++it)
	{
		p = (*it);
		if (IsClustered(p))
		{
			RemovePoint(p);
			adjusted = true;
		}
	}

	return adjusted;
}

bool TerrainPolygon::FixSliver(TerrainPoint *curr)
{
	double minAngle = EditSession::SLIVER_LIMIT;
	TerrainPoint *prev, *next;
	prev = GetLoopedPrev(curr);
	next = GetLoopedNext(curr);

	V2d pos(curr->pos.x, curr->pos.y);
	V2d prevPos(prev->pos.x, prev->pos.y);
	V2d nextPos(next->pos.x, next->pos.y);
	V2d dirA = normalize(prevPos - pos);
	V2d dirB = normalize(nextPos - pos);

	double diff = GetVectorAngleDiffCCW(dirA, dirB);
	double diffCW = GetVectorAngleDiffCW(dirA, dirB);
	if (diff < minAngle)
	{
		Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, true);
		curr->pos = trimPos;
		return true;
	}
	else if (diffCW < minAngle)
	{
		Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, false);
		curr->pos = trimPos;
		return true;
	}

	return false;
}

bool TerrainPolygon::IsSliver(TerrainPoint*curr)
{
	double minAngle = EditSession::SLIVER_LIMIT;
	TerrainPoint *prev, *next;
	prev = GetLoopedPrev(curr);
	next = GetLoopedNext(curr);

	V2d pos(curr->pos.x, curr->pos.y);
	V2d prevPos(prev->pos.x, prev->pos.y);
	V2d nextPos(next->pos.x, next->pos.y);
	V2d dirA = normalize(prevPos - pos);
	V2d dirB = normalize(nextPos - pos);

	double diff = GetVectorAngleDiffCCW(dirA, dirB);
	double diffCW = GetVectorAngleDiffCW(dirA, dirB);
	if (diff < minAngle)
	{
		return true;
		//Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, true);
		//curr->pos = trimPos;

		//affectedPoints.push_back(curr);
	}
	else if (diffCW < minAngle)
	{
		return true;
		//Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, false);
		//curr->pos = trimPos;

		//affectedPoints.push_back(curr);
	}

	return false;
}

//angle in radians
void TerrainPolygon::RemoveSlivers()
{	
	//check for slivers that are at too extreme of an angle. tiny triangle type things

	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		FixSliver(curr);
	}
}

void TerrainPolygon::InsertPoint( TerrainPoint *tp, TerrainPoint *prevPoint )
{
	tp->next = prevPoint->next;
	if( tp->next != NULL )
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

void TerrainPolygon::RemovePoint( TerrainPoint *tp )
{
	assert( pointStart != NULL );

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

GateInfoPtr gi = tp->gate;
if (gi != NULL)
{
	gi->point0->gate = NULL;
	gi->point1->gate = NULL;
	gi->edit->gates.remove(gi);
	//delete gi;

	//was deleting just fine before, but now it needs to be adjusted for undo/redo
}
//delete tp;


--numPoints;
}

TerrainPoint * TerrainPolygon::HasPointPos(Vector2i &pos)
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

void TerrainPolygon::Reset()
{
	ClearPoints();
	if (lines != NULL)
		delete[] lines;
	if (va != NULL)
		delete va;
	if (grassVA != NULL)
		delete grassVA;

	lines = NULL;
	va = NULL;
	grassVA = NULL;
	finalized = false;
}

void TerrainPolygon::SoftReset()
{
	if (lines != NULL)
		delete[] lines;
	if (va != NULL)
		delete va;
	if (grassVA != NULL)
		delete grassVA;

	lines = NULL;
	va = NULL;
	grassVA = NULL;
	finalized = false;
}

void TerrainPolygon::ClearPoints()
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

bool TerrainPolygon::SharesPoints(TerrainPolygon *poly)
{
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		for (TerrainPoint *other = poly->pointStart; other != NULL; other = other->next)
		{
			if (curr->pos == other->pos)
			{
				return true;
			}
		}
	}
	return false;
}

TerrainPoint *TerrainPolygon::GetMostLeftPoint()
{
	TerrainPoint *leftPoint = pointStart;
	
	for (TerrainPoint *currP = pointStart; currP != NULL; currP = currP->next)
	{
		if (currP->pos.x < leftPoint->pos.x)
		{
			leftPoint = currP;
		}
	}
	return leftPoint;
}


bool TerrainPolygon::IsRemovePointsOkayTerrain( EditSession *edit )
{
	TerrainPolygon tempPoly( grassTex );

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( !curr->selected )
		{
			tempPoly.AddPoint( new TerrainPoint(*curr) );
		}
	}

	tempPoly.FixWinding();

	bool isPolyValid = edit->IsPolygonValid( &tempPoly, this );

	
	return isPolyValid;
}

//0 means a window came up and they canceled. -1 means no enemies were in danger on that polygon, 1 means that you confirmed to delete the enemies
int TerrainPolygon::IsRemovePointsOkayEnemies( EditSession *edit )
{
	for( EnemyMap::iterator mapIt = enemies.begin(); mapIt != enemies.end(); ++mapIt )
	{
		for( list<ActorPtr>::iterator it = (*mapIt).second.begin(); it != (*mapIt).second.end(); ++it )
		{
			TerrainPoint *edgeEnd = (*it)->groundInfo->edgeStart->next;
			if( edgeEnd == NULL )
				edgeEnd = (*it)->groundInfo->ground->pointStart;

			if( (*it)->type->CanBeGrounded() && 
				( (*(*it)->groundInfo->edgeStart).selected || edgeEnd->selected ) )
			{
				bool removeSelectedActors = edit->ConfirmationPop("1 or more enemies will be removed by deleting these points.");

				if( removeSelectedActors )
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

void TerrainPolygon::MoveSelectedPoints( Vector2i move )
{
	movingPointMode = true;

	int ind = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			curr->pos += move;
			if( curr->gate != NULL )
			{
				curr->gate->UpdateLine();
			}
		}
		UpdateLineColor(lines, curr, ind);
		ind += 2;
	}
}

sf::IntRect TerrainPolygon::GetAABB()
{
	return IntRect(left, top, right - left, bottom - top);
}

bool TerrainPolygon::IsClockwise()
{
	assert( numPoints > 0 );

	int pointCount = numPoints;
    Vector2i *pointArray = new Vector2i[pointCount];

	int i = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		pointArray[i] = curr->pos;
		++i;
	}

    long long int sum = 0;
	for (int i = 0; i < pointCount; ++i)
    {
        Vector2<long long int> first, second;
		
        if (i == 0)
        {
			first.x = pointArray[pointCount - 1].x;
			first.y = pointArray[pointCount - 1].y;
        }
        else
        {
            first.x = pointArray[i - 1].x;
			first.y = pointArray[i - 1].y;

        }
        second.x = pointArray[i].x;
		second.y = pointArray[i].y;

        sum += (second.x - first.x) * (second.y + first.y);
    }

	delete [] pointArray;

    return sum < 0;
}

TerrainPolygon *TerrainPolygon::Copy()
{
	TerrainPolygon *newPoly = new TerrainPolygon(*this, true);
	newPoly->Finalize();
	return newPoly;
}

void TerrainPolygon::CopyPoints(TerrainPolygon *poly, bool storeSelected )
{
	bool sel;
	if (storeSelected)
	{
		sel = poly->pointStart->selected;
	}
	else
	{
		sel = false;
	}

	TerrainPoint *start = new TerrainPoint(poly->pointStart->pos, sel );
	pointStart = start;
	
	TerrainPoint *prev = pointStart;
	TerrainPoint *it = poly->pointStart->next;
	TerrainPoint *newPoint;

	for (; it != NULL; it = it->next)
	{
		if (storeSelected)
		{
			sel = it->selected;
		}
		else
		{
			sel = false;
		}

		newPoint = new TerrainPoint(it->pos, sel);
		prev->next = newPoint;
		newPoint->prev = prev;
		prev = newPoint;
	}

	pointEnd = prev;
	numPoints = poly->numPoints;
}

bool TerrainPolygon::PointOnBorder(V2d &point)
{
	TerrainPoint *prev = pointEnd;
	for (TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next)
	{
		if (EditSession::PointOnLine(V2d(prev->pos), V2d(curr->pos), point))
		{
			return true;
		}
	}
	return false;
}

void TerrainPolygon::CopyPoints(TerrainPoint *&start, TerrainPoint *&end, bool storeSelected )
{
	TerrainPoint *copyCurr = NULL;
	TerrainPoint *copyPrev = NULL;
	TerrainPoint *prev = pointEnd;
	int numNewPoints = 0;
	bool sel;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if (storeSelected)
		{
			sel = curr->selected;
		}
		else
		{
			sel = false;
		}
		copyCurr = new TerrainPoint( curr->pos, sel );

		numNewPoints++;

		if( curr == pointStart )
		{
			start = copyCurr;
		}
		else if( curr == pointEnd )
		{
			end = copyCurr;
		}
		
		if( copyPrev != NULL )
		{
			copyCurr->prev = copyPrev;
			copyPrev->next = copyCurr;
		}
		copyPrev = copyCurr;
	}
}

bool TerrainPolygon::IsTouchingEnemiesFromPoly(TerrainPolygon *p)
{
	for (EnemyMap::iterator it = p->enemies.begin(); it != p->enemies.end(); ++it)
	{
		for (list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait)
		{
			sf::VertexArray &bva = (*ait)->boundingQuad;

			if (EditSession::QuadPolygonIntersect(p, Vector2i(bva[0].position.x, bva[0].position.y),
				Vector2i(bva[1].position.x, bva[1].position.y), Vector2i(bva[2].position.x, bva[2].position.y),
				Vector2i(bva[3].position.x, bva[3].position.y)))
			{
				return true;
			}
		}
	}

	return false;
}

//returns true if LinesIntersect or 
bool TerrainPolygon::IsTouching( TerrainPolygon *p  )
{
	if (p == this)
		return false;

	if( left <= p->right && right >= p->left && top <= p->bottom && bottom >= p->top )
		return LinesIntersect(p);

	return false;
}

void TerrainPolygon::ShowGrass( bool show )
{
	
	VertexArray & grassVa = *grassVA;
	for( int i = 0; i < numGrassTotal; ++i )
	{
		if( show )
		{
			if( grassVa[i*4].color.a == 0 )
			{
				grassVa[i*4].color.a = 50;
				grassVa[i*4+1].color.a = 50;
				grassVa[i*4+2].color.a = 50;
				grassVa[i*4+3].color.a = 50;
			}
			isGrassShowing = true;
		}
		else 
		{
			if( grassVa[i*4].color.a == 50 )
			{
				grassVa[i*4].color.a = 0;
				grassVa[i*4+1].color.a = 0;
				grassVa[i*4+2].color.a = 0;
				grassVa[i*4+3].color.a = 0;
			}
			isGrassShowing = false;
		}
		/*(else if( grassVa[i*4].color.a == 255 )
		{
			grassVa[i*4].color.a = 255;
			grassVa[i*4+1].color.a = 255;
			grassVa[i*4+2].color.a = 255;
			grassVa[i*4+3].color.a = 255;
		}*/
	}
}



bool TerrainPolygon::BoundsOverlap( TerrainPolygon *poly )
{
	if( left <= poly->right && right >= poly->left && top <= poly->bottom && bottom >= poly->top )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TerrainPolygon::LinesIntersect( TerrainPolygon *poly )
{
	//my lines vs his lines
	for( TerrainPoint *my = pointStart; my != NULL; my = my->next )
	{
		TerrainPoint *myPrev;
		if( my == pointStart )
		{
			myPrev = pointEnd;
		}
		else
		{
			myPrev = my->prev;
		}

		for( TerrainPoint *pcurr = poly->pointStart; pcurr != NULL; pcurr = pcurr->next )
		{
			TerrainPoint *prev;
			if( pcurr == poly->pointStart )
			{
				prev = poly->pointEnd;
			}
			else
			{
				prev = pcurr->prev;
			}

			LineIntersection li = EditSession::SegmentIntersect( (*myPrev).pos, my->pos, (*prev).pos, pcurr->pos );
			if( !li.parallel )
			{
				return true;
			}
		}
	}
	return false;
}

bool TerrainPolygon::IsCompletionValid()
{
	EditSession *sess = EditSession::GetSession();

	if (numPoints < 3)
		return false;

	bool linesIntersect = LinesIntersectInProgress(pointStart->pos);
	if (linesIntersect)
	{
		//cout << "lines intersect" << endl;
		return false;
	}

	double minEdge = sess->GetZoomedMinEdgeLength();

	if (PointsTooCloseToSegInProgress(pointStart->pos, minEdge, true))
	{
		return false;
	}

	if (sess->PolyIntersectsGates(this))
	{
		return false;
	}
		

	return true;
}

bool TerrainPolygon::LinesIntersectInProgress(Vector2i p)
{
	//my lines vs his lines
	TerrainPoint *curr = pointStart;
	TerrainPoint *next = NULL;
	while (curr->next != NULL)
	{
		next = curr->next;

		if (next == pointEnd)
		{
			V2d a(curr->pos - next->pos);
			V2d b(p - next->pos);

			double d = dot(normalize(a), normalize(b));

			//cout << "a: " << a.x << ", " << a.y << ", b: " << b.x << ", " << b.y << endl;
			//cout << "diff: " << d << endl;

			if (d > .999)
			{
				return true;
			}


			return false;
		}


		LineIntersection li = EditSession::LimitSegmentIntersect(curr->pos, next->pos, pointEnd->pos, p );
		if (!li.parallel)
		{
			return true;
		}

		curr = next;
	}

	return false;
}

//buggy?
bool TerrainPolygon::PointTooCloseToPoints( Vector2i point, int minDistance )
{
	V2d p( point.x, point.y );
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		V2d currP( curr->pos.x, curr->pos.y );
		V2d diff = p - currP;
		

		if( lengthSqr( diff ) < minDistance * minDistance )
		{
			//cout << "blah: " << lengthSqr( diff ) << endl;
			//cout << "diff: " << diff.x << ", " << diff.y << endl;
			return true;
		}
	}
	return false;
}

bool TerrainPolygon::Contains( TerrainPolygon *poly )
{
	if (poly == this)
		return false;

	//hes inside me w/ no intersection
	if (poly->left < left || poly->top < top || poly->right > right || poly->bottom > bottom)
	{
		return false;
	}


	for( TerrainPoint *pcurr = poly->pointStart; pcurr != NULL; pcurr = pcurr->next )
	{
		//if all points are inside me
		if( !ContainsPoint( Vector2f( pcurr->pos.x, pcurr->pos.y ) ) )
		{
			return false;
		}
	}

	return true;
}

bool TerrainPolygon::PointTooClose( sf::Vector2i point, int minDistance, bool inProgress )
{
	bool a = PointTooCloseToPoints( point, minDistance );
	bool b = PointTooCloseToLines( point, minDistance, inProgress );
	if( a || b )
	{
		/*if (a && b)
		{
			cout << "both are too close" << endl;
		}
		else if( a )
			cout << "A point too close" << endl;
		else
			cout << "B point too close" << endl;*/

		return true;
	}
	else
	{
		return false;
	}
}

bool TerrainPolygon::PointTooCloseToLines( sf::Vector2i point, int minDistance, bool inProgress)
{
	V2d p( point.x, point.y );

	TerrainPoint *pcurr = pointStart;
	TerrainPoint *prev = pointEnd;

	for( ; pcurr != NULL; pcurr = pcurr->next )
	{
		if (inProgress && pcurr == pointStart)
		{
			continue;
		}

		if( pcurr == pointStart )
		{
			prev = pointEnd;
		}
		else
		{
			prev = pcurr->prev;
		}
		
		if( SegmentWithinDistanceOfPoint( prev->pos, pcurr->pos, point, minDistance ) )
		{
			return true;
		}
	}
	return false;
}

//points are circles, and the lines are bars, and you're testing to see if a point is within that or not.
bool TerrainPolygon::SegmentWithinDistanceOfPoint( sf::Vector2i startSeg, sf::Vector2i endSeg, sf::Vector2i testPoint, int distance )
{
	V2d p( testPoint.x, testPoint.y );

	V2d v0 = V2d( startSeg.x, startSeg.y );
	V2d v1 = V2d( endSeg.x, endSeg.y );
	V2d edgeDir = normalize( v1 - v0 );

	double quant = dot( p - v0, edgeDir );
	double offQuant = cross( p - v0, edgeDir );
	bool nearOnAxis = quant > 0 && quant < length( v1 - v0 );

	bool nearOffAxis = abs( offQuant ) < distance;

	if( nearOnAxis && nearOffAxis )
	{
		//cout << "false type two. quant:" << quant << ", offquant: " << offQuant << endl;
		//cout << "p: " << p.x << ", " << p.y << endl;
		//cout << "v0: " << v0.x << ", " << v0.y << endl;
		//cout << "v1: " << v1.x << ", " << v1.y << endl;
		return true;
	}

	return false;
}

TerrainPoint *TerrainPolygon::GetClosePoint(double radius, V2d &wPos)
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

bool TerrainPolygon::IsCloseToFirstPoint(double radius, V2d &p)
{
	if (length(p - V2d(pointStart->pos.x, pointStart->pos.y)) <= radius)
	{
		return true;
	}

	return false;
}

bool TerrainPolygon::IsPoint(sf::Vector2i &p)
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

sf::Rect<int> TerrainPolygon::TempAABB()
{
	assert( numPoints > 1 );
	TerrainPoint *curr = pointStart;
	int l = curr->pos.x;
	int r = curr->pos.x;
	int t = curr->pos.y;
	int b = curr->pos.y;
	curr = curr->next;
	for( ; curr != NULL; curr = curr->next )
	{
		l = min( curr->pos.x, l);
		r = max( curr->pos.x, r);
		t = min( curr->pos.y, t);
		b = max( curr->pos.y, b );
	}

	return Rect<int>( l, t, r- l, b - t );
}

//ISELECTABLE FUNCTIONS

bool TerrainPolygon::Intersects( sf::IntRect rect )
{
	TerrainPolygon poly( grassTex );
	poly.AddPoint( new TerrainPoint( Vector2i( rect.left, rect.top ), false ) );
	poly.AddPoint( new TerrainPoint( Vector2i( rect.left + rect.width, rect.top ), false ) );
	poly.AddPoint( new TerrainPoint( Vector2i( rect.left + rect.width, rect.top + rect.height ), false ) );
	poly.AddPoint( new TerrainPoint( Vector2i( rect.left, rect.top + rect.height ), false ) );

	poly.UpdateBounds();

	if( IsTouching( &poly ) || poly.Contains( this ) ) //don't need this contains polys cuz im just using this for selection for now
	{
		return true;
	}
	else
		return false;
}

bool TerrainPolygon::IsPlacementOkay()
{
	return false;
}
	//void Move( sf::Vector2i delta );
void TerrainPolygon::BrushDraw( sf::RenderTarget *target, bool valid )
{
	target->draw(lines, numPoints * 2, sf::Lines);
	//cout << "brush draw polygon" << endl;
}

void TerrainPolygon::Draw( sf::RenderTarget *target )
{
}

void TerrainPolygon::GetDetailedIntersections(TerrainPolygon *poly, std::list<DetailedInter> &outInters)
{
	outInters.clear();
		//list<DetailedInter> inters;
		//my lines vs his lines
	for (TerrainPoint *my = pointStart; my != NULL; my = my->next)
	{
		TerrainPoint *myPrev;
		if (my == pointStart)
		{
			myPrev = pointEnd;
		}
		else
		{
			myPrev = my->prev;
		}

		for (TerrainPoint *pcurr = poly->pointStart; pcurr != NULL; pcurr = pcurr->next)
		{
			TerrainPoint *prev;
			if (pcurr == poly->pointStart)
			{
				prev = poly->pointEnd;
			}
			else
			{
				prev = pcurr->prev;
			}

			LineIntersection li = EditSession::SegmentIntersect((*myPrev).pos, my->pos, (*prev).pos, pcurr->pos);
			if (!li.parallel)
			{

				//Vector2i pos( li.position.x + .5, li.position.y + .5 ); //rounding
				outInters.push_back(DetailedInter(myPrev, li.position, prev));
				//return true;
			}
		}
	}
}
//returns the intersections w/ the terrain point values of THIS polygon, NOT poly
void TerrainPolygon::GetIntersections( TerrainPolygon *poly, std::list<Inter> &outInters)
{
	//my lines vs his lines
	for( TerrainPoint *my = pointStart; my != NULL; my = my->next )
	{
		TerrainPoint *myPrev;
		if( my == pointStart )
		{
			myPrev = pointEnd;
		}
		else
		{
			myPrev = my->prev;
		}

		for( TerrainPoint *pcurr = poly->pointStart; pcurr != NULL; pcurr = pcurr->next )
		{
			TerrainPoint *prev;
			if( pcurr == poly->pointStart )
			{
				prev = poly->pointEnd;
			}
			else
			{
				prev = pcurr->prev;
			}

			LineIntersection li = EditSession::SegmentIntersect( (*myPrev).pos, my->pos, (*prev).pos, pcurr->pos );

			if (length(li.position - V2d(my->pos)) == 0)
			{
				continue;
			}

			if( !li.parallel )
			{
				
				//Vector2i pos( li.position.x + .5, li.position.y + .5 ); //rounding
				outInters.push_back( Inter( myPrev, li.position ) );
				//return true;
			}
		}
	}
}

void TerrainPolygon::CopyPointsToClipperPath(ClipperLib::Path & p)
{
	TerrainPoint *curr = pointStart;
	while (curr != NULL)
	{
		p << ClipperLib::IntPoint(curr->pos.x, curr->pos.y);
		curr = curr->next;
	}
}

void TerrainPolygon::AddPointsFromClipperPath(ClipperLib::Path &p)
{
	for (auto it = p.begin(); it != p.end(); ++it )
	{
		AddPoint(new TerrainPoint(Vector2i((*it).X, (*it).Y), false));
	}
}

void TerrainPolygon::AddPointsFromClipperPath(ClipperLib::Path &p, ClipperLib::Path &clipperIntersections,
	list<TerrainPoint*> &intersections)
{
	TerrainPoint *t;
	for (auto it = p.begin(); it != p.end(); ++it)
	{
		t = new TerrainPoint(Vector2i((*it).X, (*it).Y), false);
		for (auto intersectIt = clipperIntersections.begin(); intersectIt != clipperIntersections.end(); ++intersectIt)
		{
			if ((*intersectIt).X == (*it).X && (*intersectIt).Y == (*it).Y)
			{
				intersections.push_back(t);
				break;
			}
		}

		AddPoint(t);
	}
}

void TerrainPolygon::AddGatesToBrush(Brush *b,
	list<GateInfoPtr> &gateInfoList)
{
	TerrainPoint *pCurr = pointStart;
	bool okGate = true;
	while (pCurr != NULL)
	{
		okGate = true;
		if (pCurr->gate != NULL)
		{
			for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
			{
				if ((*it) == pCurr->gate)
				{
					okGate = false;
					break;
				}
			}
			if (okGate)
			{
				SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>(pCurr->gate);
				b->AddObject(sp1);
				gateInfoList.push_back(pCurr->gate);
			}

		}
		pCurr = pCurr->next;
	}
}

void TerrainPolygon::AddEnemiesToBrush(Brush *b)
{
	for (map<TerrainPoint*, std::list<ActorPtr>>::iterator
		mit = enemies.begin(); mit != enemies.end(); ++mit)
	{
		for (auto eit = (*mit).second.begin(); eit != (*mit).second.end(); ++eit)
		{
			SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>((*eit));
			b->AddObject(sp1);
		}
	}
}

void TerrainPolygon::AddGatesToList(std::list<GateInfoPtr> &gates)
{
	TerrainPoint *curr = pointStart;
	bool alreadyInList = false;
	while (curr != NULL)
	{
		if (curr->gate != NULL)
		{
			alreadyInList = false;
			for (auto it = gates.begin(); it != gates.end(); ++it)
			{
				if ((*it) == curr->gate)
				{
					alreadyInList = true;
					break;
				}
			}

			if (!alreadyInList)
			{
				gates.push_back(curr->gate);
			}
		}
		curr = curr->next;
	}
}

TerrainPoint::TerrainPoint( sf::Vector2i &p, bool s )
	:pos( p ), selected( s ), gate( NULL ), prev( NULL ), next( NULL )
{
}

bool TerrainPoint::ContainsPoint( Vector2f test )
{
	bool contains = length( V2d( test.x, test.y ) - V2d( pos.x, pos.y ) ) <= POINT_RADIUS;
	return contains;
}



bool TerrainPoint::Intersects( IntRect rect )
{
	return false;
}

bool TerrainPoint::HasPrimaryGate(Vector2i &gateDir)
{
	gateDir = Vector2i(0, 0);
	if (gate != NULL)
	{
		Vector2i myPoint, otherPoint;
		if (gate->point0 == this)
		{
			myPoint = gate->point0->pos;
			otherPoint = gate->point1->pos;
		}
		else
		{
			myPoint = gate->point1->pos;
			otherPoint = gate->point0->pos;
		}

		Vector2i diff = otherPoint - myPoint;
		if (diff.y == 0 )
		{
			if (diff.x > 0)
			{
				gateDir.x = 1;
			}
			else if( diff.x < 0 )
			{
				gateDir.x = -1;
			}
			else
			{
				assert(0);
			}
		}
		else if (diff.x == 0)
		{
			if (diff.y > 0)
			{
				gateDir.y = 1;
			}
			else if( diff.y < 0 )
			{
				gateDir.y = -1;
			}
			else
			{
				assert(0);
			}
		}

		return true;
	}

	return false;
}