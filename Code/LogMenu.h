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
#include "PlayerSkinShader.h"

struct Session;
struct EffectPool;
struct MusicInfo;
struct SingleAxisSelector;
struct ActorParams;
struct TerrainPolygon;
struct TerrainRail;

struct LogDetailedInfo
{
	enum LogType
	{
		LT_MOMENTA,
		LT_SKIN,
		LT_ENEMY,
		LT_WATER,
		LT_RAIL,
		LT_GRASS,
		LT_SPECIALTERRAIN,
		LT_MUSIC,
		LT_LORE,
		Count
	};

	LogDetailedInfo()
		:logType(-1), ts_preview(NULL),
		waterIndex(-1),
		railIndex(-1),
		skinIndex(-1),
		grassTypeIndex(-1),
		specialTerrainWorld(-1),
		specialTerrainVariation(-1)
	{

	}

	std::string name;
	std::string desc;
	int logType;
	std::string enemyTypeName;
	int waterIndex;
	int railIndex;
	int skinIndex;

	int grassTypeIndex;
	int specialTerrainWorld;
	int specialTerrainVariation;
	Tileset *ts_preview;
};

struct LogMenu
{
	enum State
	{
		WAIT,
		PREVIEWING_MUSIC,
	};

	enum SelectMode
	{
		SM_WORLD,
		SM_LOG,
	};

	sf::Text worldText;
	int currSelectMode;
	Tileset *ts_grass;
	sf::Sprite grassSprite;
	PlayerSkinShader pSkinShader;
	PlayerSkinShader pFaceSkinShader;
	Tileset *ts_kinFace;
	sf::Sprite kinFaceSprite;
	Tileset *ts_kin;
	sf::Sprite kinSprite;
	int currLogType;
	float waterShaderCounter;
	TerrainPolygon *previewPoly;
	TerrainRail *previewRail;
	State state;
	int selectedIndex;
	int totalFrame;
	sf::Text currLogText;
	sf::Text currLogNameText;
	LogDetailedInfo **logInfo;
	Tileset *ts_logs;
	Tileset *ts_rail;
	Session *sess;
	TilesetManager tMan;
	SoundManager sMan;
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	SingleAxisSelector *worldSelector;
	sf::Vertex previewQuad[4];
	sf::Sprite previewSpr;
	ActorParams *previewParams;
	MusicInfo *currLogMusic;
	sf::Vertex selectedBGQuad[4];
	sf::Vertex *logSelectQuads;
	sf::Vertex logBGQuad[4];
	sf::Vertex containerBGQuad[4];
	sf::Vertex descriptionBGQuad[4];
	sf::Vertex controlsQuadBGQuad[4];
	sf::Vertex shardTitleBGQuad[4];
	sf::Vertex shardButtons[4 * 4];
	Tileset *ts_shardButtons;
	Tileset *ts_notCapturedPreview;
	Tileset *ts_noPreview;
	sf::Vector2f previewCenter;
	int numWorlds;
	
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
	bool IsLogFound(int w, int li);
	bool IsCurrLogFound();
	void UpdateLogSelectQuads();
	void UpdateLogsOnWorldChange();
	void SetLogTab();
	void SetWorldMode();
	int GetLogTile(int w, int li);
};

#endif