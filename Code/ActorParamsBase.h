#ifndef __ACTORPARAMSBASE_H__
#define __ACTORPARAMSBASE_H__

#include "VectorMath.h"
#include <SFML/Graphics.hpp>
#include "ISelectable.h"

struct TerrainPoint;
struct TerrainRail;
struct TerrainPolygon;
struct ActorGroup;
struct ActorType;

struct Edge;
struct Enemy;

struct PositionInfo
{
	PositionInfo();
	int edgeIndex;
	//TerrainPoint *edgeStart;
	double groundQuantity;
	PolyPtr ground;
	RailPtr railGround;
	Edge *GetEdge();
	TerrainPoint *GetPoint();
	TerrainPoint *GetNextPoint();
	bool IsAerial();
	void AddActor(ActorPtr a);
	void RemoveActor(ActorPtr a);
	void SetAerial( V2d &pos );
	void SetGround(PolyPtr p_ground, int p_edgeIndex, double quant);
	void SetRail(RailPtr p_rail, int p_edgeIndex, double quant);
	int GetEdgeIndex();
	V2d GetPosition();
	void SetPosition(V2d &pos);
	V2d position;
};

struct ActorParams : ISelectable
{
	Edge *GetGroundEdge();
	int GetWorld();

	Enemy *GenerateEnemy();
	Enemy *myEnemy;
	void CreateMyEnemy();

	virtual ActorParams *Copy() = 0;
	~ActorParams();
	ActorParams(ActorType *at);
	bool SetLevel(int lev);
	virtual void Init() {};
	virtual void WriteParamFile(std::ofstream &of);
	void WriteBasicParamFile(std::ofstream &of);
	void WriteFile(std::ofstream &of);
	void WriteMonitor(std::ofstream &of);
	void LoadBool(std::ifstream &is, bool &b);
	void WriteBool(std::ofstream &of, bool b);
	void WriteLevel(std::ofstream &of);
	void WritePath( std::ofstream &of );
	void WriteLoop( std::ofstream &of );
	void AnchorToGround(PolyPtr poly,
		int eIndex, double quantity);
	void AnchorToRail(TerrainRail *rail,
		int eIndex, double quantity);
	void AnchorToGround(PositionInfo &gi);
	void AnchorToRail(PositionInfo &gi);
	void UnAnchor();
	void UpdateGroundedSprite();

	virtual std::list<sf::Vector2i> GetGlobalPath();
	virtual void SetPath(std::list<sf::Vector2i> &globalPath);

	void DrawLevel(sf::RenderTarget *target);
	void DrawBoundary(sf::RenderTarget *target);
	void DrawMonitor(sf::RenderTarget *target);
	void LoadGrounded(std::ifstream &is);
	void LoadRailed(std::ifstream &is);
	void LoadAerial(std::ifstream &is);
	void LoadGlobalPath(std::ifstream &is );
	void LoadMonitor(std::ifstream &is);
	void LoadEnemyLevel(std::ifstream &is);
	virtual void SetParams();
	void SetBasicParams();
	void SetBasicPanelInfo();
	virtual void SetPanelInfo();
	
	void PlaceAerial(sf::Vector2i &pos);
	void PlaceGrounded( PolyPtr tp,
		int edgeIndex, double quant);
	void PlaceRailed(TerrainRail *rail,
		int edgeIndex, double quant);


	virtual void SetBoundingQuad();
	virtual void UpdateExtraVisuals()
	{}

	//ISelectable( ISelectableType type );
	virtual bool ContainsPoint(sf::Vector2f test);
	virtual bool Intersects(sf::IntRect rect);
	virtual bool IsPlacementOkay();
	virtual void Move(sf::Vector2i delta);
	virtual void BrushDraw(sf::RenderTarget *target,
		bool valid);
	virtual void Draw(sf::RenderTarget *target);
	virtual void DrawPreview(sf::RenderTarget *target);
	virtual void Deactivate();
	virtual void Activate();

	virtual void DrawQuad(sf::RenderTarget *target);

	virtual void SetSelected(bool select);

	virtual bool CanApply();
	bool CanAdd();

	V2d GetPosition();
	void SetPosition(V2d &pos);
	void SetPosition(sf::Vector2i &pos);
	void SetPosition(const sf::Vector2f &pos);
	sf::Vector2i GetIntPos();
	sf::Vector2f GetFloatPos();

	//sf::Sprite icon;
	sf::Sprite image;
	ActorGroup *group;
	ActorType *type;

	//if groundInfo is not null
	//then you can handle ground, even 
	//if you arent on it
	
	PositionInfo posInfo;
	bool hasMonitor;
	sf::VertexArray boundingQuad;

	std::list<sf::Vector2i> localPath;
	bool loop;
	sf::VertexArray *lines;

	int enemyLevel;
};

#endif