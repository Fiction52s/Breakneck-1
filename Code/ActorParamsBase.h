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

struct GroundInfo
{
	GroundInfo();
	TerrainPoint *edgeStart;
	double groundQuantity;
	PolyPtr ground;
	RailPtr railGround;
	TerrainPoint *GetNextPoint();
	void AddActor(ActorPtr a);
	void RemoveActor(ActorPtr a);
	int GetEdgeIndex();
	V2d GetPosition();
};

struct ActorParams : ISelectable
{
	Edge *GetGroundEdge();
	int GetWorld();

	Enemy *GenerateEnemy();

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
	void AnchorToGround(GroundInfo &gi);
	void AnchorToRail(GroundInfo &gi);
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

	//sf::Sprite icon;
	sf::Sprite image;
	ActorGroup *group;
	ActorType *type;
	sf::Vector2i position;

	//if groundInfo is not null
	//then you can handle ground, even 
	//if you arent on it
	
	GroundInfo *groundInfo;
	bool hasMonitor;
	sf::VertexArray boundingQuad;

	std::list<sf::Vector2i> localPath;
	bool loop;
	sf::VertexArray *lines;

	int enemyLevel;
};

#endif