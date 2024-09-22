#include "Background.h"
#include "GameSession.h"
#include <sstream>
#include <assert.h>
#include <random>
#include "MainMenu.h"
#include "BackgroundObject.h"
#include "nlohmann\json.hpp"

using namespace std;
using namespace sf;

using json = nlohmann::json;

//depth(p_depthLevel * .01f)

ScrollingBackground::ScrollingBackground(Tileset *p_ts, int index,
	int p_depthLevel, float p_scrollSpeedX)
	:ts(p_ts), depthLevel(p_depthLevel),
	tsIndex(index), scrollSpeedX(p_scrollSpeedX)
{
	

	tsSource = ts->sourceName;

	//testMeColor = Color(rand() % 255, rand() % 255, rand() % 255);

	//MainMenu::GetInstance()->LoadShader(parallaxShader, "parallax");
	//parallaxShader.setUniform("u_texture", *ts->texture);
	//parallaxShader.setUniform("Resolution", Vector2f(1920, 1080));
	//parallaxShader.setUniform("testColor", ColorGL(testMeColor));

	depth = DrawLayer::GetDrawLayerDepthFactor(depthLevel);
	assert(p_ts != NULL);

	
	
	//ts->texture->setSmooth(false);
	SetTileIndex(tsIndex);
	SetLeftPos(Vector2f(-960, 0));
	scrollOffset = 0;
	extraZoom = 1.f;
	extraOffset = Vector2f(0, 0);
}

ScrollingBackground::~ScrollingBackground()
{

}

void ScrollingBackground::SetExtra(Vector2f &p_extra)
{
	extra = p_extra;
}

void ScrollingBackground::Update(const Vector2f &camPos, int updateFrames )
{
	Vector2f cPos = camPos;
	float testScrollOffset = extra.x + scrollOffset;
	cPos.x -= testScrollOffset;
	if (testScrollOffset * depth > 1920)
		testScrollOffset = 0;
	if (testScrollOffset * depth < -1920)
	{
		testScrollOffset = 0;
	}
	int repeatWidth = 1920 * 2;
	int p = floor(cPos.x * depth + .5f);

	int px = abs(p);
	int pxx = (px % repeatWidth) - repeatWidth / 2;

	float camXAbs = abs(cPos.x * depth);
	int m = 0;
	while (camXAbs > 1920)
	{
		camXAbs -= 1920;
		++m;
	}

	float off = camXAbs;

	if (cPos.x > 0)
		off = -off;
	else if (cPos.x < 0)
	{
		off = off - 1920;
	}

	if (p > 0)
		pxx = -pxx;

	if (scrollOffset * depth > 1920)
		scrollOffset = 0;
	if (scrollOffset * depth < -1920)
	{
		scrollOffset = 0;
	}
	scrollOffset += scrollSpeedX * updateFrames;

	//cout << "pxx: " << pxx << ", modified:  " << (pxx / .5f) << endl;
	//pxx = floor( pxx * .5 + .5 );

	//Vector2f realPos( camPos.x + pxx, camPos.y + relPos.y );
	Vector2f realPos(camPos.x + off, camPos.y);
	//realPos = camPos;
	realPos.x -= 960;
	SetLeftPos(realPos);
}

void ScrollingBackground::SetLeftPos(Vector2f &pos)
{
	xPos = pos.x;
	float currX = pos.x;
	float top = -540;//pos.y - 540;l
	float width = 1920;
	float height = 1080;

	for (int i = 0; i < 2; ++i)
	{
		SetRectTopLeft(quad + i * 4, width, height, Vector2f(currX, top));
		currX += width;
	}

	//currX += width;

	/*SetRectTopLeft(quad + 4, width, height, Vector2f(currX, top));

	quad[0].position = Vector2f(currX, top);
	quad[1].position = Vector2f(currX + width, top);
	quad[2].position = Vector2f(currX + width, top + height);
	quad[3].position = Vector2f(currX, top + height);

	quad[4 + 0].position = Vector2f(currX, top + height);
	quad[4 + 1].position = Vector2f(currX + width, top + height);
	quad[4 + 2].position = Vector2f(currX + width, top + height + height);
	quad[4 + 3].position = Vector2f(currX, top + height + height);

	currX += width;*/
}

void ScrollingBackground::SetTileIndex(int index)
{
	tsIndex = index;

	//SetRectSubRect(quad, FloatRect(0, 0, 1, 1));

	IntRect subRect = ts->GetSubRect(tsIndex);

	for (int i = 0; i < 2; ++i)
	{
		SetRectSubRect(quad + i * 4, subRect);
	}

	/*SetRectSubRectGL(quad, subRect, Vector2f(ts->texture->getSize()));
	SetRectSubRectGL(quad + 4, subRect, Vector2f(ts->texture->getSize()));*/
}

void ScrollingBackground::Set(sf::Vector2f &pos, float zoom)
{
	extraOffset = pos;
	extraZoom = zoom;
}

void ScrollingBackground::LayeredDraw(int p_depthLevel, sf::RenderTarget *target)
{
	if (p_depthLevel == depthLevel)
	{
		Draw(target);
	}
}

void ScrollingBackground::Draw(RenderTarget *target)
{
	oldView = target->getView();
	//oldView.setCenter(target->getView().getCenter());
	//oldView.setSize(target->getView().getSize());
	float depth = DrawLayer::GetDrawLayerDepthFactor(depthLevel);

	//newView.setCenter(Vector2f( oldView.getCenter().x, 0) - extraOffset);
	//Vector2f newCenter = oldView.getCenter() *depth;
	Vector2f newCenter = Vector2f(oldView.getCenter().x, 0) - extraOffset;
	/*if (oldView.getCenter().y > 0 )
	{
		newCenter.y = 0;
	}*/
	newView.setCenter(newCenter);//Vector2f(oldView.getCenter().x, oldView.getCenter().y) - extraOffset);
	//newView.setSize(oldView.getSize() / depth);//1920, 1080) / (zoom * extraZoom ));
	newView.setSize(1920, 1080);//1920, 1080) / (zoom * extraZoom ));
	//newView.setRotation(oldView.getRotation());
	
	target->setView(newView);

	//target->draw(va, ts->texture);
	//target->draw(quad, 8, sf::Quads, &parallaxShader);
	target->draw(quad, 8, sf::Quads, ts->texture);

	target->setView(oldView);
}

string Background::GetBGNameFromBGInfo(const std::string &fileName)
{
	ifstream is;
	stringstream fss;
	fss << "Resources/Backgrounds/BGInfo/" << fileName << ".bg";
	string fStr = fss.str();

	is.open(fStr);

	if (is.is_open())
	{
		string bgStr;
		is >> bgStr;

		is.close();

		return bgStr;
	}
	else
	{
		return string("bg error");
	}
}

Background *Background::SetupFullBG(const std::string &fName)
{
	ifstream is;
	stringstream fss;
	fss << "Resources/Backgrounds/BGInfo/" << fName << ".bg";
	string fStr = fss.str();

	string worldStr(1, fName[1]);

	string eStr = ".png";
	string parDirStr = "Backgrounds/W" + worldStr + "/" + fName + "/";

	try {
		is.open(fStr);
	}
	catch (std::system_error &e)
	{
		return NULL;
		//cout << "blizzle: " << e.code().message() << endl;
		//assert(0);
	}








	Background *newBG = NULL;
	string emptyBGName = fName;//"";
	if (is.is_open())
	{
		newBG = new Background(emptyBGName);

		json j;
		is >> j;

		int currLayer = 0;
		string base = "BG_";
		string testStr;
		string typeStr;

		newBG->bgWidth = j["Info"]["envWidth"];

		for (int i = 10; i >= 1; --i)
		{
			currLayer = 10 - i;
			testStr = base + to_string(i);
			if (j.contains(testStr))
			{
				BackgroundLayer *newLayer = new BackgroundLayer(currLayer);

				auto objs = j[testStr];

				for (auto it = objs.begin(); it != objs.end(); ++it)
				{
					assert((*it).contains("type"));

					typeStr = (*it)["type"];

					if (typeStr == "default")
					{
						BackgroundTile *bt = new BackgroundTile(newBG, parDirStr, currLayer);
						bt->Load((*it));
						//newBG->scrollingObjects.push_back(bt);
						newLayer->objectVec.push_back(bt);
					}
					else if (typeStr == "extrawide")
					{
						BackgroundWideSpread *bws = new BackgroundWideSpread(newBG, parDirStr, currLayer);
						bws->Load((*it));
						//newBG->scrollingObjects.push_back(bws);
						newLayer->objectVec.push_back(bws);
					}
					else if (typeStr == "glow")
					{
						BackgroundTileTranscendGlow *bttg = new BackgroundTileTranscendGlow(newBG, parDirStr, currLayer);
						bttg->Load((*it));
						newLayer->objectVec.push_back(bttg);
					}
					else if (typeStr == "waterfall")
					{
						BackgroundWaterfall *bwf = new BackgroundWaterfall(newBG, currLayer);
						bwf->Load((*it));
						newLayer->objectVec.push_back(bwf);
						//newBG->scrollingObjects.push_back(bwf);
					}
				}

				newLayer->SetupQuads();

				newBG->bgLayerList.push_back(newLayer);
			}
		}

		is.close();
	}
	
	return newBG;
}

Background::Background(const string &bgName)
{
	name = bgName;	

	char worldChar = bgName[1];

	envWorld = (worldChar - 1) - '0';

	bgView.setCenter(0, 0);
	bgView.setSize(1920, 1080);

	bgWidth = 0;

	transFrames = 0;

	show = true;

	Reset();
}

Background::Background()
{
	transFrames = 0;
	//stringstream ss;

	//string folder = "Menu/Title/";

	////int eType = envLevel + 1; //adjust for alex naming -_-
	////ss << folder << "w" << envType + 1 << "_BG";

	//show = true;
	////ss << eType;

	//string bgStr = "Resources/Backgrounds/W1/w1_01/w1_BG1";// = ss.str();


	//ts_sky = NULL;

	//bgSourceName = folder + "title_base_1920x1080.png";
	//string paletteFile = bgStr + "_palette.png";
	//shapeSourceName = folder + "titleshade_1920x1080.png";//bgStr + "_shape.png";

	//ts_bg = NULL;
	//ts_shape = GetTileset(shapeSourceName, 1920, 1080);
	//paletteLoaded = palette.loadFromFile(paletteFile);
	////assert(loadPalette);

	////background.setTexture(*ts_bg->texture);
	////background.setOrigin(background.getLocalBounds().width / 2, background.getLocalBounds().height / 2);
	////background.setPosition(0, 0);

	//SetRectCenter(backgroundSky, 1920, 1080, Vector2f(0, 0));

	//if (ts_shape != NULL)
	//{
	//	shape.setTexture(*ts_shape->texture);
	//	shape.setOrigin(shape.getLocalBounds().width / 2, shape.getLocalBounds().height / 2);
	//	shape.setPosition(0, 0);
	//	shape.setScale(2, 2);
	//}
	//

	//transFrames = 60 * 3;

	//bgView.setCenter(0, 0);
	//bgView.setSize(1920, 1080);

	//show = true;
	transFrames = 60 * 3;
	show = false;

	Reset();
}

Background::~Background()
{
	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		delete (*it);
	}
	scrollingBackgrounds.clear();

	for (auto it = scrollingObjects.begin(); it != scrollingObjects.end(); ++it)
	{
		delete (*it);
	}
	scrollingObjects.clear();

	for (auto it = bgLayerList.begin(); it != bgLayerList.end(); ++it)
	{
		delete (*it);
	}
	bgLayerList.clear();


	for (auto it = shaderMap.begin(); it != shaderMap.end(); ++it)
	{
		delete (*it).second;
	}
	shaderMap.clear();
}

void Background::Set(Vector2f &pos, float zoom )
{
	bgView.setCenter(-pos);
	bgView.setSize(1920 / zoom, 1080 / zoom);

	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Set(pos, zoom);
	}
}

void Background::SetExtra(sf::Vector2f &p_extra)
{
	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->SetExtra(p_extra);
	}
}

void Background::Update( const Vector2f &camPos, int updateFrames )
{
	//UpdateSky();
	//UpdateShape();

	frame += updateFrames;

	if (transFrames > 0)
	{
		if (frame >= transFrames * 4)
		{
			frame = frame % (transFrames * 4);
		}
	}

	for (auto it = shaderMap.begin(); it != shaderMap.end(); ++it)
	{
		if ((*it).first == "transcend_bg_energy")
		{
			int amt = 120;
			float famt = amt;
			float q = (frame % amt) / famt;
			(*it).second->setUniform("quant", q);
		}
	}

	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Update(camPos, updateFrames );
	}

	for (auto it = scrollingObjects.begin();
		it != scrollingObjects.end(); ++it)
	{
		(*it)->Update( camPos, updateFrames);
	}

	for (auto it = bgLayerList.begin();
		it != bgLayerList.end(); ++it)
	{
		for (auto it2 = (*it)->objectVec.begin(); it2 != (*it)->objectVec.end(); ++it2)
		{
			(*it2)->Update(camPos, updateFrames);
		}
		
	}
}

void Background::Reset()
{
	if (transFrames > 0)
	{
		int r = rand() % 4;
		int r2 = rand() % transFrames;
		frame = r * transFrames + r2;
	}
	else
	{
		frame = 0;
	}

	SetExtra(Vector2f( 0, 0 ));
}

void Background::Draw(sf::RenderTarget *target)
{
	if (!show)
		return;

	sf::View oldView = target->getView();

	sf::View newView = bgView;
	//newView.setRotation(oldView.getRotation());

	target->setView(newView);

	/*if (ts_sky != NULL)
	{
		target->draw(backgroundSky, 4, sf::Quads, ts_sky->texture);
	}
	else
	{
		target->draw(backgroundSky, 4, sf::Quads);
	}
	
	if (ts_shape != NULL)
	{
		target->draw(shape);
	}
	
	if (ts_bg != NULL)
	{
		target->draw(background);
	}*/
	

	target->setView(oldView);

	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		//might not need to mess w/ views here anymore
		(*it)->Draw(target);
	}

	//target->setView(newView);

	/*sf::Vertex testQuad[4];
	SetRectCenter(testQuad, 1920, 1080, Vector2f(0, 0));
	SetRectColor(testQuad, Color(0, 0, 0, 50));
	target->draw(testQuad, 4, sf::Quads);*/

	//target->setView(oldView);
	
}

void Background::DrawBackLayer(sf::RenderTarget *target)
{
	if (!show)
		return;

	//if (ts_bg != NULL)
	{
		sf::View oldView = target->getView();

		sf::View newView = bgView;
		//newView.setRotation(oldView.getRotation());

		target->setView(newView);

		//target->draw(background);

		target->setView(oldView);
	}
}

void Background::LayeredDraw(int p_drawLayer, sf::RenderTarget *target)
{
	if (!show)
	{
		return;
	}

	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		//might not need to mess w/ views here anymore
		(*it)->LayeredDraw( p_drawLayer, target);
	}

	for (auto it = scrollingObjects.begin();
		it != scrollingObjects.end(); ++it)
	{
		//might not need to mess w/ views here anymore
		(*it)->LayeredDraw(p_drawLayer, target);
	}

	for (auto it = bgLayerList.begin(); it != bgLayerList.end(); ++it)
	{
		if ((*it)->drawLayer == p_drawLayer)
		{
			(*it)->Draw(target);
		}
	}
}

sf::Shader * Background::GetShader(const std::string &shaderName)
{
	if (shaderMap.count(shaderName) != 0)
	{
		return shaderMap[shaderName];
	}
	else
	{
		sf::Shader *newShader = new sf::Shader;
		string shaderPath = "Resources/Shader/";
		shaderPath += shaderName + ".frag";
		newShader->loadFromFile(shaderPath, sf::Shader::Fragment);
		shaderMap[shaderName] = newShader;

		if (shaderName == "transcend_bg_energy")
		{
			Tileset *scrollTS = GetSizedTileset("Backgrounds/W1/w1_01/vein_energy_1920x1080.png");
			shaderTilesetMap[shaderName].push_back(scrollTS);
			newShader->setUniform("u_scrollTexture", *scrollTS->texture);

			newShader->setUniform("quant", 0.f);
		}

		return newShader;
	}
}

void Background::Show()
{
	show = true;
}

void Background::Hide()
{
	show = false;
}

void Background::FlipShown()
{
	show = !show;
}