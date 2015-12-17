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

void Brush::Select()
{
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{	
		(*it)->selected = true;
	}
}

void Brush::Deselect()
{
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{	
		(*it)->selected = false;
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
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->Move( delta );
	}
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


//Action::Action( ActionType p_actionType, Action *p_next )
//	:actionType( p_actionType ), next(p_next), performed( false )
Action::Action( Action *p_next )
	:next(p_next), performed( false )
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

CreateGateAction::CreateGateAction()
{
}

void CreateGateAction::Perform()
{
	assert( session != NULL );
	
	//create the gate
}

void CreateGateAction::Undo()
{
	assert( session != NULL );

	//destroy the gate
}

DeleteGateAction::DeleteGateAction()
{
}

void DeleteGateAction::Perform()
{
	assert( session != NULL );
	
	//create the gate
}

void DeleteGateAction::Undo()
{
	assert( session != NULL );

	//destroy the gate
}



