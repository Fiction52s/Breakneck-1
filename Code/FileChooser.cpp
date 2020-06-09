#include "FileChooser.h"
#include "Tileset.h"
#include "VectorMath.h"
#include <iostream>

using namespace std;
using namespace sf;
using namespace boost::filesystem;

void FileNode::Draw(sf::RenderTarget *target)
{
	target->draw(previewSpr, 4, sf::Quads, ts_preview->texture);
}

FolderNode::~FolderNode()
{
	for (auto it = fileNodes.begin(); it != fileNodes.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = childFolders.begin(); it != childFolders.end(); ++it)
	{
		delete (*it);
	}
}

void FolderNode::Draw(sf::RenderTarget *target)
{
	for (auto it = fileNodes.begin(); it != fileNodes.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void FolderNode::AddFile(const path &p_filePath)
{
	FileNode *fileNode = new FileNode;
	fileNode->filePath = p_filePath;
	fileNode->parentFolder = this;
	SetRectColor(fileNode->previewSpr, Color::Red);

	fileNodes.push_back(fileNode);
}

void FolderNode::AddFolder(FolderNode *fn)
{
	childFolders.push_back(fn);
}

std::string FolderNode::GetRelPath()
{
	if (parentFolder == NULL)
	{
		return "";
	}

	return parentFolder->GetRelPath() + "\\" + folderName + "\\";
}

void FolderNode::DebugPrint( int indent )
{
	for (int i = 0; i < indent; ++i)
	{
		cout << "\t";
	}
	cout << "folder: " << folderName << "\n";

	for (auto it = fileNodes.begin(); it != fileNodes.end(); ++it)
	{
		for (int i = 0; i < indent + 1; ++i)
		{
			cout << "\t";
		}

		cout << "file: " << (*it)->filePath.filename().string() << "\n";
	}

	for (auto it = childFolders.begin(); it != childFolders.end(); ++it)
	{
		(*it)->DebugPrint(indent + 1);
	}
}

FolderTree::FolderTree(const std::string &p_path, const std::string &p_ext)
	:ext( p_ext )
{
	treePath = p_path;
	treeRoot = NULL;

	SetupTree();
}

FolderTree::~FolderTree()
{
	delete treeRoot;
}

void FolderTree::SetupTree()
{
	SetupEntry(NULL, "");
}

void FolderTree::SetupEntry( FolderNode *fn, const std::string &relPath )
{
	string fnPath = "";
	if (fn != NULL)
		fnPath = fn->GetRelPath();
	string fullRelPath = fnPath + relPath;
	path p(treePath / fullRelPath);
	vector<path> v;
	try
	{
		if (exists(p))
		{
			if (is_regular_file(p))
			{
				if (p.extension().string() == ext)
				{
					//cout << "adding map file: " << p.filename().string() << endl;
					fn->AddFile( p );
				}
			}
			else if (is_directory(p))      // is p a directory?
			{
				FolderNode *newFolder = new FolderNode;
				newFolder->parentFolder = fn;
				if (fn == NULL)
				{
					treeRoot = newFolder;
				}
				else
				{
					newFolder->folderName = p.filename().string();
					fn->AddFolder(newFolder);
				}

				

				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				sort(v.begin(), v.end());             // sort, since directory iteration
													  // is not ordered on some file systems

				//cout << "setting up new folder: " << p.filename().string() << endl;
				for (vector<path>::const_iterator it(v.begin()); it != v.end(); ++it)
				{
					SetupEntry(newFolder, (*it).filename().string());
				}
			}
			else
				cout << p << " exists, but is neither a regular file nor a directory\n";
		}
		else
			cout << p << " does not exist\n";
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
	}
}

void FolderTree::DebugPrintTree()
{
	treeRoot->DebugPrint(0);
}

FileChooser::FileChooser()
{
	folderTree = new FolderTree(current_path().string(), ".brknk");
	folderTree->DebugPrintTree();
}

FileChooser::~FileChooser()
{
	delete folderTree;
}

