#include "BrushManager.h"
#include "Action.h"
#include <fstream>
#include "EditSession.h"

using namespace std;
using namespace sf;

const int BrushManager::MAX_RECENT_BRUSHES = 8;

NamedBrush::NamedBrush(const std::string &p_path, const std::string &p_name,
	Brush *b)
	:path(p_path), name(p_name), myBrush( b )
{
}

NamedBrush::~NamedBrush()
{
	myBrush->Destroy();
	delete myBrush;

	//needs to destroy the tileset too
}

void NamedBrush::LoadPreview(TilesetManager *tm)
{
	tm->SetGameResourcesMode(false);
	string previewStr = path + "/" + name + ".png";
	ts_preview = tm->GetTileset(previewStr);
	tm->SetGameResourcesMode(true); //not sure if this is always true,
	//but it is
}


BrushManager::BrushManager()
{
	edit = EditSession::GetSession();
	currLoadingBrush = NULL;
}

BrushManager::~BrushManager()
{
	for (auto it = recentBrushes.begin(); it != recentBrushes.end(); ++it)
	{
		delete (*it);
	}
}

int BrushManager::SaveBrush(Brush *b, 
	const std::string &path, 
	const std::string &name )
{
	//it has already been determined that name is not a copy, and 
	//that the write is valid
	if (b != NULL)
	{
		if (b->IsEmpty())
			return 1;

		string filePath = path + "/" + name;

		ofstream of;

		Brush *newBrush = b->Copy();

		if (newBrush == NULL)
		{
			cout << "failed to create brush. newBrush is NULL" << endl;
			return 0;
		}

		of.open(filePath + ".bnbrush");

		

		newBrush->Save(of);
		of.close();

		newBrush->CreatePreview(filePath);

		NamedBrush *nb = new NamedBrush(path, name, newBrush);
		nb->LoadPreview(edit);

		AddRecentBrush(nb);

		/*auto it = brushes.find(name);
		if (it != brushes.end())
		{
			delete (*it).second;
			(*it).second = NULL;
		}*/
	}
	else
	{
		cout << "cannot save null brush" << endl;
	}

	return 0;
}

Brush * BrushManager::LoadBrush(const std::string &path,
	const std::string &name )
{
	ifstream is;

	is.open(path + "/" + name + ".bnbrush");

	if (is.is_open())
	{
		Brush *b = new Brush;
		currLoadingBrush = b;
		b->Load(is);
		currLoadingBrush = NULL;


		NamedBrush *nb = new NamedBrush(path, name, b);
		nb->LoadPreview(edit);
		/*auto it = brushes.find(name);
		if (it != brushes.end())
		{
			delete (*it).second;
			(*it).second = NULL;
		}*/

		//brushes[name] = b;
		
		AddRecentBrush(nb);

		return b;
	}
	else
	{
		cout << "couldn't open brush file: " << path << endl;
		assert(0);
	}

	return NULL;
}

void BrushManager::AddRecentBrush(NamedBrush *b)
{
	NamedBrush *foundBrush = NULL;
	for (auto it = recentBrushes.begin(); it != recentBrushes.end(); ++it)
	{
		if ((*it)->path == b->path && (*it)->name == b->name)
		{
			foundBrush = (*it);
			recentBrushes.erase(it);
			break;
		}
	}

	if (!foundBrush && recentBrushes.size() == MAX_RECENT_BRUSHES)
	{
		recentBrushes.pop_front();
	}

	recentBrushes.push_back(b);

	//might optimize this later
	if (foundBrush)
		delete foundBrush;

	/*if (createEnemyModeUI != NULL)
	{
		createEnemyModeUI->UpdateHotbarTypes();
	}*/
}