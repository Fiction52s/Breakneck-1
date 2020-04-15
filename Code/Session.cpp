#include "Session.h"
#include "Actor.h"
#include "MainMenu.h"
#include "MapHeader.h"
#include "EditorTerrain.h"
#include "EditorRail.h"
#include "EditorActors.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

Session *Session::GetSession()
{
	EditSession *edit = EditSession::GetSession();
	if (edit != NULL)
		return edit;

	//add GameSession in here eventually

	return NULL;
}

Session::Session(const boost::filesystem::path &p_filePath)
{
	cutPlayerInput = false;
	mainMenu = MainMenu::GetInstance();
	assert(mainMenu != NULL);

	window = mainMenu->window;

	filePath = p_filePath;
	filePathStr = filePath.string();

	players.resize(MAX_PLAYERS);
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		players[i] = nullptr;
	}

	terrainTree = NULL;
	specialTerrainTree = NULL;
	railEdgeTree = NULL;
	barrierTree = NULL;

	polyShaders = NULL;
}

Session::~Session()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		delete players[i];
	}

	if (terrainTree != NULL)
		delete terrainTree;

	if (specialTerrainTree != NULL)
		delete specialTerrainTree;

	if (railEdgeTree != NULL)
		delete railEdgeTree;

	if (barrierTree != NULL)
		delete barrierTree;

	if (mapHeader != NULL)
		delete mapHeader;

	if( polyShaders != NULL)
		delete[] polyShaders;

	for (auto it = terrainDecorInfoMap.begin(); it != terrainDecorInfoMap.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = decorLayerMap.begin(); it != decorLayerMap.end(); ++it)
	{
		delete (*it).second;
	}
}

void Session::UpdateDecorLayers()
{
	for (auto mit = decorLayerMap.begin(); mit != decorLayerMap.end();
		++mit)
	{
		(*mit).second->Update();
	}
}

void Session::UpdateDecorSprites()
{
	//update relevant terrain pieces, need to do a check for camera etc.
}

void Session::AllocatePolyShaders(int numPolyTypes)
{
	numPolyShaders = numPolyTypes;
	polyShaders = new Shader[numPolyShaders];
	//polyShaders.resize(numPolyTypes);
	ts_polyShaders.resize(numPolyTypes);
}

void Session::DrawPlayerWires( RenderTarget *target )
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DrawWires(target);
		}
	}
}

void Session::UpdatePlayerWireQuads()
{
	Actor *p;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->UpdateWireQuads();
		}
	}
}

bool Session::ReadDecorImagesFile()
{
	ifstream is;
	is.open("Resources/decor.txt");
	if (is.is_open())
	{
		string name;
		int width;
		int height;
		int tile;
		while (!is.eof())
		{
			is >> name;
			is >> width;
			is >> height;

			is >> tile;

			string fullName = name + string(".png");

			Tileset *ts = GetTileset(fullName, width, height);
			assert(ts != NULL);
			decorTSMap[name] = ts;

			ProcessDecorFromFile(name, tile);
		}

		is.close();
	}
	else
	{
		return false;
	}

	return true;
}

bool Session::ReadDecorInfoFile(int tWorld, int tVar)
{
	stringstream ss;
	ifstream is;

	ss << "Resources/Terrain/Decor/" << "terraindecor_"
		<< (tWorld + 1) << "_0" << (tVar + 1) << ".txt";
	is.open(ss.str());

	if (is.is_open())
	{
		list<pair<string, int>> loadedDecorList;
		while (!is.eof())
		{
			string dStr;
			is >> dStr;

			if (dStr == "")
			{
				break;
			}

			int frequencyPercent;
			is >> frequencyPercent;

			loadedDecorList.push_back(pair<string, int>(dStr, frequencyPercent));
		}
		is.close();

		TerrainDecorInfo *tdInfo = new TerrainDecorInfo(loadedDecorList.size());
		terrainDecorInfoMap[make_pair(tWorld, tVar)] = tdInfo;

		int dI = 0;
		for (auto it = loadedDecorList.begin(); it != loadedDecorList.end(); ++it)
		{
			tdInfo->decors[dI] = TerrainPolygon::GetDecorType((*it).first);
			tdInfo->percents[dI] = (*it).second;
			++dI;
		}
	}
	else
	{
		return false;
		//not found, thats fine.
	}
}

bool Session::ReadHeader(std::ifstream &is)
{
	mapHeader = mainMenu->ReadMapHeader(is);
	if (mapHeader == NULL)
		return false;

	ProcessHeader();

	return true;
}

bool Session::ReadDecor(std::ifstream &is)
{
	int numDecorImages;
	is >> numDecorImages;

	for (int i = 0; i < numDecorImages; ++i)
	{
		string dName;
		is >> dName;
		int dLayer;
		is >> dLayer;

		Vector2f dPos;
		is >> dPos.x;
		is >> dPos.y;

		float dRot;
		is >> dRot;

		Vector2f dScale;
		is >> dScale.x;
		is >> dScale.y;

		int dTile;
		is >> dTile;

		Sprite dSpr;
		dSpr.setScale(dScale);
		dSpr.setRotation(dRot);
		dSpr.setPosition(dPos);

		Tileset *d_ts = decorTSMap[dName];
		dSpr.setTexture(*d_ts->texture);
		dSpr.setTextureRect(d_ts->GetSubRect(dTile));
		dSpr.setOrigin(dSpr.getLocalBounds().width / 2, dSpr.getLocalBounds().height / 2);

		ProcessDecorSpr( dName, dSpr, dLayer, d_ts, dTile );
	}

	return true;
}

bool Session::ReadPlayerStartPos(std::ifstream &is)
{
	is >> playerOrigPos.x;
	is >> playerOrigPos.y;

	ProcessPlayerStartPos();

	return true;
}

bool Session::ReadTerrainGrass(std::ifstream &is, PolyPtr poly)
{
	int edgesWithSegments;
	is >> edgesWithSegments;

	list<GrassSeg> segments;
	for (int i = 0; i < edgesWithSegments; ++i)
	{
		int edgeIndex;
		is >> edgeIndex;

		int numSegments;
		is >> numSegments;

		for (int j = 0; j < numSegments; ++j)
		{
			int index;
			is >> index;
			int reps;
			is >> reps;
			segments.push_back(GrassSeg(edgeIndex, index, reps));

		}
	}

	int grassIndex = 0;
	VertexArray &grassVa = *poly->grassVA;
	int polyNumP = poly->GetNumPoints();
	bool rem;

	int itReps;

	int *indexArray = new int[polyNumP];
	for (int i = 0; i < polyNumP; ++i)
	{
		indexArray[i] = grassIndex;
		grassIndex += poly->GetNumGrass(i, rem);;
	}

	for (list<GrassSeg>::iterator it = segments.begin(); it != segments.end(); ++it)
	{
		int vaIndex = indexArray[(*it).edgeIndex];
		itReps = (*it).reps;
		for (int extra = 0; extra <= itReps; ++extra)
		{
			grassVa[(vaIndex + (*it).index + extra) * 4].color.a = 255;
			grassVa[(vaIndex + (*it).index + extra) * 4 + 1].color.a = 255;
			grassVa[(vaIndex + (*it).index + extra) * 4 + 2].color.a = 255;
			grassVa[(vaIndex + (*it).index + extra) * 4 + 3].color.a = 255;
		}
	}

	delete[] indexArray;

	return true;
}

bool Session::ReadTerrain(std::ifstream &is)
{
	string hasBorderPolyStr;
	is >> hasBorderPolyStr;

	bool hasBorderPoly;
	bool hasReadBorderPoly;
	if (hasBorderPolyStr == "borderpoly")
	{
		hasBorderPoly = true;
		hasReadBorderPoly = false;
	}
	else if (hasBorderPolyStr == "no_borderpoly")
	{
		hasBorderPoly = false;
		hasReadBorderPoly = true;
	}
	else
	{
		cout << hasBorderPolyStr << endl;
		assert(0 && "what is this string?");
	}

	int numPoints = mapHeader->numVertices;

	while (numPoints > 0)
	{
		PolyPtr poly(new TerrainPolygon());

		if (!hasReadBorderPoly)
		{
			poly->inverse = true;
			hasReadBorderPoly = true;
		}

		poly->Load(is);
		numPoints -= poly->GetNumPoints();
		poly->Finalize(); //doesn't get affected by reading grass

		ReadTerrainGrass(is, poly);

		ProcessTerrain(poly);
	}
	return true;
}

bool Session::ReadSpecialTerrain(std::ifstream &is)
{
	int specialPolyNum;
	is >> specialPolyNum;

	for (int i = 0; i < specialPolyNum; ++i)
	{
		PolyPtr poly(new TerrainPolygon());

		poly->Load(is);

		poly->Finalize();

		ProcessSpecialTerrain(poly);
	}

	return true;
}

bool Session::ReadBGTerrain(std::ifstream &is)
{
	int bgPlatformNum0;
	is >> bgPlatformNum0;
	for (int i = 0; i < bgPlatformNum0; ++i)
	{
		PolyPtr poly(new TerrainPolygon());
		poly->Load(is);
		poly->Finalize();
		poly->SetLayer(1);

		ProcessBGTerrain(poly);
		//no grass for now
	}
	return true;
}

bool Session::ReadRails(std::ifstream &is)
{
	int numRails;
	is >> numRails;

	for (int i = 0; i < numRails; ++i)
	{
		RailPtr rail(new TerrainRail());
		rail->Load(is);
		ProcessRail(rail);
	}
	return true;
}

//bool Session::ReadActors(std::ifstream &is)
//{
//	//enemies here
//	int numGroups;
//	is >> numGroups;
//	cout << "num groups " << numGroups << endl;
//
//	for (int i = 0; i < numGroups; ++i)
//	{
//		string groupName;
//		is >> groupName;
//
//		int numActors;
//		is >> numActors;
//
//		ActorGroup *gr = new ActorGroup(groupName);
//		groups[groupName] = gr;
//
//		for (int j = 0; j < numActors; ++j)
//		{
//			string typeName;
//			is >> typeName;
//
//			//ActorParams *a; //= new ActorParams;
//			ActorPtr a(NULL);
//
//
//
//			ActorType *at = NULL;
//			cout << "typename: " << typeName << endl;
//			if (types.count(typeName) == 0)
//			{
//				cout << "TYPENAME: " << typeName << endl;
//				assert(false && "bad typename");
//			}
//			else
//			{
//				at = types[typeName];
//			}
//
//			at->LoadEnemy(is, a);
//
//			gr->actors.push_back(a);
//			a->group = gr;
//
//
//			mapStartBrush->AddObject(a);
//		}
//	}
//
//	return true;
//}

bool Session::ReadGates(std::ifstream &is)
{
	int numGates;
	is >> numGates;

	int gType;
	int poly0Index, vertexIndex0, poly1Index, vertexIndex1;
	int shardWorld = -1;
	int shardIndex = -1;
	for (int i = 0; i < numGates; ++i)
	{
		shardWorld = -1;
		shardIndex = -1;

		is >> gType;
		is >> poly0Index;
		is >> vertexIndex0;
		is >> poly1Index;
		is >> vertexIndex1;

		if (gType == Gate::SHARD)
		{
			is >> shardWorld;
			is >> shardIndex;
		}

		ProcessGate(gType, poly0Index, vertexIndex0, poly1Index, vertexIndex1, shardWorld,
			shardIndex);
	
	}
	return true;
}


bool Session::ReadFile()
{
	ifstream is;
	is.open(filePathStr);

	if (is.is_open())
	{
		ReadHeader(is);
		ReadDecor(is);
		ReadPlayerStartPos(is);
		ReadTerrain(is);
		ReadSpecialTerrain(is);
		ReadBGTerrain(is);
		ReadRails(is);
		ReadActors(is);
		ReadGates(is);
		is.close();
	}
	else
	{
		cout << "file read failed: " << filePathStr << endl;
		assert(false && "error getting file to edit ");

		return false;
	}

	return true;
}

ControllerState &Session::GetPrevInput(int index)
{
	return mainMenu->GetPrevInput(index);
}

ControllerState &Session::GetCurrInput(int index)
{
	return mainMenu->GetCurrInput(index);
}

ControllerState &Session::GetPrevInputUnfiltered(int index)
{
	return mainMenu->GetPrevInputUnfiltered(index);
}

ControllerState &Session::GetCurrInputUnfiltered(int index)
{
	return mainMenu->GetCurrInputUnfiltered(index);
}

GameController &Session::GetController(int index)
{
	return mainMenu->GetController(index);
}

Actor *Session::GetPlayer(int index)
{
	assert(index >= 0 && index < MAX_PLAYERS);
	return players[index];
}

void Session::UpdateControllers()
{
	bool gccEnabled = mainMenu->gccDriverEnabled;

	if (gccEnabled)
		gcControllers = mainMenu->gccDriver->getState();

	for (int i = 0; i < 4; ++i)
	{
		GetPrevInput(i) = GetCurrInput(i);
		GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);

		GameController &con = GetController(i);
		if (gccEnabled)
			con.gcController = gcControllers[i];

		con.UpdateState();

		GetCurrInput(i) = con.GetState();
		GetCurrInputUnfiltered(i) = con.GetUnfilteredState();
	}
}

void Session::UpdatePlayerInput(int index)
{
	Actor *player = GetPlayer(index);
	if (player == NULL)
		return;

	ControllerState &pCurr = player->currInput;
	GameController &controller = GetController(index);
	ControllerState &currInput = GetCurrInput(index);
	ControllerState &prevInput = GetPrevInput(index);

	bool alreadyBounce = pCurr.X;
	bool alreadyGrind = pCurr.Y;
	bool alreadyTimeSlow = pCurr.leftShoulder;

	if (cutPlayerInput)
	{
		player->currInput = ControllerState();
	}
	else
	{
		player->currInput = currInput;

		if (controller.keySettings.toggleBounce)
		{
			if (currInput.X && !prevInput.X)
			{
				pCurr.X = !alreadyBounce;
			}
			else
			{
				pCurr.X = alreadyBounce;
			}
		}
		if (controller.keySettings.toggleGrind)
		{
			if (currInput.Y && !prevInput.Y)
			{
				pCurr.Y = !alreadyGrind;
			}
			else
			{
				pCurr.Y = alreadyGrind;
			}
		}
		if (controller.keySettings.toggleTimeSlow)
		{
			if (currInput.leftShoulder && !prevInput.leftShoulder)
			{
				pCurr.leftShoulder = !alreadyTimeSlow;

			}
			else
			{
				pCurr.leftShoulder = alreadyTimeSlow;
			}
		}
	}
}

void Session::UpdateAllPlayersInput()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		UpdatePlayerInput(i);
	}
}

bool Session::IsKeyPressed(int k)
{
	return mainMenu->IsKeyPressed(k);
}

bool Session::IsMousePressed(int m)
{
	return mainMenu->IsMousePressed(m);
}