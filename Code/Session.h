#ifndef __SESSION_H__
#define __SESSION_H__

#include "Input.h"
#include "Tileset.h"
#include "Physics.h"
#include <boost/filesystem.hpp>
#include <fstream>

struct Actor;
struct MainMenu;
struct QuadTree;
struct MapHeader;

struct Session : TilesetManager
{
	Session(const boost::filesystem::path &p_filePath);
	virtual Actor *GetPlayer(int i);
	ControllerState &GetPrevInput(int index);
	ControllerState &GetCurrInput(int index);
	ControllerState &GetPrevInputUnfiltered(int index);
	ControllerState &GetCurrInputUnfiltered(int index);
	GameController &GetController(int index);
	void UpdatePlayerInput(int index);
	void UpdateControllers();
	bool IsKeyPressed(int k);
	bool IsMousePressed(int m);
	bool ReadFile();
	bool ReadDecorImagesFile();

	bool ReadHeader( std::ifstream &is );
	virtual void ProcessHeader() {}
	bool ReadDecor(std::ifstream &is);
	virtual void ProcessDecorSpr(sf::Sprite &dSpr,
		int dLayer, Tileset *d_ts, int dTile) {}
	bool ReadPlayer(std::ifstream &is);
	bool ReadTerrain(std::ifstream &is);
	bool ReadSpecialTerrain(std::ifstream &is);
	bool ReadBGTerrain(std::ifstream &is);
	bool ReadRails(std::ifstream &is);
	bool ReadActors(std::ifstream &is);
	bool ReadGates(std::ifstream &is);

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
	int totalSimulationFrames;

	boost::filesystem::path filePath;
	std::string filePathStr;

	MapHeader *mapHeader;

	std::map<std::string, Tileset*> decorTSMap;
};

#endif