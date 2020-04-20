//edit mode

#include "GUI.h"
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "Physics.h"
#include "Action.h"
#include <set>
#include "GameSession.h"
#include "ActorParams.h"
#include "Action.h"
#include "earcut.hpp"
#include "TransformTools.h"
#include "TerrainDecor.h"

#include "TouchGrass.h"

using namespace std;
using namespace sf;

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

namespace mapbox
{
	namespace util
	{
		template<>
		struct nth<0, TerrainPoint> {
			inline static auto get(const TerrainPoint &t) {
				return t.pos.x;
			}
		};

		template<>
		struct nth<1, TerrainPoint> {
			inline static auto get(const TerrainPoint &t)
			{
				return t.pos.y;
			}
		};
	}
}

#define cout std::cout

void TerrainPolygon::AddTouchGrass(int gt)
{
	list<PlantInfo> info;

	TouchGrass::TouchGrassType gType = (TouchGrass::TouchGrassType)gt;

	int tw = TouchGrass::GetQuadWidth(gType);

	//Edge *startEdge = tr->startEdge;

	int numP = GetNumPoints();

	Edge *currEdge;
	for (int i = 0; i < numP; ++i)
	{
		currEdge = GetEdge(i);
		bool valid = true;

		EdgeAngleType eat = GetEdgeAngleType(currEdge);
		if (eat == EDGE_FLAT || eat == EDGE_SLOPED)
		{
			valid = true;
		}
		else
		{
			valid = false;
		}
		//valid = true;

		if (valid)
		{
			double len = currEdge->GetLength();
			int numQuads = len / tw;
			bool tooThin = false;
			double quadWidth = tw;//len / numQuads;
			if (numQuads == 0)
			{
				tooThin = true;
				numQuads = 1;
			}
			if (numQuads > 0)
			{
				for (int i = 0; i < numQuads; ++i)
				{
					if (TouchGrass::IsPlacementOkay(gType, eat,
						currEdge, i))
					{
						if (tooThin)
						{
							info.push_back(PlantInfo(currEdge, len / 2.0, quadWidth));
						}
						else
						{
							info.push_back(PlantInfo(currEdge, quadWidth * i + quadWidth / 2, quadWidth));
						}

					}
				}
			}
		}
	}

	int infoSize = info.size();
	int vaSize = infoSize * 4;

	if (infoSize == 0)
	{
		return;
	}

	TouchGrassCollection *coll = new TouchGrassCollection(this);
	touchGrassCollections.push_back(coll);

	//cout << "number of plants: " << infoSize << endl;
	coll->touchGrassVA = new Vertex[vaSize];

	coll->ts_grass = TouchGrassCollection::GetTileset(sess, gType);
	coll->gType = gType;
	coll->numTouchGrasses = infoSize;

	int vaIndex = 0;
	for (list<PlantInfo>::iterator it = info.begin(); it != info.end(); ++it)
	{
		coll->CreateGrass(vaIndex, (*it).edge, (*it).quant);
		vaIndex++;
	}
}

void TerrainPolygon::QueryTouchGrass(QuadTreeCollider *qtc, sf::Rect<double> &r)
{
	V2d center = GetDCenter();
	sf::Rect<double> rAdjusted = r;
	rAdjusted.left -= center.x;
	rAdjusted.top -= center.y;
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->Query(qtc, rAdjusted);
	}
}

void TerrainPolygon::UpdateTouchGrass()
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->UpdateGrass();
	}
}

void TerrainPolygon::DrawTouchGrass(sf::RenderTarget *target)
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void TerrainPolygon::DestroyTouchGrass()
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		delete (*it);
	}
	touchGrassCollections.clear();
}

void TerrainPolygon::ResetTouchGrass()
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->Reset();
	}
}





void TerrainPolygon::AddDecorExpression(DecorExpression *exp)
{
	decorExprList.push_back(exp);
}

bool TerrainPolygon::IsEmptyRect(sf::Rect<double> &rect)
{
	emptyResult = true;
	decorTree->Query(this, rect);
	myTerrainTree->Query(this, rect);
	return emptyResult;
}

bool TerrainPolygon::CheckRectIntersectEdges(
	sf::Rect<double> &r)
{
	emptyResult = true;
	myTerrainTree->Query(this, r);
	return !emptyResult;
}



void TerrainPolygon::DrawDecor(sf::RenderTarget *target)
{
	for (list<DecorExpression*>::iterator it = decorExprList.begin();
		it != decorExprList.end(); ++it)
	{
		Tileset *ts = (*it)->layer->ts;
		target->draw(*(*it)->va, ts->texture);
	}
}

DecorType TerrainPolygon::GetDecorType(const std::string &dStr)
{
	if (dStr == "D_W1_BUSH_NORMAL")
	{
		return D_W1_BUSH_NORMAL;
	}
	else if (dStr == "D_W1_ROCK_1")
	{
		return D_W1_ROCK_1;
	}
	else if (dStr == "D_W1_ROCK_2")
	{
		return D_W1_ROCK_2;
	}
	else if (dStr == "D_W1_ROCK_3")
	{
		return D_W1_ROCK_3;
	}
	else if (dStr == "D_W1_PLANTROCK")
	{
		return D_W1_PLANTROCK;
	}
	else if (dStr == "D_W1_VEINS1")
	{
		return D_W1_VEINS1;
	}
	else if (dStr == "D_W1_VEINS2")
	{
		return D_W1_VEINS2;
	}
	else if (dStr == "D_W1_VEINS3")
	{
		return D_W1_VEINS3;
	}
	else if (dStr == "D_W1_VEINS4")
	{
		return D_W1_VEINS4;
	}
	else if (dStr == "D_W1_VEINS5")
	{
		return D_W1_VEINS5;
	}
	else if (dStr == "D_W1_VEINS6")
	{
		return D_W1_VEINS6;
	}
	else if (dStr == "D_W1_GRASSYROCK")
	{
		return D_W1_GRASSYROCK;
	}
	else if (dStr == "D_W2_VEINS1")
	{
		return D_W2_VEINS1;
	}
	else if (dStr == "D_W2_VEINS2")
	{
		return D_W2_VEINS2;
	}
	else if (dStr == "D_W2_VEINS3")
	{
		return D_W2_VEINS3;
	}
	else if (dStr == "D_W2_VEINS4")
	{
		return D_W2_VEINS4;
	}
	else if (dStr == "D_W2_VEINS5")
	{
		return D_W2_VEINS5;
	}
	else if (dStr == "D_W2_VEINS6")
	{
		return D_W2_VEINS6;
	}
	else if (dStr == "D_W2_BUSH0")
	{
		return D_W2_BUSH0;
	}
	else if (dStr == "D_W2_BUSH1")
	{
		return D_W2_BUSH1;
	}
	else if (dStr == "D_W2_BUSH2")
	{
		return D_W2_BUSH2;
	}
	else if (dStr == "D_W2_BUSH3")
	{
		return D_W2_BUSH3;
	}
	else if (dStr == "D_W2_BUSH4")
	{
		return D_W2_BUSH4;
	}
	else if (dStr == "D_W2_BUSH5")
	{
		return D_W2_BUSH5;
	}
	else if (dStr == "D_W2_BUSH6")
	{
		return D_W2_BUSH6;
	}
	else if (dStr == "D_W2_BUSH7")
	{
		return D_W2_BUSH7;
	}
	else if (dStr == "D_W2_BUSH8")
	{
		return D_W2_BUSH8;
	}
	else if (dStr == "D_W2_BUSH9")
	{
		return D_W2_BUSH9;
	}
	else if (dStr == "D_W2_BUSH10")
	{
		return D_W2_BUSH10;
	}
	else if (dStr == "D_W2_BUSH11")
	{
		return D_W2_BUSH11;
	}
	else if (dStr == "D_W2_BUSH12")
	{
		return D_W2_BUSH12;
	}
	else if (dStr == "D_W2_BUSH13")
	{
		return D_W2_BUSH13;
	}
	else if (dStr == "D_W2_BUSH14")
	{
		return D_W2_BUSH14;
	}
	else if (dStr == "D_W2_BUSH15")
	{
		return D_W2_BUSH15;
	}
	else if (dStr == "D_W2_BUSH_1_0")
	{
		return D_W2_BUSH_1_0;
	}
	else if (dStr == "D_W2_BUSH_1_1")
	{
		return D_W2_BUSH_1_1;
	}
	else if (dStr == "D_W2_BUSH_1_2")
	{
		return D_W2_BUSH_1_2;
	}
	else if (dStr == "D_W2_BUSH_1_3")
	{
		return D_W2_BUSH_1_3;
	}
	else if (dStr == "D_W2_BUSH_1_4")
	{
		return D_W2_BUSH_1_4;
	}
	else if (dStr == "D_W2_BUSH_1_5")
	{
		return D_W2_BUSH_1_5;
	}
	else if (dStr == "D_W2_BUSH_1_6")
	{
		return D_W2_BUSH_1_6;
	}
	else
	{
		assert(0);
		return D_W1_BUSH_NORMAL;
	}
}

void TerrainPolygon::GenerateDecor()
{
	/*if (tdInfo != NULL)
	{

	}*/

	if (tdInfo != NULL)
	{
		int numDecors = tdInfo->numDecors;
		DecorExpression *expr;
		for (int i = 0; i < numDecors; ++i)
		{
			expr = CreateDecorExpression(tdInfo->decors[i], 0);
			if (expr != NULL)
				AddDecorExpression(expr);
		}
	}
	else
	{

	}
}

void TerrainPolygon::UpdateDecorSprites()
{
	for (list<DecorExpression*>::iterator it = decorExprList.begin();
		it != decorExprList.end(); ++it)
	{
		(*it)->UpdateSprites();
	}
}



DecorExpression * TerrainPolygon::CreateDecorExpression(DecorType dType,
	int bgLayer)
{
	int minApart;
	int maxApart;
	CubicBezier apartBezier;
	int minPen;
	int maxPen;
	CubicBezier penBez;
	int animFactor = 1;

	switch (dType)
	{
	case D_W1_BUSH_NORMAL:
		minApart = 20;
		maxApart = 300;
		apartBezier = CubicBezier(0, 0, 1, 1);
		minPen = 20;
		maxPen = 1000;
		penBez = CubicBezier(0, 0, 1, 1);
		animFactor = 8;
		break;
	case D_W1_ROCK_1:
	case D_W1_ROCK_2:
	case D_W1_ROCK_3:
	case D_W1_PLANTROCK:
	case D_W1_GRASSYROCK:
	default:
		minApart = 500;
		maxApart = 700;
		apartBezier = CubicBezier(0, 0, 1, 1);
		minPen = 200;
		maxPen = 1200;
		penBez = CubicBezier(0, 0, 1, 1);
		animFactor = 1;
		break;
	}

	//assert( positions.empty() );

	int veinLoopWait = 30;

	DecorLayer *layer = NULL;
	Tileset *ts_d = NULL;
	if (sess->decorLayerMap.count(dType) == 0)
	{
		//int GameSession::TerrainPiece::bushFrame = 0;
		//int GameSession::TerrainPiece::bushAnimLength = 20;
		//int GameSession::TerrainPiece::bushAnimFactor = 8;


		switch (dType)
		{
		case D_W1_BUSH_NORMAL:
			ts_d = sess->GetTileset("Env/bush_01_64x64.png", 64, 64);
			layer = new DecorLayer(ts_d, 20, 8);
			break;
		case D_W1_ROCK_1:
			ts_d = sess->GetTileset("Env/rock_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_2:
			ts_d = sess->GetTileset("Env/rock_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_3:
			ts_d = sess->GetTileset("Env/rock_1_03_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_PLANTROCK:
			ts_d = sess->GetTileset("Env/bush_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_GRASSYROCK:
			ts_d = sess->GetTileset("Env/bush_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_VEINS1:
			ts_d = sess->GetTileset("Env/veins_w1_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS2:
			ts_d = sess->GetTileset("Env/veins_w1_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS3:
			ts_d = sess->GetTileset("Env/veins_w1_3_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS4:
			ts_d = sess->GetTileset("Env/veins_w1_4_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS5:
			ts_d = sess->GetTileset("Env/veins_w1_5_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS6:
			ts_d = sess->GetTileset("Env/veins_w1_6_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS1:
			ts_d = sess->GetTileset("Env/veins_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS2:
			ts_d = sess->GetTileset("Env/veins_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS3:
			ts_d = sess->GetTileset("Env/veins_w2_3_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS4:
			ts_d = sess->GetTileset("Env/veins_w2_4_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS5:
			ts_d = sess->GetTileset("Env/veins_w2_5_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS6:
			ts_d = sess->GetTileset("Env/veins_w2_6_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_BUSH0:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 0);
			break;
		case D_W2_BUSH1:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 1);
			break;
		case D_W2_BUSH2:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 2);
			break;
		case D_W2_BUSH3:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 3);
			break;
		case D_W2_BUSH4:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 4);
			break;
		case D_W2_BUSH5:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 5);
			break;
		case D_W2_BUSH6:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 6);
			break;
		case D_W2_BUSH7:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 7);
			break;
		case D_W2_BUSH8:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 8);
			break;
		case D_W2_BUSH9:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 9);
			break;
		case D_W2_BUSH10:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 10);
			break;
		case D_W2_BUSH11:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 11);
			break;
		case D_W2_BUSH12:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 12);
			break;
		case D_W2_BUSH13:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 13);
			break;
		case D_W2_BUSH14:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 14);
			break;
		case D_W2_BUSH15:
			ts_d = sess->GetTileset("Env/bushes_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 15);
			break;
		case D_W2_BUSH_1_0:
			ts_d = sess->GetTileset("Env/bushes_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 0);
			break;
		case D_W2_BUSH_1_1:
			ts_d = sess->GetTileset("Env/bushes_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 1);
			break;
		case D_W2_BUSH_1_2:
			ts_d = sess->GetTileset("Env/bushes_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 2);
			break;
		case D_W2_BUSH_1_3:
			ts_d = sess->GetTileset("Env/bushes_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 3);
			break;
		case D_W2_BUSH_1_4:
			ts_d = sess->GetTileset("Env/bushes_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 4);
			break;
		case D_W2_BUSH_1_5:
			ts_d = sess->GetTileset("Env/bushes_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 5);
			break;
		case D_W2_BUSH_1_6:
			ts_d = sess->GetTileset("Env/bushes_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 1, 1, 6);
			break;
		}

		sess->decorLayerMap[dType] = layer;
	}
	else
	{
		layer = sess->decorLayerMap[dType];
		ts_d = layer->ts;
	}



	assert(layer != NULL);
	list<Vector2f> positions;
	//int minDistanceApart = 10;
	//int maxDistanceApart = 300;
	//int minPen = 20;
	//int maxPen = 200;
	double penLimit;

	
	double quant = 0;
	

	double travelDistance;
	double penDistance;
	int diffApartMax = maxApart - minApart;
	int diffPenMax = maxPen - minPen;
	int r;
	int rPen;
	double momentum;
	V2d pos;

	bool loopOver = false;
	V2d cn;

	rayMode = "decor";
	QuadTree *qt = myTerrainTree;

	assert(qt != NULL);

	Edge *curr = GetEdge(0);

	int numP = GetNumPoints();
	int i = 0;

	double lenCurr = curr->GetLength();

	std::list<DecorRect*> currDecorRects;

	while (true)
	{
		curr = GetEdge(i);

		r = rand() % diffApartMax;
		travelDistance = minApart + r;

		momentum = travelDistance;

		while (!approxEquals(momentum, 0))
		{
			if ((lenCurr - quant) > momentum)
			{
				quant += momentum;
				momentum = 0;
			}
			else
			{
				++i;
				

				if (i == numP)
				{
					loopOver = true;
					break;
				}
				else
				{
					momentum = momentum - (lenCurr - quant);
					quant = 0;

					curr = GetEdge(i);
					lenCurr = curr->GetLength();
				}
			}
		}

		if (loopOver)
			break;

		cn = curr->Normal();
		rcEdge = NULL;
		rayStart = curr->GetPosition(quant);

		rPen = rand() % diffPenMax;
		penDistance = minPen + rPen; //minpen times 2 cuz gotta account for the other side too

		rayEnd = rayStart - cn * (penDistance + minPen);
		ignoreEdge = curr;

		rcPortion = 9999999;
		RayCast(this, qt->startNode, rayStart, rayEnd);

		if (rcEdge != NULL)
		{
			V2d rcPos = rcEdge->GetPosition(rcQuant);
			continue;
		}

		pos = curr->GetPosition(quant) - curr->Normal() * penDistance;


		sf::Rect<double> testRect;
		testRect.left = pos.x - ts_d->tileWidth / 2;
		testRect.top = pos.y - ts_d->tileHeight / 2;
		testRect.width = ts_d->tileWidth;
		testRect.height = ts_d->tileHeight;

		if (IsEmptyRect(testRect))
		{
			positions.push_back(Vector2f(pos.x, pos.y));
			DecorRect *dr = new DecorRect(testRect);
			currDecorRects.push_back(dr);
			decorTree->Insert(dr);
		}
	}

	if (positions.size() == 0)
		return NULL;

	DecorExpression *expr = new DecorExpression(positions, layer);

	decorTree->Clear();

	for (auto it = currDecorRects.begin(); it != currDecorRects.end(); ++it)
	{
		delete (*it);
	}

	return expr;
}

void TerrainPolygon::HandleEntrant(QuadTreeEntrant *qte)
{
	emptyResult = false;
}

void TerrainPolygon::HandleRayCollision(Edge *edge,
	double edgeQuantity, double rayPortion)
{
	if (edge == ignoreEdge)
	{
		return;
	}

	double len = length(edge->GetPosition(edgeQuantity) - rayStart);
	if (rcEdge == NULL || len < rcPortion)
	{
		rcEdge = edge;
		rcPortion = len;
		rcQuant = edgeQuantity;
	}

	/*if (edge == ignoreEdge || edge->edge1 == ignoreEdge )
	return;

	if (rayPortion < rcPortion)
	{
	rcEdge = edge;
	rcPortion = rayPortion;
	rcQuant = edgeQuantity;
	}*/
}


void BorderSizeInfo::SetWidth(int w)
{
	width = w;
	startInter = 4;
	inter = TerrainPolygon::GetBorderQuadIntersect(width);
	len = width - inter * 2;
	edgeLen = width - inter;
	startLen - width - inter;
}

TerrainPolygon::TerrainPolygon()
	:ISelectable( ISelectable::TERRAIN )
{
	decorTree = new QuadTree(1000000, 1000000);
	myTerrainTree = new QuadTree(1000000, 1000000);
	tdInfo = NULL;
	sess = Session::GetSession();

	renderMode = RENDERMODE_NORMAL;
	totalNumBorderQuads = 0;
	ts_border = NULL;
	borderQuads = NULL;

	inverse = false;
	layer = 0;
	va = NULL;
	lines = NULL;
	selected = false;
	grassVA = NULL;
	
	isGrassShowing = false;
	finalized = false;
	terrainWorldType = MOUNTAIN;
	terrainVariation = 0;
	ts_grass = NULL;
	pointVector.resize(2);

	if (sess != NULL)
	{
		ts_grass = sess->GetTileset("Env/grass_128x128.png", 128, 128);
	}
	
	grassSize = 128;
	grassSpacing = -60;
}

TerrainPolygon::TerrainPolygon(TerrainPolygon &poly, bool pointsOnly, bool storeSelectedPoints )
	:ISelectable(ISelectable::TERRAIN)
{
	decorTree = new QuadTree(1000000, 1000000);
	myTerrainTree = new QuadTree(1000000, 1000000);
	tdInfo = NULL;
	sess = poly.sess;
	layer = 0;
	inverse = poly.inverse;
	terrainWorldType = poly.terrainWorldType;
	terrainVariation = poly.terrainVariation;
	ts_grass = poly.ts_grass;
	ts_border = poly.ts_border;
	renderMode = poly.renderMode;

	grassSize = 128;
	grassSpacing = -60;

	pointVector.resize(2);

	if (pointsOnly)
	{
		totalNumBorderQuads = 0;
		ts_border = NULL;
		borderQuads = NULL;

		va = NULL;
		lines = NULL;
		selected = false;
		grassVA = NULL;
		isGrassShowing = false;
		finalized = false;
		CopyPoints(&poly, storeSelectedPoints);
	}
	else
	{

		assert(false && "havent implemented yet");
	}
}

TerrainPolygon::~TerrainPolygon()
{
	//cout << "destroying poly: " << this << endl;

	if (lines != NULL)
		delete[] lines;

	if (va != NULL)
		delete va;

	if (grassVA != NULL)
		delete grassVA;

	if (borderQuads != NULL)
		delete[] borderQuads;

	delete decorTree;
	delete myTerrainTree;

	DestroyTouchGrass();

	for (auto it = decorExprList.begin(); it != decorExprList.end(); ++it)
	{
		delete (*it);
	}

	ClearPoints();
}

bool TerrainPolygon::IsFlatGround(sf::Vector2<double> &normal)
{
	return (normal.x == 0);
}

bool TerrainPolygon::IsSlopedGround(sf::Vector2<double> &normal)
{
	return (abs(normal.y) > GetSteepThresh() && normal.x != 0);
}

bool TerrainPolygon::IsSteepGround(sf::Vector2<double> &normal)
{
	double wallThresh = EditSession::PRIMARY_LIMIT;
	return (abs(normal.y) <= GetSteepThresh() && abs(normal.x) < wallThresh);
}

bool TerrainPolygon::IsWall(sf::Vector2<double> &normal)
{
	double wallThresh = EditSession::PRIMARY_LIMIT;
	return (abs(normal.x) >= wallThresh);
}

TerrainPolygon::EdgeAngleType TerrainPolygon::GetEdgeAngleType(int index)
{
	V2d norm = GetEdge(index)->Normal();
	return GetEdgeAngleType(norm);
}

TerrainPolygon::EdgeAngleType TerrainPolygon::GetEdgeAngleType(Edge * e)
{
	V2d norm = e->Normal();
	return GetEdgeAngleType(norm);
}

TerrainPolygon::EdgeAngleType TerrainPolygon::GetEdgeAngleType(V2d &normal)
{
	bool facingUp = (normal.y < 0);
	if (IsFlatGround(normal))
	{
		if (facingUp)
			return EDGE_FLAT;
		else
			return EDGE_FLATCEILING;
	}
	else if (IsSlopedGround(normal))
	{
		if (facingUp)
		{
			return EDGE_SLOPED;
		}
		else
		{
			return EDGE_SLOPEDCEILING;
		}
	}
	else if (IsSteepGround(normal))
	{
		if (facingUp)
		{
			return EDGE_STEEPSLOPE;
		}
		else
		{
			return EDGE_STEEPCEILING;
		}
	}
	else if (IsWall(normal))
	{
		return EDGE_WALL;
	}
	else
	{
		cout << "bad edge normal: " << normal.x << ", " << normal.y << endl;
		assert(0 && "couldn't find edge angle type");
		return EDGE_FLAT;
	}
}

int TerrainPolygon::GetBorderQuadIntersect(int tileWidth)
{
	switch (tileWidth)
	{
	case 16:
		return 4;//4;
		break;
	case 32:
		return 8;//10;
		break;
	case 128:
		return 16;//20;
		break;
	default:
		assert(0);
		break;
	}
}

double TerrainPolygon::GetExtraForInward(Edge *e)
{
	double factor = 0.0;//40;//40.0;
	V2d currDir = e->Along();
	V2d prevDir = -e->GetPrevEdge()->Along();

	double cDiff = GetVectorAngleDiffCCW(currDir, prevDir);
	bool turnInward = cDiff < PI;

	if (turnInward)
	{
		return factor * (PI - cDiff) / PI;
	}
	else
	{
		return 0;
	}
}

double TerrainPolygon::GetSubForOutward(Edge *e)
{
	double factor = 160.0;
	V2d currDir = e->Along();
	V2d prevDir = -e->GetPrevEdge()->Along();
	double cDiff = GetVectorAngleDiffCCW(currDir, prevDir);
	bool turnOutward = cDiff > PI * 1.5 + .1;

	if (turnOutward)
	{
		return factor * (cDiff - PI * 1.5) / (PI / 2);
	}
	else
	{
		return 0;
	}
}

bool TerrainPolygon::IsAcute(Edge *e) //edge and its previous edge
{
	V2d currDir = e->Along();
	V2d prevDir = -e->GetPrevEdge()->Along();
	return dot(prevDir, currDir) > .6 && cross(prevDir, currDir) < 0;
}

V2d TerrainPolygon::GetBisector(Edge *e)
{
	V2d currDir = e->Along();
	V2d prevDir = -e->GetPrevEdge()->Along();
	return normalize(currDir + prevDir);
}

void TerrainPolygon::SetBorderTileset()
{
	//basically theres a way to make this invisible, but haven't set it up yet.
	//just return and set the border to NULL in this case.

	stringstream ss;

	ss << "Borders/bor_" << terrainWorldType + 1 << "_";
	ss << "01_512x512";

	//this is if we get more border variations
	/*if (terrainVariation < 10)
	{
	ss << "0" << terrainVariation + 1;
	}
	else
	{
	ss << terrainVariation + 1;
	}*/
	ss << ".png";

	ts_border = sess->GetTileset(ss.str(), 128, 64);
}

sf::IntRect TerrainPolygon::GetBorderSubRect(int tileWidth, EdgeAngleType angleType, int var)
{
	sf::IntRect ir;
	ir.width = tileWidth;
	ir.height = 64;

	//this is because atm theres no flat ceiling version. get one later, then this 
	//can be simplified.
	int adjustedType = angleType;
	if (adjustedType > 0)
		adjustedType--;

	switch (tileWidth)
	{
	/*case 16:
	{
		if (angleType < EDGE_STEEPCEILING)
		{
			ir.top = 7 * 64;
			ir.left = 128 * 2 + 64 * adjustedType + 16 * var;
		}
		else
		{
			ir.top = 8 * 64;
			ir.left = 64 * (adjustedType - EDGE_STEEPCEILING) + 16 * var;
		}
		break;
	}*/
	case 32:
	{
		if (angleType < EDGE_STEEPCEILING)
		{
			ir.top = 6 * 64;
			ir.left = 128 * adjustedType + 32 * var;
		}
		else
		{
			ir.top = 7 * 64;
			ir.left = 128 * (adjustedType - (EDGE_STEEPCEILING-1)) + 32 * var;
		}
		break;
	}
	case 128:
	{
		ir.left = var * 128;
		ir.top = adjustedType * 64;
		break;
	}
	//case 64:
	//{
	//	if (angleType < E_TRANS_SLOPED_TO_STEEP_CEILING)
	//	{
	//		ir.top = 8 * 64;
	//		ir.left = 128 + 64 * angleType;
	//	}
	//	else if (angleType < E_TRANS_WALL_TO_SLOPED_CEILING)
	//	{
	//		ir.top = 9 * 64;
	//		ir.left = 64 * (angleType - E_TRANS_WALL_TO_SLOPED_CEILING);
	//	}
	//	else
	//	{
	//		//just one on this row for now
	//		ir.top = 10 * 64;
	//		ir.left = 0;
	//	}

	//	break;
	//}
	default:
	{
		assert(0);
		break;
	}
	}

	return ir;
}

void TerrainPolygon::GenerateBorderMesh()
{
	if (ts_border == NULL)
	{
		/*if (borderQuads != NULL)
		{
			delete[] borderQuads;
			borderQuads = NULL;
		}*/
		return;
	}

	BorderInfo totalQuadCounts;

	map<Edge*, BorderInfo> numQuadMap[EDGE_WALL + 1];

	//list<Edge*> transEdges;

	BorderSizeInfo borderSizes[BorderInfo::NUM_BORDER_SIZES];
	borderSizes[0].SetWidth(128);
	borderSizes[1].SetWidth(32);
	//borderSizes[2].SetWidth(16);

	int firstType;
	int secondType;


	int numP = GetNumPoints();
	EdgeAngleType angleType;
	int edgeLen;
	Edge *edge;

	BorderInfo currEdgeQuadCounts;
	int currSizeLen;

	//get the number of different border sizes we need for this edge
	for (int i = 0; i < numP; ++i)
	{
		edge = GetEdge(i);
		angleType = GetEdgeAngleType(edge);
		edgeLen = edge->GetLength() + GetExtraForInward(edge)
			+ GetExtraForInward(edge->GetNextEdge());

		if (edgeLen <= 0)
			continue;

		currEdgeQuadCounts.Clear();
		for (int j = 0; j < BorderInfo::NUM_BORDER_SIZES; ++j)
		{
			currSizeLen = borderSizes[j].len;
			currEdgeQuadCounts.numQuads[j] += edgeLen / currSizeLen;
			edgeLen = edgeLen % currSizeLen;
			if (edgeLen > 0 && j == BorderInfo::NUM_BORDER_SIZES - 1)
			{
				currEdgeQuadCounts.numQuads[j]++;
			}
		}

		totalQuadCounts.Add(currEdgeQuadCounts);
		numQuadMap[angleType][edge] = currEdgeQuadCounts;
	}

	totalNumBorderQuads = totalQuadCounts.GetTotal();
	//totalNumBorderQuads += transEdges.size();

	if (totalNumBorderQuads == 0)
	{
		/*if (borderQuads != NULL)
		{
			delete[] borderQuads;
			borderQuads = NULL;
		}*/
		return;
	}

	borderQuads = new Vertex[totalNumBorderQuads * 4];

	int totalsPerType[EDGE_WALL + 1] = { 0 };
	int currentOffsetPerType[EDGE_WALL + 1] = { 0 };

	//get the totals of each type of edge
	for (int i = 0; i < EDGE_WALL + 1; ++i)
	{
		auto &qmRef = numQuadMap[i];
		for (auto it = qmRef.begin(); it != qmRef.end(); ++it)
		{
			totalsPerType[i] += (*it).second.GetTotal();
		}
	}

	sf::Vertex *borderQuadsByType[EDGE_WALL + 1];
	//sf::Vertex *transva;


	//transva = (borderQuads + start * 4);
	//start += transEdges.size();

	//setup the starting point for the different edge types
	int start = 0;
	for (int i = EDGE_WALL; i >= 0; --i)
	{
		borderQuadsByType[i] = (borderQuads + start * 4);
		start += totalsPerType[i];
	}
	double extraLength = 0;

	BorderInfo *currEdgeNumQuadPtr;
	V2d along,norm, startInner, startOuter;

	double out = 16;
	double in = 64 - out;

	double inwardExtra, nextInwardExtra, startAlong, truEnd, currLen;

	int currEdgeTotalQuads;


	V2d currStartInner, currStartOuter, currEndInner, currEndOuter;

	for (int i = 0; i < numP; ++i)
	{
		edge = GetEdge(i);
		currLen = edge->GetLength();
		angleType = GetEdgeAngleType(edge);
		currEdgeNumQuadPtr = &numQuadMap[angleType][edge];

		currEdgeTotalQuads = currEdgeNumQuadPtr->GetTotal();

		if (currEdgeTotalQuads == 0)
			continue;

		along = edge->Along();
		norm = edge->Normal();

		inwardExtra = GetExtraForInward(edge);
		nextInwardExtra = GetExtraForInward(edge->GetNextEdge());

		startInner = edge->v0 - along * extraLength - norm * in;
		startOuter = edge->v0 - along * extraLength + norm * out;

		double startAlong = -inwardExtra;
		double trueEnd = currLen + nextInwardExtra;

		
		Vertex *currentTypeQuads = borderQuadsByType[angleType] 
			+ currentOffsetPerType[angleType] * 4;

		
		bool isAcute = IsAcute(edge);
		bool nextAcute = IsAcute(edge->GetNextEdge());
		V2d bisector = GetBisector(edge);
		V2d nextBisector = GetBisector(edge->GetNextEdge());
		int randomEdgeSizeChoice;
		int currChosenSizeIndex;
		int currWidth;
		int currIntersect;

		for (int j = 0; j < currEdgeTotalQuads; ++j)
		{
			randomEdgeSizeChoice = rand() % currEdgeNumQuadPtr->GetNumSizesWithCountAboveZero();
			currChosenSizeIndex = currEdgeNumQuadPtr->
				DecrementSizeWithCountAboveZero(randomEdgeSizeChoice);
			currWidth = borderSizes[currChosenSizeIndex].width;

			currIntersect = GetBorderQuadIntersect(currWidth);

			startAlong -= currIntersect;
			
			double endAlong = startAlong + currWidth;
			if (endAlong > trueEnd)
			{
				endAlong = trueEnd;// +currIntersect;
				startAlong = endAlong - currWidth;
			}

			if (currEdgeTotalQuads == 1)
			{
				startAlong = currLen / 2 - currWidth / 2 + GetBorderQuadIntersect(currWidth);
				endAlong = currLen / 2 + currWidth / 2;
			}
			IntRect sub = GetBorderSubRect(currWidth, angleType,
				rand() % BorderInfo::NUM_BORDER_VARIATIONS);

			double trueAlong = startAlong;
			if (startAlong > 0)
			{
				//trueAlong -= GetBorderQuadIntersect(tw);
			}

			currStartInner = startInner + trueAlong * along;
			currStartOuter = startOuter + trueAlong * along;
			currEndInner = startInner + endAlong * along;
			currEndOuter = startOuter + endAlong * along;

			double realHeightLeft = 64.0;
			double realHeightRight = 64.0;

			//SetRectColor(currBVA + i * 4, Color(Color::Black));
			if (isAcute)
			{
				LineIntersection li = lineIntersection(currStartInner,
					currStartOuter, edge->v0, edge->v0 + bisector);
				assert(!li.parallel);
				if (!li.parallel)
				{
					double testLength = dot(li.position - currStartOuter, normalize(currStartInner - currStartOuter));//(li.position - currStartOuter);
					assert(testLength >= 0);
					if (testLength < realHeightLeft)
					{
						double diffLen = realHeightLeft - testLength;
						realHeightLeft = testLength;
						currStartInner += diffLen * normalize(startOuter - startInner);
					}
				}

				li = lineIntersection(currEndInner,
					currEndOuter, edge->v0, edge->v0 + bisector);
				assert(!li.parallel);
				if (!li.parallel)
				{
					double testLength = dot(li.position - currStartOuter, normalize(currStartInner - currStartOuter));//(li.position - currStartOuter);
					assert(testLength >= 0);
					if (testLength < realHeightRight)
					{
						double diffLen = realHeightRight - testLength;
						realHeightRight = testLength;
						currEndInner += diffLen * normalize(startOuter - startInner);
					}
				}
			}
			if (nextAcute)
			{
				bool middleSplit = false;
				LineIntersection liMiddle = lineIntersection(currStartInner,
					currEndInner, edge->v1, edge->v1 + nextBisector);//te->edge1->v1);
				if (!liMiddle.parallel)
				{
					double md = dot(liMiddle.position - currStartInner, normalize(currEndInner - currStartInner));
					double cLen = length(currEndInner - currStartInner);
					if (md < cLen && md > 0)
					{
						//currEndInner = liMiddle.position;
						//middleSplit = true;
						//SetRectColor(currBVA + i * 4, Color(Color::Red));
					}
				}

				if (!middleSplit)
				{
					LineIntersection li = lineIntersection(currStartInner,
						currStartOuter, edge->v1, edge->v1 + nextBisector);//te->edge1->v1);
					assert(!li.parallel);
					if (!li.parallel)
					{
						double testLength = length(li.position - currStartOuter);
						if (testLength < realHeightLeft)
						{
							cout << "len: " << testLength << ", x: " << li.position.x << "." << currStartOuter.x << endl;
							double diffLen = realHeightLeft - testLength;

							realHeightLeft = testLength;
							currStartInner = li.position;
							cout << i << "left lipos: " << li.position.x << ", " << li.position.y << endl;
						}
					}

					li = lineIntersection(currEndInner,
						currEndOuter, edge->v1, edge->v1 + nextBisector);//te->edge1->v1 );
					assert(!li.parallel);
					if (!li.parallel)
					{
						double testLength = length(li.position - currEndOuter);
						if (testLength < realHeightRight)
						{
							cout << "len: " << testLength << ", x: " << li.position.x << "." << currEndOuter.x << endl;
							double diffLen = realHeightRight - testLength;
							realHeightRight = testLength;
							currEndInner += diffLen * normalize(startOuter - startInner);
							cout << i << "right lipos: " << li.position.x << ", " << li.position.y << endl;
						}
					}
				}
			}

			currentTypeQuads[j * 4 + 0].position = Vector2f( currStartOuter );
			currentTypeQuads[j * 4 + 1].position = Vector2f(currEndOuter);
			currentTypeQuads[j * 4 + 2].position = Vector2f( currEndInner );
			currentTypeQuads[j * 4 + 3].position = Vector2f( currStartInner );

			float width = length(currEndInner - currStartInner);
			currentTypeQuads[j * 4 + 0].texCoords = Vector2f(sub.left, sub.top);
			currentTypeQuads[j * 4 + 1].texCoords = Vector2f(sub.left + width, sub.top);
			currentTypeQuads[j * 4 + 2].texCoords = Vector2f(sub.left + width, sub.top + realHeightRight);
			currentTypeQuads[j * 4 + 3].texCoords = Vector2f(sub.left, sub.top + realHeightLeft);

			/*Color c = Color::White;
			switch (angleType)
			{
			case EDGE_FLAT:
				c = Color::Red;
				break;
			case EDGE_FLATCEILING:
				c = Color::Magenta;
				break;
			case EDGE_SLOPED:
				break;
			case EDGE_STEEPSLOPE:
				break;
			case EDGE_SLOPEDCEILING:
				break;
			case EDGE_STEEPCEILING:
				break;
			case EDGE_WALL:
				c = Color::Blue;
				break;
			}

			currentTypeQuads[j * 4 + 0].color = c;
			currentTypeQuads[j * 4 + 1].color = c;
			currentTypeQuads[j * 4 + 2].color = c;
			currentTypeQuads[j * 4 + 3].color = c;*/

			/*currentTypeQuads[i * 4 + 0].color = Color::Red;
			currentTypeQuads[i * 4 + 1].color = Color::Blue;
			currentTypeQuads[i * 4 + 2].color = Color::Green;
			currentTypeQuads[i * 4 + 3].color = Color::Cyan;*/


			

			if (j == 0)
			{
				startAlong += currWidth - GetBorderQuadIntersect(currWidth);
			}
			else
			{
				startAlong += currWidth - GetBorderQuadIntersect(currWidth);
			}
		}

		currentOffsetPerType[angleType] += currEdgeTotalQuads;
	}



	/*for (auto it = transEdges.begin(); it != transEdges.end(); ++it)
	{
		V2d eNorm = (*it)->Normal();
		V2d pNorm = (*it)->edge0->Normal();
		V2d eDir = normalize((*it)->v1 - (*it)->v0);
		V2d pDir = normalize((*it)->edge0->v1 - (*it)->edge0->v0);
		V2d outCurr = (*it)->v0 - eNorm * 48.0;
		V2d outPrev = (*it)->v0 - pNorm * 48.0;
		V2d together = normalize(-eDir + pDir);
		V2d tn(together.y, -together.x);

		double width = length(outCurr - outPrev);
		V2d p0 = (*it)->v0 - tn * width / 2.0;
		V2d p1 = (*it)->v0 + tn * width / 2.0;
		double extra = 4.0;
		double height = length(p0 - outPrev) + extra;
		//double a = abs( dot(outCurr - (*it)->v0, tn ) );
		transva[0].position = Vector2f(p0); //Vector2f((*it)->v0);
		transva[1].position = Vector2f(p1);//Vector2f((*it)->v0);
		transva[2].position = Vector2f(outCurr + together * extra);
		transva[3].position = Vector2f(outPrev + together * extra);



		EdgeType transType = GetEdgeTransType((*it));

		IntRect sub;
		if (transType > E_WALL)
		{
			//	sub = GetBorderSubRect(64, transType, 0);
		}
		else
		{
			//	sub = GetBorderSubRect(128, transType, 0);
		}

		sub = GetBorderSubRect(128, E_WALL, 0);
		float centerX = sub.left + sub.width / 2;

		float startHeight = sub.top + (sub.height - height);

		transva[0].texCoords = Vector2f(sub.left + centerX - width / 2, startHeight);
		transva[1].texCoords = Vector2f(sub.left + centerX + width / 2, startHeight);
		transva[2].texCoords = Vector2f(sub.left + centerX + width / 2, startHeight + height);
		transva[3].texCoords = Vector2f(sub.left + centerX - width / 2, startHeight + height);
		transva += 4;
	}*/
}

void TerrainPolygon::AddEdgesToQuadTree(QuadTree *tree)
{
	int numP = GetNumPoints();
	for (int i = 0; i < numP; ++i)
	{
		tree->Insert(&edges[i]);
	}
}

vector<TerrainPoint> &TerrainPolygon::PointVector()
{
	int i = (int)inverse;
	return pointVector[i];
}

void TerrainPolygon::WriteFile(std::ofstream & of)
{
	of << terrainWorldType << " " << terrainVariation << endl;

	int numP = GetNumPoints();

	of << numP << endl;

	for (int i = 0; i < numP; ++i)
	{
		of << PointVector()[i].pos.x << " " << PointVector()[i].pos.y << endl;
	}

	if (!IsSpecialPoly())
	{
		WriteGrass(of);
	}
}

void TerrainPolygon::MakeInverse()
{
	pointVector[1] = pointVector[0];
	pointVector[0].clear();
	inverse = true;
}

Edge *TerrainPolygon::GetEdge(int index)
{
	assert(finalized);
	return &edges[index];
}

TerrainPoint *TerrainPolygon::GetPoint(int index)
{
	assert(index >= 0 && index < PointVector().size());
	return &PointVector()[index];
}

TerrainPoint *TerrainPolygon::GetInverseOuterRectPoint(int index)
{
	assert(inverse && finalized && index >= 0 && index < 4);
	return &pointVector[0][index];
}

TerrainPoint *TerrainPolygon::GetFinalizeInversePoint(int index)
{
	if (index < 4)
		return GetInverseOuterRectPoint(index);
	else
	{
		return GetPoint(index - 4);
	}
}

TerrainPoint *TerrainPolygon::GetEndPoint()
{
	return &PointVector().back();
}

TerrainPoint *TerrainPolygon::GetStartPoint()
{
	return &PointVector().front();
}

TerrainPoint *TerrainPolygon::GetNextPoint(int index)
{
	assert(index >= 0 && index < PointVector().size());
	if (index == GetNumPoints() - 1)
	{
		return &PointVector()[0];
	}
	else
	{
		return &PointVector()[index + 1];
	}
}

TerrainPoint *TerrainPolygon::GetPrevPoint(int index)
{
	assert(index >= 0 && index < PointVector().size());
	if (index == 0)
	{
		return &PointVector()[GetNumPoints() - 1];
	}
	else
	{
		return &PointVector()[index - 1];
	}
}

void TerrainPolygon::WriteGrass(std::ofstream &of)
{
	int edgesWithSegments = 0;
	list<list<GrassSeg>> grassListList;

	if (grassVA != NULL)
	{
		VertexArray &grassVa = *grassVA;

		int edgeIndex = 0;
		int i = 0;
		int numP = GetNumPoints();
		TerrainPoint *curr, *prev, *next;
		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);
			prev = GetPrevPoint(i);
			next = GetNextPoint(i);

			V2d v0(curr->pos.x, curr->pos.y);
			V2d v1(next->pos.x, next->pos.y);

			bool rem;
			int num = GetNumGrass(i, rem);

			grassListList.push_back(list<GrassSeg>());

			list<GrassSeg> &grassList = grassListList.back();

			GrassSeg *gPtr = NULL;
			bool hasGrass = false;
			for (int j = 0; j < num; ++j)
			{
				if (grassVa[i * 4].color.a == 255 || grassVa[i * 4].color.a == 254)
				{
					hasGrass = true;
					if (gPtr == NULL)
					{
						grassList.push_back(GrassSeg(edgeIndex, j, 0));
						gPtr = &grassList.back();
					}
					else
					{
						grassList.back().reps++;
					}
				}
				else
				{
					if (gPtr != NULL)
						gPtr = NULL;
				}

				++i;
			}

			if (hasGrass)
			{
				++edgesWithSegments;
			}

			++edgeIndex;

		}
	}

	of << edgesWithSegments << endl;

	for (list<list<GrassSeg>>::iterator it = grassListList.begin(); it != grassListList.end(); ++it)
	{
		int numSegments = (*it).size();

		if (numSegments > 0)
		{
			int edgeIndex = (*it).front().edgeIndex;
			of << edgeIndex << " " << numSegments << endl;

			for (list<GrassSeg>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
			{
				of << (*it2).index << " " << (*it2).reps << endl;
			}
		}
	}
}

bool TerrainPolygon::IntersectsGate(GateInfo *gi)
{
	IntRect tAABB = GetAABB();
	if (tAABB.intersects(gi->GetAABB()))
	{
		Vector2i gp0 = gi->point0->pos;
		Vector2i gp1 = gi->point1->pos;

		Vector2i myPos;
		Vector2i myPrevPos;

		int numP = GetNumPoints();
		TerrainPoint *curr, *prev;
		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);
			prev = GetPrevPoint(i);

			myPos = curr->pos;
			myPrevPos = prev->pos;

			if (gp0 == myPrevPos || gp1 == myPrevPos
				|| gp0 == myPos || gp1 == myPos)
			{
				continue;
			}

			LineIntersection li = EditSession::SegmentIntersect(myPrevPos, myPos, gp0, gp1);
			if (!li.parallel)
			{
				return true;
			}
		}
	}

	return false;
}

bool TerrainPolygon::PointsTooCloseToEachOther()
{
	EditSession *session = EditSession::GetSession();
	int numP = GetNumPoints();
	TerrainPoint *currI, *currJ;
	for (int i = 0; i < numP; ++i)
	{
		currI = GetPoint(i);
		for (int j = 0; j < numP; ++j)
		{
			if (i == j)
				continue;

			currJ = GetPoint(j);
			V2d a(currI->pos.x, currI->pos.y);
			V2d b(currJ->pos.x, currJ->pos.y);
			if (length(a - b) < session->validityRadius)
			{
				return true;
			}
		}
	}

	return false;
}

bool TerrainPolygon::LinesIntersectMyself()
{
	//line intersection on myself
	int numP = GetNumPoints();
	TerrainPoint *currI, *prevI, *currJ, *prevJ;

	for (int i = 0; i < numP; ++i)
	{
		currI = GetPoint(i);
		prevI = GetPrevPoint(i);

		for (int j = 0; j < numP; ++j)
		{
			if (i == j)
				continue;

			currJ = GetPoint(j);
			prevJ = GetPrevPoint(j);

			if (currI == currJ || currI == prevJ || prevI == currJ || prevI == prevJ)
			{
				continue;
			}

			LineIntersection li = EditSession::LimitSegmentIntersect(prevI->pos, currI->pos, prevJ->pos, currJ->pos);
			//LineIntersection li = EditSession::LimitSegmentIntersect(prevI->pos, currI->pos, prevJ->pos, currJ->pos);

			if (!li.parallel)
			{
				return true;
			}
		}
	}

	return false;
}


bool TerrainPolygon::HasSlivers()
{
	int numP = GetNumPoints();
	TerrainPoint *curr, *prev, *next;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		prev = GetPrevPoint(i);
		next = GetNextPoint(i);

		V2d pos(curr->pos.x, curr->pos.y);
		V2d prevPos(prev->pos.x, prev->pos.y);
		V2d nextPos(next->pos.x, next->pos.y);


		//this is wrong and needs fixing to the other standards anyway
		double ff = dot(normalize(prevPos - pos), normalize(nextPos - pos));
		if (ff > .99)
		{
			return true;
		}
	}

	return false;
}

bool TerrainPolygon::IntersectsMyOwnEnemies()
{
	EditSession *session = EditSession::GetSession();
	for (EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it)
	{
		for (list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait)
		{
			sf::VertexArray &bva = (*ait)->boundingQuad;
			if (session->QuadPolygonIntersect(this, Vector2i(bva[0].position.x, bva[0].position.y),
				Vector2i(bva[1].position.x, bva[1].position.y), Vector2i(bva[2].position.x, bva[2].position.y),
				Vector2i(bva[3].position.x, bva[3].position.y)))
			{
				return true;
			}
		}
	}

	return false;
}

bool TerrainPolygon::IntersectsMyOwnGates()
{
	int numP = GetNumPoints();
	TerrainPoint *curr, *prev;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		prev = GetPrevPoint(i);

		Vector2i prevPos = prev->pos;
		Vector2i pos = curr->pos;

		LineIntersection li = EditSession::LimitSegmentIntersect(prevPos, pos, curr->gate->point0->pos, curr->gate->point1->pos);

		if (!li.parallel)
		{
			return false;
		}
	}
}

bool TerrainPolygon::IsInternallyValid()
{
	if (inverse)
	{
		if (IsClockwise())
			return false;
	}
	else
	{
		if (!IsClockwise())
			return false;
	}

	//if (PointsTooCloseToEachOther())
	//	return false;

	if (HasSlivers())
		return false;

	if (LinesIntersectMyself())
		return false;

	if (IntersectsMyOwnEnemies())
		return false;

	return true;	
}

bool TerrainPolygon::CanApply()
{
	EditSession *session = EditSession::GetSession();
	auto & currPolyList = session->GetCorrectPolygonList(this);

	if (session->IsPolygonValid(this, NULL))
	{
		return true;
	}

	return false;
}

bool TerrainPolygon::CanAdd()
{
	return false;
}

void TerrainPolygon::DeactivateGates()
{
	int numP = GetNumPoints();
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		if (curr->gate != NULL)
		{
			if (curr->gate->edit != NULL)
			{
				curr->gate->Deactivate();
			}
		}
	}
}

void TerrainPolygon::ActivateGates()
{
	int numP = GetNumPoints();
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		if (curr->gate != NULL)
		{
			if (curr->gate->edit == NULL)
			{
				curr->gate->Activate();
			}
		}
	}
}

void TerrainPolygon::Deactivate()
{
	cout << "deactivating poly: " << this << endl;
	EditSession *session = EditSession::GetSession();
	active = false;
	session->GetCorrectPolygonList(this).remove(this);

	if (inverse)
	{
		//delete sess->inversePolygon;
		session->inversePolygon = NULL;
	}

	//remove gates
	DeactivateGates();
}

void TerrainPolygon::Activate()
{
	cout << "activating poly: " << this << endl;
	EditSession *session = EditSession::GetSession();
	active = true;

	session->GetCorrectPolygonList(this).push_back(this);
	
	if (inverse)
	{
		session->inversePolygon = this;
	}
	
	ActivateGates();
}

//return 0 on no fix, 1 on moved current point, and 2 on moved next point
int TerrainPolygon::FixNearPrimary(int i, bool currLocked)
{
	TerrainPoint *curr, *next;
	curr = GetPoint(i);
	next = GetNextPoint(i);

	Vector2i extreme = GetExtreme(curr, next);

	if (extreme.x == 0 && extreme.y == 0)
		return 0;

	if (currLocked)
	{
		if (extreme.x != 0)
			next->pos.y = curr->pos.y;
		else
			next->pos.x = curr->pos.x;

		if (curr->pos == next->pos)
		{
			int b = 6;
			assert(0);
		}

		return 2;
	}

	if (extreme.x != 0)
		curr->pos.y = next->pos.y;
	else
		curr->pos.x = next->pos.x;

	if (curr->pos == next->pos)
	{
		int b = 6;
		assert(0);
	}

	return 1;
}

Vector2i TerrainPolygon::GetExtreme(TerrainPoint *p0,
	TerrainPoint *p1)
{
	Vector2i extreme(0, 0);

	Vector2i diff = p1->pos - p0->pos;

	if (diff.x == 0 || diff.y == 0)
		return extreme;

	double primLimit = EditSession::PRIMARY_LIMIT;
	V2d diffDir = normalize(V2d(diff));
	if (diffDir.x > primLimit)
		extreme.x = 1;
	else if (diffDir.x < -primLimit)
		extreme.x = -1;
	if (diffDir.y > primLimit)
		extreme.y = 1;
	else if (diffDir.y < -primLimit)
		extreme.y = -1;

	return extreme;
}

bool TerrainPolygon::AlignExtremes(std::vector<PointMoveInfo> &lockPoints)
{
	double primLimit = EditSession::PRIMARY_LIMIT;
	bool adjustedAtAll = false;

	bool checkPoint;
	bool adjusted = true;

	int lockPointIndex = 0;
	assert(lockPoints.empty() || lockPoints.size() == GetNumPoints());

	bool lockPointsEmpty = lockPoints.empty();


	int numP = GetNumPoints();

	while (adjusted)
	{
		adjusted = false;
		lockPointIndex = 0;
		int result;
		bool isPointLocked;

		for (int i = 0; i < numP; ++i, lockPointIndex++)
		{
			isPointLocked = !lockPointsEmpty && lockPoints[lockPointIndex].moveIntent;
			result = FixNearPrimary(i, isPointLocked);

			if (result > 0)
			{
				adjusted = true;
				adjustedAtAll = true;
			}
		}
	}

	return adjustedAtAll;
}

bool TerrainPolygon::AlignExtremes()
{
	vector<PointMoveInfo> emptyLockPoints;
	return AlignExtremes(emptyLockPoints);
}



void TerrainPolygon::Scale(float f)
{
	SetRenderMode(RENDERMODE_TRANSFORM);

	Vector2i center((right + left) / 2, (bottom + top) / 2);
	Vector2f fCenter((right + left) / 2.f, (bottom + top) / 2.f);

	int numP = GetNumPoints();
	Vector2i diff;
	Vector2f fDiff;
	Vector2f fCurr;
	TerrainPoint *curr;

	int prevIndex;

	for (int i = 0; i < numP; ++i)
	{
		prevIndex = i * 2 - 1;
		if (i == 0)
		{
			prevIndex = numP * 2 - 1;
		}
		//curr = GetPoint(i);
		fCurr = lines[i*2].position;
		//fCurr = Vector2f(curr->pos);

		//diff = curr->pos - center;
		//diff /= 2;
		//curr->pos = center + diff;

		fDiff = fCurr - fCenter;
		fDiff *= f;

		fCurr = fCenter + fDiff;
		lines[i*2].position = fCurr;
		lines[prevIndex].position = fCurr;
		//curr->pos = Vector2i(round(fCurr.x), round(fCurr.y));
		
	}
}

void TerrainPolygon::Rotate( Vector2f &fCenter, float f)
{
	SetRenderMode(RENDERMODE_TRANSFORM);
	//Vector2i center((right + left) / 2, (bottom + top) / 2);
	//Vector2f fCenter((right + left) / 2.f, (bottom + top) / 2.f);

	int numP = GetNumPoints();
	Vector2f fDiff;
	Vector2f fCurr;
	TerrainPoint *curr;

	Transform t;
	t.rotate(f);

	int prevIndex;

	for (int i = 0; i < numP; ++i)
	{
		//curr = GetPoint(i);
		fCurr = lines[i*2].position;//Vector2f(curr->pos);

		fDiff = fCurr - fCenter;
		fDiff = t.transformPoint(fDiff);

		fCurr = fCenter + fDiff;

		lines[i*2].position = fCurr;

		if (i == 0)
		{
			lines[numP * 2 - 1].position = fCurr;
		}
		else
		{
			lines[i * 2 - 1].position = fCurr;
		}
	}

	//AlignExtremes(); happens after rotation is totally done
}

void TerrainPolygon::Move(Vector2i move )
{
	assert( finalized );

	left += move.x;
	right += move.x;
	bottom += move.y;
	top += move.y;

	

	V2d dMove(move);
	Vector2f fMove(move);

	myTerrainTree->Move(dMove);

	int numP = GetNumPoints();
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		curr->pos += move;
		if (curr->gate != NULL)
		{
			curr->gate->UpdateLine();
		}

		edges[i].v0 += dMove;
		edges[i].v1 += dMove;
	}

	if (borderQuads != NULL)
	{
		for (int i = 0; i < totalNumBorderQuads * 4; ++i)
		{
			borderQuads[i].position += fMove;
		}
	}

	for( int i = 0; i < numP; ++i )
	{
		//lines
		lines[i * 2].position += fMove;
		lines[i * 2 + 1].position += fMove;
	}

	for( int i = 0; i < vaSize; ++i )
	{
		VertexArray &vaa = *va;

		//triangles
		vaa[i].position += Vector2f( move.x, move.y );
	}

	if (grassVA != NULL)
	{
		for (int i = 0; i < numGrassTotal; ++i)
		{
			//quads
			VertexArray &gva = *grassVA;
			gva[i * 4].position += Vector2f(move.x, move.y);
			gva[i * 4 + 1].position += Vector2f(move.x, move.y);
			gva[i * 4 + 2].position += Vector2f(move.x, move.y);
			gva[i * 4 + 3].position += Vector2f(move.x, move.y);
		}
	}

	for( EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		list<ActorPtr> &actorList = (*it).second;
		for( list<ActorPtr>::iterator ait = actorList.begin(); ait != actorList.end(); ++ait )
		{
			(*ait)->UpdateGroundedSprite();
			(*ait)->SetBoundingQuad();
		}
	}

	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->Move(dMove);
	}

	for (auto it = decorExprList.begin(); it != decorExprList.end(); ++it)
	{
		(*it)->Move(fMove);
	}

	//UpdateBounds();
	return;
}

void TerrainPolygon::SetLayer( int newLayer )
{
	//Color testColor( 0x75, 0x70, 0x90 );
	if( newLayer != layer )
	{
		layer = newLayer;
		if( newLayer == 0 )
		{
			VertexArray &v = *va;
			for( int i = 0; i < vaSize; ++i )
			{
				v[i].color = fillCol;
			}
		}
		else if( newLayer == 1 )
		{
			VertexArray &v = *va;
			for( int i = 0; i < vaSize; ++i )
			{
				//gotta alter this soon
				v[i].color = COLOR_BLUE;
			}
		}
	}
}

void TerrainPolygon::UpdateBounds()
{
	int numP = GetNumPoints();

	TerrainPoint *curr = GetPoint(0);

	left = curr->pos.x;
	right = curr->pos.x;
	top = curr->pos.y;
	bottom = curr->pos.y;

	for (int i = 1; i < numP; ++i)
	{
		curr = GetPoint(i);
		left = min(curr->pos.x, left);
		right = max(curr->pos.x, right);
		top = min(curr->pos.y, top);
		bottom = max(curr->pos.y, bottom);
	}

	for( EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		list<ActorPtr> &en = (*it).second;
		for( list<ActorPtr>::iterator ait = en.begin(); ait != en.end(); ++ait )
		{
			sf::VertexArray & bq = (*ait)->boundingQuad;
			for( int i = 0; i < 4; ++i )
			{
				int x = bq[i].position.x;
				int y = bq[i].position.y;
				if( x < left )
				{
					left = x;
				}
				if( x > right )
				{
					right = x;
				}
				if( y < top )
				{
					top = y;
				}
				if( y > bottom )
				{
					//cout << "adjusting botton from: " << bottom << " to " << y << endl;
					bottom = y;
					
				}
					
			}

		}
	}
}

bool TerrainPolygon::IsSpecialPoly()
{
	return terrainWorldType > CORE;
}

void TerrainPolygon::UpdateMaterialType()
{
	int texInd = terrainWorldType * EditSession::MAX_TERRAINTEX_PER_WORLD + terrainVariation;
	pShader = &sess->polyShaders[texInd];

	tdInfo = sess->terrainDecorInfoMap[make_pair(terrainWorldType, terrainVariation)];
	//assert(tdInfo != NULL);

	Color sCol( 0x77, 0xBB, 0xDD );
	//factor in variation later
	//Color newColor;
	/*switch( world )
	{
	case 0:
		fillCol = Color::Blue;
		break;
	case 1:
		fillCol = Color::Green;
		break;
	case 2:
		fillCol = Color::Yellow;
		break;
	case 3:
		fillCol = Color( 100, 200, 200 );
		break;
	case 4:
		fillCol = Color::Red;
		break;
	case 5:
		fillCol = Color::Magenta;
		break;
	case 6:
		fillCol = Color::White;
		break;
	}*/
	fillCol = Color::White;
	selectCol = sCol;
	//selectCol = 

	int vCount = va->getVertexCount();
	VertexArray &v = *va;
	for( int i = 0; i < vCount; ++i )
	{	
		v[i].color = fillCol;
	}
}

void TerrainPolygon::SetMaterialType(int world, int variation)
{
	TerrainWorldType newWorldType = (TerrainWorldType)world;

	if( ts_border == NULL || newWorldType != terrainWorldType )
		SetBorderTileset();

	terrainWorldType = newWorldType;
	terrainVariation = variation;

	if (finalized)
	{
		//optimize this later
		UpdateMaterialType();
	}
}

int TerrainPolygon::GetNumGrass(int i, bool &rem)
{
	rem = false;
	TerrainPoint *curr, *next;
	curr = GetPoint(i);
	next = GetNextPoint(i);

	V2d v0(curr->pos.x, curr->pos.y);
	V2d v1(next->pos.x, next->pos.y);

	double len = length(v1 - v0);
	len -= grassSize / 2 + grassSpacing;

	if (grassSize + grassSpacing == 0)
	{
		//should never happen
		assert(0);
	}
	double reps = len / (grassSize + grassSpacing);
	double remainder = reps - floor(reps);
	if (remainder > 0)
	{
		reps += 1; //for the last one
		rem = true;
	}
	reps += 1;

	int num = reps;

	return num;
}

int TerrainPolygon::GetNumGrassTotal()
{
	int total = 0;
	int inds = 0;
	bool rem;

	int numP = GetNumPoints();
	for (int i = 0; i < numP; ++i)
	{
		total += GetNumGrass(i, rem);
	}

	return total;
}

void TerrainPolygon::SetupGrass(int i, int &grassIndex )
{
	if (grassVA == NULL)
		return;

	VertexArray &grassVa = *grassVA;

	TerrainPoint *curr, *next;

	curr = GetPoint(i);
	next = GetNextPoint(i);

	V2d v0(curr->pos.x, curr->pos.y);
	V2d v1(next->pos.x, next->pos.y);

	bool rem;
	int num = GetNumGrass(i, rem);

	V2d along = normalize(v1 - v0);
	V2d realStart = v0 + along * (double)(grassSize + grassSpacing);
	
	for (int j = 0; j < num; ++j)
	{
		V2d posd = realStart + along * (double)((grassSize + grassSpacing) * (j - 1));//v0 + normalize(v1 - v0) * ((grassSize + grassSpacing) * (j-1) + );

		if (j == 0)
		{
			posd = v0;
		}
		else if (j == num - 1 && rem)
		{
			//V2d prev = ;//v0 + (v1 - v0) * ((double)(j-1) / num);
			posd = v1 + normalize(v0 - v1) * (grassSize / 2.0 + grassSpacing);//(v1 + prev) / 2.0;
		}

		Vector2f pos(posd.x, posd.y);


		Vector2f topLeft = pos + Vector2f(-grassSize / 2, -grassSize / 2);
		Vector2f topRight = pos + Vector2f(grassSize / 2, -grassSize / 2);
		Vector2f bottomLeft = pos + Vector2f(-grassSize / 2, grassSize / 2);
		Vector2f bottomRight = pos + Vector2f(grassSize / 2, grassSize / 2);

		//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
		grassVa[grassIndex * 4].color.a = 0;
		grassVa[grassIndex * 4].position = topLeft;
		grassVa[grassIndex * 4].texCoords = Vector2f(0, 0);

		//grassVa[i*4+1].color = Color::Blue;
		//borderVa[i*4+1].color.a = 10;
		grassVa[grassIndex * 4 + 1].color.a = 0;
		grassVa[grassIndex * 4 + 1].position = bottomLeft;
		grassVa[grassIndex * 4 + 1].texCoords = Vector2f(0, grassSize);

		//grassVa[i*4+2].color = Color::Blue;
		//borderVa[i*4+2].color.a = 10;
		grassVa[grassIndex * 4 + 2].color.a = 0;
		grassVa[grassIndex * 4 + 2].position = bottomRight;
		grassVa[grassIndex * 4 + 2].texCoords = Vector2f(grassSize, grassSize);

		//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
		//borderVa[i*4+3].color.a = 10;
		grassVa[grassIndex * 4 + 3].color.a = 0;
		grassVa[grassIndex * 4 + 3].position = topRight;
		grassVa[grassIndex * 4 + 3].texCoords = Vector2f(grassSize, 0);
		++grassIndex;
	}
}

void TerrainPolygon::TryFixPointsTouchingLines()
{
	int numP = GetNumPoints();

	TerrainPoint *curr, *prev;
	TerrainPoint *polyCurr, *polyPrev;

	int pointTouchCount = 0;

	Vector2i lii;
	V2d diff;
	Vector2i testDiff;

	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		prev = GetPrevPoint(i);

		for (int j = 0; j < numP; ++j)
		{
			if (j == i || j == i - 1 || j == i + 1 || (i == numP - 1 && j == 0 ) || (j == numP - 1 && i == 0 ))
				continue;

			polyCurr = GetPoint(j);
			polyPrev = GetPrevPoint(j);

			LineIntersection li = EditSession::SegmentIntersect(prev->pos, curr->pos, polyPrev->pos, polyCurr->pos);
			if (!li.parallel)
			{
				{
					lii = Vector2i(round(li.position.x), round(li.position.y));
					//lii = Vector2i(xi, yi);
					int testIndex = -1;
					if (lii == prev->pos)
					{
						testIndex = prev->GetIndex();
					}
					else if (lii == curr->pos)
					{
						testIndex = curr->GetIndex();
					}
					else if (lii == polyPrev->pos)
					{
						testIndex = polyPrev->GetIndex();
					}
					else if(lii == polyCurr->pos)
					{
						testIndex = polyCurr->GetIndex();
					}

					if (testIndex >= 0)
					{
						diff = li.position - V2d(GetPoint(testIndex)->pos.x, GetPoint(testIndex)->pos.y);
						if (diff.x == 0 && diff.y == 0)
						{
							//point is directly on line. figure out solution soon
							int x = 5;
							continue;
						}
							
						
						if (diff.x > 0)
							diff.x = 1;
						else if (diff.x < 0)
							diff.x = -1;
						if (diff.y > 0)
							diff.y = 1;
						else if (diff.y < 0)
							diff.y = -1;
						
						GetPoint(testIndex)->pos += Vector2i(diff);
					}
				}
			}

		}
	}
}

void TerrainPolygon::Finalize()
{
	if (inverse)
	{
		FinalizeInverse();
		return;
	}

	finalized = true;
	isGrassShowing = false;
	
	FixWinding();
	
	int numP = GetNumPoints();
	std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(pointVector);
	vaSize = indices.size();

	lines = new sf::Vertex[numP * 2 + 1];
	va = new VertexArray( sf::Triangles , vaSize );
	
	SetupEdges();

	VertexArray & v = *va;
	Color testColor( 0x75, 0x70, 0x90 );
	Color selectCol( 0x77, 0xBB, 0xDD );

	if( selected )
	{
		testColor = selectCol;
	}

	int numTris = vaSize / 3;
	for( int i = 0; i < numTris; ++i )
	{	
		v[i * 3] = Vertex(Vector2f(GetPoint(indices[i*3])->pos), testColor);
		v[i * 3 + 1] = Vertex(Vector2f(GetPoint(indices[i * 3 + 1])->pos), testColor);
		v[i * 3 + 2] = Vertex(Vector2f(GetPoint(indices[i * 3 + 2])->pos), testColor);
	}

	SetMaterialType( terrainWorldType, terrainVariation );

	GenerateBorderMesh();

	UpdateLinePositions();
	UpdateLineColors();

	UpdateBounds();

	SetupGrass();

	AddTouchGrass(TouchGrass::TYPE_NORMAL);
	//AddTouchGrass(TouchGrass::TYPE_TEST);

	ResetTouchGrass();

	UpdateTouchGrass();

	myTerrainTree->Clear();
	AddEdgesToQuadTree(myTerrainTree);

	GenerateDecor();
}

void TerrainPolygon::FinalizeInverse()
{
	finalized = true;
	isGrassShowing = false;

	FixWindingInverse();

	UpdateBounds();

	int testExtra = inverseExtraBoxDist;

	Vector2i outerRectPositions[4];
	outerRectPositions[0] = Vector2i(left - inverseExtraBoxDist, top - inverseExtraBoxDist);
	outerRectPositions[1] = Vector2i(right + inverseExtraBoxDist, top - inverseExtraBoxDist);
	outerRectPositions[2] = Vector2i(right + inverseExtraBoxDist, bottom + inverseExtraBoxDist);
	outerRectPositions[3] = Vector2i(left - inverseExtraBoxDist, bottom + inverseExtraBoxDist);

	vector<TerrainPoint> &outerRectTerrainPoints = pointVector[0];
	outerRectTerrainPoints.clear();
	outerRectTerrainPoints.reserve(4);
	for (int i = 0; i < 4; ++i)
	{
		AddInverseBorderPoint(outerRectPositions[i], false);
	}


	std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(pointVector);
	vaSize = indices.size();

	int numP = GetNumPoints();

	SetupEdges();

	lines = new sf::Vertex[numP * 2 + 1];
	va = new VertexArray(sf::Triangles, vaSize);

	VertexArray & v = *va;
	Color testColor(0x75, 0x70, 0x90);
	Color selectCol(0x77, 0xBB, 0xDD);

	if (selected)
	{
		testColor = selectCol;
	}

	int numTris = vaSize / 3;
	for (int i = 0; i < numTris; ++i)
	{
		v[i * 3] = Vertex(Vector2f(GetFinalizeInversePoint(indices[i * 3])->pos), testColor);
		v[i * 3 + 1] = Vertex(Vector2f(GetFinalizeInversePoint(indices[i * 3 + 1])->pos), testColor);
		v[i * 3 + 2] = Vertex(Vector2f(GetFinalizeInversePoint(indices[i * 3 + 2])->pos), testColor);
	}

	SetMaterialType(terrainWorldType, terrainVariation);

	GenerateBorderMesh();

	UpdateLinePositions();
	UpdateLineColors();

	UpdateBounds();

	SetupGrass();

	AddTouchGrass(TouchGrass::TYPE_NORMAL);
	//AddTouchGrass(TouchGrass::TYPE_TEST);

	ResetTouchGrass();

	UpdateTouchGrass();

	myTerrainTree->Clear();
	AddEdgesToQuadTree(myTerrainTree);
	GenerateDecor();
	UpdateDecorSprites();
}

void TerrainPolygon::SetupGrass()
{
	numGrassTotal = GetNumGrassTotal();

	if (numGrassTotal == 0)
	{
		grassVA = NULL;
		return;
	}

	VertexArray *gva = new VertexArray(sf::Quads, numGrassTotal * 4);
	grassVA = gva;

	VertexArray &grassVa = *gva;

	int numP = GetNumPoints();
	TerrainPoint *curr;
	int grassIndex = 0;
	for (int i = 0; i < numP; ++i)
	{
		SetupGrass(i, grassIndex);
	}
}

void TerrainPolygon::SwitchGrass( V2d mousePos )
{
	if (grassVA == NULL)
		return;

	VertexArray &grassVa = *grassVA;
	double radius = grassSize / 2 - 20;//+ grassSpacing / 2;

	V2d center;
	for (int i = 0; i < numGrassTotal; ++i)
	{
		center = V2d((grassVa[i * 4 + 0].position
			+ grassVa[i * 4 + 1].position
			+ grassVa[i * 4 + 2].position
			+ grassVa[i * 4 + 3].position) / 4.f);
		if (length(center - mousePos) <= radius)
		{
			if (grassVa[i * 4].color.a == 50)
			{
				grassVa[i * 4].color.a = 254;
				grassVa[i * 4 + 1].color.a = 254;
				grassVa[i * 4 + 2].color.a = 254;
				grassVa[i * 4 + 3].color.a = 254;
				//cout << "making full: " << i << endl;
			}
			else if (grassVa[i * 4].color.a == 255)
			{
				grassVa[i * 4].color.a = 49;
				grassVa[i * 4 + 1].color.a = 49;
				grassVa[i * 4 + 2].color.a = 49;
				grassVa[i * 4 + 3].color.a = 49;
				//cout << "making seethru: " << i << endl;
			}
			//break;
		}
	}
}

void TerrainPolygon::UpdateGrass()
{
	if (grassVA == NULL)
		return;

	VertexArray & grassVa = *grassVA;
	for( int i = 0; i < numGrassTotal; ++i )
	{
		if( grassVa[i*4].color.a == 49 )
		{
			grassVa[i*4].color.a = 50;
			grassVa[i*4+1].color.a = 50;
			grassVa[i*4+2].color.a = 50;
			grassVa[i*4+3].color.a = 50;
		}
		else if( grassVa[i*4].color.a == 254 )
		{
			grassVa[i*4].color.a = 255;
			grassVa[i*4+1].color.a = 255;
			grassVa[i*4+2].color.a = 255;
			grassVa[i*4+3].color.a = 255;
		}
	}
}

void TerrainPolygon::SetRenderMode(RenderMode rm)
{
	if (renderMode != RENDERMODE_TRANSFORM && rm == RENDERMODE_TRANSFORM)
	{
		VertexArray & v = *va;
		triBackups.resize(vaSize);
		for (int i = 0; i < vaSize; ++i)
		{
			triBackups[i] = v[i].position;
		}
	}
	else if (rm != RENDERMODE_TRANSFORM && renderMode == RENDERMODE_TRANSFORM)
	{
		VertexArray &v = *va;
		for (int i = 0; i < vaSize; ++i)
		{
			v[i].position = triBackups[i];
		}
		triBackups.clear();
	}

	renderMode = rm;
	
}

void TerrainPolygon::CancelTransformation()
{
	UpdateLinePositions();
	UpdateLineColors();
	SetRenderMode(RENDERMODE_NORMAL);

	

	//triBackups.clear();
}

PolyPtr TerrainPolygon::CompleteTransformation()
{
	if (renderMode == RENDERMODE_TRANSFORM)
	{
		SetRenderMode(RENDERMODE_NORMAL);

		PolyPtr newPoly(new TerrainPolygon);

		int numP = GetNumPoints();
		TerrainPoint *curr;
		Vector2i temp;

		newPoly->Reserve(numP);
		for (int i = 0; i < numP; ++i)
		{
			temp.x = round(lines[i * 2].position.x);
			temp.y = round(lines[i * 2].position.y);
			newPoly->AddPoint(temp, false);
		}

		UpdateLinePositions();

		newPoly->Finalize();

		//check for validity here
		
		//not even going to use the same polygon here, this is just for testing. 
		//what will really happen is that I create a copy, adding in the rounded points from my lines.
		//that way I can adjust and test for correctness just like i usually would, and then just
		//do a replacebrush action
		//maybe test for correctness?

		//SoftReset();
		//Finalize();

		return newPoly;
	}

	return NULL;
}

void TerrainPolygon::UpdateTransformation(TransformTools *tr)
{
	UpdateLinePositions();

	int numP = GetNumPoints();
	Vector2f fDiff;
	Vector2f fCurr;

	Transform t;
	t.rotate(tr->rotation);
	t.scale(tr->scale);

	int prevIndex;

	Vector2f center = tr->origCenter;
	Vector2f trCenter = tr->GetCenter();

	for (int i = 0; i < numP; ++i)
	{
		fCurr = lines[i * 2].position;

		fDiff = fCurr - center;
		fDiff = t.transformPoint(fDiff);

		fCurr = fDiff + trCenter;

		lines[i * 2].position = fCurr;

		if (i == 0)
		{
			lines[numP * 2 - 1].position = fCurr;
		}
		else
		{
			lines[i * 2 - 1].position = fCurr;
		}
	}

	VertexArray & v = *va;
	for (int i = 0; i < vaSize; ++i)
	{
		fCurr = triBackups[i];
		fDiff = fCurr - center;
		fDiff = t.transformPoint(fDiff);
		fCurr = fDiff + trCenter;

		v[i].position = fCurr;
	}
}

void TerrainPolygon::DrawBorderQuads(RenderTarget *target)
{
	if (totalNumBorderQuads > 0)
	{
		//target->draw(borderQuads, totalNumBorderQuads * 4, sf::Quads);
		target->draw(borderQuads, totalNumBorderQuads * 4, sf::Quads, ts_border->texture);
	}
}

void TerrainPolygon::Draw( bool showPath, double zoomMultiple, RenderTarget *rt, bool showPoints, TerrainPoint *dontShow )
{
	int numP = GetNumPoints();
	if( renderMode == RENDERMODE_MOVING_POINTS )
	{
		TerrainPoint *curr, *start, *next;
		int lineIndex = 0;

		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);
			next = GetNextPoint(i);

			lines[lineIndex * 2].position = Vector2f(curr->pos.x, curr->pos.y);
			lines[lineIndex * 2 + 1].position = Vector2f(next->pos.x, next->pos.y);

			++lineIndex;
		}
		rt->draw( lines, numP * 2, sf::Lines );
		return;
	}
	else if (renderMode == RENDERMODE_TRANSFORM)
	{
		if (va != NULL)
			rt->draw(*va, pShader);

		rt->draw(lines, numP * 2, sf::Lines);
		return;
	}


	if( grassVA != NULL )
		rt->draw(*grassVA, ts_grass->texture);

	if( va != NULL )
		rt->draw( *va, pShader );

	DrawBorderQuads(rt);

	DrawDecor(rt);

	DrawTouchGrass(rt);

	rt->draw( lines, numP * 2, sf::Lines );

	if( showPoints )
	{
		CircleShape cs;
		cs.setRadius( 8 * zoomMultiple );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Magenta );

		CircleShape csSel;
		csSel.setRadius(8 * zoomMultiple);
		csSel.setOrigin(csSel.getLocalBounds().width / 2, csSel.getLocalBounds().height / 2);
		csSel.setFillColor(Color::Green);

		int numP = GetNumPoints();
		TerrainPoint *curr;
		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);

			if (curr == dontShow)
			{
				continue;
			}

			if (curr->selected)
			{
				csSel.setPosition(curr->pos.x, curr->pos.y);
				rt->draw(csSel);
			}
			else
			{
				cs.setPosition(curr->pos.x, curr->pos.y);
				rt->draw(cs);
			}
		}
	}

	Vector2i center( (right + left) / 2, (bottom + top) / 2 );

	if( showPath )
	{
		for( list<Vector2i>::iterator it = path.begin(); it != path.end(); ++it )
		{
			CircleShape cs;
			cs.setRadius( EditSession::POINT_SIZE * zoomMultiple );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

			cs.setFillColor( Color::Magenta );
			cs.setPosition( center.x + (*it).x, center.y + (*it).y );
			rt->draw( cs );
	
		}


		if( path.size() > 1 )
		{
			list<Vector2i>::iterator prev = path.begin();
			list<Vector2i>::iterator curr = path.begin();
			++curr;
			while( curr != path.end() )
			{
				sf::Vertex activePreview[2] =
				{
					sf::Vertex(sf::Vector2<float>(center.x + (*prev).x, center.y + (*prev).y), Color::White ),
					sf::Vertex(sf::Vector2<float>(center.x + (*curr).x, center.y + (*curr).y), Color::White )
				};
				rt->draw( activePreview, 2, sf::Lines );

				prev = curr;
				++curr;
			}
		
		}
	}
}

void TerrainPolygon::SetSelected( bool select )
{
	selected = select;
	
	if( selected )
	{
		for( int i = 0; i < vaSize; ++i )
		{
			VertexArray & v = *va;
			v[i].color = selectCol;
		}
	}
	else
	{
		//Color testColor( 0x75, 0x70, 0x90 );
		for( int i = 0; i < vaSize; ++i )
		{
			VertexArray & v = *va;
			v[i].color = fillCol;
		}

		int numP = GetNumPoints();
		TerrainPoint *curr;
		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);
			curr->selected = false;
		}
	}
}

bool TerrainPolygon::IsOtherAABBWithinMine(PolyPtr poly)
{
	IntRect myAABB = GetAABB();
	if (inverse)
	{
		myAABB.left -= inverseExtraBoxDist;
		myAABB.top -= inverseExtraBoxDist;
		myAABB.width += 2 * inverseExtraBoxDist;
		myAABB.height += 2 * inverseExtraBoxDist;
	}

	IntRect polyAABB = poly->GetAABB();
	if (poly->inverse)
	{
		polyAABB.left -= inverseExtraBoxDist;
		polyAABB.top -= inverseExtraBoxDist;
		polyAABB.width += 2 * inverseExtraBoxDist;
		polyAABB.height += 2 * inverseExtraBoxDist;
	}

	return (polyAABB.left >= myAABB.left && polyAABB.top >= myAABB.top
		&& polyAABB.left + polyAABB.width <= myAABB.left + myAABB.width
		&& polyAABB.top + polyAABB.height <= myAABB.top + myAABB.height);
}

bool TerrainPolygon::AABBIntersection(PolyPtr poly)
{
	IntRect myAABB = GetAABB();
	if (inverse)
	{
		myAABB.left -= inverseExtraBoxDist;
		myAABB.top -= inverseExtraBoxDist;
		myAABB.width += 2 * inverseExtraBoxDist;
		myAABB.height += 2 * inverseExtraBoxDist;
	}

	IntRect polyAABB = poly->GetAABB();
	if (poly->inverse)
	{
		polyAABB.left -= inverseExtraBoxDist;
		polyAABB.top -= inverseExtraBoxDist;
		polyAABB.width += 2 * inverseExtraBoxDist;
		polyAABB.height += 2 * inverseExtraBoxDist;
	}

	return IsBoxTouchingBox(myAABB, polyAABB);
	//return myAABB.contains( polyAABB ) || myAABB.intersects(polyAABB);
	/*if (inverse)
	{

	}
	else
	{
		
		IntRect myAABB
		if (poly->left < left || poly->top < top || poly->right > right || poly->bottom > bottom)
		{
			return false;
		}
	}*/
}

void TerrainPolygon::SetupEdges()
{
	int numP = GetNumPoints();
	edges.resize(numP);
	TerrainPoint *curr, *next;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		next = GetNextPoint(i);

		edges[i].v0 = V2d(curr->pos);
		edges[i].v1 = V2d(next->pos);
	}

	int prevIndex, nextIndex;
	for (int i = 0; i < numP; ++i)
	{
		if (i == 0)
			prevIndex = numP - 1;
		else
			prevIndex = i - 1;

		if (i == numP - 1)
			nextIndex = 0;
		else
			nextIndex = i + 1;

		edges[i].edge0 = &edges[prevIndex];
		edges[i].edge1 = &edges[nextIndex];
	}
}

bool TerrainPolygon::ContainsPoint( Vector2f test )
{
	bool c = false;

	int i = 0;

	TerrainPoint *it, *jt;

	int numP = GetNumPoints();
	for (int i = 0; i < numP; ++i)
	{
		it = GetPoint(i);
		jt = GetPrevPoint(i);

		Vector2f point(it->pos.x, it->pos.y);
		Vector2f pointJ(jt->pos.x, jt->pos.y);
		if (((point.y > test.y) != (pointJ.y > test.y)) &&
			(test.x < (pointJ.x - point.x) * (test.y - point.y) / (pointJ.y - point.y) + point.x))
			c = !c;
	}


	//not sure if this will affect other stuff. i hope it wont. otherwise move this is out to the point
	//select area for adding to a selected brush instead of here
	if (inverse)
	{
		if (!c)
		{
			if (test.x >= left - inverseExtraBoxDist && test.x <= right + inverseExtraBoxDist
				&& test.y >= top - inverseExtraBoxDist && test.y <= bottom + inverseExtraBoxDist)
			{
				c = true;
			}
		}
		else
		{
			c = false;
		}
	}


	return c;
}

void TerrainPolygon::FixWinding()
{
    if (IsClockwise())
    {
		//cout << "not fixing" << endl;
    }
    else
    {
		std::reverse(PointVector().begin(), PointVector().end());
		int numP = GetNumPoints();
		for (int i = 0; i < numP; ++i)
		{
			GetPoint(i)->index = i;
		}
    }
}

void TerrainPolygon::UpdateLineColor( int i)
{
	TerrainPoint *curr, *next;

	Vector2f diff;
	
	int index = i * 2;
	int nextIndex = index + 1;
	/*if (i == GetNumPoints() - 1)
	{
		nextIndex = 0;
	}*/

	if (renderMode == RENDERMODE_TRANSFORM)
	{
		Vector2f roundedNextPos(round(lines[nextIndex].position.x), round(lines[nextIndex].position.y));
		Vector2f roundedPos(round(lines[index].position.x), round(lines[index].position.y));

		diff = roundedNextPos - roundedPos;
		
	}
	else
	{
		curr = GetPoint(i);
		next = GetNextPoint(i);

		diff = Vector2f(next->pos - curr->pos);
	}
	
	V2d dir = normalize(V2d(diff));
	V2d norm = V2d(dir.y, -dir.x);

	EdgeAngleType eat = GetEdgeAngleType(norm);

	Color edgeColor;
	switch (eat)
	{
	case EDGE_FLAT:
		edgeColor = Color::Red;
		break;
	case EDGE_SLOPED:
		edgeColor = Color::Green;
		break;
	case EDGE_STEEPSLOPE:
		edgeColor = Color::White;
		break;
	case EDGE_WALL:
		edgeColor = Color::Magenta;
		break;
	case EDGE_STEEPCEILING:
		edgeColor = Color::Yellow;
		break;
	case EDGE_SLOPEDCEILING:
		edgeColor = Color::Cyan;
		break;
	case EDGE_FLATCEILING:
		edgeColor = Color::Red;
		break;
	}

	lines[index].color = edgeColor;
	lines[nextIndex].color = edgeColor;
}

void TerrainPolygon::UpdateLineColors()
{
	int numP = GetNumPoints();
	for (int i = 0; i < numP; ++i)
	{
		UpdateLineColor(i);
	}
}

void TerrainPolygon::FixWindingInverse()
{
	if( !IsClockwise() )
    {
		//cout << "not fixing" << endl;
    }
    else
    {
		std::reverse(PointVector().begin(), PointVector().end());
		int numP = GetNumPoints();
		for (int i = 0; i < numP; ++i)
		{
			GetPoint(i)->index = i;
		}
    }
}

void TerrainPolygon::UpdateLinePositions()
{
	int numP = GetNumPoints();
	if (numP > 0)
	{
		int index = 0;


		TerrainPoint *curr = GetPoint(0);
		lines[0].position = sf::Vector2f(curr->pos.x, curr->pos.y);
		lines[2 * numP - 1].position = sf::Vector2f(curr->pos.x, curr->pos.y);
		++index;

		for (int i = 1; i < numP; ++i)
		{
			curr = GetPoint(i);
			lines[index].position = sf::Vector2f(curr->pos.x, curr->pos.y);
			lines[++index].position = sf::Vector2f(curr->pos.x, curr->pos.y);
			++index;
		}
	}
}

int TerrainPolygon::GetNumPoints()
{
	return PointVector().size();
}

TerrainPoint * TerrainPolygon::AddPoint(sf::Vector2i &p, bool sel)
{
	//just testing something
	/*if (GetNumPoints() > 0)
	{
		TerrainPoint *testEnd = GetEndPoint();
		if (p == testEnd->pos)
		{
			SetMaterialType(7, 1);
			int x = 5;
			return testEnd;
		}
	}*/

	if (!PointVector().empty())
	{
		if (p == GetEndPoint()->pos)
		{
			return NULL;
		}
	}


	PointVector().push_back(TerrainPoint(p, sel));
	TerrainPoint *end = GetEndPoint();
	end->index = GetNumPoints() - 1;
	return end;
}

TerrainPoint * TerrainPolygon::AddInverseBorderPoint(sf::Vector2i &p, bool sel)
{
	pointVector[0].push_back(TerrainPoint(p, sel));
	TerrainPoint *end = &pointVector[0].back();
	end->index = pointVector[0].size() - 1;
	return end;
}

bool TerrainPolygon::PointsTooCloseToSegInProgress(sf::Vector2i point,
	int minDistance, bool finalPoint )
{
	V2d p(point.x, point.y);

	
	TerrainPoint *curr, *next = NULL;// = GetPoint(i);

	Vector2i endPos = GetEndPoint()->pos;

	int i = 0;
	if (finalPoint)
	{
		i = 1;
	}

	int numP = GetNumPoints();
	for (; i < numP; ++i)
	{
		if (i >= numP - 2)
		{
			return false;
		}

		curr = GetPoint(i);
		if (SegmentWithinDistanceOfPoint(endPos, point, curr->pos, minDistance))
		{
			return true;
		}
	}

	return false;
}

bool TerrainPolygon::IsValidInProgressPoint(sf::Vector2i point)
{
	EditSession *session = EditSession::GetSession();
	int numP = GetNumPoints();
	if (numP == 0)
		return true;

	if (numP >= 3 && IsCloseToFirstPoint(session->GetZoomedPointSize(), V2d(point)) &&
		IsCompletionValid())
	{
		return true;
	}

	double minEdge = session->GetZoomedMinEdgeLength();
	{
		bool pointTooClose = PointTooClose(point, minEdge, true);
		bool linesIntersect = LinesIntersectInProgress(point);
		if (pointTooClose || linesIntersect )
		{
			return false;
		}

		if (PointsTooCloseToSegInProgress(point, minEdge) )
		{
			return false;
		}

		return true;
	}

	return false;
}

TerrainPoint *TerrainPolygon::GetSamePoint(sf::Vector2i &p)
{
	int numP = GetNumPoints();
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		if (curr->pos == p)
		{
			return curr;
		}
	}

	return NULL;
}

sf::Vector2i TerrainPolygon::TrimSliverPos(sf::Vector2<double> &prevPos,
	sf::Vector2<double> &pos, sf::Vector2<double> &nextPos,
	double minAngle, bool cw )
{
	double halfMinAngle = minAngle / 2;
	double angle = PI / 2.0 - halfMinAngle;

	double lenA = length(pos - prevPos);
	double lenB = length(nextPos - pos);
	double minLen = min(lenA, lenB);

	V2d adjA = prevPos;
	V2d adjB = nextPos;

	adjA = pos + normalize(prevPos - pos) * minLen;
	adjB = pos + normalize(nextPos - pos) * minLen;

	V2d dA = normalize(adjB - adjA);
	V2d dB = -dA;
	double xx = length(dA);
	double yy = length(dB);

	if (cw)
	{
		RotateCCW(dA, angle);
		RotateCW(dB, angle);
	}
	else
	{
		RotateCW(dA, angle);
		RotateCCW(dB, angle);
	}
	

	V2d aStart = adjA;
	V2d bStart = adjB;

	V2d aEnd = aStart + dA * 100.0;
	V2d bEnd = bStart + dB * 100.0;

	LineIntersection li = lineIntersection(aStart, aEnd, bStart, bEnd);
	if (!li.parallel)
	{

		double lenA = length(aStart - li.position);
		double lenB = length(bStart - li.position);
		double lenDA = length(dA);
		double lenDB = length(dB);
		//might miss minAngle slightly
		return Vector2i( round(li.position.x), round(li.position.y));
	}
	else
	{
		if (aStart == aEnd && aStart == bStart && aStart == bEnd)
		{
			cout << "all points the same clipping" << endl;
			return Vector2i(aStart); //all points the same?
		}
		else
		{
			cout << "another case?" << endl;
			return Vector2i(aStart);
		}
		
		
		//assert(0);
		//return Vector2i(-5, -5);
	}
}

bool TerrainPolygon::TryToMakeInternallyValid()
{
	return false;
	//remove slivers
	//align extremes
	//fix clusters
}

bool TerrainPolygon::TryFixAllSlivers()
{
	set<int> brokenSlivers;
	int sliverAttempts = 5;
	int numP = GetNumPoints();
	bool done = false;
	bool error;
	int s;
	for (s = 0; s < sliverAttempts; ++s)
	{
		done = true;
		for (int i = 0; i < numP; ++i)
		{
			error = false;
			if (FixSliver( i, brokenSlivers, error))
			{
				done = false;
				if (error)
					return false;
			}
		}
		
		if (s > 0)
		{
			int xxxxx = 5;
		}

		if (done)
			break;
	}

	if (s == sliverAttempts)
	{
		//error, too hard to fix slivers
		int xxx = 56;
		return false;
	}

	int brokenSize = brokenSlivers.size();
	if (brokenSize > 0)
	{
		if (numP - brokenSize < 3)
		{
			//resulting poly would be too small anyway
			return false;
		}

		std::vector<TerrainPoint> copyVec = PointVector();
		ClearPoints();
		for (int i = 0; i < numP; ++i)
		{
			if (brokenSlivers.find(i) != brokenSlivers.end())
			{
				continue;
			}

			AddPoint(copyVec[i].pos, false);
		}
	}

	return true;
}

bool TerrainPolygon::FixSliver(int i, std::set<int> &brokenSlivers, bool &error)
{
	double minAngle = EditSession::SLIVER_LIMIT;
	TerrainPoint *curr, *prev, *next;
	curr = GetPoint(i);
	//prev = GetPrevPoint(i);
	//next = GetNextPoint(i);
	int pSize = GetNumPoints();

	if (brokenSlivers.find(i) != brokenSlivers.end())
		return false;

	int temp;

	prev = NULL;
	for (int j = 1; j < pSize; ++j)
	{
		temp = i - j;
		if (temp < 0)
		{
			temp += pSize;
		}
		prev = GetPoint(temp);
		if (brokenSlivers.find(temp) != brokenSlivers.end())
		{
			prev = NULL;
		}
		else
		{
			break;
		}
	}
	if (prev == NULL)
	{
		error = true;
		return false;
	}

	next = NULL;
	for (int j = 1; j < pSize; ++j)
	{
		temp = i + j;
		if (temp >=  pSize )
		{
			temp -= pSize;
		}
		next = GetPoint(temp);
		if (brokenSlivers.find(temp) != brokenSlivers.end())
		{
			next = NULL;
		}
		else
		{
			break;
		}
	}
	if (next == NULL)
	{
		error = true;
		return false;
	}

	V2d pos(curr->pos.x, curr->pos.y);
	V2d prevPos(prev->pos.x, prev->pos.y);
	V2d nextPos(next->pos.x, next->pos.y);
	V2d dirA = normalize(prevPos - pos);
	V2d dirB = normalize(nextPos - pos);

	double diff = GetVectorAngleDiffCCW(dirA, dirB);
	double diffCW = GetVectorAngleDiffCW(dirA, dirB);
	if (approxEquals(dirA.x, -dirB.x) && approxEquals(dirA.y, -dirB.y))
	{
		brokenSlivers.insert(i);
		return true;
	}
	else if (diff < minAngle)
	{
		Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, true);
		curr->pos = trimPos;

		if (curr->pos == prev->pos || curr->pos == next->pos)
		{
			brokenSlivers.insert(i);
			return true;
		}
	}
	else if (diffCW < minAngle)
	{
		Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, false);
		curr->pos = trimPos;

		if (curr->pos == prev->pos || curr->pos == next->pos)
		{
			brokenSlivers.insert(i);
			return true;
		}
	}

	return false;
}

bool TerrainPolygon::FixSliver(int i)
{
	double minAngle = EditSession::SLIVER_LIMIT;
	TerrainPoint *curr, *prev, *next;
	curr = GetPoint(i);
	prev = GetPrevPoint(i);
	next = GetNextPoint(i);

	V2d pos(curr->pos.x, curr->pos.y);
	V2d prevPos(prev->pos.x, prev->pos.y);
	V2d nextPos(next->pos.x, next->pos.y);
	V2d dirA = normalize(prevPos - pos);
	V2d dirB = normalize(nextPos - pos);

	double diff = GetVectorAngleDiffCCW(dirA, dirB);
	double diffCW = GetVectorAngleDiffCW(dirA, dirB);
	if (diff < minAngle)
	{
		Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, true);
		curr->pos = trimPos;

		if (curr->pos == prev->pos || curr->pos == next->pos)
		{
			int b = 6;
			assert(0);
		}
		return true;
	}
	else if (diffCW < minAngle)
	{
		Vector2i trimPos = TrimSliverPos(prevPos, pos, nextPos, minAngle, false);
		curr->pos = trimPos;

		if (curr->pos == prev->pos || curr->pos == next->pos)
		{
			int b = 6;
			assert(0);
		}
		return true;
	}

	return false;
}

void TerrainPolygon::RemoveSlivers()
{	
	//check for slivers that are at too extreme of an angle. tiny triangle type things
	int numP = GetNumPoints();
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		FixSliver(i);
	}
}

void TerrainPolygon::RemoveLastPoint()
{
	if (PointVector().empty())
		return;

	TerrainPoint *end = GetEndPoint();

	if (finalized || end->gate != NULL || enemies[end].size() != 0)
	{
		assert(0); //cant remove points when things are attached!
	}
	else
	{
		PointVector().pop_back();
	}
}

void TerrainPolygon::Reset()
{
	SoftReset();
	ClearPoints();
	/*if (lines != NULL)
		delete[] lines;
	if (va != NULL)
		delete va;
	if (grassVA != NULL)
		delete grassVA;

	if (borderQuads != NULL)
		delete[] borderQuads;

	lines = NULL;
	va = NULL;
	grassVA = NULL;
	finalized = false;


	myTerrainTree->Clear();

	DestroyTouchGrass();*/
}

void TerrainPolygon::SoftReset()
{
	if (lines != NULL)
		delete[] lines;
	if (va != NULL)
		delete va;
	if (grassVA != NULL)
		delete grassVA;

	if (borderQuads != NULL)
		delete[] borderQuads;

	lines = NULL;
	va = NULL;
	grassVA = NULL;
	finalized = false;
	//myTerrainTree->Clear();

	DestroyTouchGrass();

	for (auto it = decorExprList.begin(); it != decorExprList.end(); ++it)
	{
		delete (*it);
	}
}

void TerrainPolygon::ClearPoints()
{
	PointVector().clear();
	edges.clear();
}


int TerrainPolygon::GetNumSelectedPoints()
{
	int numP = GetNumPoints();
	int numSelected = 0;
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		if (curr->selected)
		{
			++numSelected;
		}
	}

	return numSelected;
}

PolyPtr TerrainPolygon::CreateCopyWithSelectedPointsRemoved()
{
	int numP = GetNumPoints();
	int numDeletePoints = GetNumSelectedPoints();
	int newPolyPoints = numP - numDeletePoints;

	if (newPolyPoints < 3)
	{
		return NULL;
	}

	PolyPtr newPoly = new TerrainPolygon();
	newPoly->Reserve(newPolyPoints);

	newPoly->layer = 0;
	newPoly->inverse = inverse;
	newPoly->terrainWorldType = terrainWorldType;
	newPoly->terrainVariation = terrainVariation;

	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		if (!curr->selected)
		{
			newPoly->AddPoint(curr->pos, false);
		}
	}

	return newPoly;
}

//0 means a window came up and they canceled. -1 means no enemies were in danger on that polygon, 1 means that you confirmed to delete the enemies
int TerrainPolygon::IsRemovePointsOkayEnemies( EditSession *edit )
{
	TerrainPoint *next;
	for( EnemyMap::iterator mapIt = enemies.begin(); mapIt != enemies.end(); ++mapIt )
	{
		for( list<ActorPtr>::iterator it = (*mapIt).second.begin(); it != (*mapIt).second.end(); ++it )
		{
			next = (*it)->posInfo.GetNextPoint();

			if( (*it)->type->CanBeGrounded() && 
				( (*(*it)->posInfo.GetPoint()).selected || next->selected ) )
			{
				bool removeSelectedActors = edit->ConfirmationPop("1 or more enemies will be removed by deleting these points.");

				if( removeSelectedActors )
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
	}

	return -1;	
}

sf::IntRect TerrainPolygon::GetAABB()
{
	return IntRect(left, top, right - left, bottom - top);
}

V2d TerrainPolygon::GetDCenter()
{
	return V2d((left + right) / 2.0,
		(top + bottom) / 2.0);
}

bool TerrainPolygon::IsClockwise()
{
	int numP = GetNumPoints();

	assert(numP > 0);

	TerrainPoint *curr, *prev;

	long long int sum = 0;
	Vector2<long long int> first, second;

	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		prev = GetPrevPoint(i);

		first.x = prev->pos.x;
		first.y = prev->pos.y;

		second.x = curr->pos.x;
		second.y = curr->pos.y;

		sum += (second.x - first.x) * (second.y + first.y);
	}

	return sum < 0;
}

PolyPtr TerrainPolygon::Copy()
{
	PolyPtr newPoly = new TerrainPolygon(*this, true);
	newPoly->Finalize();
	return newPoly;
}

void TerrainPolygon::CopyPoints(PolyPtr poly, bool storeSelected )
{
	bool sel;
	int polyNumP = poly->GetNumPoints();
	Reserve(polyNumP);

	TerrainPoint *polyCurr;
	for (int i = 0; i < polyNumP; ++i)
	{
		polyCurr = poly->GetPoint(i);
		if (storeSelected)
		{
			sel = polyCurr->selected;
		}
		else
		{
			sel = false;
		}

		AddPoint(polyCurr->pos, sel);
	}
}

bool TerrainPolygon::IsTouchingEnemiesFromPoly(PolyPtr p)
{
	for (EnemyMap::iterator it = p->enemies.begin(); it != p->enemies.end(); ++it)
	{
		for (list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait)
		{
			sf::VertexArray &bva = (*ait)->boundingQuad;

			if (EditSession::QuadPolygonIntersect(p, Vector2i(bva[0].position.x, bva[0].position.y),
				Vector2i(bva[1].position.x, bva[1].position.y), Vector2i(bva[2].position.x, bva[2].position.y),
				Vector2i(bva[3].position.x, bva[3].position.y)))
			{
				return true;
			}
		}
	}

	return false;
}

//returns true if LinesIntersect or 
bool TerrainPolygon::IsTouching( PolyPtr p  )
{
	if (p == this)
		return false;

	if( left <= p->right && right >= p->left && top <= p->bottom && bottom >= p->top )
		return LinesIntersect(p);

	return false;
}

void TerrainPolygon::ShowGrass( bool show )
{
	if (grassVA == NULL)
		return;

	VertexArray & grassVa = *grassVA;
	for( int i = 0; i < numGrassTotal; ++i )
	{
		if( show )
		{
			if( grassVa[i*4].color.a == 0 )
			{
				grassVa[i*4].color.a = 50;
				grassVa[i*4+1].color.a = 50;
				grassVa[i*4+2].color.a = 50;
				grassVa[i*4+3].color.a = 50;
			}
			isGrassShowing = true;
		}
		else 
		{
			if( grassVa[i*4].color.a == 50 )
			{
				grassVa[i*4].color.a = 0;
				grassVa[i*4+1].color.a = 0;
				grassVa[i*4+2].color.a = 0;
				grassVa[i*4+3].color.a = 0;
			}
			isGrassShowing = false;
		}
	}
}

//returns 2 is lines intersect, 1 is only 1 point intersects, and 0 if no intersection
int TerrainPolygon::LinesIntersect( PolyPtr poly )
{
	//my lines vs his lines

	int numP = GetNumPoints();
	int polyNumP = poly->GetNumPoints();

	TerrainPoint *curr, *prev;
	TerrainPoint *polyCurr, *polyPrev;

	int pointTouchCount = 0;

	bool pointIsTouched  = false;
	Vector2i touchedPoint;

	//set<pair<int,int>> touchedPoints;

	Vector2i lii;
	double xi, yi;
	double xd, yd;
	double lid;

	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		prev = GetPrevPoint(i);

		for (int j = 0; j < polyNumP; ++j)
		{
			polyCurr = poly->GetPoint(j);
			polyPrev = poly->GetPrevPoint(j);

			LineIntersection li = EditSession::SegmentIntersect(prev->pos, curr->pos, polyPrev->pos, polyCurr->pos);
			if (!li.parallel)
			{
				xi, yi;
				xd, yd;
				lid = li.position.x;
				xd = std::modf(li.position.x, &xi);
				yd = std::modf(li.position.y, &yi);

				if (xd == 0.0 && yd == 0.0)
				{
					lii = Vector2i(xi, yi);
					if (lii == prev->pos || lii == curr->pos || lii == polyPrev->pos || lii == polyCurr->pos)
					{
						if (!pointIsTouched)
						{
							pointIsTouched = true;
							touchedPoint = lii;
							continue;
						}
						else if( lii == touchedPoint )
						{
							continue;
						}
					}
				}
				return 2;
			}

		}
	}

	if (pointIsTouched)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool TerrainPolygon::IsCompletionValid()
{
	EditSession *session = EditSession::GetSession();

	if (GetNumPoints() < 3)
		return false;

	bool linesIntersect = LinesIntersectInProgress(GetPoint(0)->pos);
	if (linesIntersect)
	{
		//cout << "lines intersect" << endl;
		return false;
	}

	double minEdge = session->GetZoomedMinEdgeLength();

	if (PointsTooCloseToSegInProgress(GetPoint(0)->pos, minEdge, true))
	{
		return false;
	}

	if (session->PolyIntersectsGates(this))
	{
		return false;
	}
		

	return true;
}

bool TerrainPolygon::LinesIntersectInProgress(Vector2i p)
{
	//my lines vs his lines

	int numP = GetNumPoints();
	TerrainPoint *curr, *next, *end;
	end = GetEndPoint();

	for (int i = 0; i < numP-1; ++i)
	{
		curr = GetPoint(i);
		next = GetNextPoint(i);

		if (i == numP - 2)
		{
			V2d a(curr->pos - next->pos);
			V2d b(p - next->pos);

			double d = dot(normalize(a), normalize(b));

			if (d > .999)
			{
				return true;
			}

			return false;
		}

		LineIntersection li = EditSession::LimitSegmentIntersect(curr->pos, next->pos, end->pos, p);
		if (!li.parallel)
		{
			return true;
		}
	}


	return false;
}

bool TerrainPolygon::PointTooCloseToPoints( Vector2i point, int minDistance )
{
	V2d p( point.x, point.y );

	int numP = GetNumPoints();
	TerrainPoint *curr;

	V2d currP, diff;

	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);

		currP = V2d(curr->pos.x, curr->pos.y);
		diff = p - currP;

		if (lengthSqr(diff) < minDistance * minDistance)
		{
			return true;
		}
	}

	return false;
}

bool TerrainPolygon::Contains( PolyPtr poly )
{
	if (poly == this)
		return false;

	//hes inside me w/ no intersection
	if (!IsOtherAABBWithinMine(poly))
	{
		return false;
	}

	/*if (poly->left < left || poly->top < top || poly->right > right || poly->bottom > bottom)
	{
		return false;
	}*/


	int polyNumP = poly->GetNumPoints();
	TerrainPoint *polyCurr;
	for (int i = 0; i < polyNumP; ++i)
	{
		polyCurr = poly->GetPoint(i);
		if (!ContainsPoint(Vector2f(polyCurr->pos.x, polyCurr->pos.y)))
		{
			return false;
		}
	}

	return true;
}

bool TerrainPolygon::PointTooClose( sf::Vector2i point, int minDistance, bool inProgress )
{
	bool a = PointTooCloseToPoints( point, minDistance );
	bool b = PointTooCloseToLines( point, minDistance, inProgress );
	if( a || b )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TerrainPolygon::PointTooCloseToLines( sf::Vector2i point, int minDistance, bool inProgress)
{
	V2d p( point.x, point.y );

	int numP = GetNumPoints();
	TerrainPoint *curr, *prev;
	for (int i = 0; i < numP; ++i)
	{
		if (inProgress && i == 0)
		{
			continue;
		}

		curr = GetPoint(i);
		prev = GetPrevPoint(i);
		
		if (SegmentWithinDistanceOfPoint(prev->pos, curr->pos, point, minDistance))
		{
			return true;
		}
	}

	return false;
}

//points are circles, and the lines are bars, and you're testing to see if a point is within that or not.
bool TerrainPolygon::SegmentWithinDistanceOfPoint( sf::Vector2i startSeg, sf::Vector2i endSeg, sf::Vector2i BlahPoint, int distance )
{
	V2d p( BlahPoint.x, BlahPoint.y );

	V2d v0 = V2d( startSeg.x, startSeg.y );
	V2d v1 = V2d( endSeg.x, endSeg.y );
	V2d edgeDir = normalize( v1 - v0 );

	double quant = dot( p - v0, edgeDir );
	double offQuant = cross( p - v0, edgeDir );
	bool nearOnAxis = quant > 0 && quant < length( v1 - v0 );

	bool nearOffAxis = abs( offQuant ) < distance;

	if( nearOnAxis && nearOffAxis )
	{
		//cout << "false type two. quant:" << quant << ", offquant: " << offQuant << endl;
		//cout << "p: " << p.x << ", " << p.y << endl;
		//cout << "v0: " << v0.x << ", " << v0.y << endl;
		//cout << "v1: " << v1.x << ", " << v1.y << endl;
		return true;
	}

	return false;
}

TerrainPoint *TerrainPolygon::GetClosePoint(double radius, V2d &wPos)
{
	if (wPos.x <= right + radius && wPos.x >= left - radius
		&& wPos.y <= bottom + radius && wPos.y >= top - radius)
	{
		int numP = GetNumPoints();
		TerrainPoint *curr;
		for (int i = 0; i < numP; ++i)
		{
			curr = GetPoint(i);
			if (length(wPos - V2d(curr->pos.x, curr->pos.y)) <= radius)
			{
				return curr;
			}
		}
	}
	return NULL;
}

bool TerrainPolygon::IsCloseToFirstPoint(double radius, V2d &p)
{
	TerrainPoint *start = GetPoint(0);
	if (length(p - V2d(start->pos.x, start->pos.y)) <= radius)
	{
		return true;
	}

	return false;
}

void TerrainPolygon::Reserve( int nPoints )
{
	PointVector().reserve(nPoints);
}


bool TerrainPolygon::Load(std::ifstream &is)
{
	int matWorld;
	int matVariation;
	is >> matWorld;
	is >> matVariation;

	SetMaterialType(matWorld, matVariation);

	int polyPoints;
	is >> polyPoints;

	Reserve(polyPoints);
	for (int j = 0; j < polyPoints; ++j)
	{
		int x, y;
		is >> x;
		is >> y;
		AddPoint(Vector2i(x, y), false);
	}

	return true;
}

//ISELECTABLE FUNCTIONS




bool TerrainPolygon::Intersects( sf::IntRect rect )
{
	if (rect.width == 0 || rect.height == 0)
		return false;

	sf::Rect<double> rectD(rect);

	if (!IsTouchingBox(rectD))
	{
		return false;
	}

	if (CheckRectIntersectEdges(rectD))
	{
		return true;
	}
	/*emptyResult = true;
	sf::Rect<double> r(rect);
	myTerrainTree->Query(this, r);
	if (!emptyResult)
		return true;*/

	Vector2f center(rect.left + rect.width / 2, rect.top + rect.height / 2);
	if (ContainsPoint(center))
	{
		return true;
	}

	if (rectD.contains(GetDCenter()))
	{
		return true;
	}

	return false;
	//TerrainPolygon poly;
	//poly.AddPoint( Vector2i( rect.left, rect.top ), false );
	//poly.AddPoint( Vector2i( rect.left + rect.width, rect.top ), false);
	//poly.AddPoint( Vector2i( rect.left + rect.width, rect.top + rect.height ), false);
	//poly.AddPoint( Vector2i( rect.left, rect.top + rect.height ), false);

	//poly.UpdateBounds();

	//if( IsTouching( &poly ) || poly.Contains( this ) ) //don't need this contains polys cuz im just using this for selection for now
	//{
	//	return true;
	//}
	//else
	//	return false;

	return false;
}

bool TerrainPolygon::IsPlacementOkay()
{
	return false;
}

void TerrainPolygon::BrushDraw( sf::RenderTarget *target, bool valid )
{
	target->draw(lines, GetNumPoints() * 2, sf::Lines);
}

void TerrainPolygon::Draw( sf::RenderTarget *target )
{
	Draw(false, 1.0, target, false, NULL);
}

void TerrainPolygon::CopyPointsToClipperPath(ClipperLib::Path & p)
{
	int numP = GetNumPoints();
	TerrainPoint *curr;
	p.reserve(numP);
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		p << ClipperLib::IntPoint(curr->pos.x, curr->pos.y);
	}
}

void TerrainPolygon::AddPointsFromClipperPath(ClipperLib::Path &p)
{
	//disallow duplicates
	int pSize = p.size();
	//Reserve(p.size());
	ClipperLib::IntPoint *prev, *curr;
	for (int i = 0; i < pSize; ++i)
	{
		curr = &p[i];

		if (i == 0)
			prev = &p[pSize - 1];
		else
			prev = &p[i - 1];

		//if removing slivers has given me the same point twice
		if (prev->X == curr->X && prev->Y == curr->Y)
		{
			continue;
		}

		AddPoint(Vector2i(curr->X, curr->Y), false);
	}
}

void TerrainPolygon::AddPointsFromClipperPath(ClipperLib::Path &p,
	ClipperIntPointSet &fusedPoints)
{
	//disallow duplicates
	int pSize = p.size();
	ClipperLib::IntPoint *prev, *curr;
	TerrainPoint *end;
	for (int i = 0; i < pSize; ++i)
	{
		curr = &p[i];

		if (i == 0)
			prev = &p[pSize - 1];
		else
			prev = &p[i - 1];

		//this is handled in AddPoint
		/*if (prev->X == curr->X && prev->Y == curr->Y)
		{
			continue;
		}*/

		if (fusedPoints.find(make_pair(curr->X, curr->Y)) != fusedPoints.end())
		{
			continue;
		}

		AddPoint(Vector2i(curr->X, curr->Y), false);
	}
}

void TerrainPolygon::AddPointsFromClipperPath(ClipperLib::Path &p, ClipperLib::Path &clipperIntersections,
	list<TerrainPoint*> &intersections)
{
	TerrainPoint *t;
	for (auto it = p.begin(); it != p.end(); ++it)
	{
		t = AddPoint(Vector2i((*it).X, (*it).Y), false);
		for (auto intersectIt = clipperIntersections.begin(); intersectIt != clipperIntersections.end(); ++intersectIt)
		{
			if ((*intersectIt).X == (*it).X && (*intersectIt).Y == (*it).Y)
			{
				intersections.push_back(t);
				break;
			}
		}
	}
}


void TerrainPolygon::AddGatesToBrush(Brush *b,
	list<GateInfoPtr> &gateInfoList)
{
	bool okGate = true;

	int numP = GetNumPoints();
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		okGate = true;
		if (curr->gate != NULL)
		{
			for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
			{
				if ((*it) == curr->gate)
				{
					okGate = false;
					break;
				}
			}
			if (okGate)
			{
				b->AddObject(curr->gate);
				gateInfoList.push_back(curr->gate);
			}

		}
	}
}

void TerrainPolygon::AddEnemiesToBrush(Brush *b)
{
	for (auto mit = enemies.begin(); mit != enemies.end(); ++mit)
	{
		for (auto eit = (*mit).second.begin(); eit != (*mit).second.end(); ++eit)
		{
			b->AddObject((*eit));
		}
	}
}

void TerrainPolygon::AddGatesToList(std::list<GateInfoPtr> &gates)
{
	bool alreadyInList = false;

	int numP = GetNumPoints();
	TerrainPoint *curr;
	for (int i = 0; i < numP; ++i)
	{
		curr = GetPoint(i);
		if (curr->gate != NULL)
		{
			alreadyInList = false;
			for (auto it = gates.begin(); it != gates.end(); ++it)
			{
				if ((*it) == curr->gate)
				{
					alreadyInList = true;
					break;
				}
			}

			if (!alreadyInList)
			{
				gates.push_back(curr->gate);
			}
		}
	}
}

void TerrainPolygon::HandleQuery(QuadTreeCollider *qtc)
{
	qtc->HandleEntrant(this);
}

bool TerrainPolygon::IsTouchingBox(const sf::Rect<double> &r)
{
	Rect<double> aabb(GetAABB());
	return IsBoxTouchingBox(aabb, r);
}

TerrainPoint::TerrainPoint( sf::Vector2i &p, bool s )
	:pos( p ), selected( s ), gate( NULL )
{
}

int TerrainPoint::GetIndex()
{
	return index;
}

bool TerrainPoint::ContainsPoint( Vector2f test )
{
	bool contains = length( V2d( test.x, test.y ) - V2d( pos.x, pos.y ) ) <= POINT_RADIUS;
	return contains;
}


bool TerrainPoint::Intersects( IntRect rect )
{
	return false;
}

bool TerrainPoint::HasPrimaryGate(Vector2i &gateDir)
{
	gateDir = Vector2i(0, 0);
	if (gate != NULL)
	{
		Vector2i myPoint, otherPoint;
		if (gate->point0 == this)
		{
			myPoint = gate->point0->pos;
			otherPoint = gate->point1->pos;
		}
		else
		{
			myPoint = gate->point1->pos;
			otherPoint = gate->point0->pos;
		}

		Vector2i diff = otherPoint - myPoint;
		if (diff.y == 0 )
		{
			if (diff.x > 0)
			{
				gateDir.x = 1;
			}
			else if( diff.x < 0 )
			{
				gateDir.x = -1;
			}
			else
			{
				assert(0);
			}
		}
		else if (diff.x == 0)
		{
			if (diff.y > 0)
			{
				gateDir.y = 1;
			}
			else if( diff.y < 0 )
			{
				gateDir.y = -1;
			}
			else
			{
				assert(0);
			}
		}

		return true;
	}

	return false;
}