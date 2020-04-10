#ifndef __ISELECTABLE_H__
#define __ISELECTABLE_H__

#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

struct EditSession;
struct TerrainPolygon;
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
	static boost::shared_ptr<TerrainPolygon> GetAsTerrain(
		boost::shared_ptr<ISelectable> select );
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
	virtual void Move(boost::shared_ptr<ISelectable> me,
		sf::Vector2i delta) {}
	virtual void BrushDraw(sf::RenderTarget *target,
		bool valid) {}
	virtual void Draw(sf::RenderTarget *target) {}
	virtual void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select) = 0;
	virtual void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select) = 0;
	virtual bool CanApply() { return true; }
	virtual bool CanAdd() { return true; }
	virtual void SetSelected(bool select) {}
	//virtual bool CanSubtract() = 0;

	ISelectableType selectableType;
	bool active;
	bool selected;
};

typedef boost::shared_ptr<ISelectable> SelectPtr;
typedef std::list<SelectPtr> SelectList;
typedef SelectList::iterator SelectIter;

#endif