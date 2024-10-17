#include "BackgroundObject.h"
#include "VectorMath.h"
#include "Tileset.h"
#include "DrawLayer.h"
#include <iostream>
#include "Background.h" 
#include "Session.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

BackgroundLayer::BackgroundLayer( Background *p_bg, int p_drawLayer)
	:drawLayer( p_drawLayer ), bg( p_bg )
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

		if (ts_test == NULL)
		{
			selfDrawingObjects.push_back((*it));
			continue;
		}
		

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
	std::string currShaderName;

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

				if (currShaderName == "")
				{
					currShaderName = (*it2)->shaderName;
				}
			}
		}

		quadPtrVec.push_back(new QuadInfo(currQuad, currNumQuads, (*it).first.first, (*it).first.second, currShaderName ));
	}
}

void BackgroundLayer::Update(const sf::Vector2f &camPos)
{
	for (auto it = objectVec.begin(); it != objectVec.end(); ++it )
	{
		(*it)->Update(camPos);
	}
}

void BackgroundLayer::Update(const sf::Vector2f &camPos, const int numFrames)
{
	for (int i = 0; i < numFrames; ++i)
	{
		Update(camPos);
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
		if ((*it)->sh != NULL )
		{
			if ((*it)->shaderName == "transcend_bg_energy")
			{
				Session *sess = Session::GetSession();
				float camAngle = (float)(sess->view.getRotation() * PI / 180.0);

				float depth = DrawLayer::GetDrawLayerDepthFactor(drawLayer);

				int amt = 120;
				float famt = round(amt / depth);
				amt = famt;
				float q = (bg->frame % amt) / famt;
				(*it)->sh->setUniform("quant", q);

				//Vector2f vSize(1920, 1080);// sess->view.getSize();
				//Vector2f vSize( sess->view.getSize() );
				Vector2f center = newView.getCenter();//sess->view.getCenter();//newView.getCenter();// *depth;// *depth;
				center.x *= depth;
				//center.x *= 3.f;
				Vector2f vSize = newView.getSize();//sess->view.getSize();//newView.getSize();// / 2.f;
				//vSize = Vector2f(1920, 1080) / depth;

				Vector2f botLeftTest(-vSize.x / 2, vSize.y / 2);
				RotateCW(botLeftTest, camAngle);

				botLeftTest += center;

				//(*it)->sh->setUniform("u_depth", depth);
				(*it)->sh->setUniform("topLeft", botLeftTest);

				Color filterColor = Color::Transparent;
				float test = .2;
				if (drawLayer == DrawLayer::BG_7)
				{
					filterColor = Color(0x23 * test, 0x79 * test, 0xbe * test);
				}
				(*it)->sh->setUniform("u_filterColor", ColorGL(filterColor));
			}

			(*it)->sh->setUniform("u_texture", *(*it)->ts->texture);
			target->draw((*it)->verts, (*it)->numQuads * 4, sf::Quads, (*it)->sh);
			
		}
		else
		{
			target->draw((*it)->verts, (*it)->numQuads * 4, sf::Quads, (*it)->ts->texture);
		}
	}

	for (auto it = selfDrawingObjects.begin(); it != selfDrawingObjects.end(); ++it)
	{
		(*it)->Draw(target);
	}

	target->setView(oldView);
}



BackgroundObject::BackgroundObject(Background *p_bg, int p_layer )
{
	bg = p_bg;
	depthLayer = p_layer;
	scrollSpeedX = 0;
	repetitionFactor = 1; //zero means never repeats, 1 is repeat every screen.
	numQuads = 0;
	ts = NULL;
	sh = NULL;
	quads = NULL;
	loopWidth = bg->bgWidth;
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
		if (sh != NULL)
		{
			SetRectSubRectGL(quads + i * 4, sub, Vector2f(ts->texture->getSize()));
		}
		else
		{
			SetRectSubRect(quads + i * 4, sub);
		}
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
		extra += bg->extraOffset;

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
		//Vector2f newCenter = Vector2f(oldView.getCenter().x * depth * .5f, 0);// -extraOffset;
		Vector2f newCenter = Vector2f(oldView.getCenter().x * depth, 0);// -extraOffset;

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

BackgroundTile::BackgroundTile(Background *p_bg, const std::string &p_folder, int p_layer)
	:BackgroundObject( p_bg, p_layer )
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

	if (jobj.contains("shader"))
	{
		string myShaderName = jobj["shader"];
		sh = bg->GetShader(myShaderName);
		assert(sh != NULL);
		shaderName = myShaderName;
	}


	string tsPath = folder + pngName + ".png";

	ts = bg->GetTileset(tsPath);

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


BackgroundWideSpread::BackgroundWideSpread(Background *p_bg, const std::string &p_folder, int p_layer)
	:BackgroundObject(p_bg, p_layer)
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

	ts = bg->GetTileset(tsPath);

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
			if (sh != NULL)
			{
				SetRectSubRectGL(quads + i * 4, sub, Vector2f(ts->texture->getSize()));
			}
			else
			{
				SetRectSubRect(quads + i * 4, sub);
			}
		}

		for (int i = 2; i < 4; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub2.width, sub2.height, Vector2f(realX + loopWidth * (i - 2) + sub.width, myPos.y));

			if (sh != NULL)
			{
				SetRectSubRectGL(quads + i * 4, sub2, Vector2f(ts->texture->getSize()));
			}
			else
			{
				SetRectSubRect(quads + i * 4, sub2);
			}
		}
	}
	else
	{
		SetRectTopLeft(quads, sub.width, sub.height, Vector2f(myPos));
		SetRectTopLeft(quads + 4, sub2.width, sub2.height, Vector2f(myPos.x + sub.width, myPos.y));

		if (sh != NULL)
		{
			SetRectSubRectGL(quads, sub, Vector2f(ts->texture->getSize()));
			SetRectSubRectGL(quads + 4, sub2, Vector2f(ts->texture->getSize()));
		}
		else
		{
			SetRectSubRect(quads, sub);
			SetRectSubRect(quads + 4, sub2);
		}
	}
}


BackgroundWaterfall::BackgroundWaterfall(Background *p_bg, int p_layer )
	:BackgroundObject( p_bg, p_layer )
{
	ts = bg->GetTileset("Backgrounds/W4/w4_01/SpriteSheet3.png");

	shortWaterfall = false;

	actionLength[A_IDLE] = 18;

	animFactor[A_IDLE] = 6;

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

	if (jobj["heightType"] == "short")
	{
		shortWaterfall = true;
	}
	else
	{
		shortWaterfall = false;
	}

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

	numQuads *= 2; //for the foam

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

void BackgroundWaterfall::UpdateQuads(float realX)
{
	int f = frame / animFactor[action];

	IntRect sub;
	IntRect foamSub = ts->GetCustomSubRect(Vector2i(192, 48), Vector2i(872, 1024), Vector2i(3, 6), f);
	sf::Vector2f foamOffset(0, 0);

	float waterfallHeight = 0;
	
	if (shortWaterfall)
	{
		waterfallHeight = 96;
		sub = ts->GetCustomSubRect(Vector2i(32, waterfallHeight), Vector2i(1160, 1312), Vector2i(9, 2), f);
		foamOffset = Vector2f(-80, 60);
	}
	else
	{
		waterfallHeight = 160;
		sub = ts->GetCustomSubRect(Vector2i(32, waterfallHeight), Vector2i(872, 1312), Vector2i(9, 2), f);
		foamOffset = Vector2f(-80, 124);
	}
	
	if (repetitionFactor > 0)
	{
		float depth = DrawLayer::GetDrawLayerDepthFactor(depthLayer);
		float foamRealX = realX + foamOffset.x;// *depth;//Vector2f extra(myPos.x / depth, 0);
		for (int i = 0; i < numQuads / 2; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub.width, sub.height, Vector2f(realX + loopWidth * i, myPos.y));
			SetRectTopLeft(quads + (i + numQuads / 2) * 4, foamSub.width, foamSub.height, Vector2f(foamRealX + loopWidth * i, myPos.y + foamOffset.y));
		}
	}
	else
	{
		for (int i = 0; i < numQuads / 2; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub.width, sub.height, Vector2f(myPos));
			SetRectTopLeft(quads + (i + numQuads / 2) * 4, foamSub.width, foamSub.height, Vector2f(myPos + foamOffset));
		}
	}

	for (int i = 0; i < numQuads / 2; ++i)
	{
		if (sh != NULL)
		{
			SetRectSubRectGL(quads + i * 4, sub, Vector2f(ts->texture->getSize()));
			SetRectSubRectGL(quads + (i + numQuads/2) * 4, foamSub, Vector2f(ts->texture->getSize()));
		}
		else
		{
			SetRectSubRect(quads + i * 4, sub);
			SetRectSubRect(quads + (i + numQuads/2) * 4, foamSub);
		}
	}
}

void BackgroundWaterfall::DrawObject(sf::RenderTarget *target)
{
	target->draw(quads, 4 * numQuads, sf::Quads, ts->texture);
}