#include "Sequence.h"

using namespace std;
using namespace sf;

SceneBG::SceneBG(const std::string &p_name, list<Tileset*> &p_tilesets, int p_animFactor)
{
	name = p_name;
	tilesets.resize(p_tilesets.size());
	int i = 0;
	for (auto it = p_tilesets.begin(); it != p_tilesets.end(); ++it)
	{
		tilesets[i] = (*it);
		++i;
	}
	animFactor = p_animFactor;
}

Tileset * SceneBG::GetCurrTileset(int frame)
{
	int numBG = tilesets.size();
	if (numBG > 1)
	{
		int f = (frame / animFactor) % numBG;
		return tilesets[f];
		//SetRectSubRect(bgQuad, tilesets[0]->GetSubRect(0));
	}
	else
	{
		return tilesets[0];
	}
}