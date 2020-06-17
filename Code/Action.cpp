#include "Action.h"
#include <assert.h>
#include <iostream>
#include "EditSession.h"
#include "ActorParams.h"
#include "EditorRail.h"
#include "EditorDecorInfo.h"
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

DecorPtr ISelectable::GetAsDecor()
{
	if (selectableType == ISelectableType::IMAGE)
	{
		return (DecorPtr)this;
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

int Brush::GetNumActors()
{
	int counter = 0;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		if ((*it)->GetAsActor() != NULL)
			++counter;
	}

	return counter;
}

int Brush::GetNumTerrain()
{
	int counter = 0;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		if ((*it)->GetAsTerrain() != NULL)
			++counter;
	}

	return counter;
}

sf::Vector2f Brush::GetTerrainSize()
{
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;

	PolyPtr tp;
	ActorPtr ap;
	DecorPtr dec;
	RailPtr rp;
	if (objects.size() > 0)
	{
		SelectPtr sp = objects.front();
		tp = sp->GetAsTerrain();
		dec = sp->GetAsDecor();
		rp = sp->GetAsRail();

		if (tp != NULL)
		{
			left = tp->left;
			right = tp->right;
			top = tp->top;
			bottom = tp->bottom;
		}
		else if (rp != NULL)
		{
			left = rp->left;
			right = rp->right;
			top = rp->top;
			bottom = rp->bottom;
		}
		else if (dec != NULL)
		{
			IntRect ir = dec->GetAABB();
			left = ir.left;
			right = ir.left + ir.width;
			top = ir.top;
			bottom = ir.top + ir.height;
		}
	}

	auto it = objects.begin();
	++it;

	for (; it != objects.end(); ++it)
	{
		tp = (*it)->GetAsTerrain();
		dec = (*it)->GetAsDecor();
		rp = (*it)->GetAsRail();

		if (tp != NULL)
		{
			left = min(left, tp->left);
			right = max(right, tp->right);
			top = min(top, tp->top);
			bottom = max(bottom, tp->bottom);
		}
		else if (rp != NULL)
		{
			left = min(left, rp->left);
			right = max(right, rp->right);
			top = min(top, rp->top);
			bottom = max(bottom, rp->bottom);
		}
		else if (dec != NULL)
		{
			IntRect ir = dec->GetAABB();
			left = min(left, ir.left);
			right = max(right, ir.left + ir.width);
			top = min(top, ir.top);
			bottom = max(bottom, ir.top + ir.height);
		}
	}

	return Vector2f(right - left, bottom - top);
}

sf::IntRect Brush::GetAABB()
{
	IntRect aabb;
	if (objects.empty())
	{
		assert(0);
		return aabb;
	}

	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;

	PolyPtr tp;
	ActorPtr ap;
	RailPtr rp;
	SelectPtr sp;
	DecorPtr dec;

	bool init = false;

	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		sp = (*it);
		tp = sp->GetAsTerrain();
		rp = sp->GetAsRail();
		dec = sp->GetAsDecor();
		ap = sp->GetAsActor();

		if (tp != NULL)
		{
			if (!init)
			{
				init = true;
				left = tp->left;
				right = tp->right;
				top = tp->top;
				bottom = tp->bottom;
			}
			else
			{
				left = min(left, tp->left);
				right = max(right, tp->right);
				top = min(top, tp->top);
				bottom = max(bottom, tp->bottom);
			}
		}
		else if (rp != NULL)
		{
			if (!init)
			{
				init = true;
				left = rp->left;
				right = rp->right;
				top = rp->top;
				bottom = rp->bottom;
			}
			else
			{
				left = min(left, rp->left);
				right = max(right, rp->right);
				top = min(top, rp->top);
				bottom = max(bottom, rp->bottom);
			}
		}
		else if( ap != NULL )
		{
			if (ap->posInfo.ground == NULL
				&& ap->posInfo.railGround == NULL)
			{

				Vector2i intPos(ap->GetGrabAABBCenter());
				if (!init)
				{
					init = true;
					//Vector2i intPos = ap->GetIntPos();
					left = intPos.x;
					right = intPos.x;
					top = intPos.y;
					bottom = intPos.y;
				}
				else
				{
					//Vector2i intPos = ap->GetIntPos();
					left = min(left, intPos.x);
					right = max(right, intPos.x);
					top = min(top, intPos.y);
					bottom = max(bottom, intPos.y);
				}
			}
		}
		else if (dec != NULL)
		{
			IntRect aabb = dec->GetAABB();
			if (!init)
			{
				init = true;
				left = aabb.left;
				right = aabb.left + aabb.width;
				top = aabb.top;
				bottom = aabb.top + aabb.height;
			}
			else
			{
				left = min(left, aabb.left);
				right = max(right, aabb.left + aabb.width);
				top = min(top, aabb.top);
				bottom = max(bottom, aabb.top + aabb.height);
			}
		}
	}

	aabb.left = left;
	aabb.top = top;
	aabb.width = right - left;
	aabb.height = bottom - top;

	return aabb;
}

sf::Vector2i &Brush::GetCenter()
{
	if( objects.empty() )
	{
		center = Vector2i(0, 0);
		assert(0);
		return center;
	}

	IntRect ir = GetAABB();

	center.x = ir.left + ir.width / 2;
	center.y = ir.top + ir.height / 2;

	return center;
}

sf::Vector2f &Brush::GetCenterF()
{
	int left;
	int right;
	int top;
	int bottom;

	PolyPtr tp;
	RailPtr rp;
	ActorPtr ap;
	DecorPtr dec;
	if (objects.size() > 0)
	{
		SelectPtr sp = objects.front();
		tp = sp->GetAsTerrain();
		rp = sp->GetAsRail();
		dec = sp->GetAsDecor();

		if (tp != NULL)
		{
			left = tp->left;
			right = tp->right;
			top = tp->top;
			bottom = tp->bottom;
		}
		else if (rp != NULL)
		{
			left = rp->left;
			right = rp->right;
			top = rp->top;
			bottom = rp->bottom;
		}
		else if(dec != NULL)
		{
			IntRect ir = dec->GetAABB();
			left = ir.left;
			right = ir.left + ir.width;
			top = ir.top;
			bottom = ir.top + ir.height;
		}
		/*ap = sp->GetAsActor();
		if (ap != NULL)
		{
			ap = sp->GetAsActor();
			return ap->position;
		}*/
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
		rp = (*it)->GetAsRail();
		dec = (*it)->GetAsDecor();


		if (tp != NULL)
		{
			left = min(left, tp->left);
			right = max(right, tp->right);
			top = min(top, tp->top);
			bottom = max(bottom, tp->bottom);
		}
		else if (rp != NULL)
		{
			left = min(left, rp->left);
			right = max(right, rp->right);
			top = min(top, rp->top);
			bottom = max(bottom, rp->bottom);
		}
		else if (dec != NULL)
		{
			IntRect ir = dec->GetAABB();
			left = min( left, ir.left);
			right = max( right, ir.left + ir.width);
			top = min( top, ir.top);
			bottom = max( bottom, ir.top + ir.height);
		}
	}

	centerF.x = (left + right) / 2.f;
	centerF.y = (top + bottom) / 2.f;

	return centerF;
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
			PolyPtr ptr = tp->Copy();
			newBrush->AddObject(ptr);
		}
	}

	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		ap = (*it)->GetAsActor();
		if (ap != NULL)
		{
			if (ap->type == sess->types["player"])
			{
				continue;
			}

			PolyPtr myPoly = ap->posInfo.ground;
			
			
			ActorPtr aPtr = ap->Copy();
			aPtr->CreateMyEnemy();
			aPtr->selected = false;

			if (myPoly != NULL)
			{	
				if (myPoly->selected)
				{
					aPtr->posInfo.ground = myPoly->mostRecentCopy;
					aPtr->AnchorToGround(PositionInfo(aPtr->posInfo));
					aPtr->posInfo.AddActor(aPtr);
				}
				else
				{
					aPtr->UnAnchor();
					if (aPtr->myEnemy != NULL)
					{
						aPtr->myEnemy->UpdateFromEditParams(0);
						//aPtr->myEnemy->UpdateFromEditParams(0);
						//myEnemy->UpdateSprite(); //this is just for testing
					}
				}
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

Brush *Brush::CopyFreeActors()
{
	EditSession *sess = EditSession::GetSession();
	Brush *newBrush = new Brush;
	ActorPtr ap;


	set<PolyPtr> polys;

	PolyPtr p;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		p = (*it)->GetAsTerrain();
		if (p != NULL)
		{
			polys.insert(p);
		}
	}

	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		ap = (*it)->GetAsActor();
		if (ap != NULL)
		{
			if (ap->type == sess->types["player"])
			{
				continue;
			}

			PolyPtr myPoly = ap->posInfo.ground;

			//if (myPoly != NULL && !myPoly->selected )
			if( myPoly != NULL && polys.find( myPoly  ) == polys.end() )
			{
				ActorPtr aPtr = ap->Copy();
				aPtr->CreateMyEnemy();
				aPtr->selected = false;

				aPtr->UnAnchor();
				if (aPtr->myEnemy != NULL)
				{
					aPtr->myEnemy->UpdateFromEditParams(0);
				}
				newBrush->AddObject(aPtr);
			}
			else if (myPoly == NULL)
			{
				ActorPtr aPtr = ap->Copy();
				aPtr->CreateMyEnemy();
				aPtr->selected = false;

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

Brush *Brush::CopyTerrainAndAttachedActors()
{
	EditSession *sess = EditSession::GetSession();

	Brush *newBrush = new Brush;
	PolyPtr tp;
	ActorPtr ap;
	RailPtr rp;
	DecorPtr dp;

	set<PolyPtr> polys;
	set<RailPtr> rails;

	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		tp = (*it)->GetAsTerrain();
		rp = (*it)->GetAsRail();
		dp = (*it)->GetAsDecor();

		if (tp != NULL)
		{
			PolyPtr ptr = tp->Copy();
			newBrush->AddObject(ptr);
			polys.insert(tp);
		}
		else if (rp != NULL)
		{
			RailPtr ptr = rp->Copy();
			newBrush->AddObject(ptr);

			rails.insert(rp);
		}
		else if (dp != NULL)
		{
			DecorPtr ptr = dp->Copy();
			newBrush->AddObject(ptr);
		}
	}

	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		ap = (*it)->GetAsActor();
		if (ap != NULL)
		{
			if (ap->type == sess->types["player"])
			{
				continue;
			}

			PolyPtr myPoly = ap->posInfo.ground;
			RailPtr myRail = ap->posInfo.railGround;

			//if (myPoly != NULL && myPoly->selected)
			if( myPoly != NULL && polys.find( myPoly ) != polys.end() )
			{
				ActorPtr aPtr = ap->Copy();
				aPtr->CreateMyEnemy();
				aPtr->selected = false;

				aPtr->posInfo.ground = myPoly->mostRecentCopy;
				aPtr->AnchorToGround(PositionInfo(aPtr->posInfo));
				aPtr->posInfo.AddActor(aPtr);
				newBrush->AddObject(aPtr);
			}
			//else if (myRail != NULL && myRail->selected)
			else if( myRail != NULL && rails.find( myRail ) != rails.end() )
			{
				ActorPtr aPtr = ap->Copy();
				aPtr->CreateMyEnemy();
				aPtr->selected = false;

				aPtr->posInfo.railGround = myRail->mostRecentCopy;
				aPtr->AnchorToRail(PositionInfo(aPtr->posInfo));
				aPtr->posInfo.AddActor(aPtr);
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

bool Brush::IsSingleDecor()
{
	return (objects.size() == 1 && objects.front()->selectableType == ISelectable::IMAGE);
}

bool Brush::IsSingleActor()
{
	return (objects.size() == 1 && objects.front()->selectableType == ISelectable::ACTOR);
}

bool Brush::IsSingleRail()
{
	return (objects.size() == 1 && objects.front()->selectableType == ISelectable::RAIL);
}

SelectPtr Brush::GetFirst()
{
	assert(!objects.empty());
	return objects.front();
}

void Brush::CreatePreview(const std::string &filePath)
{
	sf::RenderTexture *previewTex = EditSession::GetSession()->brushPreviewTex;

	previewTex->clear(Color::Black);

	//auto aabb = //GetAABB();
	Vector2f center(GetCenter());
	IntRect aabb = GetAABB();
	int longestEdge = max(aabb.width, aabb.height) + 300;

	sf::View previewView;
	previewView.setCenter(center);
	previewView.setSize(Vector2f(longestEdge, longestEdge));

	previewTex->setView(previewView);

	Draw(previewTex);

	previewTex->display();

	Image img = previewTex->getTexture().copyToImage();

	std::string previewFile = filePath + ".png";
	img.saveToFile(previewFile);
}

void Brush::Save(std::ofstream &of)
{
	int terrainCounter = 0;

	set<PolyPtr> polys;

	PolyPtr poly;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		poly = (*it)->GetAsTerrain();
		if (poly != NULL)
		{
			poly->writeIndex = terrainCounter;
			poly->BrushSave(of);
			++terrainCounter;

			polys.insert(poly);
		}
	}

	Brush freeGroundedActors;

	ActorPtr a;
	PolyPtr myPoly;

	bool freeGrounded = false;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		poly = (*it)->GetAsTerrain();
		if (poly != NULL)
		{
			continue;
		}

		a = (*it)->GetAsActor();
		freeGrounded = false;

		if (a != NULL)
		{
			myPoly = a->posInfo.ground;

			if (myPoly != NULL && polys.find(myPoly) == polys.end())
			{
				myPoly->writeIndex = -2; //so that it can't show up in the list
				freeGrounded = true;
			}

			a->BrushSave(of);
			of << (int)freeGrounded << "\n";
		}
		else
		{
			(*it)->BrushSave(of);
		}
	}
}

bool Brush::Load(std::ifstream &is)
{
	int selType;
	int inversePoly;
	string typeName;
	string groupName;

	EditSession *edit = EditSession::GetSession();

	while (true)
	{
		is >> selType;
		if (is.fail())
		{
			break;
		}
		switch (selType)
		{
		case ISelectable::TERRAIN:
		{
			is >> inversePoly;
			PolyPtr poly(new TerrainPolygon());
			if (inversePoly)
			{
				poly->inverse = true;
			}

			poly->Load(is);
			poly->Finalize();
			poly->LoadGrass(is);
			objects.push_back(poly);
			break;
		}
		case ISelectable::ACTOR:
		{
			is >> groupName;
			
			Vector2i savedPos;
			is >> savedPos.x;
			is >> savedPos.y;

			is >> typeName;

			ActorPtr a(NULL);
			ActorType *at = NULL;

			if (edit->types.count(typeName) == 0)
			{
				cout << "TYPENAME: " << typeName << endl;
				assert(false && "bad typename");
			}
			else
			{
				at = edit->types[typeName];
			}

			at->LoadEnemy(is, a);

			ActorGroup *ag = edit->groups[groupName];
			a->group = ag;

			int freeGrounded;
			is >> freeGrounded;

			if (freeGrounded)
			{
				a->MoveTo(savedPos);
			}

			objects.push_back(a);
			break;
		}
			

			
		case ISelectable::GATE:
			break;
		case ISelectable::IMAGE:
			break;
		case ISelectable::RAIL:
			break;
		}
	}

	return true;
	/*for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->Save(of);
	}*/
}

bool Brush::IsSingleFlyPoly()
{
	if (objects.size() == 1 && objects.front()->selectableType == ISelectable::TERRAIN)
	{
		PolyPtr poly = objects.front()->GetAsTerrain();
		if (poly->GetSpecialPolyIndex() == 2)
		{
			return true;
		}
	}
	return false;
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

int Brush::GetTerrainLayer()
{
	int currLayer = -1;
	int specialIndex;
	PolyPtr poly;
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		poly = (*it)->GetAsTerrain();
		if (poly != NULL)
		{
			if (currLayer < 0)
			{
				currLayer = poly->GetSpecialPolyIndex();
			}
			else
			{
				specialIndex = poly->GetSpecialPolyIndex();
				if (specialIndex != currLayer)
				{
					return -1;
				}
			}
		}
	}

	return currLayer;
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
			if (ap->posInfo.ground != NULL && !ap->posInfo.ground->selected )
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

CreateGateAction::CreateGateAction( GateInfo &info)
{
	//EditSession *session = EditSession::GetSession();
	gate = new GateInfo(info);
	//GateInfo *gi = new GateInfo;

	//gate->SetType( type );
	//if (gate->type == Gate::SHARD)
	//{
	//	gate->SetShard(info.shardWorld, info.shardIndex);
//	}


	
	//gate->poly0 = info.poly0;
	//gate->vertexIndex0 = info.vertexIndex0;
	//gate->point0 = info.point0;

	//gate->poly1 = info.poly1;
	//gate->vertexIndex1 = info.vertexIndex1;
	//gate->point1 = info.point1;


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
	EditSession *edit = EditSession::GetSession();
	performed = true;

	edit->gateInfoList.push_back( gate );
	gate->point0->gate = gate;
	gate->point1->gate = gate;
	//create the gate
}

void CreateGateAction::Undo()
{
	assert( performed );
	EditSession *edit = EditSession::GetSession();
	performed = false;

	edit->gateInfoList.remove( gate );
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
	EditSession *edit = EditSession::GetSession();
	performed = true;
	edit->gateInfoList.remove( gate );
	gate->point0->gate = NULL;
	gate->point1->gate = NULL;
	//create the gate
}

void DeleteGateAction::Undo()
{
	EditSession *edit = EditSession::GetSession();
	performed = false;
	edit->gateInfoList.push_back( gate );
	gate->point0->gate = gate;
	gate->point1->gate = gate;
	//destroy the gate
}

ModifyGateAction::ModifyGateAction( GateInfoPtr ptr, GateInfoPtr orig )
	:origInfo( *orig ), newInfo( *ptr )
{
	gate = ptr;
}

void ModifyGateAction::Perform()
{
	assert( !performed );

	performed = true;

	(*gate) = newInfo;

	gate->UpdateLine();

	EditSession *edit = EditSession::GetSession();
	if (edit->createGatesModeUI->modifyGate == gate)
	{
		edit->createGatesModeUI->SetFromGateInfo(gate);
	}
}

void ModifyGateAction::Undo()
{
	assert( performed );

	performed = false;
	
	(*gate) = origInfo;

	//gate->SetType( oldType );
	//gate->type = oldType;
	gate->UpdateLine();

	EditSession *edit = EditSession::GetSession();
	if (edit->createGatesModeUI->modifyGate == gate)
	{
		edit->createGatesModeUI->SetFromGateInfo(gate);
	}
}

//doesn't make a copy of the brush!
MoveBrushAction::MoveBrushAction( Brush *p_brush, sf::Vector2i p_delta, bool p_moveOnFirstPerform,
	PointMover *pm)
	:delta( p_delta ), moveOnFirstPerform( p_moveOnFirstPerform ), pointMover( pm)
{
	//moveValid = true;
	movingBrush = *p_brush;
}

MoveBrushAction::~MoveBrushAction()
{
	if (pointMover != NULL)
		delete pointMover;
}

void MoveBrushAction::Perform()
{
	assert( !performed );

	performed = true;

	movingBrush.Move( delta );

	if (pointMover != NULL)
	{
		for (auto it = pointMover->movePoints.begin(); it != pointMover->movePoints.end(); ++it)
		{
			vector<PointMoveInfo> &pVec = (*it).second;
			for (auto pit = pVec.begin(); pit != pVec.end(); ++pit)
			{
				(*pit).poly->SetPointPos((*pit).pointIndex, (*pit).newPos);
			}

			PolyPtr poly = (*it).first;
			poly->SoftReset();
			poly->Finalize();

			poly->SetGrassFromPointMoveInfoVectors(pVec);

			/*int prevIndex;
			for (auto pit = pVec.begin(); pit != pVec.end(); ++pit)
			{
				prevIndex = (*pit).pointIndex - 1;
				if (prevIndex < 0)
					prevIndex = (*pit).poly->GetNumPoints() - 1;

				(*pit).poly->SetGrassVecOn((*pit).pointIndex, (*pit).grassVec);
			}*/

			poly->SetRenderMode(TerrainPolygon::RENDERMODE_NORMAL);
		}

		for (auto it = pointMover->railMovePoints.begin(); it != pointMover->railMovePoints.end(); ++it)
		{
			vector<PointMoveInfo> &pList = (*it).second;
			for (auto pit = pList.begin(); pit != pList.end(); ++pit)
			{
				(*pit).rail->SetPointPos((*pit).pointIndex, (*pit).newPos);
			}
			(*it).first->UpdateLines();
			(*it).first->UpdateBounds();
			/*(*it).first->SoftReset();
			(*it).first->Finalize();
			(*it).first->movingPointMode = false;*/
		}

		for (auto it = pointMover->newEnemyPosInfo.begin(); it != pointMover->newEnemyPosInfo.end(); ++it)
		{
			(*it).first->posInfo = (*it).second;

			if ((*it).first->myEnemy != NULL)
			{
				(*it).first->myEnemy->UpdateOnEditPlacement();
			}
				

			(*it).first->UpdateGroundedSprite();
			(*it).first->SetBoundingQuad();
		}

		//if I want to optimize later, have the Finalize call not update bounds, because
		//its forced to be called twice in this function. Once before enemies and once after.
		if (!pointMover->newEnemyPosInfo.empty())
		{
			for (auto it = pointMover->movePoints.begin(); it != pointMover->movePoints.end(); ++it)
			{
				(*it).first->UpdateBounds();
			}
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

	//for( auto it = movingPoints.myMap.begin(); it != movingPoints.myMap.end(); ++it )
	if (pointMover != NULL)
	{
		for (auto it = pointMover->movePoints.begin(); it != pointMover->movePoints.end(); ++it)
		{
			vector<PointMoveInfo> &pVec = (*it).second;

			for (auto pit = pVec.begin(); pit != pVec.end(); ++pit)
			{
				(*pit).poly->SetPointPos((*pit).pointIndex, (*pit).origPos);
			}

			PolyPtr poly = (*it).first;

			poly->SoftReset();
			poly->Finalize();

			/*for (auto pit = pVec.begin(); pit != pVec.end(); ++pit)
			{
				(*pit).poly->SetGrassVecOn((*pit).pointIndex, (*pit).grassVec);
			}*/
			poly->SetGrassFromPointMoveInfoVectors(pVec);

			poly->SetRenderMode(TerrainPolygon::RENDERMODE_NORMAL);

			/*for ( auto pit = poly->enemies.begin();
				pit != poly->enemies.end(); ++pit)
			{
				list<ActorPtr> &enemies = (*pit).second;
				for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
				{
					(*ait)->UpdateGroundedSprite();
					(*ait)->SetBoundingQuad();
				}
			}*/
		}

		for (auto it = pointMover->railMovePoints.begin(); it != pointMover->railMovePoints.end(); ++it)
		{
			vector<PointMoveInfo> &pList = (*it).second;
			for (auto pit = pList.begin(); pit != pList.end(); ++pit)
			{
				(*pit).rail->SetPointPos((*pit).pointIndex, (*pit).origPos);
			}
			(*it).first->UpdateLines();
			(*it).first->UpdateBounds();
			/*(*it).first->SoftReset();
			(*it).first->Finalize();
			(*it).first->movingPointMode = false;*/
		}

		for (auto it = pointMover->oldEnemyPosInfo.begin(); it != pointMover->oldEnemyPosInfo.end(); ++it)
		{
			(*it).first->posInfo = (*it).second;
			if ((*it).first->myEnemy != NULL)
				(*it).first->myEnemy->UpdateOnEditPlacement();

			(*it).first->UpdateGroundedSprite();
			(*it).first->SetBoundingQuad();
			
		}

		if (!pointMover->oldEnemyPosInfo.empty())
		{
			//if I want to optimize later, have the Finalize call not update bounds, because
			//its forced to be called twice in this function. Once before enemies and once after.
			for (auto it = pointMover->movePoints.begin(); it != pointMover->movePoints.end(); ++it)
			{
				(*it).first->UpdateBounds();
			}
		}
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
	
	


	mapStartBrush->TransferMyDuplicates(&newOrig, &mapStartOwned);

	bool found;
	for (auto removedIt = newOrig.objects.begin(); removedIt != removedEnd; ++removedIt)
	{
		found = false;
		for( auto appIt = applied.objects.begin(); appIt != applied.objects.end(); ++appIt)
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
	for (auto addIt = newResult.objects.begin(); addIt != newResult.objects.end(); ++addIt)
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

GrassAction::GrassAction(GrassDiff *p_gDiffs, int p_numDiffs )
	:gDiffs(p_gDiffs), numDiffs( p_numDiffs )
{
	assert(gDiffs != NULL);
}

GrassAction::~GrassAction()
{
	delete[] gDiffs;
}

void GrassAction::Perform()
{
	assert(!performed);

	EditSession *edit = EditSession::GetSession();
	bool showGrass = edit->editModeUI->IsShowGrassOn();
	for (int i = 0; i < numDiffs; ++i)
	{
		GrassDiff &gDiff = gDiffs[i];
		gDiff.poly->SetGrassFromAction(gDiff.index, gDiff.newValue, showGrass );
	}
	performed = true;
}
void GrassAction::Undo()
{
	assert(performed);
	EditSession *edit = EditSession::GetSession();
	bool showGrass = edit->editModeUI->IsShowGrassOn();
	for (int i = 0; i < numDiffs; ++i)
	{
		GrassDiff &gDiff = gDiffs[i];
		gDiff.poly->SetGrassFromAction(gDiff.index, gDiff.oldValue, showGrass);
	}
	performed = false;
}

//MovePointsAction::MovePointsAction(Brush *brush, list<GateInfoPtr> gateList,
//	PointMap &pm)
//	:Action()
//{
//	polyBrush = *brush;
//	gList = gateList;
//	pMap = pm;
//}
//
//void MovePointsAction::Perform()
//{
//	if (!performed)
//	{
//		polyBrush.Deactivate();
//		newBrush.Activate();
//	}
//}
//
//void MovePointsAction::Undo()
//{
//	if (performed)
//	{
//		newBrush.Deactivate();
//		polyBrush.Activate();
//	}
//}