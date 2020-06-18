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
struct ParamsInfo;

template<typename X> ActorParams *MakeParams(
	ActorType *, int );
template<typename X> ActorParams *LoadParams(
	ActorType *, std::ifstream &isa);
template<typename X> Enemy* CreateEnemy(ActorParams*);
template <typename X> void SetParamsType(ParamsInfo*);

typedef Enemy * EnemyCreator(ActorParams*);
typedef ActorParams* ParamsMaker(ActorType*, int);
typedef ActorParams* ParamsLoader(ActorType*,
	std::ifstream &is);
typedef void ParamsCreator(ParamsInfo*);
typedef bool PositionChecker();

struct ParamsInfo
{
	ParamsInfo(const std::string &n,
		EnemyCreator *p_enemyCreator,
		ParamsCreator *p_paramsCreator,
		sf::Vector2i &off, sf::Vector2i &p_size,
		bool w_monitor, bool w_level,
		bool w_path, bool w_loop,
		bool p_canBeAerial, bool p_canBeGrounded,
		bool p_canBeRailGrounded, int p_numLevels = 1,
		int p_world = -1,
		Tileset *p_ts = NULL, int imageTile = 0)
		:name(n), 
		enemyCreator( p_enemyCreator ),
		offset(off), 
		size(p_size),
		ts(p_ts), 
		imageTileIndex(imageTile),
		writeMonitor(w_monitor), 
		writeLevel(w_level),
		writePath(w_path), 
		writeLoop(w_loop),
		numLevels(p_numLevels),
		world( p_world ),
		canBeAerial( p_canBeAerial),
		canBeGrounded( p_canBeGrounded ),
		canBeRailGrounded( p_canBeRailGrounded)

	{
		pLoader = NULL;
		pMaker = NULL;
		if( p_paramsCreator != NULL )
			p_paramsCreator(this);
	}

	
	std::string name;

	EnemyCreator *enemyCreator;
	ParamsLoader *pLoader;
	ParamsMaker* pMaker;
	

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

	bool canBeAerial;
	bool canBeGrounded;
	bool canBeRailGrounded;
};

typedef ActorParams* ActorPtr;
typedef std::map<TerrainPoint*, std::list<ActorPtr>> EnemyMap;



struct ActorType
{
	ActorType(ParamsInfo &pi);
	~ActorType();
	
	void CreateDefaultEnemy();
	void LoadEnemy(std::ifstream &is,
		ActorPtr &a);
	Panel * CreatePanel();
	Panel *CreateDefaultPanel();
	bool IsGoalType();
	sf::Sprite GetSprite(int xSize = 0, int ySize = 0);
	sf::Sprite GetSprite(bool grounded);
	bool CanBeGrounded();
	bool CanBeAerial();
	bool CanBeRailGrounded();
	bool LoadSpecialTypeOptions();
	int GetSpecialOptionsIndex(const std::string & n);
	const std::string &GetSelectedSpecialDropStr();
	const std::string &GetSpecialDropStr(int ind);
	int GetSelectedSpecialDropIndex();
	void SetSpecialDropIndex(int i);
	void AddSpecialOptionDropdown( Panel * p);
	void AddSetDirectionButton(Panel *p);
	void AddLabeledSlider( Panel *p, 
		const std::string &name,
		const std::string &label, int minValue,
		int maxValue, int defaultValue);

	Panel *panel;

	std::vector<ActorPtr> defaultParamsVec;
	std::vector<std::string> specialTypeOptions;

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