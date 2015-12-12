#include "Action.h"
#include <assert.h>

EditSession *Action::session = NULL;

Action::Action()
	:next(NULL)
{
}

CreatePolygonAction::CreatePolygonAction()
{
}

void CreatePolygonAction::Perform()
{
	assert( session != NULL );


	//all checks are done before this is performed so it doesnt have to care
	//session->CreatePolygon( <params from inside this action>
}

void CreatePolygonAction::Undo()
{
	//remove the polygon from the active list, but store it in case you need it for later
	assert( session != NULL );
}

AddToPolygonAction::AddToPolygonAction()
{
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



