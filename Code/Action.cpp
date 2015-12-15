#include "Action.h"
#include <assert.h>
#include <iostream>


EditSession *Action::session = NULL;

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
	//:brush( p_brush )
{
	for( SelectIter it = brush->objects.begin(); it != brush->objects.end(); ++it )
	{
		switch( (*it)->selectableType )
		{
		case ISelectable::TERRAIN:
			{
				PolyPtr tp = boost::dynamic_pointer_cast<TerrainPolygon>(*it);
				
				SelectPtr selectable( new TerrainPolygon( *tp, true ) );
				//PolyPtr ptr = boost::dynamic_pointer_cast<ISelectable>( selectable );
				PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( selectable );

				poly->Finalize();

				appliedBrush.AddObject( selectable );



				//poly->Finalize();
				//TerrainPolygon *poly = new TerrainPolygon( //(*it)
				//appliedBrush.AddObject( 
				//	 );

				//session->polygons.push_back( poly );

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
}

void ApplyBrushAction::Perform()
{
	//cout << "performing!" << endl;
	assert( session != NULL );
	assert( appliedBrush.objects.size() > 0 );
	assert( !performed );

	performed = true;

	
	for( SelectIter it = appliedBrush.objects.begin(); it != appliedBrush.objects.end(); ++it )
	{
		switch((*it)->selectableType )
		{
		case ISelectable::TERRAIN:
			{
				PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( (*it) );
				//session->polygons.push_back( poly );

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
}

void ApplyBrushAction::Undo()
{
	//cout << "undoing" << endl;
	assert( session != NULL );
	assert( performed );
	
	performed = false;

	//deactivate everything first. when you deactivate something it sets its active bool to false
	//so you dont try to deactivate something that you have already deleted.
	/*for( SelectIter it = appliedBrush.objects.begin(); it != appliedBrush.objects.end(); ++it )
	{
		(*it)->Deactivate();
	}*/

	for( SelectIter it = appliedBrush.objects.begin(); it != appliedBrush.objects.end(); ++it )
	{
	switch( (*it)->selectableType )
		{
		case ISelectable::TERRAIN:
			{
				//cout << "removing polygon!" << endl;
				//cout << "before undo: " << session->polygons.size() << endl;
				PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( (*it) );
				
				
				//session->polygons.remove( (TerrainPolygon*)(*it) );
				
				
				//cout << "there are now: " << session->polygons.size() << endl;
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

	//appliedBrush.Destroy();

	//need to remove actors and gates attached to this? or would undos 
	//back to this point always give me the same thing back?
	//leave it this way for now
}

AddBrushAction::AddBrushAction( Brush *p_brush, Brush *p_intersectingPolys )
	:brush( p_brush ), intersectingPolys( p_intersectingPolys )
{
	assert( brush->terrainOnly );
	
	//intersectingPolys->
	//adding multiple polygons is actually adding them one at a time and doing the checks each time.
	//this is the same way i have to do the checks to see if i can add in the first place
}


void AddBrushAction::Perform()
{
	assert( session != NULL );
	assert( !performed );

	//all checks are done before this is performed so it doesnt have to care
	
	//combine old polygon and new polygon into a new one, and store the 2 old ones.
}

void AddBrushAction::Undo()
{
	//remove the polygon from the active list, but store it in case you need it for later
	assert( session != NULL );
	assert( performed );
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



