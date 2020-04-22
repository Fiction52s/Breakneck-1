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

PolyPtr ISelectable::GetAsTerrain()
{
	if (selectableType == ISelectableType::TERRAIN)
	{
		return (PolyPtr)this;
	}
	else
	{
		return nullptr;
	}
}

ActorPtr ISelectable::GetAsActor()
{
	if (selectableType == ISelectableType::ACTOR)
	{
		return (ActorPtr)this;
	}
	else
	{
		return nullptr;
	}
}

GateInfoPtr ISelectable::GetAsGateInfo()
{
	if (selectableType == ISelectableType::GATE)
	{
		return (GateInfoPtr)this;
	}
	else
	{
		return nullptr;
	}
}

RailPtr ISelectable::GetAsRail()
{
	if (selectableType == ISelectableType::RAIL)
	{
		return (RailPtr)this;
	}
	else
	{
		return nullptr;
	}
}

Brush::Brush()
	:terrainOnly( true )
{
}

bool Brush::Has(SelectPtr sp)
{
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		if (sp == (*it))
			return true;
	}

	return false;
}

void Brush::TransferMyDuplicates(Brush *compare, Brush *dest)
{
	bool found;

	auto myEnd = objects.end();
	auto compareEnd = compare->objects.end();
	for (auto myIt = objects.begin(); myIt != myEnd;)
	{
		found = false;
		for (auto compIt = compare->objects.begin(); compIt != compareEnd; ++compIt)
		{
			if ((*compIt) == (*myIt))
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			dest->AddObject((*myIt));
			myIt = objects.erase(myIt);
		}
		else
		{
			++myIt;
		}
	}
}

void Brush::TransferMyActiveMembers(Brush *dest)
{
	auto end = objects.end();
	for (auto it = objects.begin(); it != end;)
	{
		if ((*it)->active)
		{
			dest->AddObject((*it));
			it = objects.erase(it);
		}
		else
			++it;
	}
}

void Brush::CenterOnPoint(sf::Vector2i &point )
{
	Move(point - GetCenter());
}

sf::Vector2f Brush::GetTerrainSize()
{
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;

	PolyPtr tp;
	ActorPtr ap;
	if (objects.size() > 0)
	{
		SelectPtr sp = objects.front();
		tp = sp->GetAsTerrain();
		if (tp != NULL)
		{
			left = tp->left;
			right = tp->right;
			top = tp->top;
			bottom = tp->bottom;
		}
	}

	auto it = objects.begin();
	++it;

	for (; it != objects.end(); ++it)
	{
		tp = (*it)->GetAsTerrain();
		if (tp != NULL)
		{
			left = min(left, tp->left);
			right = max(right, tp->right);
			top = min(top, tp->top);
			bottom = max(bottom, tp->bottom);
		}
	}

	return Vector2f(right - left, bottom - top);
}

sf::Vector2i &Brush::GetCenter()
{
	int left;
	int right;
	int top;
	int bottom;

	PolyPtr tp;
	ActorPtr ap;
	if (objects.size() > 0)
	{
		SelectPtr sp = objects.front();
		tp = sp->GetAsTerrain();
		if (tp != NULL)
		{
			left = tp->left;
			right = tp->right;
			top = tp->top;
			bottom = tp->bottom;
		}
		else
		{
			ap = sp->GetAsActor();
			if( ap != NULL )
			{
				ap = sp->GetAsActor();
				return ap->GetIntPos();
			}
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
		tp = (*it)->GetAsTerrain();
		if (tp != NULL)
		{
			left = min(left, tp->left);
			right = max(right, tp->right);
			top = min(top, tp->top);
			bottom = max(bottom, tp->bottom);
		}
	}

	center.x = (left + right) / 2;
	center.y = (top + bottom) / 2;

	return center;
}

sf::Vector2f &Brush::GetCenterF()
{
	int left;
	int right;
	int top;
	int bottom;

	PolyPtr tp;
	ActorPtr ap;
	if (objects.size() > 0)
	{
		SelectPtr sp = objects.front();
		tp = sp->GetAsTerrain();
		if (tp != NULL)
		{
			left = tp->left;
			right = tp->right;
			top = tp->top;
			bottom = tp->bottom;
		}
		else
		{
			/*ap = sp->GetAsActor();
			if (ap != NULL)
			{
				ap = sp->GetAsActor();
				return ap->position;
			}*/
		}
	}
	else
	{
		centerF = Vector2f(0, 0);
		assert(0);
		return centerF;
	}

	auto it = objects.begin();
	++it;

	for (; it != objects.end(); ++it)
	{
		tp = (*it)->GetAsTerrain();
		if (tp != NULL)
		{
			left = min(left, tp->left);
			right = max(right, tp->right);
			top = min(top, tp->top);
			bottom = max(bottom, tp->bottom);
		}
	}

	centerF.x = (left + right) / 2.f;
	centerF.y = (top + bottom) / 2.f;

	return centerF;
}

void Brush::Rotate(float fDegrees)
{
	PolyPtr p;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		p = (*it)->GetAsTerrain();
		if (p != NULL)
		{
			//get center needs to be a float later for this
			//p->SoftReset();
			p->Rotate(GetCenterF(), fDegrees);
			//p->Finalize();
		}
			
	}
}

Brush *Brush::Copy()
{
	Brush *newBrush = new Brush;
	EditSession *sess = EditSession::GetSession();

	PolyPtr tp;
	ActorPtr ap;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		tp = (*it)->GetAsTerrain();
		if (tp != NULL)
		{
			if (tp->inverse)
			{
				continue;
			}

			PolyPtr ptr = tp->Copy();
			newBrush->AddObject(ptr);
		}
		else
		{
			ap = (*it)->GetAsActor();
			if (ap != NULL)
			{
				if (ap->type == sess->types["player"])
				{
					continue;
				}

				ActorPtr aPtr = ap->Copy();
				if (aPtr->posInfo.ground != NULL)
				{
					aPtr->AnchorToGround(aPtr->posInfo);
				}
				newBrush->AddObject(aPtr);
			}
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
	for( auto it = objects.begin(); it != objects.end(); ++it )
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

bool Brush::IsEmpty()
{
	return objects.empty();
}



void Brush::RemoveObject( SelectPtr obj )
{
	objects.remove( obj );

	terrainOnly = true;
	for( auto it = objects.begin(); it != objects.end(); ++it )
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
	for( auto it = objects.begin(); it != objects.end(); ++it )
	{
		//cout << "destroying: " << (*it) << endl;
		delete (*it);
	}
	
	Clear();
}

void Brush::Move( Vector2i delta )
{
	for( auto it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->Move( delta );
	}
}

int Brush::GetNumGroundedActors()
{
	int counter = 0;
	ActorPtr ap;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		ap = (*it)->GetAsActor();
		if (ap != NULL)
		{
			if (ap->posInfo.ground != NULL)
			{
				counter++;
			}
		}
	}
	return counter;
}

int Brush::GetNumActorsThatMustBeAnchored()
{
	int counter = 0;
	ActorPtr ap;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		ap = (*it)->GetAsActor();
		if (ap != NULL)
		{
			if (ap->posInfo.ground == NULL && ap->type->CanBeGrounded() && !ap->type->CanBeAerial() )
			{
				counter++;
			}
		}
	}
	return counter;
}

//returns the compound action of selected actors unanchoring
CompoundAction * Brush::UnAnchor() //only works with grounded actors
{
	CompoundAction * action = NULL;
	ActorPtr ap;
	for( auto it = objects.begin(); it != objects.end(); ++it )
	{
		ap = (*it)->GetAsActor();
		if (ap != NULL)
		{
			if (ap->posInfo.ground != NULL)
			{
				//if (objects.size() == 1 || ap->type->CanBeAerial())
				//if( ap->type->CanBeAerial() )
				{

					Action *newAction = new LeaveGroundAction(ap, V2d( 0, 0 ));

					if (action == NULL)
					{
						action = new CompoundAction;
					}

					action->subActions.push_back(newAction);
				}
			}
		}
	}

	return action;
}


void Brush::Draw( RenderTarget *target )
{
	for( auto it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->BrushDraw( target, true );
	}
}

void Brush::Deactivate()
{	
	for( auto it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->Deactivate();
	}
}

void Brush::Activate()
{
	for( auto it = objects.begin(); it != objects.end(); ++it )
	{
		(*it)->Activate();
	}
}

bool Brush::CanApply()
{
	for( auto it = objects.begin(); it != objects.end(); ++it )
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

ApplyBrushAction::~ApplyBrushAction()
{
	appliedBrush.Destroy();
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

RemoveBrushAction::RemoveBrushAction( Brush *brush, Brush *p_mapStartBrush )
	:mapStartBrush( p_mapStartBrush )
{
	storedBrush = *brush;

	mapStartBrush->TransferMyDuplicates(&storedBrush, &mapStartOwned);
}

RemoveBrushAction::~RemoveBrushAction()
{
	mapStartOwned.TransferMyActiveMembers(mapStartBrush);
	mapStartOwned.Destroy();
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
ReplaceBrushAction::ReplaceBrushAction( Brush *p_orig, Brush *p_replacement, Brush *p_mapStartBrush )
	:mapStartBrush( p_mapStartBrush )
{
	original = *p_orig;
	replacement = *p_replacement;

	auto startEnd = mapStartBrush->objects.end();
	auto origEnd = original.objects.end();

	//if something you're deactivating is part of the original map, you now own it.
	mapStartBrush->TransferMyDuplicates(&original, &mapStartOwned);
	
	//intersectingPolys->
	//adding multiple polygons is actually adding them one at a time and doing the checks each time.
	//this is the same way i have to do the checks to see if i can add in the first place
}

ReplaceBrushAction::~ReplaceBrushAction()
{
	replacement.Destroy();

	mapStartOwned.TransferMyActiveMembers(mapStartBrush);
	mapStartOwned.Destroy();
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

CreateGateAction::CreateGateAction( GateInfo &info, const std::string &type )
{
	//EditSession *session = EditSession::GetSession();
	gate = new GateInfo;
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

CreateGateAction::~CreateGateAction()
{
	//assert(gate->point0 == NULL && gate->point1 == NULL);
	delete gate;
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

DeleteGateAction::DeleteGateAction( GateInfoPtr ptr, Brush *mapStartBrush )
{
	owned = false;
	gate = ptr;

	auto mapEnd = mapStartBrush->objects.end();
	for (auto it = mapStartBrush->objects.begin(); it != mapEnd; ++it)
	{
		if ((*it) == gate)
		{
			owned = true;
			it = mapStartBrush->objects.erase(it);
			break;
		}
	}
}

DeleteGateAction::~DeleteGateAction()
{
	if (owned)
		delete gate;
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

ModifyGateAction::ModifyGateAction( GateInfoPtr ptr, const std::string &type )
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
		if (!sess->IsPolygonValid(poly, NULL))
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

	EditSession *sess = EditSession::GetSession();

	if( !moveOnFirstPerform )
	{
		for (auto it = movingPoints.begin(); it != movingPoints.end(); ++it)
		{
			PolyPtr poly = (*it).first;
			if (sess->IsPolygonValid(poly, NULL))
			{
				poly->AlignExtremes((*it).second);
				if (!sess->IsPolygonValid(poly, NULL))
				{
					moveValid = false;
					break;
				}
			}
			else
			{
				moveValid = false;
				break;
			}
		}

		if (moveValid)
		{
			for (auto it = movingPoints.begin(); it != movingPoints.end(); ++it)
			{
				PolyPtr poly = (*it).first;
				poly->SoftReset();
				poly->Finalize();
				poly->SetRenderMode(TerrainPolygon::RENDERMODE_NORMAL);
			}

			for (auto it = movingRailPoints.begin(); it != movingRailPoints.end(); ++it)
			{
				(*it).first->SoftReset();
				(*it).first->Finalize();
				(*it).first->movingPointMode = false;
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

				/*for (auto it = movingPoints.begin(); it != movingPoints.end(); ++it)
				{
					PolyPtr poly = (*it).first;
					poly->AlignExtremes((*it).second);
				}*/

				for (auto it = movingPoints.begin(); it != movingPoints.end(); ++it)
				{
					PolyPtr poly = (*it).first;
					if (sess->IsPolygonValid(poly, NULL))
					{
						poly->AlignExtremes((*it).second);
						if (!sess->IsPolygonValid(poly, NULL))
						{
							moveValid = false;
							break;
						}
					}
					else
					{
						moveValid = false;
						break;
					}
				}

				if (moveValid)
				{
					PolyPtr poly = (*it).first;

					poly->SoftReset();
					poly->Finalize();
					poly->SetRenderMode(TerrainPolygon::RENDERMODE_NORMAL);

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
	if (!performed)
	{
		assert(performed);
	}
	

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
		poly->SetRenderMode(TerrainPolygon::RENDERMODE_NORMAL);

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

LeaveGroundAction::LeaveGroundAction( ActorPtr p_actor, V2d &p_extra )
	:actor( p_actor )
{
	extra = p_extra;
	assert( actor->posInfo.ground!= NULL );

	gi = actor->posInfo;
}

void LeaveGroundAction::Perform()
{
	assert( !performed );

	performed = true;

	//might be wrong
	actor->UnAnchor();
	actor->SetPosition(actor->GetPosition() + extra);
}

void LeaveGroundAction::Undo()
{
	assert( performed );

	performed = false;

	assert( actor->posInfo.ground == NULL );

	actor->AnchorToGround(gi);
	gi.AddActor(actor);
	
	//cout << "undoing and adding to ground" << endl;
}

GroundAction::GroundAction( ActorPtr p_actor )
	:actor( p_actor )
{
	gi = actor->posInfo;
}

void GroundAction::Perform()
{
	assert( !performed );

	actor->AnchorToGround( gi );
	gi.AddActor(actor);

	performed = true;
}

void GroundAction::Undo()
{
	assert( performed );

	performed = false;

	//might be wrong
	actor->UnAnchor();
}

CompoundAction::CompoundAction()
	//:actionType( COMPOUND )
{

}

void CompoundAction::AddSubAction(Action *a)
{
	subActions.push_back(a);
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

ComplexPasteAction::ComplexPasteAction( Brush *p_mapStartBrush )
{
	performed = true;
	mapStartBrush = p_mapStartBrush;
}

ComplexPasteAction::~ComplexPasteAction()
{
	applied.Destroy();

	mapStartOwned.TransferMyActiveMembers(mapStartBrush);
	mapStartOwned.Destroy();
}

void ComplexPasteAction::SetNewest(
	Brush &newOrig, Brush &newResult)
{
	auto removedEnd = newOrig.objects.end();
	auto appEnd = applied.objects.end();
	auto addsEnd = newResult.objects.end();


	mapStartBrush->TransferMyDuplicates(&newOrig, &mapStartOwned);

	bool found;
	for (auto removedIt = newOrig.objects.begin(); removedIt != removedEnd; ++removedIt)
	{
		found = false;
		for( auto appIt = applied.objects.begin(); appIt != appEnd; ++appIt)
		{
			if ((*appIt) == (*removedIt))
			{
				delete (*appIt);
				applied.objects.erase(appIt);
				found = true;
				break;
			}
		}

		if (!found)
		{
			orig.objects.push_back((*removedIt));
		}
	}

	for (auto addIt = newResult.objects.begin(); addIt != addsEnd; ++addIt)
	{
		applied.objects.push_back((*addIt));
	}
}

void ComplexPasteAction::Undo()
{
	assert(performed);

	applied.Deactivate();
	orig.Activate();

	performed = false;
}

void ComplexPasteAction::Perform()
{
	assert(!performed);

	orig.Deactivate();
	applied.Activate();

	performed = true;
}

ModifyTerrainTypeAction::ModifyTerrainTypeAction( Brush *brush,
	int p_newTerrainWorld, int p_newVariation )
	:newTerrainWorld( p_newTerrainWorld ), newVariation( p_newVariation )
{
	terrainBrush = *brush;

	PolyPtr tp;
	for( SelectList::iterator it = terrainBrush.objects.begin(); 
		it != terrainBrush.objects.end(); ++it )
	{
		tp = (*it)->GetAsTerrain();
		if (tp != NULL)
		{
			terrainTypeMap[tp] = pair<int, int>((int)tp->terrainWorldType,
				tp->terrainVariation);
		}
	}
}

void ModifyTerrainTypeAction::Perform()
{
	assert( !performed );

	EditSession *edit = EditSession::GetSession();

	int tWorldType;
	int tVar;

	PolyPtr poly;
	for (auto it = terrainBrush.objects.begin();
		it != terrainBrush.objects.end(); ++it)
	{
		poly = (*it)->GetAsTerrain();
		if (poly != NULL)
			poly->Deactivate();
	}

	for (auto it = terrainTypeMap.begin();
		it != terrainTypeMap.end(); ++it)
	{
		(*it).first->SetMaterialType(newTerrainWorld, newVariation);
	}

	for (auto it = terrainBrush.objects.begin();
		it != terrainBrush.objects.end(); ++it)
	{
		poly = (*it)->GetAsTerrain();
		if (poly != NULL)
			poly->Activate();
	}

	performed = true;
}

void ModifyTerrainTypeAction::Undo()
{
	assert( performed );

	EditSession *edit = EditSession::GetSession();

	PolyPtr poly;
	for (auto it = terrainBrush.objects.begin();
		it != terrainBrush.objects.end(); ++it)
	{
		poly = (*it)->GetAsTerrain();
		if (poly != NULL)
			poly->Deactivate();
	}

	for( auto it = terrainTypeMap.begin();
		it != terrainTypeMap.end(); ++it )
	{
		(*it).first->SetMaterialType( (*it).second.first, (*it).second.second );
	}

	for (auto it = terrainBrush.objects.begin();
		it != terrainBrush.objects.end(); ++it)
	{
		poly = (*it)->GetAsTerrain();
		if (poly != NULL)
			poly->Activate();
	}

	performed = false;
}

