#ifndef __EDITORRAIL_H__
#define __EDITORRAIL_H__

#include <list>
#include <SFML\Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include "ISelectable.h"
#include "VectorMath.h"

struct Panel;
struct TerrainPoint;
struct ActorParams;

struct TerrainRail //: ISelectable
{
	//TerrainRail();
	//TerrainRail(TerrainRail &r);
	//~TerrainRail();

	//void Init();
	//bool ContainsPoint(sf::Vector2f test);
	//bool ContainsPoint(sf::Vector2f test,double rad);

	//bool Intersects(sf::IntRect rect);
	////bool IsPlacementOkay();

	//void Move(SelectPtr me,
	//	sf::Vector2i delta);
	//void BrushDraw(sf::RenderTarget *target,
	//	bool valid);

	//void Draw(double zoomMultiple, bool showPoints, 
	//	sf::RenderTarget *target);

	////void Draw(sf::RenderTarget *target);
	//void Deactivate(EditSession *edit,
	//	SelectPtr select);
	//void Activate(EditSession *edit,
	//	SelectPtr select);
	//bool CanApply();
	//bool CanAdd();


	//void SetSelected(bool select);


	//void UpdateLineColor(sf::Vertex *li, 
	//	TerrainPoint *p, int index);
	//void UpdateLines();
	//void SwitchDirection();
	//void SetParams(Panel *p);
	//void UpdatePanel(Panel *p);
	//void CopyPoints(TerrainPoint *&start,
	//	TerrainPoint *&end);
	//void CopyOtherPoints(TerrainPoint *&start,
	//	TerrainPoint *&end);
	//void CopyPoints(TerrainRail *rail);
	//TerrainRail *Copy();

	//bool IsPoint(sf::Vector2i &p);

	//void AddPoint(TerrainPoint* tp);
	//void InsertPoint(TerrainPoint *tp, TerrainPoint *prevPoint);
	//void RemovePoint(TerrainPoint *tp);

	//void ClearPoints();

	//int GetPointIndex(TerrainPoint *p);
	//TerrainPoint *GetPointAtIndex(int index);

	//TerrainPoint * HasPointPos(sf::Vector2i &pos);

	//void RemoveSelectedPoints();

	//int IsRemovePointsOkayEnemies(EditSession *edit);

	//void Finalize();

	//void Reset();
	//void SoftReset();

	//void AlignExtremes(double primLimit);

	//void MoveSelectedPoints(sf::Vector2i move);
	//
	//void UpdateBounds();

	//void WriteFile(std::ofstream &of);
	//void Load(std::ifstream &is);

	//TerrainPoint *GetClosePoint(double radius, V2d &pos);

	//sf::Rect<int> TempAABB();

	//bool movingPointMode;
	//int numPoints;
	//TerrainPoint *pointStart;
	//TerrainPoint *pointEnd;

	//double railRadius;

	//sf::Vertex *lines;
	//int numLineVerts;

	//sf::Vertex *quads;

	//int left;
	//int right;
	//int top;
	//int bottom;

	//std::map<TerrainPoint*, std::list<
	//	boost::shared_ptr<ActorParams>>> enemies;

	//int writeIndex;
	//bool finalized;

	//bool requirePower;
	//bool accelerate;
	//int level;

	//const static int MAX_RAIL_LEVEL = 12;
};

typedef boost::shared_ptr<TerrainRail> RailPtr;

#endif