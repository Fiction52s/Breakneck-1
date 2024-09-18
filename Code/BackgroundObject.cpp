#include "BackgroundObject.h"
#include "VectorMath.h"
#include "Tileset.h"
#include "DrawLayer.h"
#include <iostream>

using namespace sf;
using namespace std;

BackgroundLayer::BackgroundLayer(int p_drawLayer)
	:drawLayer( p_drawLayer )
{

}

BackgroundLayer::~BackgroundLayer()
{
	for (auto it = objectVec.begin(); it != objectVec.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = quadPtrVec.begin(); it != quadPtrVec.end(); ++it)
	{
		delete (*it);
	}
}

void BackgroundLayer::SetupQuads()
{
	//get rid of the map here. need to use something ordered, just loop through a list or something.
	//std::map<Tileset*, int> numQuadsPerTileset;
	std::vector<std::pair<std::pair<Tileset*, sf::Shader *>, int>> tilesetOrderPairVec;
	Tileset *ts_test = NULL;
	sf::Shader *shaderTest = NULL;
	bool alreadyHas = false;
	for (auto it = objectVec.begin(); it != objectVec.end(); ++it)
	{
		ts_test = (*it)->ts;
		shaderTest = (*it)->sh;
		alreadyHas = false;
		for (auto it2 = tilesetOrderPairVec.begin(); it2 != tilesetOrderPairVec.end(); ++it2)
		{
			if ((*it2).first.first == ts_test && (*it2).first.second == shaderTest )
			{
				alreadyHas = true;
				break;
			}
		}

		if (!alreadyHas)
		{
			tilesetOrderPairVec.push_back(make_pair(make_pair(ts_test, shaderTest), 0));
		}
	}

	for (auto it = objectVec.begin(); it != objectVec.end(); ++it)
	{
		for (auto it2 = tilesetOrderPairVec.begin(); it2 != tilesetOrderPairVec.end(); ++it2)
		{
			if ((*it)->ts == (*it2).first.first && (*it)->sh == (*it2).first.second )
			{
				(*it2).second += (*it)->numQuads;
				break;
			}
		}
	}

	Vertex *currQuad = NULL;
	Vertex *currObjQuads = NULL;
	int currNumQuads = 0;
	for (auto it = tilesetOrderPairVec.begin(); it != tilesetOrderPairVec.end(); ++it)
	{
		currNumQuads = (*it).second;
		currQuad = new Vertex[currNumQuads * 4];
		for (int i = 0; i < (*it).second; ++i)
		{
			ClearRect(currQuad + i * 4);
		}
		currObjQuads = currQuad;
		for (auto it2 = objectVec.begin(); it2 != objectVec.end(); ++it2)
		{
			if ((*it2)->ts == (*it).first.first && (*it2)->sh == (*it).first.second)
			{
				(*it2)->quads = currObjQuads;
				currObjQuads += (*it2)->numQuads * 4;
			}
		}

		quadPtrVec.push_back(new QuadInfo(currQuad, currNumQuads, (*it).first.first, (*it).first.second));
	}
}

void BackgroundLayer::Draw(sf::RenderTarget *target)
{
	oldView = target->getView();
	Vector2f newCenter = Vector2f(oldView.getCenter().x, 0);// -extraOffset;
	newView.setCenter(newCenter);
	newView.setSize(1920, 1080);
	target->setView(newView);

	for (auto it = quadPtrVec.begin(); it != quadPtrVec.end(); ++it)
	{
		if ((*it)->sh != NULL)
		{
			(*it)->sh->setUniform("u_texture", *(*it)->ts->texture);
			target->draw((*it)->verts, (*it)->numQuads * 4, sf::Quads, (*it)->sh);
		}
		else
		{
			target->draw((*it)->verts, (*it)->numQuads * 4, sf::Quads, (*it)->ts->texture);
		}
	}

	target->setView(oldView);
}



BackgroundObject::BackgroundObject(TilesetManager *p_tm, int p_loopWidth, int p_layer )
{
	tm = p_tm;
	depthLayer = p_layer;
	scrollSpeedX = 0;
	repetitionFactor = 1; //zero means never repeats, 1 is repeat every screen.
	numQuads = 0;
	ts = NULL;
	sh = NULL;
	quads = NULL;
	loopWidth = p_loopWidth;
	Reset();
}

BackgroundObject::~BackgroundObject()
{
	/*if (quads != NULL)
	{
		delete[] quads;
	}*/
}

void BackgroundObject::Reset()
{
	action = 0;
	frame = 0;
	scrollOffset = 0;
}

void BackgroundObject::Load(std::ifstream &is )
{
	//is >> depthLayer;
	is >> scrollSpeedX;
	is >> repetitionFactor;
}

void BackgroundObject::ProcessAction()
{
	//empty
}

void BackgroundObject::UpdateQuads( float realX )
{
	IntRect sub = GetSubRect();
	if (repetitionFactor > 0)
	{
		for (int i = 0; i < numQuads; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub.width, sub.height, Vector2f(realX + loopWidth * i, myPos.y));
		}
	}
	else
	{
		for (int i = 0; i < numQuads; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub.width, sub.height, Vector2f(myPos));
		}
	}

	for (int i = 0; i < numQuads; ++i)
	{
		SetRectSubRect(quads + i * 4, sub);
	}
}

void BackgroundObject::Update(const sf::Vector2f &camPos)
{
	ProcessAction();

	if (repetitionFactor > 0)
	{
		float depth = DrawLayer::GetDrawLayerDepthFactor(depthLayer);
		//Vector2f extra(-loopWidth + myPos.x, 0);
		Vector2f extra(myPos.x / depth, 0);

		Vector2f cPos = camPos;
		float testScrollOffset = extra.x + scrollOffset;
		cPos.x -= testScrollOffset;
		if (testScrollOffset * depth > loopWidth)
			testScrollOffset = 0;
		if (testScrollOffset * depth < -loopWidth)
		{
			testScrollOffset = 0;
		}

		float maxWidth = loopWidth * repetitionFactor;

		float camXAbs = abs(cPos.x * depth);
		int m = 0;
		while (camXAbs > maxWidth)
		{
			camXAbs -= maxWidth;
			++m;
		}

		float off = camXAbs;

		if (cPos.x > 0)
			off = -off;
		else if (cPos.x < 0)
		{
			off = off - maxWidth;
		}

		if (scrollOffset * depth > maxWidth)
			scrollOffset = 0;
		if (scrollOffset * depth < -maxWidth)
		{
			scrollOffset = 0;
		}
		scrollOffset += scrollSpeedX;// *updateFrames;

		float realX = (camPos.x + off) -1920 / 2;

		//realX = round(realX);

		UpdateQuads(realX);
		
	}
	else
	{
		UpdateQuads(0);
	}

	++frame;
}

void BackgroundObject::Update(const sf::Vector2f &camPos, int numFrames)
{
	for (int i = 0; i < numFrames; ++i)
	{
		Update(camPos);
	}
}

void BackgroundObject::DrawObject(sf::RenderTarget *target)
{
	target->draw(quads, 4 * numQuads, sf::Quads, ts->texture);
}

void BackgroundObject::Draw(sf::RenderTarget *target)
{
	oldView = target->getView();

	if (repetitionFactor == 0)
	{
		float depth = DrawLayer::GetDrawLayerDepthFactor(depthLayer);
		//the .5 is because the scrolling bgs do it, fix it soon
		Vector2f newCenter = Vector2f(oldView.getCenter().x * depth * .5f, 0);// -extraOffset;

		newView.setCenter(newCenter);
		newView.setSize(1920, 1080);

		target->setView(newView);

		DrawObject(target);
	}
	else
	{
		Vector2f newCenter = Vector2f(oldView.getCenter().x, 0);// -extraOffset;
		newView.setCenter(newCenter);
		newView.setSize(1920, 1080);
		target->setView(newView);

		//if (repetitionFactor == 1)
		{
			DrawObject(target);
		}
		/*else
		{
		target->draw(quads, 4, sf::Quads, ts->texture);
		}*/
	}

	target->setView(oldView);
}

void BackgroundObject::LayeredDraw(int p_depthLevel, sf::RenderTarget *target)
{
	if (p_depthLevel == depthLayer)
	{
		Draw(target);
	}
}

sf::IntRect BackgroundObject::GetSubRect()
{
	return IntRect(0, 0, 0, 0);
}

BackgroundTile::BackgroundTile(TilesetManager *p_tm, const std::string &p_folder, int p_loopWidth, int p_layer)
	:BackgroundObject( p_tm, p_loopWidth, p_layer )
{
	folder = p_folder;
}

sf::IntRect BackgroundTile::GetSubRect()
{
	return subRect;
}

void BackgroundTile::Load(nlohmann::basic_json<> &jobj)
{
	if (jobj.contains("scrollRate"))
	{
		scrollSpeedX = jobj["scrollRate"];
	}
	else
	{
		scrollSpeedX = 0;
	}

	repetitionFactor = jobj["repFactor"];

	string pngName = jobj["texture"];

	Vector2i tileSize;
	Vector2i sheetPos;
	Vector2i bgPos;

	tileSize.x = jobj["size"][0];
	tileSize.y = jobj["size"][1];

	sheetPos.x = jobj["sheetPos"][0];
	sheetPos.y = jobj["sheetPos"][1];

	bgPos.x = jobj["bgPos"][0];
	bgPos.y = jobj["bgPos"][1];

	string tsPath = folder + pngName + ".png";

	ts = tm->GetTileset(tsPath);

	if (ts == NULL)
	{
		cout << "background tile tileset is null: " << tsPath << endl;
		assert(0);
	}

	subRect.left = sheetPos.x;
	subRect.top = sheetPos.y;
	subRect.width = tileSize.x;
	subRect.height = tileSize.y;

	if (repetitionFactor == 0)
	{
		bgPos.x -= 960;
	}

	bgPos.y -= 540;

	myPos = Vector2f(bgPos);

	if (repetitionFactor > 0)
	{
		numQuads = 2;
	}
	else
	{
		numQuads = 1;
	}

	/*quads = new Vertex[4 * numQuads];

	for (int i = 0; i < numQuads; ++i)
	{
		ClearRect(quads + i * 4);
	}*/
}


BackgroundWideSpread::BackgroundWideSpread(TilesetManager *p_tm, const std::string &p_folder, int p_loopWidth, int p_layer)
	:BackgroundObject(p_tm, p_loopWidth, p_layer)
{
	folder = p_folder;
	extraWidth = 0;
}

sf::IntRect BackgroundWideSpread::GetSubRect()
{
	return subRect;
}

void BackgroundWideSpread::Load(nlohmann::basic_json<> &jobj)
{
	extraWidth = jobj["extraWidth"];

	if (jobj.contains("scrollRate"))
	{
		scrollSpeedX = jobj["scrollRate"];
	}
	else
	{
		scrollSpeedX = 0;
	}

	repetitionFactor = jobj["repFactor"];

	string pngName = jobj["texture"];
	
	Vector2i tileSize;
	Vector2i sheetPos;
	Vector2i bgPos;

	tileSize.x = jobj["size"][0];
	tileSize.y = jobj["size"][1];

	sheetPos.x = jobj["sheetPos"][0];
	sheetPos.y = jobj["sheetPos"][1];

	bgPos.x = jobj["bgPos"][0];
	bgPos.y = jobj["bgPos"][1];

	string tsPath = folder + pngName + ".png";

	ts = tm->GetTileset(tsPath);

	if (ts == NULL)
	{
		cout << " background wide spread tileset is null: " << tsPath << endl;
		assert(0);
	}

	subRect.left = sheetPos.x;
	subRect.top = sheetPos.y;
	subRect.width = tileSize.x;
	subRect.height = tileSize.y;

	if (repetitionFactor == 0)
	{
		bgPos.x -= 960;
	}

	bgPos.y -= 540;

	myPos = Vector2f(bgPos);

	if (repetitionFactor > 0)
	{
		numQuads = 2;
	}
	else
	{
		numQuads = 1;
	}

	numQuads *= 2;

	/*quads = new Vertex[4 * numQuads];

	for (int i = 0; i < numQuads; ++i)
	{
		ClearRect(quads + i * 4);
	}*/
}

void BackgroundWideSpread::UpdateQuads(float realX)
{
	IntRect sub = GetSubRect();
	IntRect sub2 = sub;
	sub2.top += sub.height;
	sub2.width = extraWidth;

	if (repetitionFactor > 0)
	{
		for (int i = 0; i < 2; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub.width, sub.height, Vector2f(realX + loopWidth * i, myPos.y));
			SetRectSubRect(quads + i * 4, sub);
		}

		for (int i = 2; i < 4; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub2.width, sub2.height, Vector2f(realX + loopWidth * (i - 2) + sub.width, myPos.y));
			SetRectSubRect(quads + i * 4, sub2);
		}
	}
	else
	{
		SetRectTopLeft(quads, sub.width, sub.height, Vector2f(myPos));
		SetRectSubRect(quads, sub);

		SetRectTopLeft(quads + 4, sub2.width, sub2.height, Vector2f(myPos.x + sub.width, myPos.y));
		SetRectSubRect(quads + 4, sub2);
	}
}


BackgroundWaterfall::BackgroundWaterfall( TilesetManager *p_tm, int p_loopWidth, int p_layer )
	:BackgroundObject( p_tm, p_loopWidth, p_layer )
{
	ts = p_tm->GetSizedTileset("Backgrounds/W1/w1_01/waterfall_w4_128x320.png");

	actionLength[A_IDLE] = 12;

	animFactor[A_IDLE] = 3;

	quads = NULL;

	//ClearRect(quad);
}

BackgroundWaterfall::~BackgroundWaterfall()
{
}

void BackgroundWaterfall::Load(nlohmann::basic_json<> &jobj)
{
	if (jobj.contains("scrollRate"))
	{
		scrollSpeedX = jobj["scrollRate"];
	}
	else
	{
		scrollSpeedX = 0;
	}

	repetitionFactor = jobj["repFactor"];

	Vector2i bgPos;

	bgPos.x = jobj["bgPos"][0];
	bgPos.y = jobj["bgPos"][1];

	if (repetitionFactor == 0)
	{
		bgPos.x -= 960;
	}
	
	bgPos.y -= 540;

	myPos = Vector2f(bgPos);

	if (repetitionFactor > 0)
	{
		numQuads = 2;
	}
	else
	{
		numQuads = 1;
	}

	quads = new Vertex[4 * numQuads];

	for (int i = 0; i < numQuads; ++i)
	{
		ClearRect(quads + i * 4);
	}
}

void BackgroundWaterfall::ProcessAction()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
	}
}

sf::IntRect BackgroundWaterfall::GetSubRect()
{
	return ts->GetSubRect(frame / animFactor[action]);
}