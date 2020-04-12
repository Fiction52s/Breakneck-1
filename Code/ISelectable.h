#ifndef __ISELECTABLE_H__
#define __ISELECTABLE_H__

#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

struct EditSession;
struct TerrainPolygon;
struct ActorParams;
struct GateInfo;
struct TerrainRail;
struct EditorDecorInfo;

typedef TerrainPolygon* PolyPtr;
typedef ActorParams* ActorPtr;
typedef GateInfo* GateInfoPtr;
typedef TerrainRail* RailPtr;
typedef EditorDecorInfo* DecorPtr;

struct ISelectable
{
	enum ISelectableType
	{
		TERRAIN,
		ACTOR,
		GATE,
		IMAGE,
		RAIL,
		Count
	};
	//copyable
	//is a point inside me
	//is a rectangle intersecting me
	//is a move valid
	//execute move
	ISelectable(ISelectableType type);
	virtual ~ISelectable() {}
	PolyPtr GetAsTerrain();
	ActorPtr GetAsActor();
	GateInfoPtr GetAsGateInfo();
	RailPtr GetAsRail();

	virtual bool ContainsPoint(sf::Vector2f test)
	{
		return false;
	}
	virtual bool Intersects(sf::IntRect rect)
	{
		return false;
	}
	virtual bool IsPlacementOkay()
	{
		return true;
	}
	virtual void Move(sf::Vector2i delta) {}
	virtual void BrushDraw(sf::RenderTarget *target,
		bool valid) {}
	virtual void Draw(sf::RenderTarget *target) {}
	virtual void Deactivate() = 0;
	virtual void Activate() = 0;
	virtual bool CanApply() { return true; }
	virtual bool CanAdd() { return true; }
	virtual void SetSelected(bool select) {}

	ISelectableType selectableType;
	bool active;
	bool selected;
};

typedef ISelectable* SelectPtr;
typedef std::list<SelectPtr> SelectList;

#endif