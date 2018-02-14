#ifndef __SAVEFILE_H__
#define __SAVEFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>

struct Sector;
struct World;
struct Level
{
	Level();
	Sector *sec;
	bool completed;
	unsigned int optionField;
	std::string name;
	bool Load(std::ifstream &is);
	void SetOption(int index, bool value);
	void SetCompleted( bool comp );
	void Reset();
	int index;
	std::string GetFullName();
};

struct Sector
{
	Sector();
	~Sector();
	int numLevels;
	Level *levels;
	int index;
	World *world;
	bool Load(std::ifstream &is);
};

struct World
{
	World();
	~World();
	int numSectors;
	Sector *sectors;
	int index;
	bool Load(std::ifstream &is);
};

struct SaveFile
{
	SaveFile( const std::string &name );
	~SaveFile();
	void SaveCurrent();
	void LoadFromFile();
	std::string fileName;
	const static int NUM_WORLDS = 8;
	World worlds[NUM_WORLDS];	
};

#endif