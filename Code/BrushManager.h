#ifndef __BRUSHMANAGER_H__
#define __BRUSHMANAGER_H__

#include <map>
#include <string>

struct Brush;
struct EditSession;

struct BrushManager
{
	BrushManager();
	int SaveBrush(Brush *b, const std::string &path,
		const std::string &name);
	Brush * LoadBrush(const std::string &path,
		const std::string &name );

	EditSession *edit;
	std::map<std::string, Brush*> brushes;
	Brush *currLoadingBrush;
};

#endif