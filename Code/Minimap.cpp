#include "Minimap.h"
#include <iostream>
#include <assert.h>
#include "GameSession.h"
#include "MapHeader.h"
#include "Session.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

Color Minimap::terrainColor(0x75, 0x70, 0x90);// , 191);

const float Minimap::MINIMAP_ZOOM = 16.f;

Minimap::Minimap(TilesetManager *tm)
{
	sess = NULL;

	MainMenu *mm = MainMenu::GetInstance();

	minimapTex = mm->minimapTexture;

	if (!minimapShader.loadFromFile("Resources/Shader/minimap.frag", sf::Shader::Fragment))
	{
		cout << "minimap SHADER NOT LOADING CORRECTLY" << endl;
		assert(0 && "minimap shader not loaded");
	}
	minimapShader.setUniform("imageSize", Vector2f(minimapTex->getSize().x,
		minimapTex->getSize().y));
	minimapSprite.setTexture(minimapTex->getTexture());
	minimapSprite.setOrigin(minimapSprite.getLocalBounds().width / 2,
		minimapSprite.getLocalBounds().height / 2);
	
	minimapSprite.setScale(1, -1);

	ts_miniIcons = mm->GetSizedTileset("HUD/Minimap/minimap_icons_64x64.png");
	kinMinimapIcon.setTexture(*ts_miniIcons->texture);
	kinMinimapIcon.setTextureRect(ts_miniIcons->GetSubRect(0));
	kinMinimapIcon.setOrigin(kinMinimapIcon.getLocalBounds().width / 2,
		kinMinimapIcon.getLocalBounds().height / 2);
	kinMinimapIcon.setPosition(0, 0);
	kinMinimapIcon.setScale(.5, .5);


	goalMapIcon.setTexture(*ts_miniIcons->texture);
	goalMapIcon.setTextureRect(ts_miniIcons->GetSubRect(0));
	goalMapIcon.setOrigin(goalMapIcon.getLocalBounds().width / 2,
		goalMapIcon.getLocalBounds().height / 2);

	SetCenter(Vector2f(200, 1080 - 200));
}

void Minimap::SetSession(Session *p_sess)
{
	sess = p_sess;
}

void Minimap::SetCenter(sf::Vector2f &center)
{
	minimapSprite.setPosition(center);
}

void Minimap::Update()
{

}

void Minimap::SetupBorderQuads(
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

void Minimap::DrawToTex()
{
	//Actor *p0 = owner->GetPlayer(0);

	//double minimapZoom = 16;//12;// * cam.GetZoom();// + cam.GetZoom();
	V2d pos0 = sess->GetPlayerPos(0);
	View vv;
	vv.setCenter(pos0.x, pos0.y);
	vv.setSize(minimapTex->getSize().x * MINIMAP_ZOOM, minimapTex->getSize().y * MINIMAP_ZOOM);

	minimapTex->setView(vv);
	minimapTex->clear(Color(0, 0, 0, 191));


	for (auto it = sess->zones.begin(); it != sess->zones.end(); ++it)
	{
		(*it)->Draw(minimapTex);
	}

	sf::Rect<double> minimapRect(vv.getCenter().x - vv.getSize().x / 2.0,
		vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y);

	DrawSpecialTerrain(minimapRect, minimapTex);

	DrawTerrain(minimapRect, minimapTex);

	DrawRails(minimapRect, minimapTex);

	DrawZones( minimapTex );

	DrawMapBorders(minimapTex);

	DrawGates(minimapRect, minimapTex);

	sess->DrawAllMapWires(minimapTex);

	sess->EnemiesCheckedMiniDraw(minimapTex, FloatRect(minimapRect));

	sess->DrawPlayersMini(minimapTex);

	sess->DrawPlayersToMap(minimapTex, false, false, 1.f);

	sf::View iconView;
	iconView.setCenter(0, 0);
	iconView.setSize(minimapTex->getSize().x, minimapTex->getSize().y);
	minimapTex->setView(iconView);

	//minimapTex->draw(kinMinimapIcon);
}

void Minimap::Draw(sf::RenderTarget *target)
{
	sess = Session::GetSession();
	DrawToTex();

	minimapTex->display();
	const Texture &miniTex = minimapTex->getTexture();
	minimapShader.setUniform("u_texture", minimapTex->getTexture());

	minimapSprite.setTexture(miniTex);

	target->draw(minimapSprite, &minimapShader);
}

void Minimap::DrawZones( RenderTarget *target)
{
	auto &zones = sess->zones;
	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->DrawMinimap(target);
	}
}

void Minimap::DrawTerrain( sf::Rect<double> &rect, sf::RenderTarget *target )
{
	sess->QueryBorderTree(rect);
	sess->DrawColoredMapTerrain(target, terrainColor);
}

void Minimap::DrawRails(sf::Rect<double> &rect,
	sf::RenderTarget *target)
{
	sess->QueryRailDrawTree(rect);
	sess->DrawQueriedRailsToMinimap(target);
}

void Minimap::DrawSpecialTerrain(sf::Rect<double> &rect, sf::RenderTarget *target)
{
	sess->QuerySpecialTerrainTree(rect);
	sess->DrawSpecialMapTerrain(target);
}

void Minimap::DrawMapBorders(
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

void Minimap::DrawGates(sf::Rect<double> &rect,
	sf::RenderTarget *target)
{
	sess->QueryGateTree(rect);
	Gate *gateList = sess->gateList;
	while (gateList != NULL)
	{
		gateList->MiniDraw(target);

		Gate *next = gateList->next;
		gateList = next;
	}
	sess->gateList = NULL;
}