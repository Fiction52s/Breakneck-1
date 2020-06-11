#ifndef __BRUSHMANAGER_H__
#define __BRUSHMANAGER_H__

//#include <map>
#include <string>
#include <list>

struct Brush;
struct EditSession;
struct Tileset;
struct TilesetManager;

struct NamedBrush
{
	NamedBrush(
		const std::string &p_path,
		const std::string &p_name,
		Brush *b);
	~NamedBrush();
	std::string name;
	std::string path;
	Brush *myBrush;
	Tileset *ts_preview;
	void LoadPreview(TilesetManager *tm);
};

struct BrushManager
{
	BrushManager();
	~BrushManager();
	int SaveBrush(Brush *b, const std::string &path,
		const std::string &name);
	Brush * LoadBrush(const std::string &path,
		const std::string &name );
	void AddRecentBrush(NamedBrush *nb);

	const static int MAX_RECENT_BRUSHES;
	EditSession *edit;
	//std::map<std::string, Brush*> brushes;
	Brush *currLoadingBrush;
	std::list<NamedBrush*> recentBrushes;
};

#endif