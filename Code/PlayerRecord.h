#ifndef __PLAYER_RECORD_H__
#define __PLAYER_RECORD_H__

#include <SFML\Graphics.hpp>
#include "Input.h"
#include <boost\filesystem.hpp>
#include <list>
#include <map>
#include "ItemSelector.h"
#include "UIWindow.h"
#include "PlayerSkinShader.h"
#include "BitField.h"

struct Actor;

//what is recorded each frame for the ghost
struct SprInfo
{
	sf::Vector2f position;
	sf::Vector2f origin;
	float rotation;
	bool flipX;
	bool flipY;

	int action;
	int tileIndex;

	int speedLevel;
};

struct GhostHeader
{
	enum GhostType
	{
		G_SINGLE_LEVEL_COMPLETE,
		G_SINGLE_FAILURE,
		G_SINGLE_RESTART_OR_QUIT,
		G_MULTI
	};
	

	GhostHeader();
	~GhostHeader();

	void Read(std::ifstream &is);
	void Write(std::ofstream &of);
	void SetVer(int v1, int v2);

	struct PlayerInfo
	{
		int skinIndex;
		void Write(std::ofstream &of);
		//might have other params here later
	};
	void Init(int numberOfPlayers);

	int ver1;
	int ver2;
	GhostType gType;
	int numberOfPlayers;
	PlayerInfo *playerInfo; //array 
};

//record the players sprite info
struct RecordGhost
{
	RecordGhost(Actor *p);
	int frame;
	void StartRecording();
	void StopRecording();
	
	int numTotalFrames;
	Actor *player;
	
	void WriteToFile(const std::string &fileName);
	void RecordFrame();
	const static int MAX_RECORD = 3600 * 60;
	SprInfo sprBuffer[MAX_RECORD]; //1 hour
	GhostHeader header;
};

//play back the players sprite info
struct ReplayGhost
{
	PlayerSkinShader playerSkinShader;
	sf::CircleShape cs;
	bool init;
	int action;
	sf::Sprite replaySprite;
	SprInfo *sprBuffer;
	Actor *player;
	int frame;
	int numTotalFrames;
	GhostHeader header;

	ReplayGhost(Actor *p);
	~ReplayGhost();
	void Reset();
	bool OpenGhost(const boost::filesystem::path &fileName);
	void UpdateReplaySprite();
	void Draw(sf::RenderTarget *target);
};

struct RecordGhost;
struct ReplayGhost;

struct ReplayHeader
{
	int ver1;
	int ver2;
	int startPowerMode;
	BitField bUpgradeField;


	ReplayHeader();
	void Read(std::ifstream &is);
	void Write(std::ofstream &of);
	void SetVer(int v1, int v2);
};

//record the player's inputs to create a full replay of a map
struct RecordPlayer
{
	RecordPlayer(Actor *p);
	int frame;
	ReplayHeader header;
	
	void StartRecording();
	void StopRecording();
	int numTotalFrames;
	Actor *player;
	void WriteToFile(const std::string &fileName);
	void RecordFrame();
	const static int MAX_RECORD = 3600 * 60;
	int inputBuffer[MAX_RECORD];
};


//replay the player's inputs to set up a replay playback
struct ReplayPlayer
{
	ReplayHeader header;
	bool init;
	ReplayPlayer(Actor *p);
	~ReplayPlayer();
	bool OpenReplay(const std::string &fileName);
	void Reset();
	void UpdateInput(ControllerDualStateQueue *controllerInput);
	int *inputBuffer;
	Actor *player;
	int frame;
	int numTotalFrames;
};

struct RecordGhostManager
{

};

struct MainMenu;

struct GhostInfo
{

};



struct GhostFolder;
struct GhostEntry
{
	GhostEntry(boost::filesystem::path &p_path,
		GhostHeader *gh)
		:gPath(p_path), headerInfo(gh), activeForMap( false )
	{
	}
	~GhostEntry() { if (headerInfo != NULL) delete headerInfo; }

	boost::filesystem::path gPath;
	GhostFolder *folder;
	GhostHeader *headerInfo;
	bool activeForMap;
};

struct GhostFolder
{
	GhostFolder( const boost::filesystem::path &p )
		:expanded( false ), folderPath( p ),
		autoExpanded( false ), saveExpanded( false ),
		autoLoaded( false ), saveLoaded( false )
	{
	}
	~GhostFolder();

	//std::string folderName;
	boost::filesystem::path folderPath;
	//std::list<GhostEntry*> ghosts;
	bool autoExpanded;
	bool saveExpanded;
	void DestroyAutoGhosts();
	void DestroySaveGhosts();
	std::list<GhostEntry*> autoGhosts;
	std::list<GhostEntry*> saveGhosts;
	bool expanded;
	int folderLevel; //0 for map 
	bool autoLoaded;
	bool saveLoaded;
	bool IsSaveActive();
	bool IsAutoActive();
	bool IsActive() { return IsSaveActive() || IsAutoActive(); }
};

struct GhostIndexInfo
{
	GhostFolder*folder;
	GhostEntry *entry;
	bool onAuto;
	bool onSave;
	std::string GetName();
};

struct RecordGhostMenu
{
	static const int NUM_BOXES = 24;
	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;

	RecordGhostMenu(MainMenu *p_mainMenu,
		sf::Vector2f &p_pos);
	~RecordGhostMenu();
	MainMenu *mainMenu;
	void SetupBoxes();
	void LoadItems();
	void Update(ControllerState &currInput, ControllerState &prevInput);

	static GhostHeader * ReadGhostHeader(std::ifstream &is);
	static bool WriteGhostHeader(std::ofstream &of, GhostHeader *mh);
	static bool ReplaceGhostHeader(boost::filesystem::path &p,
		GhostHeader *mh);

	void SetupInds( int &startInd, GhostFolder *gf );
	void UpdateLoadedFolders();
	void MoveUp();
	void MoveDown();
	void UpdateItemText();
	void UpdateBoxesDebug();
	void Draw(sf::RenderTarget *target);
	GhostIndexInfo GetIndexInfo(int index);

	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text itemName[NUM_BOXES];
	//std::pair<std::string, GhostIndexInfo> *allItems;
	sf::Vector2f topMid;
	//std::list<GhostFolder*> folders;
	//GhostFolder *autoFolder;
	//GhostFolder *saveFolder;

	int oldCurrIndex;
	int topIndex;

	std::list<boost::filesystem::path> items;
	void LoadPath(boost::filesystem::path & p);
	sf::Font &font;
	SingleAxisSelector *saSelector;
	std::map<std::string, GhostFolder*> ghostFolders;
	std::list<GhostFolder*> sortedFolders;
	void GetActiveList(std::list<GhostEntry*> &entries);

	int numTotalItems;

	UIVerticalControlList *ghostOptions;

	void LoadFolders();
	void LoadMapGhosts( bool save, const boost::filesystem::path &p);

	//list<
	void LoadDir(boost::filesystem::path &p, std::list<GhostEntry*> &entries);
};

#endif