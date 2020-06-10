#ifndef __FILECHOOSER_H__
#define __FILECHOOSER_H__

#include "GUI.h"
#include <boost/filesystem.hpp>
#include "Tileset.h"


struct FolderNode;
struct EditSession;

struct FileNode
{
	enum Type
	{
		FILE,
		FOLDER,
	};

	Type type;
	void Draw(sf::RenderTarget *target);
	boost::filesystem::path filePath;
	Tileset *ts_preview;
};

struct FolderNode
{
	~FolderNode();
	void AddFile(
		const boost::filesystem::path &filePath);
	void AddFolder(FolderNode *fn);
	void Draw(sf::RenderTarget *target);

	std::list<FileNode*> fileNodes;
	std::list<FolderNode*> childFolders;
	FolderNode *parentFolder;
	std::string GetRelPath();
	std::string folderName;
	void DebugPrint( int indent );
};

struct FolderTree
{
	FolderTree(const std::string &p_path,
		const std::string &ext );
	~FolderTree();
	void SetupEntry( FolderNode *fn, const std::string &relPath );
	void SetupTree();
	void DebugPrintTree();

	FolderNode *treeRoot;
	std::string ext;
	std::string treePath;
};

//struct EnemyVariationSelector : PanelUpdater
//{
//	EnemyVariationSelector(bool createMode);
//	~EnemyVariationSelector();
//	bool MouseUpdate();
//	EnemyChooseRect *centerRect;
//	EnemyChooseRect *varRects[6];
//	void Deactivate();
//	int numVariations;
//	void SetType(ActorType *type);
//	void Draw(sf::RenderTarget *target);
//	void SetPosition(sf::Vector2f &pos);
//
//	sf::Sprite orbSpr;
//	//sf::Vertex testQuad[4];
//	sf::Vertex enemyQuads[28];
//	//bool show;
//	Panel *panel;
//	EditSession *edit;
//	bool createMode;
//};

struct FileChooser : GUIHandler, TilesetManager,
	PanelUpdater
{
	enum Mode : int
	{
		OPEN,
		SAVE
	};

	Mode fMode;

	FileChooser();
	~FileChooser();

	//panelupdater functions
	bool MouseUpdate();
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void MouseScroll(int delta);
	//---------

	//guihandler functions
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	//---------

	void SetRelativePath(const std::string &p_relPath);
	void SetPath(const std::string &p_path);
	void AddFile(const boost::filesystem::path &filePath);
	void AddFolder(const boost::filesystem::path &folderPath);
	void ClearNodes();
	void Print();
	void PopulateRects();
	void Start( const std::string &ext, 
		Mode fMode, const std::string &path );
	void StartRelative(const std::string &ext,
		Mode fMode, const std::string &path);
	void Init();
	void TurnOff();

	Button *upButton;
	sf::Text *folderPathText;
	
	int topRow;
	int maxTopRow;
	int numEntries;

	boost::filesystem::path currPath;
	ImageChooseRect **imageRects;
	int cols;
	int rows;
	int totalRects;
	std::vector<FileNode*> nodes;
	std::string ext;
	//boost::filesystem::path basePath;
	sf::Vertex largePreview[4];
	Tileset *ts_largePreview;
	Panel *panel;
	TextBox *fileNameTextBox;
	EditSession *edit;
};

#endif