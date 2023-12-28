#include "PauseMap.h"
#include <iostream>
#include <assert.h>
#include "GameSession.h"
#include "MapHeader.h"
#include "Session.h"
#include "MainMenu.h"
#include "Actor.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

Color PauseMap::terrainColor(0x75, 0x70, 0x90);// , 191);

PauseMap::PauseMap(TilesetManager *p_tm)
{
	tm = p_tm;
	game = NULL;
	

	cameraRect.setFillColor(Color::Transparent);
	cameraRect.setOutlineColor(Color( 0, 255, 0, 100 ));
	cameraRect.setOutlineThickness(-5);

	MainMenu *mm = MainMenu::GetInstance();

	mapTex = mm->mapTexture;

	mapSprite.setTexture(mapTex->getTexture());
	mapSprite.setOrigin(mapSprite.getLocalBounds().width / 2,
		mapSprite.getLocalBounds().height / 2);

	//mapSprite.setScale(1, -1);

	//global tileset
	ts_miniIcons = mm->GetTileset("HUD/Minimap/minimap_icons_64x64.png", 64, 64);
	kinMapIcon.setTexture(*ts_miniIcons->texture);
	kinMapIcon.setTextureRect(ts_miniIcons->GetSubRect(0));
	kinMapIcon.setScale(2, 2);
	kinMapIcon.setOrigin(kinMapIcon.getLocalBounds().width / 2,
		kinMapIcon.getLocalBounds().height / 2);
	kinMapIcon.setPosition(0, 0);
	//.5, .5);

	goalMapIcon.setTexture(*ts_miniIcons->texture);
	goalMapIcon.setTextureRect(ts_miniIcons->GetSubRect(5));
	goalMapIcon.setScale(1.5, 1.5);
	goalMapIcon.setOrigin(goalMapIcon.getLocalBounds().width / 2,
		goalMapIcon.getLocalBounds().height / 2);

	SetCenter(Vector2f(200, 1080 - 200));

	//Reset();
}

PauseMap::~PauseMap()
{
}

void PauseMap::Reset()
{
	mapCenter.x = game->GetPlayerPos(0).x;
	mapCenter.y = game->GetPlayerPos(0).y;
	mapZoomFactor = 8;
}

void PauseMap::SetGame(GameSession *p_game)
{
	game = p_game;
	Reset();
}

void PauseMap::SetCenter(sf::Vector2f &center)
{
	mapSprite.setPosition(center);
}

void PauseMap::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	assert(game != NULL);
	float fac = .05;
	if (currInput.A)
	{
		mapZoomFactor -= fac * mapZoomFactor;
	}
	else if (currInput.B)
	{
		mapZoomFactor += fac * mapZoomFactor;
	}

	if (mapZoomFactor < 1.f)
	{
		mapZoomFactor = 1.f;
	}
	else if (mapZoomFactor > 128.f)
	{
		mapZoomFactor = 128.f;
	}

	float move = 20.0 * mapZoomFactor / 2.0;
	if (currInput.LLeft() || currInput.PLeft())
	{
		mapCenter.x -= move;
	}
	else if (currInput.LRight() || currInput.PRight())
	{
		mapCenter.x += move;
	}

	if (currInput.LUp() || currInput.PUp())
	{
		mapCenter.y -= move;
	}
	else if (currInput.LDown() || currInput.PDown())
	{
		mapCenter.y += move;
	}

	if (mapCenter.x < game->mapHeader->leftBounds)
	{
		mapCenter.x = game->mapHeader->leftBounds;
	}
	else if (mapCenter.x > game->mapHeader->leftBounds + game->mapHeader->boundsWidth)
	{
		mapCenter.x = game->mapHeader->leftBounds + game->mapHeader->boundsWidth;
	}

	if (mapCenter.y < game->mapHeader->topBounds)
	{
		mapCenter.y = game->mapHeader->topBounds;
	}
	else if (mapCenter.y > game->mapHeader->topBounds + game->mapHeader->boundsHeight)
	{
		mapCenter.y = game->mapHeader->topBounds + game->mapHeader->boundsHeight;
	}
}

void PauseMap::SetupBorderQuads(
	bool *p_blackBorderOn, bool p_topBorderOn,
	MapHeader *mapHeader)
{
	blackBorderOn[0] = p_blackBorderOn[0];
	blackBorderOn[1] = p_blackBorderOn[1];
	topBorderOn = p_topBorderOn;

	int miniQuadWidth = 4000;
	int inverseTerrainBorder = 4000;
	int blackMiniTop = mapHeader->topBounds - inverseTerrainBorder;
	int blackMiniBot = mapHeader->topBounds + mapHeader->boundsHeight + inverseTerrainBorder;
	int blackMiniLeft = mapHeader->leftBounds - miniQuadWidth;
	int rightBounds = mapHeader->leftBounds + mapHeader->boundsWidth;
	int blackMiniRight = rightBounds + miniQuadWidth;

	blackBorderQuadsMini[1].position.x = mapHeader->leftBounds;
	blackBorderQuadsMini[2].position.x = mapHeader->leftBounds;
	blackBorderQuadsMini[0].position.x = mapHeader->leftBounds - miniQuadWidth;
	blackBorderQuadsMini[3].position.x = mapHeader->leftBounds - miniQuadWidth;

	blackBorderQuadsMini[0].position.y = blackMiniTop;
	blackBorderQuadsMini[1].position.y = blackMiniTop;

	blackBorderQuadsMini[2].position.y = blackMiniBot;
	blackBorderQuadsMini[3].position.y = blackMiniBot;


	blackBorderQuadsMini[5].position.x = rightBounds + miniQuadWidth;
	blackBorderQuadsMini[6].position.x = rightBounds + miniQuadWidth;
	blackBorderQuadsMini[4].position.x = rightBounds;
	blackBorderQuadsMini[7].position.x = rightBounds;

	blackBorderQuadsMini[4].position.y = blackMiniTop;
	blackBorderQuadsMini[5].position.y = blackMiniTop;

	blackBorderQuadsMini[6].position.y = blackMiniBot;
	blackBorderQuadsMini[7].position.y = blackMiniBot;

	Color miniBorderColor = Color(100, 100, 100);
	Color miniTopBorderColor = Color(0x10, 0x40, 0xff);
	//SetRectColor(blackBorderQuads + 4, Color( 100, 100, 100 ));

	if (blackBorderOn[0])
		SetRectColor(blackBorderQuadsMini, miniTopBorderColor);
	else
	{
		SetRectColor(blackBorderQuadsMini, Color::Transparent);
	}
	if (blackBorderOn[1])
		SetRectColor(blackBorderQuadsMini + 4, miniTopBorderColor);
	else
	{
		SetRectColor(blackBorderQuadsMini + 4, Color::Transparent);
	}

	if (topBorderOn)
	{
		SetRectColor(topBorderQuadMini, miniTopBorderColor);

		topBorderQuadMini[0].position.x = blackMiniLeft;
		topBorderQuadMini[1].position.x = blackMiniRight;
		topBorderQuadMini[2].position.x = blackMiniRight;
		topBorderQuadMini[3].position.x = blackMiniLeft;

		topBorderQuadMini[0].position.y = blackMiniTop;
		topBorderQuadMini[1].position.y = blackMiniTop;
		topBorderQuadMini[2].position.y = mapHeader->topBounds;
		topBorderQuadMini[3].position.y = mapHeader->topBounds;
	}
	else
	{
		SetRectColor(topBorderQuadMini, Color::Transparent);
	}
}

void PauseMap::DrawToTex()
{
	//Actor *p0 = owner->GetPlayer(0);

	//double minimapZoom = 16;//12;// * cam.GetZoom();// + cam.GetZoom();

	cameraRect.setPosition(game->cam.GetPos());
	float zoom = game->cam.GetZoom();
	cameraRect.setSize(Vector2f(960 * zoom, 540 * zoom));
	cameraRect.setOrigin(cameraRect.getLocalBounds().width / 2, cameraRect.getLocalBounds().height / 2);

	V2d pos0 = game->GetPlayerPos(0);
	View vv;
	vv.setCenter(mapCenter.x, mapCenter.y);
	vv.setSize(mapTex->getSize().x * mapZoomFactor, mapTex->getSize().y * mapZoomFactor);

	mapTex->setView(vv);
	mapTex->clear(Color::Black);//Color(0, 0, 0, 191));

	sf::Rect<double> minimapRect(vv.getCenter().x - vv.getSize().x / 2.0,
		vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y);

	game->QueryBorderTree(minimapRect);
	Vector2f vSize = vv.getSize();
	Vector2f botLeft(vv.getCenter().x - vSize.x / 2, vv.getCenter().y + vSize.y / 2);
	Vector2f botLeftTest(-vSize.x / 2, vSize.y / 2);
	//RotateCW(botLeftTest, camAngle);
	botLeftTest += vv.getCenter();
	botLeft = botLeftTest;
	Vector2f blank;
	float realZ = vSize.x / (mapTex->getSize().x / 2.f);//960.f;
	game->UpdatePolyShaders(botLeft, blank, realZ);


	DrawSpecialTerrain(minimapRect, mapTex);

	DrawTerrain(minimapRect, mapTex);

	DrawRails(minimapRect, mapTex);

	DrawZones(mapTex);

	DrawMapBorders(mapTex);

	DrawGates(minimapRect, mapTex);

	//mapTex->draw(cameraRect);

	game->DrawAllMapWires(mapTex);

	game->EnemiesCheckPauseMapDraw(mapTex, FloatRect(minimapRect));

	//game->DrawPlayersMini(mapTex);

	bool drawKins = mapZoomFactor < 3.2;

	game->DrawPlayersToMap(mapTex, drawKins, true, 2.f );
	/*if (mapZoomFactor <= 3.2)
	{
		game->UpdateNameTagsPixelPos(mapTex);
		game->DrawNameTags(mapTex);
		game->DrawPlayers(mapTex);
	}
	else
	{
		game->DrawPlayerIcons(mapTex);
	}*/

	

	Vector2i kinIconPixel = mapTex->mapCoordsToPixel(Vector2f(pos0));

	Vector2i goalIconPixel = mapTex->mapCoordsToPixel(Vector2f(game->goalPos));

	sf::View iconView;
	iconView.setCenter(0, 0);
	iconView.setSize(mapTex->getSize().x, mapTex->getSize().y);
	mapTex->setView(iconView);

	Vector2f trueKinMapIconPos = mapTex->mapPixelToCoords(kinIconPixel);

	Vector2f trueGoalMapIconPos = mapTex->mapPixelToCoords(goalIconPixel);

	kinMapIcon.setPosition(trueKinMapIconPos);

	goalMapIcon.setPosition(trueGoalMapIconPos);

	mapTex->draw(goalMapIcon);

	//mapTex->draw(kinMapIcon);
}

void PauseMap::Draw(sf::RenderTarget *target)
{
	DrawToTex();

	mapTex->display();
	const Texture &tex = mapTex->getTexture();
	//mapShader.setUniform("u_texture", mapTex->getTexture());

	mapSprite.setTexture(tex);

	//target->draw(mapSprite, &mapShader);
	target->draw(mapSprite);
}

void PauseMap::DrawZones(RenderTarget *target)
{
	auto &zones = game->zones;
	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		if ((*it)->zType == Zone::SECRET && (*it)->data.action == Zone::UNEXPLORED)
		{
			(*it)->zonePoly->DrawAsSecretCover(target, false );
		}
		else
		{
			//(*it)->Draw(target);
			//(*it)->DrawMinimap(target);
		}
		
	}
}

void PauseMap::DrawTerrain(sf::Rect<double> &rect, sf::RenderTarget *target)
{
	game->QueryBorderTree(rect);

	PolyPtr poly = game->polyQueryList;
	while (poly != NULL)
	{
		TerrainPolygon::RenderMode oldPolyMode = poly->renderMode;
		poly->SetRenderMode(TerrainPolygon::RENDERMODE_MAP);
		
		poly->Draw(target); //preScreenTex

		poly->SetRenderMode(oldPolyMode);
		poly = poly->queryNext;
	}

	//game->DrawQueriedTerrain(target);
	//game->DrawColoredMapTerrain(target, terrainColor);
}

void PauseMap::DrawRails(sf::Rect<double> &rect,
	sf::RenderTarget *target)
{
	game->QueryRailDrawTree(rect);
	game->DrawQueriedRails(target);
}

void PauseMap::DrawSpecialTerrain(sf::Rect<double> &rect, sf::RenderTarget *target)
{
	game->QuerySpecialTerrainTree(rect);
	//game->DrawSpecialMapTerrain(target);

	PolyPtr poly = game->specialPieceList;
	while (poly != NULL)
	{
		poly->Draw(target);
		poly = poly->queryNext;
	}
}

void PauseMap::DrawMapBorders(
	sf::RenderTarget *target)
{
	if (blackBorderOn[0] || blackBorderOn[1])
	{
		target->draw(blackBorderQuadsMini, 8, sf::Quads);
	}

	if (topBorderOn)
	{
		target->draw(topBorderQuadMini, 4, sf::Quads);
	}
}

void PauseMap::DrawGates(sf::Rect<double> &rect,
	sf::RenderTarget *target)
{
	game->QueryGateTree(rect);
	Gate *gateList = game->gateList;
	while (gateList != NULL)
	{
		if(gateList->IsSecret() && (gateList->zoneA != NULL && gateList->zoneA->secretZone && gateList->zoneA->data.action == Zone::UNEXPLORED)
			|| (gateList->zoneB != NULL && gateList->zoneB->secretZone && gateList->zoneB->data.action == Zone::UNEXPLORED))
		{
			gateList->DrawSecret(target);
		}
		else
		{
			gateList->MapDraw(target);
		}
		

		Gate *next = gateList->next;
		gateList = next;
	}
	game->gateList = NULL;

	/*int index = 0;
	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
	{
		if (hideSecret && ((*it)->category == Gate::SECRET || gates[index]->IsSecret()))
		{
			(*it)->DrawSecretPreview(target);
		}
		else
		{
			(*it)->DrawPreview(target);
		}

		++index;
	}*/
}