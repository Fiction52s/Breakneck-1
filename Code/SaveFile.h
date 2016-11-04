#ifndef __SAVEFILE_H__
#define __SAVEFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>

struct Level
{
	Level();
	bool completed;
	bool shards[3];
};


struct SaveFile
{
	SaveFile( const std::string &name );
	~SaveFile();
	void SaveCurrent();
	void LoadFromFile();
	std::string fileName;

	sf::VertexArray iconVA;

	int currentWorld;
	Level* worlds[6];
	//Level levels[6][6];
	//bool levelsComplete[6 * 6];

	
};

#endif