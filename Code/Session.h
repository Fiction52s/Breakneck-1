#ifndef __SESSION_H__
#define __SESSION_H__

#include "Input.h"
#include "Tileset.h"
#include "Physics.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include "ISelectable.h"

struct Actor;
struct MainMenu;
struct QuadTree;
struct MapHeader;

struct Session : TilesetManager
{
	Session(const boost::filesystem::path &p_filePath);
	virtual ~Session();
	Actor *GetPlayer(int i);
	ControllerState &GetPrevInput(int index);
	ControllerState &GetCurrInput(int index);
	ControllerState &GetPrevInputUnfiltered(int index);
	ControllerState &GetCurrInputUnfiltered(int index);
	GameController &GetController(int index);
	void UpdatePlayerInput(int index);
	void UpdateAllPlayersInput();
	void UpdateControllers();
	bool IsKeyPressed(int k);
	bool IsMousePressed(int m);
	bool ReadFile();
	bool ReadDecorImagesFile();
	virtual void ProcessDecorFromFile(const std::string &name,
		int tile) {}
	bool ReadHeader( std::ifstream &is );
	virtual void ProcessHeader() {}

	bool ReadDecor(std::ifstream &is);
	virtual void ProcessDecorSpr( const std::string &name, 
		sf::Sprite &dSpr, int dLayer, Tileset *d_ts, int dTile) {}

	bool ReadPlayerStartPos(std::ifstream &is);
	virtual void ProcessPlayerStartPos() {}

	bool ReadTerrain(std::ifstream &is);
	bool ReadTerrainGrass(std::ifstream &is, PolyPtr poly );
	virtual void ProcessTerrain( PolyPtr poly ){}

	bool ReadSpecialTerrain(std::ifstream &is);
	virtual void ProcessSpecialTerrain(PolyPtr poly) {}

	bool ReadBGTerrain(std::ifstream &is);
	virtual void ProcessBGTerrain(PolyPtr poly) {}

	bool ReadRails(std::ifstream &is);
	virtual void ProcessRail(RailPtr rail) {}

	virtual bool ReadActors(std::ifstream &is) = 0;

	bool ReadGates(std::ifstream &is);
	virtual void ProcessGate(int gType,
		int poly0Index, int vertexIndex0, int poly1Index,
		int vertexIndex1, int shardWorld,
		int shardIndex ) {}
		
	//virtual void ProcessActor( const std::string &groupName, int numActors,
	//	const std::string &actorType ) {}
//	virtual void ProcessActor(ActorPtr actor);
//	virtual void ProcessActorGroup(const std::string &groupName, int numActors);
	

	virtual int Run() = 0;

	bool cutPlayerInput;
	MainMenu *mainMenu;

	std::vector<GCC::GCController> gcControllers;

	std::vector<Actor*> players;

	const static int MAX_PLAYERS = 4;

	QuadTree * terrainTree;
	QuadTree *specialTerrainTree;

	int substep;
	double currentTime;
	double accumulator;
	sf::Clock gameClock;
	Collider collider;
	int totalGameFrames;

	boost::filesystem::path filePath;
	std::string filePathStr;

	MapHeader *mapHeader;

	std::map<std::string, Tileset*> decorTSMap;

	sf::RenderWindow *window;

	sf::Vector2i playerOrigPos;
};

#endif