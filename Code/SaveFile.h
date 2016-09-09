#ifndef __SAVEFILE_H__
#define __SAVEFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>

struct SaveFile
{
	SaveFile( const std::string &name );
	//icon sprites
	void LoadFromFile();
	std::string fileName;
	std::string defaultFileName;

	sf::VertexArray iconVA;

	int currentWorld;
	bool levelsComplete[6 * 6];

	void SaveCurrent();
};

#endif