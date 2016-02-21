#include "Action.h"
#include <assert.h>
#include <iostream>


EditSession *Action::session = NULL;
EditSession *Brush::session = NULL;

using namespace sf;
using namespace std;



ISelectable::ISelectable( ISelectable::ISelectableType p_selectableType )
	:selectableType( p_selectableType ), active( true ), selected( false )
{
}

/*void ISelectable::BrushDraw( RenderTarget *target, bool valid )
{

}

void ISelectable::Draw( RenderTarget *target )
{

}

bool ISelectable::ContainsPoint( sf::Vector2i point )
{
	return false;
}

bool ISelectable::Intersects( sf::IntRect rect )
{
}*/

Brush::Brush()
	:terrainOnly( true )
{
}

void Brush::SetSelected( bool select )
{
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{	
		(*it)->SetSelected( select );
	}
}

void Brush::AddObject( SelectPtr &obj )
{
	if( obj->selectableType != ISelectable::TERRAIN )
	{
		terrainOnly = false;
	}

	objects.push_back( obj );
}

void Brush::RemoveObject( SelectPtr &obj )
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
				if( objects.size() == 1 || actor->type->canBeAerial )
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
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->Deactivate( session, (*it) );
	}
}

void Brush::Activate()
{
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
	assert( session != NULL );
	assert( appliedBrush.objects.size() > 0 );
	assert( !performed );

	performed = true;
	appliedBrush.Activate();
}

void ApplyBrushAction::Undo()
{
	assert( session != NULL );
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
	assert( session != NULL );
	assert( !performed );
	performed = true;

	storedBrush.Deactivate();

	cout << "performing remove brush" << endl;
}

void RemoveBrushAction::Undo()
{
	assert( session != NULL );
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
	assert( session != NULL );
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
	assert( session != NULL );
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
	assert( session != NULL );
	
	//set the parameters and store the old parameters
}

void EditObjectAction::Undo()
{
	assert( session != NULL );
	//restore the old parameters
}

/*MoveBrushAction::MoveBrushAction( Brush *brush, Vector2i p_delta )
	:delta( p_delta )
{
	movingBrush = *brush;
}

void MoveBrushAction::Perform()
{
	assert( session != NULL );
	assert( !performed );
	
	performed = false;
	movingBrush.Move( delta );
}

void MoveBrushAction::Undo()
{
	assert( session != NULL );
	assert( performed );

	movingBrush.Move( -delta );
	//move the object back
}*/

DeletePointsAction::DeletePointsAction()
{
}

void DeletePointsAction::Perform()
{
	assert( session != NULL );
	
	//delete the points
}

void DeletePointsAction::Undo()
{
	assert( session != NULL );

	//delete the polygon which resulted from the deletion and replace it with a copy of the old polygon
}

CreateGateAction::CreateGateAction( GateInfo &info, const std::string &type )
{
	gate.reset( new GateInfo );
	//GateInfo *gi = new GateInfo;

	gate->SetType( type );

	gate->edit = session;
	gate->poly0 = info.poly0;
	gate->vertexIndex0 = info.vertexIndex0;
	gate->point0 = info.point0;
	//gate->point0->gate = gate;

	gate->poly1 = info.poly1;
	gate->vertexIndex1 = info.vertexIndex1;
	gate->point1 = info.point1;

	gate->reformBehindYou = info.reformBehindYou;
	//gate->point1->gate = gate;


	gate->UpdateLine();
}

void CreateGateAction::Perform()
{
	assert( session != NULL );
	assert( !performed );

	performed = true;

	session->gates.push_back( gate );
	gate->point0->gate = gate;
	gate->point1->gate = gate;
	//create the gate
}

void CreateGateAction::Undo()
{
	assert( session != NULL );
	assert( performed );

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
	assert( session != NULL );
	
	performed = true;
	session->gates.remove( gate );
	gate->point0->gate = NULL;
	gate->point1->gate = NULL;
	//create the gate
}

void DeleteGateAction::Undo()
{
	assert( session != NULL );

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
	assert( session != NULL );
	assert( !performed );

	performed = true;

	gate->SetType( newType );
	gate->UpdateLine();
}

void ModifyGateAction::Undo()
{
	assert( session != NULL );
	assert( performed );

	performed = false;
	

	//gate->SetType( oldType );
	gate->type = oldType;
	gate->UpdateLine();
}

//doesn't make a copy of the brush!
MoveBrushAction::MoveBrushAction( Brush *p_brush, sf::Vector2i p_delta, bool p_moveOnFirstPerform,
	PointMap &points )
	:delta( p_delta ), moveOnFirstPerform( p_moveOnFirstPerform ), movingPoints( points )
{
	movingBrush = *p_brush;
}

void MoveBrushAction::Perform()
{
	assert( session != NULL );
	assert( !performed );

	performed = true;

	if( !moveOnFirstPerform )
	{
		for( PointMap::iterator it = movingPoints.begin(); it != movingPoints.end(); ++it )
		{
			(*it).first->SoftReset();
			(*it).first->Finalize();
			(*it).first->movingPointMode = false;
		}
		moveOnFirstPerform = true;
	}
	else
	{
		movingBrush.Move( delta );

		for( PointMap::iterator it = movingPoints.begin(); it != movingPoints.end(); ++it )
		{
			list<PointMoveInfo> &pList = (*it).second;
			for( list<PointMoveInfo>::iterator pit = pList.begin(); pit != pList.end(); ++pit )
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
	assert( session != NULL );
	assert( performed );

	performed = false;

	movingBrush.Move( -delta );

	for( PointMap::iterator it = movingPoints.begin(); it != movingPoints.end(); ++it )
	{
		list<PointMoveInfo> &pList = (*it).second;
		for( list<PointMoveInfo>::iterator pit = pList.begin(); pit != pList.end(); ++pit )
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
	assert( session != NULL );
	assert( !performed );

	performed = true;

	actor->UnAnchor( actor );
}

void LeaveGroundAction::Undo()
{
	assert( session != NULL );
	assert( performed );

	performed = false;

	assert( actor->groundInfo == NULL );

	//cout << "applied: " << gi.GetEdgeIndex() << ", " << gi.groundQuantity << endl;
	actor->AnchorToGround( gi );//gi.ground, gi.GetEdgeIndex(), gi.groundQuantity );

	gi.ground->enemies[gi.edgeStart].push_back( actor );
	gi.ground->UpdateBounds();
	//cout << "undoing and adding to ground" << endl;
}

GroundAction::GroundAction( ActorPtr &p_actor )
	:actor( p_actor )
{
	gi = *actor->groundInfo;
}

void GroundAction::Perform()
{
	assert( session != NULL );
	assert( !performed );

	actor->AnchorToGround( gi );

	gi.ground->enemies[gi.edgeStart].push_back( actor );
	gi.ground->UpdateBounds();

	performed = true;
}

void GroundAction::Undo()
{
	assert( session != NULL );
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



