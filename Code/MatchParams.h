#ifndef __MATCH_PARAMS_H__
#define __MATCH_PARAMS_H__

#include <boost/filesystem.hpp>
#include "steam\steam_api.h"

struct ConnectionManager;
struct SaveFile;

struct MatchParams
{
	MatchParams();
	boost::filesystem::path filePath;
	SaveFile *saveFile;
	ConnectionManager *connectionManager;
	int numPlayers;

};

#endif