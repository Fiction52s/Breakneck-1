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
#include <set>

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

bool TerrainPolygon::CanApply()
{
	bool applyOkay = true;
	bool linesNotOkay = true;
	bool containsNotOkay = true;
	for(list<PolyPtr>::iterator it = session->polygons.begin() ; it != session->polygons.end(); ++it )
	{
		if( (*it).get() == this )
			continue;
		
		linesNotOkay = this->LinesTooClose((*it).get(), session->minimumEdgeLength) || this->LinesIntersect((*it).get());
		containsNotOkay = this->Contains((*it).get()) || (*it)->Contains(this);

		//fail if intersecting others. ignore containment when dealing w/ inverses for now
		if( linesNotOkay || ( containsNotOkay && !((*it)->inverse || this->inverse )) )
		{
			applyOkay = false;
			break;
		}

		//else if( this->LinesIntersect( (*it).get() ) )
		//{
			//not too close and I intersect, so I can add
			//intersectingPolys.push_back( (*it) );

		//}
	}

	return applyOkay;
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

bool CanApply()
{
	//can only apply if:
	//-its not too close to other polygons
	//-its not overlapping an actor who cares about collision, and none of its actors are overlapping other polygons
	//-its not intersecting any gates and none of its gates are intersecting other polygons

	/*Rect<int> currAABB( (*polyIt)->left, (*polyIt)->top, (*polyIt)->right - (*polyIt)->left,
			(*polyIt)->bottom - (*polyIt)->top);
		currAABB.left -= minimumEdgeLength;
		currAABB.top -= minimumEdgeLength;
		currAABB.width += minimumEdgeLength * 2;
		currAABB.height += minimumEdgeLength * 2;

		if( !polyAABB.intersects( currAABB ) )
		{
			continue;
		}*/

	///commented out for some reason
		//points vs points
		/*for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
		{
			for( PointList::iterator myPit = poly.points.begin(); myPit != poly.points.end(); ++myPit )
			{
				V2d mine( (*myPit).pos.x, (*myPit).pos.y );
				V2d other( (*pit).pos.x, (*pit).pos.y );

				if( length( mine - other ) < minimumEdgeLength )
				{
					return false;
				}
			}
		}*/


		//my points vs his lines
		/*for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			Vector2i oldPoint, currPoint;
			if( my == poly.pointStart )
			{
				oldPoint = poly.pointEnd->pos;
			}
			else
			{
				oldPoint = my->prev->pos;
			}

			currPoint = my->pos;

			if( !IsPointValid( oldPoint, currPoint, (*polyIt).get() ) )
			{

				cout << "a: old: " << oldPoint.x << ", " << oldPoint.y << ", curr: " << currPoint.x << ", " << currPoint.y << endl;
				return false;
			}
			//IsPointValid(
		}

		//his points vs my lines
		for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
		{
			Vector2i oldPoint, currPoint;
			if( pcurr == (*polyIt)->pointStart )
			{
				oldPoint = (*polyIt)->pointEnd->pos;
			}
			else
			{
				oldPoint = pcurr->prev->pos;
			}

			currPoint = pcurr->pos;

			if( !IsPointValid( oldPoint, currPoint, &poly ) )
			{
				cout << "b: old: " << oldPoint.x << ", " << oldPoint.y << ", curr: " << currPoint.x << ", " << currPoint.y << endl;
				return false;
			}
			//IsPointValid(
		}

		//my lines vs his lines
		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			TerrainPoint *myPrev;
			if( my == poly.pointStart )
			{
				myPrev = poly.pointEnd;
			}
			else
			{
				myPrev = my->prev;
			}

			for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
			{
				TerrainPoint *prev;
				if( pcurr == (*polyIt)->pointStart )
				{
					prev = (*polyIt)->pointEnd;
				}
				else
				{
					prev = pcurr->prev;
				}

				LineIntersection li = SegmentIntersect( (*myPrev).pos, my->pos, (*prev).pos, pcurr->pos );
				if( !li.parallel )
				{
					return false;
				}
			}
		}



		//hes inside me w/ no intersection
		for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
		//for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
		{
			if( poly.ContainsPoint( Vector2f( pcurr->pos.x, pcurr->pos.y ) ) )
			{
				cout << "c" << endl;
				return false;
			}
		}


		//im inside him w/ no intersection
		//for( PointList::iterator myPit = poly.points.begin(); myPit != poly.points.end(); ++myPit )
		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			if( (*polyIt)->ContainsPoint( Vector2f( my->pos.x, my->pos.y ) ) )
			{
				cout << "d" << endl;
				return false;
			}
		}

		//for( PointList::iterator pit = poly.points.begin(); pit != poly.points.end(); ++pit )
		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			Vector2i oldPoint, currPoint;
			if( my == poly.pointStart )
			{
				oldPoint = poly.pointEnd->pos;
			}
			else
			{
				oldPoint = my->prev->pos;
			}

			currPoint = my->pos;

			//for( list<GateInfo*>::iterator it = (*polyIt)->attachedGates.begin(); it != (*polyIt)->attachedGates.end(); ++it )
			//{
			//	LineIntersection li = LimitSegmentIntersect( oldPoint, currPoint, (*it)->v0, (*it)->v1 );
			//	if( !li.parallel )
			//	{
			//		return false;
			//	}
			//}
		}


		for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
		//for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
		{
			Vector2i oldPoint, currPoint;
			if( pcurr == (*polyIt)->pointStart )
			{
				oldPoint = (*polyIt)->pointEnd->pos;
			}
			else
			{
				oldPoint = pcurr->prev->pos;
			}

			currPoint = pcurr->pos;

			//for( list<GateInfo*>::iterator it = poly.attachedGates.begin(); it != poly.attachedGates.end(); ++it )
			//{
			//	LineIntersection li = LimitSegmentIntersect( oldPoint, currPoint, (*it)->v0, (*it)->v1 );
			//	if( !li.parallel )
			//	{
			//		return false;
			//	}
			//}
		}

		//me touching his enemies
		for( EnemyMap::iterator it = (*polyIt)->enemies.begin(); it != (*polyIt)->enemies.end(); ++it )
		{
			for( list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
			{
				//need to round these floats probably
				//cout << "calling this" << endl;
				sf::VertexArray &bva = (*ait)->boundingQuad;
				//V2d along = (*ait)->groundInfo->
				if( QuadPolygonIntersect( &poly, Vector2i( bva[0].position.x, bva[0].position.y ), 
					Vector2i( bva[1].position.x, bva[1].position.y ), Vector2i( bva[2].position.x, bva[2].position.y ),
						Vector2i( bva[3].position.x, bva[3].position.y ) ) )
				{
					
					//cout << "poly top: " << poly.top << endl;
					//cout << "other bottom: " << (*polyIt)->bottom << endl;

					//cout << "touched polygon!" << endl;
					return false;
				}
				else
				{
					//cout << "no collision" << endl;
				}
			}
		}

		//him touching my enemies
		for( EnemyMap::iterator it = poly.enemies.begin(); it != poly.enemies.end(); ++it )
		{
			for( list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
			{
				//need to round these floats probably
				//cout << "calling this" << endl;
				sf::VertexArray &bva = (*ait)->boundingQuad;
				//V2d along = (*ait)->groundInfo->
				if( QuadPolygonIntersect( (*polyIt).get(), Vector2i( bva[0].position.x, bva[0].position.y ), 
					Vector2i( bva[1].position.x, bva[1].position.y ), Vector2i( bva[2].position.x, bva[2].position.y ),
						Vector2i( bva[3].position.x, bva[3].position.y ) ) )
				{
					
					//cout << "poly top: " << poly.top << endl;
					//cout << "other bottom: " << (*polyIt)->bottom << endl;

					//cout << "touched polygon!" << endl;
					return false;
				}
				else
				{
					//cout << "no collision" << endl;
				}
			}
		}

		
		
	}

	if( PolyIntersectGate( poly ) )
		return false;*/

	return true;
}

bool TerrainPolygon::CanAdd()
{
	return false;
}

void TerrainPolygon::Deactivate(EditSession *edit, SelectPtr select )
{
	cout << "deactivating polygon" << endl;
	PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( select );

	edit->polygons.remove(poly);

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
				//curr->gate->edit = NULL;
				//edit->gates.remove( curr->gate );
			}
		}
	}
}

void TerrainPolygon::Activate( EditSession *edit, SelectPtr select )
{
	PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( select );

	//cout << "num polygons before: " << edit->polygons.size();
	edit->polygons.push_back(poly);
	//cout << "num polygons is now: " << edit->polygons.size();
	
	if (inverse)
	{
		edit->inversePolygon = poly;
	}
	
	/*if (enemies.size() == 1)
	{

	}
	else
	{
		assert(0);
	}*/
	
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
				//curr->gate->edit = edit;
				//edit->gates.push_back( curr->gate );
			}
		}
	}
}

TerrainPolygon::TerrainPolygon( TerrainPolygon &poly, bool pointsOnly )
	:ISelectable( ISelectable::TERRAIN )
{
	layer = 0;
	inverse = poly.inverse;
	grassTex = poly.grassTex;
	terrainWorldType = poly.terrainWorldType;
	terrainVariation = poly.terrainVariation;
	//SetMaterialType( poly.terrainWorldType, poly.terrainVariation );
	if( pointsOnly )
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

		poly.CopyPoints( pointStart, pointEnd );
		numPoints = poly.numPoints;
	}
	else
	{

		assert( false && "havent implemented yet" );
	}	
}

void TerrainPolygon::AlignExtremes( double primLimit )
{
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *prev;
		if( curr == pointStart )
		{
			prev = pointEnd;
		}
		else
		{
			prev = curr->prev;
		}

		TerrainPoint *next;
		if( curr == pointEnd )
		{
			next = pointStart;
		}
		else
		{
			next = curr->next;
		}

		V2d prevExtreme( 0, 0 );
		V2d nextExtreme( 0, 0 );
		Vector2i prevVec = curr->pos - prev->pos;
		Vector2i nextVec = curr->pos - next->pos;
		V2d prevNormVec = normalize( V2d( prevVec.x, prevVec.y ) );
		V2d nextNormVec = normalize( V2d( nextVec.x, nextVec.y ) );

		if( prevNormVec.x > primLimit )
			prevExtreme.x = 1;
		else if( prevNormVec.x < -primLimit )
			prevExtreme.x = -1;
		if( prevNormVec.y > primLimit )
			prevExtreme.y = 1;
		else if( prevNormVec.y < -primLimit )
			prevExtreme.y = -1;

		if( nextNormVec.x > primLimit )
			nextExtreme.x = 1;
		else if( nextNormVec.x < -primLimit )
			nextExtreme.x = -1;
		if( nextNormVec.y > primLimit )
			nextExtreme.y = 1;
		else if( nextNormVec.y < -primLimit )
			nextExtreme.y = -1;


		if( finalized )
		{
			if( !curr->selected )
			{
				continue;
			}

			bool prevValid = true, nextValid = true;
			if( nextNormVec.x == 0 || nextNormVec.y == 0 )
			{
				nextValid = false;
			} 

			if( prevNormVec.x == 0 || prevNormVec.y == 0 )
			{
				prevValid = false;
			} 

			if( prevValid && nextValid )
			{
				if( prevExtreme.x != 0 )
				{
					if( nextExtreme.x != 0 )
					{
						double sum = curr->pos.y + prev->pos.y + next->pos.y;
						int avg = round(sum / 3.0);
						prev->pos.y = avg;
						curr->pos.y = avg;
						next->pos.y = avg;

						//cout << "a" << endl;
						//prev->pos.y = curr->pos.y;
						//next->pos.y = curr->pos.y;
					}
					else if( nextExtreme.y != 0 )
					{
						//cout << "b" << endl;
						curr->pos.y = prev->pos.y;
						curr->pos.x = next->pos.x;
					}
					else
					{
						//cout << "c" << endl;
						curr->pos.y = prev->pos.y;
					}
				}
				else if( prevExtreme.y != 0 )
				{
					if( nextExtreme.y != 0 )
					{
						double sum = curr->pos.x + prev->pos.x + next->pos.x;
						int avg = round(sum / 3.0);
						prev->pos.x = avg;
						curr->pos.x = avg;
						next->pos.x = avg;



						//cout << "d" << endl;
						//prev->pos.x = curr->pos.x;
						//next->pos.x = curr->pos.x;
					}
					else if( nextExtreme.x != 0 )
					{
						//double sum = curr->pos.x + prev->pos.x + next->pos.x;
						//int avg = round(sum / 3.0);
						//prev->pos.x = avg;
						//curr->pos.x = avg;
						//next->pos.x = avg;
						//cout << "e" << endl;

						curr->pos.x = prev->pos.x;
						curr->pos.y = next->pos.y;
					}
					else
					{
						//cout << "f" << endl;
						curr->pos.x = prev->pos.x;
					}
				}
			}
			else if( prevValid )
			{
				if( prevExtreme.y != 0 )
				{
					double sum = curr->pos.x + prev->pos.x;
					int avg = round(sum / 2.0);
					curr->pos.x = avg;
					prev->pos.x = avg;
					//curr->pos.x = prev->pos.x;
				}
				else if( prevExtreme.x != 0 )
				{
					double sum = curr->pos.y + prev->pos.y;
					int avg = round(sum / 2.0);
					curr->pos.y = avg;
					prev->pos.y = avg;

					//curr->pos.y = prev->pos.y;
				}
			}
			else if( nextValid )
			{
				if( nextExtreme.y != 0 )
				{
					double sum = curr->pos.x + next->pos.x;
					int avg = round(sum / 2.0);
					curr->pos.x = avg;
					next->pos.x = avg;

					//curr->pos.x = next->pos.x;
				}
				else if( nextExtreme.x != 0 )
				{
					double sum = curr->pos.y + next->pos.y;
					int avg = round(sum / 2.0);
					curr->pos.y = avg;
					next->pos.y = avg;
					//curr->pos.y = next->pos.y;
				}
			}
			else
			{
				continue;
			}
		}
		else
		{
			if( nextNormVec.x == 0 || nextNormVec.y == 0 )
			{
				continue;
			}


			if( nextExtreme.x != 0 )
			{

				double sum = curr->pos.y + next->pos.y;
				int avg = round(sum / 2.0);
				curr->pos.y = avg;
				next->pos.y = avg;

				//curr->pos.y = next->pos.y;
				//cout << "lining up x" << endl;
			}

			if( nextExtreme.y != 0 )
			{
				double sum = curr->pos.x + next->pos.x;
				int avg = round(sum / 2.0);
				curr->pos.x = avg;
				next->pos.x = avg;

				//curr->pos.x = next->pos.x;
				//cout << "lining up y" << endl;
			}
		}

		


	}
}

TerrainPolygon::~TerrainPolygon()
{
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;

	if( grassVA != NULL )
		delete grassVA;

	//DestroyEnemies();

	ClearPoints();
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

void TerrainPolygon::SetMaterialType( int world, int variation )
{
	terrainWorldType = (TerrainPolygon::TerrainWorldType)world;
	terrainVariation = variation;
	Color sCol( 0x77, 0xBB, 0xDD );
	//factor in variation later
	//Color newColor;
	switch( world )
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
	}

	selectCol = sCol;
	//selectCol = 

	int vCount = va->getVertexCount();
	VertexArray &v = *va;
	for( int i = 0; i < vCount; ++i )
	{	
		v[i].color = fillCol;
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

	int testExtra = 500;
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
	for (int i = 0; i < numPoints; ++i)
	{
		delete polyline[i];
		//	delete tris[i];
	}

	if( numPoints > 0 )
	{
		int i = 0;
		curr = pointStart;
		lines[0] = sf::Vector2f( curr->pos.x, curr->pos.y );
		lines[2 * numPoints - 1 ] = sf::Vector2f( curr->pos.x, curr->pos.y );
		curr = curr->next;
		++i;
		while( curr != NULL )
		{
			lines[i] = sf::Vector2f( curr->pos.x, curr->pos.y );
			lines[++i] = sf::Vector2f( curr->pos.x, curr->pos.y ); 
			++i;
			curr = curr->next;
		}
	}

	UpdateBounds();
	

	double grassSize = 22;
	double grassSpacing = -5;

	numGrassTotal = 0;
	int inds = 0;
	for( curr = pointStart; curr != NULL; curr = curr->next )
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = pointStart->pos;
		}
		else
		{
			//++temp;
			next = temp->pos;
			//--temp;
		}

		V2d v0( curr->pos.x, curr->pos.y );
		V2d v1( next.x, next.y );


		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );
				
		int num = floor( remainder ) + 1;

		numGrassTotal += num;
		//cout << "plus: " << v0.x << ", " << v0.y << " " << v1.x << ", " << v1.y << endl;
		++inds;
	}
	//assert( numGrassTotal !=  0 );
	//cout << "total grass: " << numGrassTotal << endl;
	VertexArray *gva = new VertexArray( sf::Quads, numGrassTotal * 4 );


	VertexArray &grassVa = *gva;

	int i = 0;
	for( curr = pointStart; curr != NULL; curr = curr->next )
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


		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

		int num = floor( remainder ) + 1;

		for( int j = 0; j < num; ++j )
		{
			V2d posd = v0 + (v1- v0) * ((double)j / num);
			Vector2f pos( posd.x, posd.y );

			Vector2f topLeft = pos + Vector2f( -grassSize / 2, -grassSize / 2 );
			Vector2f topRight = pos + Vector2f( grassSize / 2, -grassSize / 2 );
			Vector2f bottomLeft = pos + Vector2f( -grassSize / 2, grassSize / 2 );
			Vector2f bottomRight = pos + Vector2f( grassSize / 2, grassSize / 2 );

			//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
			grassVa[i*4].color.a = 0;
			grassVa[i*4].position = topLeft;
			grassVa[i*4].texCoords = Vector2f( 0, 0 );

			//grassVa[i*4+1].color = Color::Blue;
			//borderVa[i*4+1].color.a = 10;
			grassVa[i*4+1].color.a = 0;
			grassVa[i*4+1].position = bottomLeft;
			grassVa[i*4+1].texCoords = Vector2f( 0, grassSize );

			//grassVa[i*4+2].color = Color::Blue;
			//borderVa[i*4+2].color.a = 10;
			grassVa[i*4+2].color.a = 0;
			grassVa[i*4+2].position = bottomRight;
			grassVa[i*4+2].texCoords = Vector2f( grassSize, grassSize );

			//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
			//borderVa[i*4+3].color.a = 10;
			grassVa[i*4+3].color.a = 0;
			grassVa[i*4+3].position = topRight;
			grassVa[i*4+3].texCoords = Vector2f( grassSize, 0 );
			++i;
		}
		
	
	}

	/*if( grassVA != NULL )
	{
		delete grassVA;
	}*/
	grassVA = gva;	
}

void TerrainPolygon::Finalize()
{
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

	if( numPoints > 0 )
	{
		int i = 0;
		curr = pointStart;
		lines[0] = sf::Vector2f( curr->pos.x, curr->pos.y );
		lines[2 * numPoints - 1 ] = sf::Vector2f( curr->pos.x, curr->pos.y );
		curr = curr->next;
		++i;
		while( curr != NULL )
		{
			lines[i] = sf::Vector2f( curr->pos.x, curr->pos.y );
			lines[++i] = sf::Vector2f( curr->pos.x, curr->pos.y ); 
			++i;
			curr = curr->next;
		}
	}

	UpdateBounds();
	

	double grassSize = 22;
	double grassSpacing = -5;

	numGrassTotal = 0;
	int inds = 0;
	for( curr = pointStart; curr != NULL; curr = curr->next )
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = pointStart->pos;
		}
		else
		{
			//++temp;
			next = temp->pos;
			//--temp;
		}

		V2d v0( curr->pos.x, curr->pos.y );
		V2d v1( next.x, next.y );


		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );
				
		int num = floor( remainder ) + 1;

		numGrassTotal += num;
		//cout << "plus: " << v0.x << ", " << v0.y << " " << v1.x << ", " << v1.y << endl;
		++inds;
	}
	//assert( numGrassTotal !=  0 );
	//cout << "total grass: " << numGrassTotal << endl;
	VertexArray *gva = new VertexArray( sf::Quads, numGrassTotal * 4 );


	VertexArray &grassVa = *gva;

	int i = 0;
	for( curr = pointStart; curr != NULL; curr = curr->next )
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


		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

		int num = floor( remainder ) + 1;

		for( int j = 0; j < num; ++j )
		{
			V2d posd = v0 + (v1- v0) * ((double)j / num);
			Vector2f pos( posd.x, posd.y );

			Vector2f topLeft = pos + Vector2f( -grassSize / 2, -grassSize / 2 );
			Vector2f topRight = pos + Vector2f( grassSize / 2, -grassSize / 2 );
			Vector2f bottomLeft = pos + Vector2f( -grassSize / 2, grassSize / 2 );
			Vector2f bottomRight = pos + Vector2f( grassSize / 2, grassSize / 2 );

			//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
			grassVa[i*4].color.a = 0;
			grassVa[i*4].position = topLeft;
			grassVa[i*4].texCoords = Vector2f( 0, 0 );

			//grassVa[i*4+1].color = Color::Blue;
			//borderVa[i*4+1].color.a = 10;
			grassVa[i*4+1].color.a = 0;
			grassVa[i*4+1].position = bottomLeft;
			grassVa[i*4+1].texCoords = Vector2f( 0, grassSize );

			//grassVa[i*4+2].color = Color::Blue;
			//borderVa[i*4+2].color.a = 10;
			grassVa[i*4+2].color.a = 0;
			grassVa[i*4+2].position = bottomRight;
			grassVa[i*4+2].texCoords = Vector2f( grassSize, grassSize );

			//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
			//borderVa[i*4+3].color.a = 10;
			grassVa[i*4+3].color.a = 0;
			grassVa[i*4+3].position = topRight;
			grassVa[i*4+3].texCoords = Vector2f( grassSize, 0 );
			++i;
		}
		
	
	}

	/*if( grassVA != NULL )
	{
		delete grassVA;
	}*/
	grassVA = gva;	
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

void TerrainPolygon::Cut2( TerrainPoint* startPoint, TerrainPoint*endPoint, boost::shared_ptr<TerrainPolygon> inProgress )
{
	if( inProgress->numPoints < 1 )
	{
		return;
	}

	//inProgress->RemovePoint( inProgress->pointEnd );

	TerrainPolygon *p0 = new TerrainPolygon( grassTex );
	TerrainPolygon *p1 = new TerrainPolygon( grassTex );

	for( TerrainPoint *curr = inProgress->pointStart; curr != NULL; curr = curr->next )
	{
		p0->AddPoint( new TerrainPoint( *curr ) );
		p1->AddPoint( new TerrainPoint( *curr ) );
	}

	TerrainPoint *t = endPoint;
	while( t != startPoint )
	{
		p0->AddPoint( new TerrainPoint( *t ) );
		t = t->next;
		if( t == NULL )
			t = pointStart;
	}

	t = endPoint;
	while( t != startPoint )
	{
		p1->AddPoint( new TerrainPoint( *t ) );
		t = t->prev;
		if( t == NULL )
			t = pointEnd;
	}

	p0->Finalize();
	p1->Finalize();

	session->cutPoly0 = p0;
	session->cutPoly1 = p1;

	session->cutChoose = true;


	/*for( TerrainPoint *curr = inProgress->pointStart; curr != NULL; curr = curr->next )
	{
		p0->AddPoint( new TerrainPoint( *curr ) );
	}*/
}

void TerrainPolygon::Extend2( TerrainPoint* startPoint, TerrainPoint*endPoint, PolyPtr inProgress )
{
	bool specialCW;
	TerrainPoint *prevDest = endPoint->prev;
	TerrainPoint *nextDest = endPoint->next;
	if( prevDest == NULL )
	{
		prevDest = pointEnd;
	}
	else if( nextDest == NULL )
	{
		nextDest = pointStart;
	}

	if( inverse )
	{
		bool add = true;

		FixWinding();

	}

	//if I try to just go from one edge to another it chooses for me?
	if( inProgress->numPoints < 2 )
	{
		return;
	}

	bool startFirst = true;
	TerrainPoint *start = NULL;
	TerrainPoint *end = NULL;

	bool startFound = false;
	bool endFound = false;

	//finds out if start is first or if end is first on the full poly
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr == startPoint )
		{
			start = curr;

			if( endFound )
				break;

			startFound = true;
		}
		else if( curr == endPoint )
		{
			end = curr;

			if( startFound )
				break;

			endFound = true;
			startFirst = false;
		}
	}

	TerrainPolygon newList( grassTex );
	inProgress->AddPoint( new TerrainPoint( *endPoint ) );
	bool inProgresscw = inProgress->IsClockwise();

	if( !inProgresscw )
	{
		inProgress->FixWinding();
		if( startFirst )
		{
			TerrainPoint *temp = end;
			end = start;
			start = temp;
			startFirst = false;
			inProgresscw = true;
		}
		else
		{
			TerrainPoint *temp = end;
			end = start;
			start = temp;
			startFirst = true;
			inProgresscw = true;
			//cout << "changing" << endl;
		}
	}
	else
	{

		/*if( inverse )
		{
			inProgress->FixWindingInverse();
		}*/
	}

	inProgress->RemovePoint( inProgress->pointEnd );

	//if( showKeep == prevDest )
	//{
	//	//go one way
	//	FixWindingInverse();
	//}
	//else if( showKeep == nextDest )
	//{
	//	FixWinding();
	//	
	//	//go the other way
	//}

	//inProgress->FixWindingInverse();

	if( startFirst )
	{	
		for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
		{
			//cout << "normal type: " << inProgresscw << endl;
			if( curr == start )
			{
				for( TerrainPoint *progressCurr = inProgress->pointStart; progressCurr != NULL; progressCurr = progressCurr->next )
				{
					TerrainPoint *tp = new TerrainPoint( *progressCurr );
					tp->gate = progressCurr->gate;
					if( tp->gate != NULL )
					{
						if( progressCurr == tp->gate->point0 )
						{
							tp->gate->point0 = tp;
						}
						else if( progressCurr == tp->gate->point1 )
						{
							tp->gate->point1 = tp;	
						}
						else
						{
							assert( false );
						}
					}

					//cant be enemies here because its the in progress polygon
		
					newList.AddPoint( tp );
				}

				curr = end;
				TerrainPoint *tp = new TerrainPoint( *end );
				tp->gate = end->gate;
				if( tp->gate != NULL )
				{
					if( end == tp->gate->point0 )
					{
						tp->gate->point0 = tp;
					}
					else if( end == tp->gate->point1 )
					{
						tp->gate->point1 = tp;	
					}
					else
					{
						assert( false );
					}
				}

				if( enemies.count( curr ) > 0 )
				{
					list<ActorPtr> &en = newList.enemies[tp];
					en = enemies[curr];
				}

				newList.AddPoint( tp );	
			}
			else
			{
				TerrainPoint *tp = new TerrainPoint( *curr );
				tp->gate = curr->gate;
				if( tp->gate != NULL )
				{
					if( curr == tp->gate->point0 )
					{
						tp->gate->point0 = tp;
					}
					else if( curr == tp->gate->point1 )
					{
						tp->gate->point1 = tp;	
					}
					else
					{
						assert( false );
					}
				}

				if( enemies.count( curr ) > 0 )
				{
					list<ActorPtr> &en = newList.enemies[tp];
					en = enemies[curr];
				}

				newList.AddPoint( tp );
			}
		}
	}	
	else
	{
		//cout << "other type: " << inProgresscw << endl;
		for( TerrainPoint *curr = end; curr != NULL; curr = curr->next )
		{
			if( curr == start )
			{	
				for( TerrainPoint *progressCurr = inProgress->pointStart; progressCurr != NULL; progressCurr = progressCurr->next )
				{
					TerrainPoint *tp = new TerrainPoint( *progressCurr ); 
					tp->gate = progressCurr->gate;
					if( tp->gate != NULL )
					{
						if( progressCurr == tp->gate->point0 )
						{
							tp->gate->point0 = tp;
						}
						else if( progressCurr == tp->gate->point1 )
						{
							tp->gate->point1 = tp;	
						}
						else
						{
							assert( false );
						}
					}

					//in progress so no enemies

					newList.AddPoint( tp );
				}	
				break;
			}
			else
			{
				TerrainPoint *tp = new TerrainPoint( *curr );
				tp->gate = curr->gate;
				if( tp->gate != NULL )
				{
					if( curr == tp->gate->point0 )
					{
						tp->gate->point0 = tp;
					}
					else if( curr == tp->gate->point1 )
					{
						tp->gate->point1 = tp;	
					}
					else
					{
						assert( false );
					}
				}

				if( enemies.count( curr ) > 0 )
				{
					list<ActorPtr> &en = newList.enemies[tp];
					en = enemies[curr];
				}

				newList.AddPoint( tp );
			}
		}
	}

	Reset();


	for( TerrainPoint *curr = newList.pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *tp = new TerrainPoint( *curr );
		tp->gate = curr->gate;
		if( tp->gate != NULL )
		{
			if( curr == tp->gate->point0 )
			{
				tp->gate->point0 = tp;
			}
			else if( curr == tp->gate->point1 )
			{
				tp->gate->point1 = tp;	
			}
			else
			{
				assert( false );
			}
		}

		if( newList.enemies.count( curr ) > 0 )
		{
			list<ActorPtr> &en = enemies[tp];
			en = newList.enemies[curr];
			//cout << "zsize: " << en.size() << endl;
			for( list<ActorPtr>::iterator it = en.begin(); it != en.end(); ++it )
			{
				//cout << "setting new ground on actor params" << endl;
				
				
				
				
				//(*it)->groundInfo->ground = this;

				//extending is broken right now. this line above needs to be uncommented!^
				(*it)->groundInfo->edgeStart = tp;
				assert(tp != NULL);
			}
		}

		AddPoint( tp );
	}

	if( inverse )
	{
		FinalizeInverse();
	}
	else
	{
		Finalize();
	}
	
}

void TerrainPolygon::SwitchGrass( V2d mousePos )
{
	
	VertexArray &grassVa = *grassVA;
	double grassSize = 22;
	double radius = grassSize / 2;
	double grassSpacing = -5;

	int i = 0;


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

		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

		int num = floor( remainder ) + 1;

		for( int j = 0; j < num; ++j )
		{
			V2d pos = v0 + (v1- v0) * ((double)(j )/ num);

			//Vector2f pos( posd.x, posd.y );
				
			if( length( pos - mousePos ) <= radius )
			{
				if( grassVa[i*4].color.a == 50 )
				{
					grassVa[i*4].color.a = 254;
					grassVa[i*4+1].color.a = 254;
					grassVa[i*4+2].color.a = 254;
					grassVa[i*4+3].color.a = 254;
					//cout << "making full: " << i << endl;
				}
				else if( grassVa[i*4].color.a == 255 )
				{
					grassVa[i*4].color.a = 49;
					grassVa[i*4+1].color.a = 49;
					grassVa[i*4+2].color.a = 49;
					grassVa[i*4+3].color.a = 49;
					//cout << "making seethru: " << i << endl;
				}
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
		rt->draw( *va );


	//for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	//{
	//	CircleShape cs;
	//	cs.setRadius( 8 * zoomMultiple );
	//	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

	//	if( curr->selected )
	//		cs.setFillColor( Color::Red );
	//	else
	//		cs.setFillColor( Color::Green );

	//	cs.setPosition( curr->pos.x, curr->pos.y );
	////	rt->draw( cs );
	//}

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

		for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
		{
			if( curr == dontShow )
			{
				continue;
			}
			

			cs.setPosition( curr->pos.x, curr->pos.y );
			rt->draw( cs );
		}
	}

	Vector2i center( (right + left) / 2, (bottom + top) / 2 );

	if( showPath )
	{
		for( list<Vector2i>::iterator it = path.begin(); it != path.end(); ++it )
		{
			CircleShape cs;
			cs.setRadius( 5 * zoomMultiple );
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


	return c;
	//if( !inverse )
	//{
	//	
	//}
	//else
	//{
	//	return !c;

	//}
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
		assert(0);
		return Vector2i(-5, -5);
	}
}

//angle in radians
void TerrainPolygon::RemoveSlivers( double minAngle )
{	
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
		V2d dirA = normalize(prevPos - pos);
		V2d dirB = normalize(nextPos - pos);

		double diff = GetVectorAngleDiffCCW(dirA, dirB);
		double diffCW = GetVectorAngleDiffCW(dirA, dirB);
		if ( diff < minAngle )
		{
			Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, true);
			curr->pos = trimPos;
		}
		else if( diffCW < minAngle )
		{
			Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, false);
			curr->pos = trimPos;
		}
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

//void TerrainPolygon::MovePoint(

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

	bool isPolyValid = edit->IsPolygonValid( tempPoly, this );

	
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

			if( (*it)->type->canBeGrounded && 
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

bool TerrainPolygon::IsMovePointsOkay( EditSession *edit, Vector2i delta )
{
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			curr->pos += delta;
		}
	}

	bool result = edit->IsPolygonValid( *this, this );

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			curr->pos -= delta;
		}
	}

	return result;
}

bool TerrainPolygon::IsMovePointsOkay( EditSession *edit, Vector2i pointGrabDelta, Vector2i *deltas )
{
	//TerrainPolygon tempPoly( grassTex );

	int arraySize = numPoints;

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *next;
		if( curr == pointEnd )
			next = pointStart;
		else
			next = curr->next;
		//eventually this will let you move the points and keep the actors in the right spots. for now just give a popup
		//more of the code is below
		
		if( ( curr->selected || next->selected ) && enemies.count( curr ) > 0 )
		{
			//cout << "move not okay" << endl;
			edit->pointGrab = false;
			edit->MessagePop( "sorry, in this build you can't yet move points\n that have enemies attached to their edges" );
			return false;
		}
	}

	int i = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{


		if( curr->selected )
		{
			curr->pos += pointGrabDelta - deltas[i];
		//	tempPoly.AddPoint( new TerrainPoint( *curr ) );
		}
		/*else
		{
			TerrainPoint *tp = new TerrainPoint( *curr );
			
			tp->pos += pointGrabDelta - deltas[i];

			tempPoly.AddPoint( tp );
		}*/

		++i;
	}

	//eventually going to need this back again!

	/*for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *next;
		if( curr == pointEnd )
			next = pointStart;
		else
			next = curr->next;


		//eventually this will let you move the points and keep the actors in the right spots. for now just give a popup
		
		if( curr->selected || next->selected )
		{
			//edit->MessagePop( "sorry, in this build you can't yet move points\n that have enemies attached to their edges" );
			//return false;
			if( enemies.count( curr ) > 0 )
			{
				list<ActorParams*> &en = enemies[curr];
				for( list<ActorParams*>::iterator it = en.begin(); it != en.end(); ++it )
				{
					(*it)->SetBoundingQuad();
				}
			}
		}
	}*/

	UpdateBounds();

	bool res = edit->IsPolygonValid( *this, this );


	
	bool result = true;
	if( !res )
	{
		result = false;
	}
	/*else
	{
		bool res2 = true;
		for( std::map<std::string, ActorGroup*>::iterator it = edit->groups.begin(); it != edit->groups.end() && res2; ++it )
		{
			for( list<ActorParams*>::iterator ait = (*it).second->actors.begin(); ait != (*it).second->actors.end(); ++ait )
			{
				//need to round these floats probably

				sf::VertexArray &bva = (*ait)->boundingQuad;
				if( edit->QuadPolygonIntersect( this, Vector2i( bva[0].position.x, bva[0].position.y ), 
					Vector2i( bva[1].position.x, bva[1].position.y ), Vector2i( bva[2].position.x, bva[2].position.y ),
					 Vector2i( bva[3].position.x, bva[3].position.y ) ) )
				{
					cout << "polygon collide with quad" << endl;
					res2 = false;
					break;
				}
				else
				{
					cout << "no collision with quad" << endl;
				}
			}
		}
		result = res2;
	}*/
	

	i = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			curr->pos -= pointGrabDelta - deltas[i];
		}
		++i;
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			if( enemies.count( curr ) > 0 )
			{
				list<ActorPtr> &en = enemies[curr];
				for( list<ActorPtr>::iterator it = en.begin(); it != en.end(); ++it )
				{
					(*it)->SetBoundingQuad();
				}
			}
		}
	}

	UpdateBounds();

	return result;
}

void TerrainPolygon::MoveSelectedPoints( Vector2i move )
{
	movingPointMode = true;

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
	}
}

bool TerrainPolygon::IsMovePolygonOkay( EditSession *edit, sf::Vector2i delta )
{
	TerrainPolygon tempPoly( grassTex );

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		//TerrainPoint  *tp =  new TerrainPoint( *curr );
		//tp->pos += delta;
		//tempPoly.AddPoint( tp );

		curr->pos += delta;
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( enemies.count( curr ) > 0 )
		{
			list<ActorPtr> &actors = enemies[curr];
			for( list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it )
			{
				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}
	}


	UpdateBounds();
	//tempPoly.UpdateBounds();

	bool f = edit->IsPolygonExternallyValid( *this, this );
	if( !f )
	{

		cout << "failed delta: " << delta.x << ", " << delta.y << endl;
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		curr->pos -= delta;

		
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( enemies.count( curr ) > 0 )
		{
			list<ActorPtr> &actors = enemies[curr];
			for( list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it )
			{
				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}
	}

	

	UpdateBounds();

	return f;
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

void TerrainPolygon::CopyPoints(TerrainPolygon *poly)
{
	TerrainPoint *start = new TerrainPoint(poly->pointStart->pos, false );
	pointStart = start;
	
	TerrainPoint *prev = pointStart;
	TerrainPoint *it = poly->pointStart->next;
	TerrainPoint *newPoint;

	for (; it != NULL; it = it->next)
	{
		newPoint = new TerrainPoint(it->pos, false);
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

void TerrainPolygon::CopyPoints( TerrainPoint *&start, TerrainPoint *&end )
{
	//start.prev = &end;
	//end.next = &start;
	TerrainPoint *copyCurr = NULL;
	TerrainPoint *copyPrev = NULL;
	TerrainPoint *prev = pointEnd;
	int numNewPoints = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		//cout << "copying " << endl;
		copyCurr = new TerrainPoint( curr->pos, false );

		numNewPoints++;

		if( curr == pointStart )
		{
			start = copyCurr;

			//prev = pointEnd;
		}
		else if( curr == pointEnd )
		{
			end = copyCurr;

			//end->next = start;
			//start->prev = end;
		}
		
		if( copyPrev != NULL )
		{
			copyCurr->prev = copyPrev;
			copyPrev->next = copyCurr;
		}
		copyPrev = copyCurr;
	}


}

//returns true if LinesIntersect or 
bool TerrainPolygon::IsTouching( TerrainPolygon *p  )
{
	//make sure its not ourselves!
	assert( p != this );

	if( left <= p->right && right >= p->left && top <= p->bottom && bottom >= p->top )
		return LinesIntersect(p);

	return false;
	//check aabb
	/*if( left <= p->right && right >= p->left && top <= p->bottom && bottom >= p->top )
	{	
		TerrainPoint *curr = pointStart;
		Vector2i currPos = curr->pos;
		curr = curr->next;
		Vector2i nextPos;
		
		for( ;; curr = curr->next )
		{
			if( curr == NULL )
				curr = pointStart;

			nextPos = curr->pos;


			TerrainPoint *pit = p->pointStart;
			Vector2i pcurr = pit->pos;
			pit = pit->next;

			Vector2i pnext;

			for( ;; pit = pit->next )		
			{
				if( pit == NULL )
					pit = p->pointStart;

				pnext = pit->pos;
			
				LineIntersection li = EditSession::SegmentIntersect( currPos, nextPos, pcurr, pnext );	

				if( !li.parallel )
				{
					//cout << "touching!" << endl;
					return true;
				}

				pcurr = pit->pos;

				if( pit == p->pointStart )
					break;
			}
			currPos = curr->pos;

			if( curr == pointStart )
			{
				break;
			}
		}
	}
	return false;*/
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
			cout << "diff: " << diff.x << ", " << diff.y << endl;
			return true;
		}
	}
	return false;
}

bool TerrainPolygon::Contains( TerrainPolygon *poly )
{
	//hes inside me w/ no intersection
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

bool TerrainPolygon::PointTooClose( sf::Vector2i point, int minDistance )
{
	bool a = PointTooCloseToPoints( point, minDistance );
	bool b = PointTooCloseToLines( point, minDistance );
	if( a || b )
	{
		if( a )
			cout << "A point too close" << endl;
		else
			cout << "B point too close" << endl;

		return true;
	}
	else
	{
		return false;
	}
}

bool TerrainPolygon::PointTooCloseToLines( sf::Vector2i point, int minDistance )
{
	V2d p( point.x, point.y );

	TerrainPoint *pcurr = pointStart;
	TerrainPoint *prev = pointEnd;

	for( ; pcurr != NULL; pcurr = pcurr->next )
	{
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

//doesn't check for line intersections. just point/line distances for validity
bool TerrainPolygon::SegmentTooClose( Vector2i a, Vector2i b, int minDistance )
{
	//if points are too close to points, or too close to lines
	//if my points are too close to his lines
	if( PointTooClose( a, minDistance ) || PointTooClose( b, minDistance ) )
	{
		cout << "pointtoo close" << endl;
		return true;
	}
	else
	{
		for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
		{
			TerrainPoint *prev;
			if( curr == pointStart )
			{
				prev = pointEnd;
			}
			else
			{
				prev = curr->prev;
			}

			if( SegmentWithinDistanceOfPoint( a,b, curr->pos, minDistance ) )
			{
				cout << "line stuff" << endl;
				return true;
			}
		}
		return false;
	}
}

bool TerrainPolygon::LinesTooClose( TerrainPolygon *poly, int minDistance )
{
	//my lines with his points
	for( TerrainPoint *pcurr = poly->pointStart; pcurr != NULL; pcurr = pcurr->next )
	{
		Vector2i oldPoint, currPoint;
		if( pcurr == poly->pointStart )
		{
			oldPoint = poly->pointEnd->pos;
		}
		else
		{
			oldPoint = pcurr->prev->pos;
		}

		currPoint = pcurr->pos;

		if( SegmentTooClose( oldPoint, currPoint, minDistance ) )
		{
			cout << "lines 1" << endl;
			return true;
		}
	}

	//his lines with my points
	for( TerrainPoint *pcurr = pointStart; pcurr != NULL; pcurr = pcurr->next )
	{
		Vector2i oldPoint, currPoint;
		if( pcurr == pointStart )
		{
			oldPoint = pointEnd->pos;
		}
		else
		{
			oldPoint = pcurr->prev->pos;
		}

		currPoint = pcurr->pos;

		if( poly->SegmentTooClose( oldPoint, currPoint, minDistance ) )
		{
			cout << "lines 2" << endl;
			return true;
		}
	}

	/*for( list<GateInfo*>::iterator git = poly->attachedGates.begin(); git != poly->attachedGates.end(); ++git )
	{
		LineIntersection li = LimitSegmentIntersect( (*git)->v0, (*git)->v1, oldPoint, point );
		if( !li.parallel )
		{
		//	return false;
		}
	}*/


	return false;

		//if( !IsPointValid( oldPoint, currPoint, this ) )
		//{
		//	cout << "b: old: " << oldPoint.x << ", " << oldPoint.y << ", curr: " << currPoint.x << ", " << currPoint.y << endl;
		//	return false;
		//}
		//IsPointValid(
}

bool TerrainPolygon::TooClose( TerrainPolygon *poly, bool intersectAllowed, int minDistance )
{
	if( LinesTooClose( poly, minDistance ) )
	{
		cout << "reason 1" << endl;
		return true;
	}

	if( intersectAllowed )
	{
		return false;
	}
	else
	{
		if( LinesIntersect( poly ) )
		{
			cout << "reason 2" << endl;
			return true;
		}
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

