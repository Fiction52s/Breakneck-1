#ifndef __FILECHOOSER_H__
#define __FILECHOOSER_H__

#include "GUI.h"
#include <boost/filesystem.hpp>
#include "Tileset.h"


struct FolderNode;

struct FileNode
{
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

struct FileChooser : GUIHandler, TilesetManager
{
	FileChooser();
	~FileChooser();

	void SetPath(const std::string &relPath);
	void AddFile(const boost::filesystem::path &filePath);
	void ClearFiles();
	void Print();
	void ChooseRectEvent(ChooseRect *cr, int eventType);

	ImageChooseRect **imageRects;
	int cols;
	int rows;
	int totalRects;
	std::list<FileNode*> fileNodes;
	std::list<boost::filesystem::path> childFolders;
	std::string ext;
	boost::filesystem::path basePath;
	sf::Vertex largePreview[4];
	Tileset *ts_largePreview;
	Panel *panel;
};

#endif