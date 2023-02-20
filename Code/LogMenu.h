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

	LogDetailedInfo();
	void Reset();
};

struct LogPreview
{
	LogDetailedInfo logInfo;
	TilesetManager tMan;
	SoundManager sMan;
	Session *sess;
	sf::Vector2f center;
	sf::Vector2f terrainCenter;

	//LT_ENEMY
	ActorParams *previewParams;

	//LT_WATER/LT_TERRAIN
	float waterShaderCounter;
	TerrainPolygon *previewPoly;

	//LT_RAIL
	Tileset *ts_rail;
	TerrainRail *previewRail;

	//LT_SKIN
	Tileset *ts_kin;
	Tileset *ts_kinFace;

	sf::Sprite kinSprite;
	sf::Sprite kinFaceSprite;

	PlayerSkinShader pSkinShader;
	PlayerSkinShader pFaceSkinShader;

	//LT_GRASS
	Tileset *ts_grass;
	sf::Sprite grassSprite;

	//LT_MUSIC
	MusicInfo *currLogMusic;

	LogPreview();
	~LogPreview();
	void Clear();
	void PlayMusic();
	void StopMusic();
	void Update();
	void SetCenter(sf::Vector2f &pos);
	void SetInfo(LogDetailedInfo &li);
	void Draw(sf::RenderTarget *target);

private:
	void ClearActorParams();
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

	LogPreview logPreview;
	sf::Vector2f topLeft;
	sf::Text worldText;
	int currSelectMode;
	int currLogType;
	State state;
	int selectedIndex;
	int totalFrame;
	sf::Text currLogText;
	sf::Text currLogNameText;
	LogDetailedInfo **logInfo;
	Tileset *ts_logs;
	Session *sess;
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	SingleAxisSelector *worldSelector;
	sf::Vertex previewQuad[4];
	sf::Sprite previewSpr;
	
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
	void SetTopLeft(sf::Vector2f &pos);
	MusicInfo *GetLogMusic(const std::string &str);
	void SetCurrMusic();
	void LoadLogInfo();
	void SetupLogImages();
	std::string GetLogDesc(int w, int li);
	std::string GetLogName(int w, int li);
	LogDetailedInfo GetLogInfo(int w, int li);
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