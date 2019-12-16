#ifndef __EDITORMODES_H__
#define __EDITORMODES_H__

#include <SFML/Graphics.hpp>

struct EditorMode
{
	EditorMode();
	EditSession *edit;
	virtual void HandleEvent(sf::Event e) = 0;
};

struct CreateTerrainMode : EditorMode
{
	virtual void HandleEvent(sf::Event e);


	TerrainPolygon *cutPoly0;
	TerrainPolygon *cutPoly1;
	void ChooseCutPoly(TerrainPolygon *choice);
	bool cutChoose;
	bool cutChooseUp;

	boost::shared_ptr<TerrainPolygon> extendingPolygon;
	TerrainPoint *extendingPoint;
};

#endif