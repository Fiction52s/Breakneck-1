#ifndef __EDITORTERRAIN_H__
#define __EDITORTERRAIN_H__

#include <list>
#include <SFML\Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include "ISelectable.h"
#include "VectorMath.h"
#include "clipper.hpp"
#include "EditorGateInfo.h"
#include "Physics.h"
     
struct ISelectable;
struct GateInfo;
struct EditSession;
struct TerrainPolygon;
struct ActorParams;
struct TerrainRender;
struct TerrainRail;
struct Brush;

struct QuadTree;

typedef std::set<std::pair<__int64,__int64>> ClipperIntPointSet;



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
	}
	sf::Vector2i pos;
	bool selected;
	std::list<int> grass;
	GateInfoPtr gate;
	bool firstPoint;
	int GetIndex();

	bool HasPrimaryGate(sf::Vector2i &gateDir);
	bool ContainsPoint(sf::Vector2f test);
	bool Intersects(sf::IntRect rect);

	int index;
	static const int POINT_RADIUS = 5;
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

struct PointMoveInfo
{
	PointMoveInfo(TerrainPoint *poi)
		:point(poi), origPos(poi->pos),
		delta(0, 0), moveIntent(false)
	{}
	TerrainPoint *point;
	sf::Vector2i delta;
	sf::Vector2i origPos;
	bool moveIntent;
};

struct BorderInfo
{
	const static int NUM_BORDER_SIZES = 2;
	const static int NUM_BORDER_VARIATIONS = 4;
	int numQuads[NUM_BORDER_SIZES];

	BorderInfo()
	{
		Clear();
	}
	void Add(BorderInfo &bi)
	{
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			numQuads[i] += bi.numQuads[i];
		}
	}

	int GetNumSizesWithCountAboveZero()
	{
		int total = 0;
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			if (numQuads[i] > 0)
				++total;
		}
		return total;
	}

	int DecrementSizeWithCountAboveZero(int index)
	{
		int aboveZeroCounter = 0;
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			if (numQuads[i] > 0)
			{
				if (aboveZeroCounter == index)
				{
					--(numQuads[i]);
					return i;
				}
				else
					aboveZeroCounter++;
			}
		}
	}

	int GetTotal()
	{
		int total = 0;
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			total += numQuads[i];
		}
		return total;
	}
	void Clear()
	{
		for (int i = 0; i < NUM_BORDER_SIZES; ++i)
		{
			numQuads[i] = 0;
		}
	}
};

struct BorderSizeInfo
{
	void SetWidth(int w);
	int width;
	int startInter;
	int inter;
	int len;
	int edgeLen;
	int startLen;
};

struct TerrainPolygon : ISelectable
{
	enum RenderMode
	{
		RENDERMODE_NORMAL,
		RENDERMODE_MOVING_POINTS,
		RENDERMODE_TRANSFORM,
	};

	enum TerrainWorldType
	{
		MOUNTAIN,
		GLADE,
		DESERT,
		COVE,
		JUNGLE,
		FORTRESS,
		CORE,
		SECRETCORE,
		WATER0,
		Count
	};

	enum EdgeAngleType : int
	{
		EDGE_FLAT,
		EDGE_FLATCEILING,
		EDGE_SLOPED,
		EDGE_STEEPSLOPE,
		EDGE_SLOPEDCEILING,
		EDGE_STEEPCEILING,
		EDGE_WALL,
	};

	RenderMode renderMode;
	void SetRenderMode(RenderMode rm);

	static double GetSteepThresh() { return .4; }

	EdgeAngleType GetEdgeAngleType(int index);
	static EdgeAngleType GetEdgeAngleType(Edge * e);
	static EdgeAngleType GetEdgeAngleType(V2d &normal);

	static bool IsFlatGround(sf::Vector2<double> &normal);
	static bool IsSlopedGround(sf::Vector2<double> &normal);
	static bool IsSteepGround(sf::Vector2<double> &normal);
	static bool IsWall(sf::Vector2<double> &normal);

	void DrawBorderQuads(sf::RenderTarget *target);

	void Scale(float f);
	void Rotate( sf::Vector2f &center, float degrees);

	void SetBorderTileset();
	static sf::IntRect GetBorderSubRect(int tileWidth, EdgeAngleType et, int var);

	sf::Vertex *borderQuads;
	int totalNumBorderQuads;
	void GenerateBorderMesh();
	Tileset *ts_border;

	static int GetBorderQuadIntersect(int tileWidth);

	static double GetExtraForInward(Edge *e);
	static double GetSubForOutward(Edge *e);

	static bool IsAcute(Edge *e0);
	static V2d GetBisector(Edge *e);

	


	Tileset *ts_grass;
	void SetupEdges();
	std::vector<Edge> edges;
	void AddEdgesToQuadTree(QuadTree *tree);

	//QuadTree *edgeTree;
	bool isBrushTest;
	bool Load(std::ifstream &is);
	
	void MakeInverse();
	static sf::Vector2i GetExtreme(TerrainPoint *p0,
		TerrainPoint *p1);
	Edge *GetEdge(int index);
	TerrainPoint *GetPoint(int index);
	TerrainPoint *GetInverseOuterRectPoint(int index);
	TerrainPoint *GetFinalizeInversePoint(int index);
	TerrainPoint *GetEndPoint();
	TerrainPoint *GetStartPoint();
	TerrainPoint *GetNextPoint(int index);
	TerrainPoint *GetPrevPoint(int index);
	void DeactivateGates();
	void ActivateGates();

	bool AABBIntersection(PolyPtr poly);
	bool IsOtherAABBWithinMine(PolyPtr poly);
	bool PointsTooCloseToEachOther();
	bool LinesIntersectMyself();
	bool HasSlivers();
	bool IntersectsMyOwnEnemies();
	bool IntersectsMyOwnGates();
	//TerrainRender *tr;
	int terrainVariation;
	TerrainWorldType terrainWorldType;
	int GetNumGrassTotal();
	int GetNumGrass(int i, bool &rem);
	void SetupGrass(int i, int &grassIndex);
	void SetupGrass();
	sf::Shader *pShader;
	bool IsValidInProgressPoint(sf::Vector2i point);
	void UpdateLines();
	TerrainPolygon();
	TerrainPolygon(TerrainPolygon &poly, bool pointsOnly,
		bool storeSelectedPoints = false );
	~TerrainPolygon();
	void UpdateLineColor(sf::Vertex *line, int i, int index);
	static sf::Vector2i TrimSliverPos(sf::Vector2<double> &prevPos,
		sf::Vector2<double> &pos, sf::Vector2<double> &nextPos,
		double minAngle, bool cw);
	void CopyPoints(PolyPtr poly,
		bool storeSelected = false );
	PolyPtr Copy();
	void MovePoint(sf::Vector2i &delta,
		TerrainPoint *tp);
	bool IsSpecialPoly();
	bool IsTouchingEnemiesFromPoly(PolyPtr p);

	//TerrainPoint *pointStart;
	//TerrainPoint *pointEnd;
	std::vector<std::vector<TerrainPoint>> pointVector;
	std::vector<TerrainPoint> &PointVector();
	void Reserve(int nPoints);

	TerrainPoint *GetSamePoint(sf::Vector2i &p);
	TerrainPoint * AddPoint(sf::Vector2i &p, bool sel);
	TerrainPoint * AddInverseBorderPoint(sf::Vector2i &p, bool sel);
	int GetNumPoints();
	void RemoveLastPoint();
	void ClearPoints();
	void SetMaterialType(
		int world, int variation);
	void UpdateMaterialType();
	void RemoveSlivers();
	

	bool TryFixAllSlivers();

	bool TryToMakeInternallyValid();
	bool FixSliver(int i);
	bool FixSliver(int i, std::set<int> &brokenSlivers,
		bool &error );

	int FixNearPrimary(int i,bool currLocked = false);

	//std::string material;
	int IsRemovePointsOkayEnemies(EditSession *edit);
	void Finalize();
	void FinalizeInverse();
	void Reset();
	void SoftReset();
	void Draw(bool showPath, double zoomMultiple, sf::RenderTarget * rt, bool showPoints, TerrainPoint *dontShow);
	void FixWinding();
	void FixWindingInverse();
	bool IsClockwise();
	bool AlignExtremes();
	bool AlignExtremes(
		std::vector<PointMoveInfo> &lockPoints);
	void UpdateGrass();

	void WriteFile(std::ofstream & of);
	void WriteGrass(std::ofstream &of);

	int grassSize;
	int grassSpacing;


	void ShowGrass(bool show);

	void SwitchGrass(sf::Vector2<double> mousePos);
	//bool ContainsPoint( sf::Vector2f p );
	void SetSelected(bool select);

	void UpdateBounds();

	void SetLayer(int p_layer);
	bool IsInternallyValid();
	sf::IntRect GetAABB();

	bool ContainsPoint(sf::Vector2f point);
	bool Intersects(sf::IntRect rect);
	bool IntersectsGate(GateInfo *gi);
	bool IsPlacementOkay();
	//void Move( sf::Vector2i delta );
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void Activate();
	void AddGatesToList(std::list<GateInfoPtr> &gates);
	bool IsTouching(PolyPtr poly);
	bool Contains(PolyPtr poly);

	int LinesIntersect(PolyPtr poly);
	void TryFixPointsTouchingLines();
	bool LinesIntersectInProgress(sf::Vector2i p );
	bool IsCompletionValid();
	void CopyPointsToClipperPath(ClipperLib::Path & p);
	void AddPointsFromClipperPath(ClipperLib::Path &p);
	void AddPointsFromClipperPath(ClipperLib::Path &p,
		ClipperIntPointSet &fusedPoints );
	void AddPointsFromClipperPath(ClipperLib::Path &p,
		ClipperLib::Path &clipperIntersections,
		std::list<TerrainPoint*> &intersections );
	bool PointTooCloseToPoints(sf::Vector2i point,
		int minDistance);
	bool PointTooClose(sf::Vector2i point,
		int minDistance, bool inProgress = false);
	bool PointTooCloseToLines(sf::Vector2i point,
		int minDistance, bool inProgress = false);
	bool PointsTooCloseToSegInProgress(sf::Vector2i point,
		int minDistance, bool finalPoint = false );
	bool SegmentWithinDistanceOfPoint(
		sf::Vector2i startSeg,
		sf::Vector2i endSeg,
		sf::Vector2i testPoint,
		int distance);
	void AddGatesToBrush(Brush *b,
		std::list<GateInfoPtr> &gateInfoList);
	void AddEnemiesToBrush(Brush *b);
	TerrainPoint *GetClosePoint(double radius, V2d &pos);
	bool IsCloseToFirstPoint(double radius, V2d &p);
	int GetNumSelectedPoints();
	PolyPtr CreateCopyWithSelectedPointsRemoved();
	sf::Color selectCol;
	sf::Color fillCol;


	bool CanApply();
	bool CanAdd();




	//bool movingPointMode;

	void Move(sf::Vector2i move);

	sf::Vertex *lines;
	sf::VertexArray *va;
	sf::VertexArray *grassVA;
	int numGrassTotal;
	int vaSize;
	//bool selected;
	int left;
	int right;
	int top;
	int bottom;
	std::list<sf::Vector2i> path;

	const static int inverseExtraBoxDist = 500;

	//enemymap
	std::map<TerrainPoint*, std::list<ActorPtr>> enemies;
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
	PolyPtr ground;
	TerrainRail *railGround;
	TerrainPoint *GetNextPoint();
	void AddActor(ActorPtr a);
	void RemoveActor(ActorPtr a);
	int GetEdgeIndex();
	V2d GetPosition();
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
	void Move(sf::Vector2i delta);
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	void Deactivate();
	void Activate();
	void SetSelected(bool select);
	void WriteFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	sf::Sprite spr;
	int layer;
	std::string decorName;
	int tile;
	std::list<DecorPtr> *myList;
	static bool CompareDecorInfoLayer(EditorDecorInfo &di0, EditorDecorInfo &di1);
};

typedef std::map<PolyPtr, std::list<PointMoveInfo>> PointMap;
typedef std::map<PolyPtr, std::vector<PointMoveInfo>> PointVectorMap;
typedef std::map<RailPtr, std::list<PointMoveInfo>> RailPointMap;

#endif