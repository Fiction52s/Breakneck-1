#ifndef __FILECHOOSER_H__
#define __FILECHOOSER_H__

#include "GUI.h"
#include <boost/filesystem.hpp>
#include "Tileset.h"


struct FolderNode;
struct EditSession;
struct FileChooser;

struct FileNode
{
	enum Type
	{
		FILE,
		FOLDER,
	};

	Type type;
	boost::filesystem::path filePath;
	Tileset *ts_preview;
	int index;

	FileNode() { ts_preview = NULL; type = FILE; index = -1; }
	
	void Draw(sf::RenderTarget *target);
	
};

struct FileChooserHandler : GUIHandler
{
	FileChooserHandler( int cols, int rows, int extraImageRects = 0 );
	virtual ~FileChooserHandler();
	virtual void Cancel() = 0;
	virtual void Confirm() = 0;
	virtual void ClickFile(ChooseRect *cr) = 0;
	virtual void FocusFile(ChooseRect *cr) = 0;
	virtual void UnfocusFile(ChooseRect *cr) = 0;
	virtual bool MouseUpdate() { return true; }
	virtual void Draw(sf::RenderTarget *target) {}
	virtual void ChangePath() {}
	virtual void LateDraw(sf::RenderTarget *target) {}

	//guihandler functions
	virtual void ChooseRectEvent(ChooseRect *cr, int eventType);
	virtual void ButtonCallback(Button *b, const std::string & e);
	virtual void SliderCallback(Slider *slider) {}
	//---------

	FileChooser *chooser;
};

struct DefaultFileSelector : FileChooserHandler
{
	DefaultFileSelector();
	void Cancel();
	void Confirm();
	void ClickFile(ChooseRect *cr);
	void FocusFile(ChooseRect *cr);
	void UnfocusFile(ChooseRect *cr);
	void Draw(sf::RenderTarget *target);
	void ChangePath();

	sf::Vertex largePreview[4];
	Tileset *ts_largePreview;
};

struct FileChooser : TilesetManager,
	PanelUpdater
{
	enum Mode : int
	{
		OPEN,
		SAVE
	};

	

	FileChooser(FileChooserHandler *handler,
		int p_cols, int p_rows, int extraImageRects = 0);
	~FileChooser();

	//panelupdater functions
	bool MouseUpdate();
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void MouseScroll(int delta);
	void LateDraw(sf::RenderTarget *target);
	//---------
	

	void SetRelativePath(const std::string &p_relPath);
	void SetPath(const std::string &p_path);
	void AddFile(const boost::filesystem::path &filePath);
	void AddFolder(const boost::filesystem::path &folderPath);
	void ClearNodes();
	void PopulateRects();
	void Start( const std::string &ext, 
		Mode fMode, const std::string &path );
	void StartRelative(const std::string &ext,
		Mode fMode, const std::string &path);
	void Init();
	void TurnOff();
	//void HideConfirmButton();


	Mode fMode;
	FileChooserHandler *handler;

	std::string ext;
	
	int topRow;
	int maxTopRow;
	int numEntries;
	int cols;
	int rows;
	int totalRects;

	boost::filesystem::path currPath;

	std::vector<FileNode*> nodes;
	
	

	EditSession *edit;

	Panel *panel;
	ImageChooseRect **imageRects;
	TextBox *fileNameTextBox;
	Button *upButton;
	Label *folderPathLabel;
};

//struct FolderNode
//{
//	~FolderNode();
//	void AddFile(
//		const boost::filesystem::path &filePath);
//	void AddFolder(FolderNode *fn);
//	void Draw(sf::RenderTarget *target);
//
//	std::list<FileNode*> fileNodes;
//	std::list<FolderNode*> childFolders;
//	FolderNode *parentFolder;
//	std::string GetRelPath();
//	std::string folderName;
//	void DebugPrint( int indent );
//};
//
//struct FolderTree
//{
//	FolderTree(const std::string &p_path,
//		const std::string &ext );
//	~FolderTree();
//	void SetupEntry( FolderNode *fn, const std::string &relPath );
//	void SetupTree();
//	void DebugPrintTree();
//
//	FolderNode *treeRoot;
//	std::string ext;
//	std::string treePath;
//};

#endif