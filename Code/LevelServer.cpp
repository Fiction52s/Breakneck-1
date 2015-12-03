#include "LevelServer.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace sf;

LevelServer::LevelServer()
{
	http.setHost("http://www.breakneckgame.com/");
}

bool LevelServer::DownloadFile( const std::string &path, const std::string &fname)
{
	sf::Http::Request req(path + fname);
    sf::Http::Response file_res = http.sendRequest(req);

	if (file_res.getStatus() == sf::Http::Response::Ok)
    {
		std::string body = file_res.getBody();
		std::ofstream file;
		std::string localFile = "Maps/Downloads/";
		localFile.append( fname );

		file.open(localFile);
		file.write(body.c_str(), body.size());
		file.close();
		return true;
    }
    else
    {
		std::cout << "request failed: " << file_res.getStatus() << std::endl;
		return false;
    }
}