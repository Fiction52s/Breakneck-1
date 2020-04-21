#ifndef __ACTION_H__
#define __ACTION_H__

#include <list>
#include "EditSession.h"

struct CompoundAction;
struct Brush
{
	Brush();
	SelectList objects;
	void AddObject( SelectPtr obj );
	void RemoveObject( SelectPtr obj );
	int GetNumGroundedActors();
	int GetNumActorsThatMustBeAnchored();
	void Clear();
	void Destroy();
	void Move( sf::Vector2i delta );
	void Draw( sf::RenderTarget *target );
	Brush *Copy();
	sf::Vector2i &GetCenter();
	sf::Vector2f &GetCenterF();
	void Rotate(float fDegrees );
	void CenterOnPoint(sf::Vector2i &point );
	sf::Vector2i center;
	sf::Vector2f centerF;
	CompoundAction * UnAnchor(); //only works with grounded actors
	void Deactivate();//multiple calls covered?
	void Activate();//multiple calls covered?
	void SetSelected( bool select );
	bool CanApply();
	bool CanAdd();
	bool Has(SelectPtr sp);
	void TransferMyDuplicates(Brush *compare, Brush *dest);
	void TransferMyActiveMembers(Brush *dest);
	bool terrainOnly;
	bool IsEmpty();

	sf::Vector2f GetTerrainSize();
};

struct Action
{
	enum ActionType
	{
		APPLY_BRUSH,
		REMOVE_BRUSH,
		REPLACE_BRUSH,
		DESTROY_ACTOR,
		MOVE_BRUSH,
		MOVE_POINTS,
		DELETE_POINTS,
		EDIT_OBJECT,
		CREATE_GATE,
		DELETE_GATE,
		COMPOUND,
		Count
	};
	//Action( ActionType actionType, Action *next = NULL );
	Action();
	virtual ~Action();
	virtual void Perform() = 0;
	virtual void Undo() = 0;

	//Action *next;
	//Action *prev;
	ActionType actionType;
	bool performed;
};

struct CompoundAction : Action
{
	CompoundAction();
	~CompoundAction();
	std::list<Action*> subActions;
	void Perform();
	void AddSubAction(Action *);
	void Undo();
};

struct ComplexPasteAction : Action
{
	ComplexPasteAction( Brush *mapStartBrush );
	~ComplexPasteAction();
	void SetNewest(Brush &orig, Brush &result );
	void Undo();
	void Perform();

	Brush *mapStartBrush;

	Brush orig;
	Brush applied;

	Brush mapStartOwned;
};

struct ApplyBrushAction : Action
{
	ApplyBrushAction( Brush *brush );
	~ApplyBrushAction();

	void Perform();
	void Undo();


	//Brush *brush; //action doesn't own this
	Brush appliedBrush;
};

struct RemoveBrushAction : Action
{
	RemoveBrushAction( Brush *brush, Brush *mapStartBrush );
	~RemoveBrushAction();
	void Perform();
	void Undo();

	Brush storedBrush;
	Brush mapStartOwned;

	Brush *mapStartBrush;
};

struct ReplaceBrushAction : Action
{
	ReplaceBrushAction( Brush *original,
		Brush *replacement, Brush *mapStartBrush );
	~ReplaceBrushAction();
	void Perform();
	void Undo();

	Brush original;
	Brush replacement;
	Brush mapStartOwned;

	Brush *mapStartBrush;
};


//--NOTE: DOESNT ACTUALLY MOVE BRUSH
//IF PARAMETER IS SET
struct MoveBrushAction : Action
{
	MoveBrushAction( Brush *brush, sf::Vector2i delta,
		bool moveOnFirstPerform,
		PointVectorMap &points, RailPointMap &railPoints);
	void Perform();
	void Undo();
	void CheckValidPointMove();

	bool moveValid;
	Brush movingBrush;
	//std::list<PointMoveInfo> movingPoints;
	PointVectorMap movingPoints;
	RailPointMap movingRailPoints;
	bool moveOnFirstPerform;
	sf::Vector2i delta;
};

struct LeaveGroundAction : Action
{
	LeaveGroundAction( ActorPtr a, V2d &extra );
		//sf::Vector2i newPos );

	V2d extra;
	ActorPtr actor;
	PositionInfo gi;
	void Perform();
	void Undo();
};

struct GroundAction : Action
{
	GroundAction( ActorPtr actor );
	void Perform();
	void Undo();

	ActorPtr actor;
	PositionInfo gi;
	//sf::Vector2i oldPos;
};

struct EditObjectAction : Action
{
	EditObjectAction();
	void Perform();
	void Undo();

	ActorParams *actor;
};

struct CreateGateAction : Action
{
	CreateGateAction( GateInfo &info, 
		const std::string &type);
	~CreateGateAction();
	void Perform();
	void Undo();

	GateInfoPtr gate;
};

struct DeleteGateAction : Action
{
	DeleteGateAction( GateInfoPtr ptr, Brush *mapStartBrush );
	~DeleteGateAction();
	void Perform();
	void Undo();

	GateInfoPtr gate;
	bool owned;
};

struct ModifyGateAction : Action
{
	ModifyGateAction( GateInfoPtr ptr, const
		std::string &type );
	void Perform();
	void Undo();

	GateInfoPtr gate;
	Gate::GateType oldType;
	std::string newType;
};

struct ModifyTerrainTypeAction : Action
{
	ModifyTerrainTypeAction( Brush *brush,
		int newTerrainWorld,
		int newVariation );
	void Perform();
	void Undo();

	int newTerrainWorld;
	int newVariation;

	std::map<PolyPtr,std::pair<int,int>> 
		terrainTypeMap;
	Brush terrainBrush; 
	//also has other stuff
	//in it possibly
	
};



#endif