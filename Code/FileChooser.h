#ifndef __FILECHOOSER_H__
#define __FILECHOOSER_H__

#include "GUI.h"
#include <boost/filesystem.hpp>

struct Tileset;

struct FolderNode;

struct FileNode
{
	void Draw(sf::RenderTarget *target);

	Tileset *ts_preview;
	boost::filesystem::path filePath;
	sf::Vertex previewSpr[4];
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

struct FileChooser
{
	FileChooser();
	~FileChooser();

	void SetPath(const std::string &relPath);
	void AddFile(const boost::filesystem::path &filePath);
	std::list<FileNode*> fileNodes;
	void ClearFiles();
	std::list<boost::filesystem::path> childFolders;

	void Print();

	std::string ext;
	boost::filesystem::path basePath;

	Panel *panel;
};

#endif