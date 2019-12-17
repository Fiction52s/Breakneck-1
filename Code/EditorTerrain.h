#ifndef __EDITORTERRAIN_H__
#define __EDITORTERRAIN_H__

#include <list>
#include <SFML\Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include "ISelectable.h"
#include "VectorMath.h"

struct ISelectable;
struct GateInfo;
struct EditSession;
struct TerrainPolygon;
struct ActorParams;
struct TerrainRender;

struct GrassSeg
{
	GrassSeg(int edgeI, int grassIndex, int rep)
		:edgeIndex(edgeI), index(grassIndex),
		reps(rep)
	{}
	int edgeIndex;
	int index;
	int reps;
};

struct TerrainPoint
{
	TerrainPoint(sf::Vector2i &pos, bool selected);
	~TerrainPoint()
	{
		//delete gate;
	}
	sf::Vector2i pos;
	bool selected;
	std::list<int> grass;
	boost::shared_ptr<GateInfo> gate;
	//GateInfo *gate;
	bool firstPoint;
	TerrainPoint *next;
	TerrainPoint *prev;

	bool ContainsPoint(sf::Vector2f test);
	bool Intersects(sf::IntRect rect);
	bool IsPlacementOkay();
	void Move(boost::shared_ptr<ISelectable> &me,
		sf::Vector2i delta);
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	void Draw(sf::RenderTarget *target);
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	bool CanApply();
	bool CanAdd();
	void SetSelected(bool select);

	static const int POINT_RADIUS = 5;
	//int special;
};

struct Inter
{
	Inter()
		:point(NULL)
	{

	}
	Inter(TerrainPoint *p_point, sf::Vector2<double> &p_pos)
		:point(p_point), position(p_pos)
	{

	}
	TerrainPoint *point;
	sf::Vector2<double> position;
};

struct DetailedInter
{
	DetailedInter()
		:otherPoint(NULL)
	{
		inter.point = NULL;
	}
	DetailedInter(TerrainPoint *p_point, sf::Vector2<double> &p_pos, TerrainPoint *p_otherPoint)
		:inter(p_point, p_pos), otherPoint(p_otherPoint)
	{

	}
	Inter inter;
	TerrainPoint *otherPoint;
};

struct TerrainBrush
{
	TerrainBrush(boost::shared_ptr<TerrainPolygon> poly);
	TerrainBrush(TerrainBrush &brush);
	~TerrainBrush();
	void AddPoint(TerrainPoint* tp);

	void UpdateLines();
	void Move(sf::Vector2i delta);
	void Draw(sf::RenderTarget *target);
	int numPoints;
	sf::VertexArray lines;
	TerrainPoint *pointStart;
	TerrainPoint *pointEnd;
	int left;
	int right;
	int top;
	int bot;

};



struct TerrainPolygon : ISelectable
{
	enum TerrainWorldType
	{
		MOUNTAIN,
		GLADE,
		DESERT,
		COVE,
		JUNGLE,
		FORTRESS,
		CORE,
		Count
	};

	//TerrainRender *tr;
	int terrainVariation;
	TerrainWorldType terrainWorldType;
	int GetNumGrassTotal();
	int GetNumGrass(TerrainPoint *tp, bool &rem);
	void SetupGrass(TerrainPoint *tp, int &i);
	void SetupGrass();
	sf::Shader *pShader;

	void UpdateLines();
	TerrainPolygon(sf::Texture *grassTex);
	TerrainPolygon(TerrainPolygon &poly, bool pointsOnly);
	~TerrainPolygon();
	void UpdateLineColor(sf::Vertex *line, TerrainPoint *p, int index);
	bool SwitchPolygon(bool cw, TerrainPoint *rootPoint,
		TerrainPoint *switchStart);
	void CopyPoints(TerrainPoint *&start,
		TerrainPoint *&end);
	sf::Vector2i TrimSliverPos(sf::Vector2<double> &prevPos,
		sf::Vector2<double> &pos, sf::Vector2<double> &nextPos,
		double minAngle, bool cw);
	void CopyPoints(TerrainPolygon *poly);
	TerrainPolygon *Copy();
	bool PointOnBorder(V2d &point);
	void MovePoint(sf::Vector2i &delta,
		TerrainPoint *tp);

	TerrainPoint *pointStart;
	TerrainPoint *pointEnd;
	bool IsPoint(sf::Vector2i &p);
	TerrainPoint *GetSamePoint(sf::Vector2i &p);
	int numPoints;
	void AddPoint(TerrainPoint* tp);
	void InsertPoint(TerrainPoint *tp, TerrainPoint *prevPoint);
	void RemovePoint(TerrainPoint *tp);
	void DestroyEnemies();
	void ClearPoints();
	TerrainPoint *GetPointAtIndex(int index);
	int GetPointIndex(TerrainPoint *p);
	void SetMaterialType(
		int world, int variation);
	void RemoveSlivers(double minAngle);
	int GetIntersectionNumber(sf::Vector2i &a, sf::Vector2i &b,
		Inter &inter, TerrainPoint *&outSegStart, bool &outFirstPoint);
	TerrainPoint *GetMostLeftPoint();
	bool SharesPoints(TerrainPolygon *poly);
	TerrainPoint * HasPointPos(sf::Vector2i &pos);
	LineIntersection GetSegmentFirstIntersection(sf::Vector2i &a, sf::Vector2i &b,
		TerrainPoint *&outSegStart, TerrainPoint *&outSegEnd,
		bool ignoreStartPoint = false);
	//std::string material;
	void RemoveSelectedPoints();
	void GetIntersections(TerrainPolygon *poly, std::list<Inter> &outInters);
	void GetDetailedIntersections(TerrainPolygon *poly, std::list<DetailedInter> &outInters);
	bool IsRemovePointsOkayTerrain(EditSession *edit);
	int IsRemovePointsOkayEnemies(EditSession *edit);
	void Finalize();
	void FinalizeInverse();
	void Reset();
	void SoftReset();
	void Draw(bool showPath, double zoomMultiple, sf::RenderTarget * rt, bool showPoints, TerrainPoint *dontShow);
	void FixWinding();
	void FixWindingInverse();
	bool IsClockwise();
	void AlignExtremes(double primLimit);
	void UpdateGrass();

	int grassSize;
	int grassSpacing;


	void ShowGrass(bool show);
	void Extend2(TerrainPoint* startPoint, TerrainPoint*endPoint, boost::shared_ptr<TerrainPolygon> inProgress);
	void Cut2(TerrainPoint* startPoint, TerrainPoint*endPoint, boost::shared_ptr<TerrainPolygon> inProgress);

	void SwitchGrass(sf::Vector2<double> mousePos);
	//bool ContainsPoint( sf::Vector2f p );
	void SetSelected(bool select);

	bool IsMovePointsOkay(
		sf::Vector2i delta);
	bool IsMovePointsOkay(
		sf::Vector2i pointGrabDelta,
		sf::Vector2i *deltas);
	bool IsMovePolygonOkay(
		sf::Vector2i delta);
	void MoveSelectedPoints(sf::Vector2i move);
	void UpdateBounds();

	void SetLayer(int p_layer);

	bool ContainsPoint(sf::Vector2f point);
	bool Intersects(sf::IntRect rect);
	bool IsPlacementOkay();
	//void Move( sf::Vector2i delta );
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	void Draw(sf::RenderTarget *target);
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);

	bool IsTouching(TerrainPolygon *poly);
	bool Contains(TerrainPolygon *poly);
	//bool IsTouching( TerrainPolygon * p );
	bool BoundsOverlap(TerrainPolygon *poly);
	bool LinesIntersect(TerrainPolygon *poly);
	bool PointTooCloseToPoints(sf::Vector2i point,
		int minDistance);
	bool PointTooClose(sf::Vector2i point,
		int minDistance);
	bool LinesTooClose(TerrainPolygon *poly,
		int minDistance);
	bool PointTooCloseToLines(sf::Vector2i point,
		int minDistance);
	bool SegmentTooClose(sf::Vector2i a,
		sf::Vector2i b, int minDistance);
	bool SegmentWithinDistanceOfPoint(
		sf::Vector2i startSeg,
		sf::Vector2i endSeg,
		sf::Vector2i testPoint,
		int distance);
	bool TooClose(TerrainPolygon *poly,
		bool intersectAllowed,
		int minDistance);
	TerrainPoint *GetClosePoint(double radius, V2d &pos);

	sf::Color selectCol;
	sf::Color fillCol;


	bool CanApply();
	bool CanAdd();




	bool movingPointMode;

	sf::Rect<int> TempAABB();

	void Move(SelectPtr me, sf::Vector2i move);

	sf::Vertex *lines;
	sf::VertexArray *va;
	sf::VertexArray *grassVA;
	int numGrassTotal;
	sf::Texture *grassTex;
	int vaSize;
	//bool selected;
	int left;
	int right;
	int top;
	int bottom;
	std::list<sf::Vector2i> path;

	//enemymap
	std::map<TerrainPoint*, std::list<
		boost::shared_ptr<ActorParams>>> enemies;
	int writeIndex;
	bool isGrassShowing;
	bool finalized;

	int layer; //0 is game layer. 1 is bg

	bool inverse;
};

struct GroundInfo
{
	GroundInfo();
	TerrainPoint *edgeStart;
	double groundQuantity;
	TerrainPolygon *ground;
	int GetEdgeIndex();
	V2d GetPosition();
};

struct PointMoveInfo
{
	PointMoveInfo(TerrainPoint *poi)
		:point(poi),
		delta(0, 0)
	{}
	TerrainPoint *point;
	sf::Vector2i delta;
};

struct EditorDecorInfo : ISelectable
{
	EditorDecorInfo(sf::Sprite &s, int lay,
		const std::string &dName, int p_tile)
		:ISelectable(ISelectable::ISelectableType::IMAGE),
		spr(s), layer(lay),
		decorName(dName), tile(p_tile) {}

	bool ContainsPoint(sf::Vector2f test);
	bool Intersects(sf::IntRect rect);
	void Move(boost::shared_ptr<ISelectable> me,
		sf::Vector2i delta);
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	void SetSelected(bool select);
	void WriteFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	sf::Sprite spr;
	int layer;
	std::string decorName;
	int tile;
	std::list<boost::shared_ptr<EditorDecorInfo>> *myList;
	static bool CompareDecorInfoLayer(EditorDecorInfo &di0, EditorDecorInfo &di1);
};

typedef boost::shared_ptr<EditorDecorInfo> EditorDecorPtr;



typedef boost::shared_ptr<TerrainPolygon> PolyPtr;
typedef boost::shared_ptr<TerrainPoint> PointPtr;
typedef std::pair<sf::Vector2i, sf::Vector2i> PointPair;
typedef std::map<TerrainPolygon*, std::list<PointMoveInfo>> PointMap;

#endif