#include "Action.h"
#include <assert.h>
#include <iostream>
#include "EditSession.h"
#include "ActorParams.h"
#include "EditorRail.h"

using namespace sf;
using namespace std;



ISelectable::ISelectable( ISelectable::ISelectableType p_selectableType )
	:selectableType( p_selectableType ), active( true ), selected( false )
{
}


Brush::Brush()
	:terrainOnly( true )
{
}

bool Brush::Has(SelectPtr sp)
{
	for (SelectIter it = objects.begin(); it != objects.end(); ++it)
	{
		if (sp == (*it))
			return true;
	}

	return false;
}

sf::Vector2i &Brush::GetCenter()
{
	int left;
	int right;
	int top;
	int bottom;

	if (objects.size() > 0)
	{
		SelectPtr sp = objects.front();
		if (sp->selectableType == ISelectable::TERRAIN)
		{
			PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>(objects.front());
			left = poly->left;
			right = poly->right;
			top = poly->top;
			bottom = poly->bottom;
		}
		else if (sp->selectableType == ISelectable::ACTOR)
		{
			ActorPtr a = boost::dynamic_pointer_cast<ActorParams>(objects.front());
			return a->position;
		}
		
	}
	else
	{
		center = Vector2i(0, 0);
		assert(0);
		return center;
	}
	
	auto it = objects.begin();
	++it;

	for ( ; it != objects.end(); ++it)
	{
		if ((*it)->selectableType == ISelectable::TERRAIN)
		{
			PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>((*it));
			left = min(left, poly->left);
			right = max(right, poly->right);
			top = min(top, poly->top);
			bottom = max(bottom, poly->bottom);
		}
	}

	center.x = (left + right) / 2;
	center.y = (top + bottom) / 2;

	return center;
}

Brush *Brush::Copy()
{
	Brush *newBrush = new Brush;
	EditSession *sess = EditSession::GetSession();

	for (SelectIter it = objects.begin(); it != objects.end(); ++it)
	{
		if ((*it)->selectableType == ISelectable::TERRAIN)
		{
			TerrainPolygon *tp = (TerrainPolygon*)((*it).get());

			if (tp->inverse)
			{
				continue;
			}

			PolyPtr ptr(tp->Copy());
			newBrush->AddObject(ptr);
		}
		else if ((*it)->selectableType == ISelectable::ACTOR)
		{
			ActorParams *ap = (ActorParams*)(*it).get();
			
			
			if (ap->type == sess->types["player"])
			{
				continue;
			}

			ActorPtr aPtr(ap->Copy());
			if (aPtr->groundInfo != NULL)
			{
				aPtr->AnchorToGround(*aPtr->groundInfo);
			}
			newBrush->AddObject(aPtr);
		}
	}

	if (newBrush->objects.size() == 0)
	{
		delete newBrush;
		newBrush = NULL;
	}


	return newBrush;
}

void Brush::SetSelected( bool select )
{
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{	
		(*it)->SetSelected( select );
	}
}

void Brush::AddObject( SelectPtr obj )
{
	if( obj->selectableType != ISelectable::TERRAIN )
	{
		terrainOnly = false;
	}

	objects.push_back( obj );
}

void Brush::RemoveObject( SelectPtr obj )
{
	objects.remove( obj );

	terrainOnly = true;
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		if( (*it)->selectableType != ISelectable::TERRAIN )
		{
			terrainOnly = false;
			break;
		}
	}
}

void Brush::Clear()
{
	objects.clear();
	terrainOnly = true;
}

void Brush::Destroy()
{
	//automatically delete on clear now if there are no other
	//references

	//for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	//{
		//delete (*(*it));
	//}
	
	Clear();
}

void Brush::Move( Vector2i delta )
{
	//if( objects.size() == 1 && objects.front()->selectableType == ISelectable::ACTOR )
	//{

	//}
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->Move( (*it), delta );
	}
}

//returns the compound action of selected actors unanchoring
CompoundAction * Brush::UnAnchor() //only works with grounded actors
{
	CompoundAction * action = NULL;
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		if( (*it)->selectableType == ISelectable::ACTOR )
		{
			ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( (*it) );

			if( actor->groundInfo != NULL )
			{
				if( objects.size() == 1 || actor->type->CanBeAerial() )
				{

					Action *newAction = new LeaveGroundAction( actor );

					if( action == NULL )
					{
						action = new CompoundAction;
					}
			
					action->subActions.push_back( newAction );
				}
			}
		}
	}

	return action;
}


void Brush::Draw( RenderTarget *target )
{
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->BrushDraw( target, true );
	}
}

void Brush::Deactivate()
{
	EditSession *session = EditSession::GetSession();
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->Deactivate( session, (*it) );
	}
}

void Brush::Activate()
{
	EditSession *session = EditSession::GetSession();
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->Activate( session, (*it) );
	}
}

bool Brush::CanApply()
{
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		if( !(*it)->CanApply() )
		{
			return false;
		}
	}
	return true;
}

bool Brush::CanAdd()
{
	return false;
}



//Action::Action( ActionType p_actionType, Action *p_next )
//	:actionType( p_actionType ), next(p_next), performed( false )
Action::Action( )
	:performed( false )
{
}

Action::~Action()
{

}

//get a brush that I don't own and make a copy of it which I can 
//add and remove from the world at will

ApplyBrushAction::ApplyBrushAction( Brush *brush )
{
	appliedBrush = *brush;
}

void ApplyBrushAction::Perform()
{
	//cout << "performing!" << endl;
	assert( appliedBrush.objects.size() > 0 );
	assert( !performed );

	performed = true;
	appliedBrush.Activate();
}

void ApplyBrushAction::Undo()
{
	assert( performed );
	
	performed = false;

	appliedBrush.Deactivate();
}

RemoveBrushAction::RemoveBrushAction( Brush *brush )
{
	storedBrush = *brush;
}

void RemoveBrushAction::Perform()
{
	assert( !performed );
	performed = true;

	storedBrush.Deactivate();

	cout << "performing remove brush" << endl;
}

void RemoveBrushAction::Undo()
{
	assert( performed );

	performed = false;
	storedBrush.Activate();

	cout << "undoing remove brush" << endl;
}

//use for add and subtract
ReplaceBrushAction::ReplaceBrushAction( Brush *p_orig, Brush *p_replacement )
{
	original = *p_orig;

	replacement = *p_replacement;
	//assert( brush->terrainOnly );
	
	//intersectingPolys->
	//adding multiple polygons is actually adding them one at a time and doing the checks each time.
	//this is the same way i have to do the checks to see if i can add in the first place
}

void ReplaceBrushAction::Perform()
{
	assert( !performed );

	performed = true;

	original.Deactivate();
	replacement.Activate();
	
	
	//all checks are done before this is performed so it doesnt have to care
	
	//combine old polygon and new polygon into a new one, and store the 2 old ones.
}

void ReplaceBrushAction::Undo()
{
	//remove the polygon from the active list, but store it in case you need it for later
	assert( performed );

	performed = false;
	//original.Activate()
	
	replacement.Deactivate();
	original.Activate();
}

EditObjectAction::EditObjectAction()
{
}

void EditObjectAction::Perform()
{
	
	//set the parameters and store the old parameters
}

void EditObjectAction::Undo()
{
	//restore the old parameters
}


DeletePointsAction::DeletePointsAction()
{
}

void DeletePointsAction::Perform()
{
	
	//delete the points
}

void DeletePointsAction::Undo()
{

	//delete the polygon which resulted from the deletion and replace it with a copy of the old polygon
}

CreateGateAction::CreateGateAction( GateInfo &info, const std::string &type )
{
	//EditSession *session = EditSession::GetSession();
	gate.reset( new GateInfo );
	//GateInfo *gi = new GateInfo;

	gate->SetType( type );
	if (gate->type == Gate::SHARD)
	{
		gate->SetShard(info.shardWorld, info.shardIndex);
	}


	//gate->edit = session;
	gate->poly0 = info.poly0;
	gate->vertexIndex0 = info.vertexIndex0;
	gate->point0 = info.point0;
	//gate->point0->gate = gate;

	gate->poly1 = info.poly1;
	gate->vertexIndex1 = info.vertexIndex1;
	gate->point1 = info.point1;
	//gate->point1->gate = gate;


	gate->UpdateLine();
}

void CreateGateAction::Perform()
{
	assert( !performed );
	EditSession *session = EditSession::GetSession();
	performed = true;

	session->gates.push_back( gate );
	gate->point0->gate = gate;
	gate->point1->gate = gate;
	//create the gate
}

void CreateGateAction::Undo()
{
	assert( performed );
	EditSession *session = EditSession::GetSession();
	performed = false;

	session->gates.remove( gate );
	gate->point0->gate = NULL;
	gate->point1->gate = NULL;
	//destroy the gate
}

DeleteGateAction::DeleteGateAction( GateInfoPtr &ptr )
{
	gate = ptr;
}

void DeleteGateAction::Perform()
{	
	EditSession *session = EditSession::GetSession();
	performed = true;
	session->gates.remove( gate );
	gate->point0->gate = NULL;
	gate->point1->gate = NULL;
	//create the gate
}

void DeleteGateAction::Undo()
{
	EditSession *session = EditSession::GetSession();
	performed = false;
	session->gates.push_back( gate );
	gate->point0->gate = gate;
	gate->point1->gate = gate;
	//destroy the gate
}

ModifyGateAction::ModifyGateAction( GateInfoPtr &ptr, const std::string &type )
	:newType( type )
{
	gate = ptr;
	oldType = gate->type;
}

void ModifyGateAction::Perform()
{
	assert( !performed );

	performed = true;

	gate->SetType( newType );
	gate->UpdateLine();
}

void ModifyGateAction::Undo()
{
	assert( performed );

	performed = false;
	

	//gate->SetType( oldType );
	gate->type = oldType;
	gate->UpdateLine();
}

//doesn't make a copy of the brush!
MoveBrushAction::MoveBrushAction( Brush *p_brush, sf::Vector2i p_delta, bool p_moveOnFirstPerform,
	PointVectorMap &points, RailPointMap &railPoints )
	:delta( p_delta ), moveOnFirstPerform( p_moveOnFirstPerform ), movingPoints( points ),
	movingRailPoints( railPoints )
{
	moveValid = true;
	movingBrush = *p_brush;
}

void MoveBrushAction::CheckValidPointMove()
{
	EditSession *sess = EditSession::GetSession();

	moveValid = true;
	for (auto it = movingPoints.begin(); it != movingPoints.end(); ++it)
	{
		PolyPtr poly = (*it).first;
		if (!sess->IsPolygonValid(poly.get(), NULL))
		{
			moveValid = false;
			return;
		}
	}
}

void MoveBrushAction::Perform()
{
	assert( !performed );

	performed = true;

	if( !moveOnFirstPerform )
	{
		for (auto it = movingPoints.begin(); it != movingPoints.end(); ++it)
		{
			PolyPtr poly = (*it).first;
			poly->AlignExtremes(EditSession::PRIMARY_LIMIT);
		}

		CheckValidPointMove();
		if (moveValid)
		{
			for (auto it = movingPoints.begin(); it != movingPoints.end(); ++it)
			{
				PolyPtr poly = (*it).first;
				poly->SoftReset();
				poly->Finalize();
				poly->movingPointMode = false;
			}
		}

		moveOnFirstPerform = true;
	}
	else
	{
		movingBrush.Move( delta );

		//CheckValidPointMove();

		if (moveValid)
		{
			for (auto it = movingPoints.begin(); it != movingPoints.end(); ++it)
			{
				vector<PointMoveInfo> &pVec = (*it).second;
				for (auto pit = pVec.begin(); pit != pVec.end(); ++pit)
				{
					(*pit).point->pos += (*pit).delta;

					if ((*pit).point->gate != NULL)
					{
						(*pit).point->gate->UpdateLine();
					}
				}

				PolyPtr poly = (*it).first;

				poly->SoftReset();
				poly->Finalize();
				poly->movingPointMode = false;

				for (auto pit = poly->enemies.begin();
					pit != poly->enemies.end(); ++pit)
				{
					list<ActorPtr> &enemies = (*pit).second;
					for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
					{
						(*ait)->UpdateGroundedSprite();
						(*ait)->SetBoundingQuad();
					}
				}


			}
		}

		for (auto it = movingRailPoints.begin(); it != movingRailPoints.end(); ++it)
		{
			list<PointMoveInfo> &pList = (*it).second;
			for (list<PointMoveInfo>::iterator pit = pList.begin(); pit != pList.end(); ++pit)
			{
				(*pit).point->pos += (*pit).delta;
			}

			(*it).first->SoftReset();
			(*it).first->Finalize();
			(*it).first->movingPointMode = false;
		}
	}
}

void MoveBrushAction::Undo()
{
	assert( performed );

	performed = false;

	movingBrush.Move( -delta );

	for( auto it = movingPoints.begin(); it != movingPoints.end(); ++it )
	{
		vector<PointMoveInfo> &pList = (*it).second;

		for( vector<PointMoveInfo>::iterator pit = pList.begin(); pit != pList.end(); ++pit )
		{
			(*pit).point->pos = (*pit).origPos;

			if ((*pit).point->gate != NULL)
			{
				(*pit).point->gate->UpdateLine();
			}
		}

		PolyPtr poly = (*it).first;

		poly->SoftReset();
		poly->Finalize();
		poly->movingPointMode = false;

		for ( auto pit = poly->enemies.begin();
			pit != poly->enemies.end(); ++pit)
		{
			list<ActorPtr> &enemies = (*pit).second;
			for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
			{
				(*ait)->UpdateGroundedSprite();
				(*ait)->SetBoundingQuad();
			}
		}
	}

	for (auto it = movingRailPoints.begin(); it != movingRailPoints.end(); ++it)
	{
		list<PointMoveInfo> &pList = (*it).second;
		for (list<PointMoveInfo>::iterator pit = pList.begin(); pit != pList.end(); ++pit)
		{
			(*pit).point->pos -= (*pit).delta;
		}

		(*it).first->SoftReset();
		(*it).first->Finalize();
		(*it).first->movingPointMode = false;
	}
}

LeaveGroundAction::LeaveGroundAction( ActorPtr &p_actor )
	:actor( p_actor )
{
	assert( actor->groundInfo != NULL );

	gi = *actor->groundInfo;

	//cout << "stored: " << gi.GetEdgeIndex() << ", " << gi.groundQuantity << endl;

}

void LeaveGroundAction::Perform()
{
	assert( !performed );

	performed = true;

	actor->UnAnchor( actor );
}

void LeaveGroundAction::Undo()
{
	assert( performed );

	performed = false;

	assert( actor->groundInfo == NULL );

	//cout << "applied: " << gi.GetEdgeIndex() << ", " << gi.groundQuantity << endl;
	//gi.ground, gi.GetEdgeIndex(), gi.groundQuantity );

	if (gi.ground != NULL)
	{
		actor->AnchorToGround(gi);
		gi.ground->enemies[gi.edgeStart].push_back(actor);
		gi.ground->UpdateBounds();
	}
	else if (gi.railGround != NULL)
	{
		actor->AnchorToRail(gi);
		gi.railGround->enemies[gi.edgeStart].push_back(actor);
		gi.railGround->UpdateBounds();
	}
	else
	{
		assert(0);
	}
	
	//cout << "undoing and adding to ground" << endl;
}

GroundAction::GroundAction( ActorPtr &p_actor )
	:actor( p_actor )
{
	gi = *actor->groundInfo;
}

void GroundAction::Perform()
{
	assert( !performed );

	actor->AnchorToGround( gi );

	if (gi.ground != NULL)
	{
		gi.ground->enemies[gi.edgeStart].push_back(actor);
		gi.ground->UpdateBounds();
	}
	else if (gi.railGround != NULL)
	{
		gi.railGround->enemies[gi.edgeStart].push_back(actor);
		gi.railGround->UpdateBounds();
	}

	performed = true;
}

void GroundAction::Undo()
{
	assert( performed );

	performed = false;

	actor->UnAnchor( actor );
}

CompoundAction::CompoundAction()
	//:actionType( COMPOUND )
{

}

CompoundAction::~CompoundAction()
{
	for( list<Action*>::iterator it = subActions.begin(); it != subActions.end(); ++it )
	{
		delete (*it);
	}
}

void CompoundAction::Perform()
{
	for( list<Action*>::iterator it = subActions.begin(); it != subActions.end(); ++it )
	{
		(*it)->Perform();
	}
}

void CompoundAction::Undo()
{
	for( list<Action*>::reverse_iterator rit = subActions.rbegin(); rit != subActions.rend(); ++rit )
	{
		(*rit)->Undo();
	}
}

ModifyTerrainTypeAction::ModifyTerrainTypeAction( Brush *brush,
	int p_newTerrainWorld, int p_newVariation )
	:newTerrainWorld( p_newTerrainWorld ), newVariation( p_newVariation )
{
	terrainBrush = *brush;

	for( SelectList::iterator it = terrainBrush.objects.begin(); 
		it != terrainBrush.objects.end(); ++it )
	{
		PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( (*it) );

		if( (*it)->selectableType == ISelectable::TERRAIN )
		{
			terrainTypeMap[poly.get()] = pair<int,int>( (int)poly->terrainWorldType,
				poly->terrainVariation );
		}
	}
}

void ModifyTerrainTypeAction::Perform()
{
	assert( !performed );

	EditSession *edit = EditSession::GetSession();

	int tWorldType;
	int tVar;

	for (SelectList::iterator it = terrainBrush.objects.begin();
		it != terrainBrush.objects.end(); ++it)
	{
		PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>((*it));
		poly->Deactivate(edit, (*it));
	}

	for (map<TerrainPolygon*, pair<int, int>>::iterator it = terrainTypeMap.begin();
		it != terrainTypeMap.end(); ++it)
	{
		(*it).first->SetMaterialType(newTerrainWorld, newVariation);
	}

	for (SelectList::iterator it = terrainBrush.objects.begin();
		it != terrainBrush.objects.end(); ++it)
	{
		PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>((*it));
		poly->Activate(edit, (*it));
	}

	performed = true;
}

void ModifyTerrainTypeAction::Undo()
{
	assert( performed );

	EditSession *edit = EditSession::GetSession();

	for (SelectList::iterator it = terrainBrush.objects.begin();
		it != terrainBrush.objects.end(); ++it)
	{
		PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>((*it));
		poly->Deactivate(edit, (*it));
	}

	for( map<TerrainPolygon*, pair<int,int>>::iterator it = terrainTypeMap.begin();
		it != terrainTypeMap.end(); ++it )
	{
		(*it).first->SetMaterialType( (*it).second.first, (*it).second.second );
	}

	for (SelectList::iterator it = terrainBrush.objects.begin();
		it != terrainBrush.objects.end(); ++it)
	{
		PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>((*it));
		poly->Activate(edit, (*it));
	}

	performed = false;
}

