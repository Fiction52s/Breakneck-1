#include "Action.h"
#include <assert.h>

EditSession *Action::session = NULL;

using namespace sf;
using namespace std;

ISelectable::ISelectable( ISelectable::ISelectableType p_selectableType )
	:selectableType( p_selectableType )
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

void Brush::AddObject( ISelectable *obj )
{
	if( obj->selectableType != ISelectable::TERRAIN )
	{
		terrainOnly = false;
	}
}

void Brush::Clear()
{
	objects.clear();
	terrainOnly = true;
}

void Brush::Destroy()
{
	for( SelectIter it = objects.begin(); it != objects.end(); ++it )
	{
		delete (*it);
	}
	
	Clear();
}



Action::Action( Action *p_next )
	:next(p_next), performed( false )
{
}

//get a linked list of the points involved. 
ApplyBrushAction::ApplyBrushAction( Brush *p_brush )
	:brush( p_brush ), appliedBrush( NULL );
{
	//poly->CopyPoints( pointStart, pointEnd );
	//numPoints = poly->numPoints;
}

void ApplyBrushAction::Perform()
{
	assert( session != NULL );
	assert( !performed );

	performed = true;

	for( SelectIter it = brush->objects.begin(); it != brush->objects.end(); ++it )
	{
		switch( (*it)->selectableType )
		{
		case ISelectable::TERRAIN:
			{
				//add terrain to the environment. this comes first
				break;
			}
		case ISelectable::ACTOR:
			{
				//add last. order could possibly not matter because just adding the stuff to the list shouldnt change anything?
				//no, creating with a brush actually makes a copy of the brush when you set it down
				//be sure to update everything accordingly to point to the right spots
				break;
			}
		case ISelectable::GATE:
			{
				//add brushes 2nd. only gates that connect two terrain parts of a brush can be included in a brush
				break;
			}
		}
	}
	//session->polygons.push_back( createdPoly );
}

void CreatePolygonAction::Undo()
{
	assert( session != NULL );
	assert( performed );
	
	session->polygons.remove( createdPoly );
	
	

	//need to remove actors and gates attached to this? or would undos 
	//back to this point always give me the same thing back?
	//leave it this way for now
}

AddToPolygonAction::AddToPolygonAction( TerrainPolygon *p_brush, TerrainPolygon *poly )
	:brush( p_brush ), oldPoly( poly ), newPoly( NULL 
{
	//session->Add( 
}


void AddToPolygonAction::Perform()
{
	assert( session != NULL );


	//all checks are done before this is performed so it doesnt have to care
	
	//combine old polygon and new polygon into a new one, and store the 2 old ones.
}

void AddToPolygonAction::Undo()
{
	//remove the polygon from the active list, but store it in case you need it for later
	assert( session != NULL );
}

CreateActorAction::CreateActorAction()
{
}

void CreateActorAction::Perform()
{
	assert( session != NULL );
	
	//put the actor into the active list
}

void CreateActorAction::Undo()
{
	assert( session != NULL );

	//remove the actor from the active list and store it
}

DeleteActorAction::DeleteActorAction()
{
}

void DeleteActorAction::Perform()
{
	assert( session != NULL );
	
	//take the actor out of the active list and store it
}

void DeleteActorAction::Undo()
{
	assert( session != NULL );

	//put the actor back into the active list
}

EditActorAction::EditActorAction()
{
}

void EditActorAction::Perform()
{
	assert( session != NULL );
	
	//set the parameters and store the old parameters
}

void EditActorAction::Undo()
{
	assert( session != NULL );

	//restore the old parameters
}

MoveObjectAction::MoveObjectAction()
{
}

void MoveObjectAction::Perform()
{
	assert( session != NULL );
	
	//just move the object
}

void MoveObjectAction::Undo()
{
	assert( session != NULL );

	//move the object back
}

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



