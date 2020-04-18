#ifndef __EDITORACTORS_H__
#define __EDITORACTORS_H__

#include <fstream>
#include <SFML/Graphics.hpp>
#include <list>
//#include "ActorParams.h"

struct ActorType;
struct ActorParams;
struct Tileset;
struct TerrainPoint;
struct Panel;

struct Enemy;

template<typename X> ActorParams *MakeParamsGrounded(
	ActorType *);
template<typename X> ActorParams *MakeParamsRailed(
	ActorType *);
template<typename X> ActorParams *MakeParamsAerial(
	ActorType *);
template<typename X> ActorParams *LoadParams(
	ActorType *, std::ifstream &isa);
template<typename X> Enemy* CreateEnemy(ActorParams*);


typedef Enemy * EnemyCreator(ActorParams*);
typedef ActorParams* ParamsMaker(ActorType*);
typedef ActorParams* ParamsLoader(ActorType*,
	std::ifstream &is);

struct ParamsInfo
{
	ParamsInfo(const std::string &n,
		EnemyCreator *p_enemyCreator,
		ParamsLoader *p_pLoader,
		ParamsMaker *pg, ParamsMaker *pa,
		sf::Vector2i &off, sf::Vector2i &p_size,
		bool w_monitor, bool w_level,
		bool w_path, bool w_loop, int p_numLevels = 1,
		Tileset *p_ts = NULL, int imageTile = 0,
		int p_world = -1)
		:name(n), 
		enemyCreator( p_enemyCreator ),
		pLoader(p_pLoader),
		pmGround(pg), pmAir(pa),
		offset(off), size(p_size),
		ts(p_ts), imageTileIndex(imageTile),
		writeMonitor(w_monitor), writeLevel(w_level),
		writePath(w_path), writeLoop(w_loop),
		numLevels(p_numLevels),
		pmRail(NULL), world( p_world )

	{

	}
	EnemyCreator *enemyCreator;
	//void SetRailLoader(ParamsM)
	std::string name;
	ParamsLoader *pLoader;
	ParamsMaker* pmGround;
	ParamsMaker* pmAir;
	ParamsMaker* pmRail;
	sf::Vector2i offset;
	sf::Vector2i size;
	Tileset *ts;
	int imageTileIndex;

	bool writeMonitor;
	bool writeLevel;
	bool writePath;
	bool writeLoop;

	int numLevels;
	int world;
};

typedef ActorParams* ActorPtr;
typedef std::map<TerrainPoint*, std::list<ActorPtr>> EnemyMap;

struct ActorType
{
	ActorType(ParamsInfo &pi);
	~ActorType();
	void PlaceEnemy();
	void PlaceEnemy(ActorParams *ap);
	void LoadEnemy(std::ifstream &is,
		ActorPtr &a);
	Panel * CreatePanel();
	Panel *CreateDefaultPanel(const std::string &n,
		bool mon,
		bool level,
		bool path = false,
		bool loop = false);
	bool IsGoalType();
	sf::Sprite GetSprite(int xSize = 0, int ySize = 0);
	sf::Sprite GetSprite(bool grounded);
	bool CanBeGrounded();
	bool CanBeAerial();
	bool CanBeRailGrounded();

	Panel *panel;

	ParamsInfo info;
};

//no params for goal and foottrap atm
struct ActorGroup
{
	ActorGroup(const std::string &name);
	~ActorGroup();
	std::string name;
	std::list<ActorPtr> actors;
	void Draw(sf::RenderTarget *target);
	void WriteFile(std::ofstream &of);
	void DrawPreview(sf::RenderTarget *target);
};



#endif