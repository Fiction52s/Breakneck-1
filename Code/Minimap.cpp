#include "Minimap.h"
#include <iostream>
#include <assert.h>
#include "GameSession.h"

using namespace sf;
using namespace std;

Minimap::Minimap( GameSession *p_owner)
{
	owner = p_owner;

	minimapTex = owner->minimapTex;

	if (!minimapShader.loadFromFile("Resources/Shader/minimap_shader.frag", sf::Shader::Fragment))
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

	ts_miniIcons = owner->GetTileset("HUD/minimap_icons_64x64.png", 64, 64);
	kinMinimapIcon.setTexture(*ts_miniIcons->texture);
	kinMinimapIcon.setTextureRect(ts_miniIcons->GetSubRect(0));
	kinMinimapIcon.setOrigin(kinMinimapIcon.getLocalBounds().width / 2,
		kinMinimapIcon.getLocalBounds().height / 2);
	kinMinimapIcon.setPosition(0, 0);


	goalMapIcon.setTexture(*ts_miniIcons->texture);
	goalMapIcon.setTextureRect(ts_miniIcons->GetSubRect(0));
	goalMapIcon.setOrigin(goalMapIcon.getLocalBounds().width / 2,
		goalMapIcon.getLocalBounds().height / 2);

	SetCenter(Vector2f(200, owner->preScreenTex->getSize().y - 200));
}

void Minimap::SetCenter(sf::Vector2f &center)
{
	minimapSprite.setPosition(center);
}

void Minimap::Update()
{

}

void Minimap::DrawToTex()
{
	Actor *p0 = owner->GetPlayer(0);

	double minimapZoom = 16;//12;// * cam.GetZoom();// + cam.GetZoom();

	View vv;
	vv.setCenter(p0->position.x, p0->position.y);
	vv.setSize(minimapTex->getSize().x * minimapZoom, minimapTex->getSize().y * minimapZoom);

	minimapTex->setView(vv);
	minimapTex->clear(Color(0, 0, 0, 191));


	for (list<Zone*>::iterator it = owner->zones.begin(); it != owner->zones.end(); ++it)
	{
		(*it)->Draw(minimapTex);
	}

	sf::Rect<double> minimapRect(vv.getCenter().x - vv.getSize().x / 2.0,
		vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y);

	DrawTerrain(minimapRect, minimapTex);

	DrawMapBorders(minimapTex);

	DrawGates(minimapRect, minimapTex);

	owner->DrawAllMapWires(minimapTex);

	owner->EnemiesCheckedMiniDraw(minimapTex, FloatRect(minimapRect));

	sf::View iconView;
	iconView.setCenter(0, 0);
	iconView.setSize(minimapTex->getSize().x, minimapTex->getSize().y);
	minimapTex->setView(iconView);

	minimapTex->draw(kinMinimapIcon);
}

void Minimap::Draw(sf::RenderTarget *target)
{
	DrawToTex();

	minimapTex->display();
	const Texture &miniTex = minimapTex->getTexture();
	minimapShader.setUniform("u_texture", minimapTex->getTexture());

	minimapSprite.setTexture(miniTex);

	target->draw(minimapSprite, &minimapShader);
}

void Minimap::DrawTerrain( sf::Rect<double> &rect, sf::RenderTarget *target )
{
	owner->QueryBorderTree(rect);
	Color testColor(0x75, 0x70, 0x90, 191);
	owner->DrawColoredMapTerrain(target, testColor);
}

void Minimap::DrawMapBorders(
	sf::RenderTarget *target)
{
	target->draw(blackBorderQuadsMini, 8, sf::Quads);
	target->draw(topBorderQuadMini, 4, sf::Quads);
}

void Minimap::DrawGates(sf::Rect<double> &rect,
	sf::RenderTarget *target)
{
	owner->QueryGateTree(rect);
	Gate *gateList = owner->gateList;
	while (gateList != NULL)
	{
		if (gateList->locked && gateList->visible)
		{
			V2d along = normalize(gateList->edgeA->v1 - gateList->edgeA->v0);
			V2d other(along.y, -along.x);
			double width = 25;

			V2d leftGround = gateList->edgeA->v0 + other * -width;
			V2d rightGround = gateList->edgeA->v0 + other * width;
			V2d leftAir = gateList->edgeA->v1 + other * -width;
			V2d rightAir = gateList->edgeA->v1 + other * width;
			//cout << "drawing color: " << gateList->c.b << endl;
			sf::Vertex activePreview[4] =
			{
				sf::Vertex(sf::Vector2<float>(leftGround.x, leftGround.y), gateList->c),
				sf::Vertex(sf::Vector2<float>(leftAir.x, leftAir.y), gateList->c),


				sf::Vertex(sf::Vector2<float>(rightAir.x, rightAir.y), gateList->c),


				sf::Vertex(sf::Vector2<float>(rightGround.x, rightGround.y), gateList->c)
			};
			target->draw(activePreview, 4, sf::Quads);
		}

		Gate *next = gateList->next;//edgeA->edge1;
		gateList = next;
	}
	owner->gateList = NULL;
}