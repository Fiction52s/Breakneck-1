#ifndef __LOGMENU_H__
#define __LOGMENU_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "Tileset.h"
#include "Input.h"
#include <SFML\Audio.hpp>
#include <map>
#include <string>
#include <boost/thread.hpp>
#include "SoundManager.h"

struct Session;
struct EffectPool;
struct MusicInfo;
struct SingleAxisSelector;
struct ActorParams;
struct TerrainPolygon;

struct LogDetailedInfo
{
	enum LogType
	{
		MOMENTA,
		SKIN,
		ENEMY,
		WATER,
		RAIL,
		MUSIC,
		LORE,
		Count
	};

	LogDetailedInfo()
		:logType(0),ts_preview( NULL ),
		waterIndex(-1)
	{

	}

	std::string name;
	std::string desc;
	int logType;
	std::string enemyTypeName;
	int waterIndex;
	Tileset *ts_preview;
};

struct LogMenu
{
	enum State
	{
		WAIT,
		PREVIEWING_MUSIC,
	};


	TerrainPolygon *currWaterPoly;
	State state;
	EffectPool *sparklePool;
	int selectedIndex;
	int totalFrame;
	sf::Text currLogText;
	sf::Text currLogNameText;
	LogDetailedInfo **logInfo;
	Tileset *ts_logs;
	sf::Sprite selectedLogHighlight;
	Session *sess;
	sf::Vector2f imagePos;
	TilesetManager tMan;
	SoundManager sMan;
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	Tileset **ts_preview;
	sf::Vertex previewQuad[4];
	sf::Sprite previewSpr;
	ActorParams *testParams;
	MusicInfo *currLogMusic;
	sf::Vertex selectedBGQuad[4];
	sf::Vertex *logSelectQuads;
	sf::Vertex logBGQuad[4];
	sf::Vertex containerBGQuad[4];
	sf::Vertex descriptionBGQuad[4];
	sf::Vertex controlsQuadBGQuad[4];
	sf::Vertex shardTitleBGQuad[4];
	sf::Vertex largeShardContainer[4];
	sf::Vertex largeShard[4];
	sf::Vertex shardButtons[4 * 4];
	Tileset *ts_shardButtons;
	Tileset *ts_shardContainer;
	Tileset *ts_sparkle;
	Tileset *ts_notCapturedPreview;
	Tileset *ts_noPreview;
	
	LogMenu(Session *sess);
	~LogMenu();
	void Update(ControllerState &currInput,
		ControllerState &prevInput);
	void Draw(sf::RenderTarget *target);
	void SetCurrLog();
	MusicInfo *GetLogMusic(const std::string &str);
	void SetCurrMusic();
	void StopMusic();
	void LoadLogInfo();
	void SetupLogImages();
	std::string GetLogDesc(int w, int li);
	std::string GetLogName(int w, int li);
	bool SetDescription(std::string &nameStr, std::string &destStr, const std::string &shardTypeStr);
	void SetCurrentDescription(bool captured);
	void UpdateUnlockedLogs();
	bool IsLogFound(int x, int y);
	bool IsCurrLogFound();
	void UpdateLogSelectQuads();
	void SetLogTab();
};

#endif