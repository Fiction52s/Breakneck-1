#include "BrushManager.h"
#include "Action.h"
#include <fstream>
#include "EditSession.h"

using namespace std;
using namespace sf;

BrushManager::BrushManager()
{
	edit = EditSession::GetSession();
	currLoadingBrush = NULL;
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
		of.open(filePath + ".bnbrush");

		Brush *newBrush = b->Copy();
		brushes[name] = newBrush;

		newBrush->Save(of);
		of.close();

		newBrush->CreatePreview(filePath);
		
		auto it = brushes.find(name);
		if (it != brushes.end())
		{
			delete (*it).second;
			(*it).second = NULL;
		}
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

		auto it = brushes.find(name);
		if (it != brushes.end())
		{
			delete (*it).second;
			(*it).second = NULL;
		}

		brushes[name] = b;

		return b;
	}
	else
	{
		cout << "couldn't open brush file: " << path << endl;
		assert(0);
	}

	return NULL;
}