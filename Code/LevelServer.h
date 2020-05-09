#ifndef __LEVELSERVER_H__
#define __LEVELSERVER_H__
#include <SFML/Network.hpp>

struct LevelServer
{
	LevelServer();
	bool DownloadFile( const std::string &path,
		const std::string &file );
	void PrintMaps();
	sf::Http http;
};

#endif