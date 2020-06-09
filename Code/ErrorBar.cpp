#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

ErrorBar::ErrorBar(sf::Font &f)
	:myFont(&f)
{
	CreateErrorTable();
	int rectHeight = 30;
	rect.setSize(Vector2f(1920, rectHeight));
	rect.setPosition(Vector2f(0, 1080 - rectHeight));
	Color rectColor = Color::Red;
	//rectColor.a = 25;
	rect.setFillColor(rectColor);
	show = false;

	errorText.setCharacterSize(20);
	errorText.setFillColor(Color::Black);
	errorText.setFont(f);
	errorText.setPosition(5, (1080 - rectHeight) + 5);

	//SetText("BLAH BLAH BLAH");

	SetShown(false);
}

void ErrorBar::SetShown(bool s)
{
	show = s;
}

void ErrorBar::SetText(const std::string &msg)
{
	errorText.setString(msg);
}

void ErrorBar::ShowText(const std::string &msg)
{
	SetShown(true);
	SetText(msg);
}

void ErrorBar::ShowError(ErrorType er)
{
	ShowText(errorStringVec[er]);
}

void ErrorBar::Draw(sf::RenderTarget *target)
{
	if (show)
	{
		target->draw(rect);
		target->draw(errorText);
	}
}

void ErrorBar::CreateErrorTable()
{
	errorStringVec.resize(ERR_Count);
	errorStringVec[ERR_PLAYER_INTERSECTS_POLY] = "The player illegally intersects one or more polygons";
	errorStringVec[ERR_POLY_INTERSECTS_POLY] = "A polygon illegally intersects one or more polygons";
	errorStringVec[ERR_POLY_CONTAINS_POLY] = "A polygon illegally contains one or more polygons";
	errorStringVec[ERR_POLY_INTERSECTS_ENEMY] = "A polygon illegally intersects one or more enemies";
	errorStringVec[ERR_POLY_INTERSECTS_GATE] = "A polygon illegally intersects one or more gates";


	errorStringVec[ERR_POINTS_MOVE_TOO_CLOSE] = "Attempted to move points too close to one another";
	errorStringVec[ERR_POLY_HAS_SLIVER] = "A polygon has created an illegal sliver shape";
	errorStringVec[ERR_POLY_INTERSECTS_ITSELF] = "A polygon has illegally intersected its own lines";
	errorStringVec[ERR_ENEMY_NEEDS_GROUND] = "An enemy needed to be attached to a polygon and couldn't find one";
	errorStringVec[ERR_ENEMY_NEEDS_RAIL] = "An enemy neeed to be attached to a rail and couldn't find one";
	errorStringVec[ERR_ENEMY_NEEDS_GROUND_OR_RAIL] = "An enemy needed to be attached to a polygon or rail and couldn't either";

	//these might nevRer be relevant?
	errorStringVec[ERR_POLY_INCORRECT_WINDING_INVERSE] = "An inverse polygon has the wrong winding";
	errorStringVec[ERR_POLY_INCORRECT_WINDING] = "A polygon has the wrong winding";

	errorStringVec[ERR_GATE_INSTERSECTS_ENEMY] = "A gate illegally intersects one or more polygons";
	errorStringVec[ERR_GATE_SAME_POINT] = "A gate cannot have the same start and end point";
	errorStringVec[ERR_GATE_CREATES_SLIVER] = "A gate created an illegal sliver shape";
	errorStringVec[ERR_GATE_INTERSECTS_GATE] = "A gate illegally intersected another gate";
	errorStringVec[ERR_GATE_POINT_ALREADY_OCCUPIED] = "A gate cannot use the same point that another gate is already using";
	errorStringVec[ERR_GATE_INSTERSECTS_ENEMY] = "A gate illegally intersects an enemy";
	errorStringVec[ERR_GATE_NEEDS_BOTH_POINTS] = "A gate needs 2 points to be valid";

	errorStringVec[ERR_POLY_NEEDS_THREE_OR_MORE_POINTS] = "A polygon needs at least 3 points to be valid";
	errorStringVec[ERR_POINTS_TOO_CLOSE_TO_SEG_IN_PROGRESS] = "Points on polygon too close to the draw segment. You can adjust minimum draw separation in settings";
	errorStringVec[ERR_POINT_TO_CLOSE_TO_OTHERS] = "Points on polygon in progress too close together. You can adjust minimum draw separation in settings";
	errorStringVec[ERR_LINES_INTERSECT_IN_PROGRESS] = "Polygon in progress intersects itself";

	errorStringVec[ERR_SELECTED_TERRAIN_MULTIPLE_LAYERS] = "Cannot modify material type on multiple layers at once";
	errorStringVec[ERR_INVERSE_CANT_INTERSECT_NORMAL_POLYS] = "New inverse polygon can't intersect other polygons";

	errorStringVec[ERR_CANT_DELETE_WHILE_MOVING] = "Can't delete while moving objects";
}