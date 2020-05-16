#ifndef __LEVEL_SELECTOR_H__
#define __LEVEL_SELECTOR_H__

#include <boost/filesystem.hpp>
#include <list>
#include <string>
#include <map>
#include <SFML/Graphics.hpp>
#include <vector>

struct CustomMapClient;

struct TreeNode
{
	TreeNode();
	TreeNode *parent;
	TreeNode *next;
	std::list<boost::filesystem::path> files;
	std::list<TreeNode*> dirs;
	std::string name;
	boost::filesystem::path filePath;
	int GetLevel();
	std::string GetLocalPath();
	bool expanded;
};

struct LevelHolder
{
	TreeNode **dirNode;
};

struct MainMenu;
struct Tileset;

struct LevelDirectory
{
	//std::string * localPaths;
	std::vector<std::string> localPaths;
};

struct LevelSelector
{
	void LoadAndRewriteAllMaps();
	void BackupAllMapsAndPreviews();
	LevelSelector( MainMenu *mainMenu );
	~LevelSelector();
	void UpdateMapList(TreeNode *parentNode, const std::string &relativePath);
	void UpdateMapList();
	void PrintDir( TreeNode * dir );
	void ClearEntries();
	void ClearEntries(TreeNode *n);
	void SetLocalPath(int index, TreeNode *entry);
	void Draw( sf::RenderTarget *target );
	void MouseUpdate( sf::Vector2f mousePos );
	void LeftClick( bool click, sf::Vector2f mousePos );
	const std::string &GetSelectedPath();
	void GetPreview(const std::string &pathName,
		const std::string &mName,
		bool update );
	void UpdateSelectedPreview();
	TreeNode *GetEntryByName( const std::string &dirName );
	//std::list<std::string> maps;
	TreeNode *entries;
	void Print();
	int numTotalEntries;
	std::map<std::string, TreeNode*> entryMap;
	int Tex(int index, int level, TreeNode *entry);
	sf::Font font;
	sf::Text * text;
	std::string * localPaths;
	std::string * fullPaths;
	TreeNode **dirNode;
	int mouseOverIndex;
	int selectedIndex;
	sf::Vector2f position;
	bool mouseDown;
	//sf::Vector2f mouseDownPos;
	int mouseDownIndex;
	int width;
	int height;
	int fontHeight;
	int xspacing;
	int yspacing;
	//sf::Vector2f windowStretch;
	std::string newLevelName;
	sf::RenderTexture drawPanel;
	int viewOffset;
	void ChangeViewOffset( int delta );
	sf::Sprite previewSpr;
	std::map<std::string, Tileset*> previewTS;
	Tileset *ts_previewNotFound;
	MainMenu *mainMenu;
	
	//used for loading and resaving all files
	std::list<std::string> allMapPaths;

	CustomMapClient *customMapClient;
	
};

void copyDirectoryRecursively(
	const boost::filesystem::path& sourceDir, 
	const boost::filesystem::path& destinationDir);
#endif