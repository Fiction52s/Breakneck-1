#include "ShardMenu.h"
#include <sstream>
#include <list>
#include <assert.h>
#include <iostream>
#include "ItemSelector.h"
#include "VectorMath.h"
#include "Enemy_Shard.h"
#include "MainMenu.h"
#include "MusicSelector.h"
#include "VisualEffects.h"
#include "SaveFile.h"
#include "GameSession.h"
#include "Actor.h"
#include "AdventureManager.h"

using namespace sf;
using namespace std;

ShardMenu::ShardMenu(TilesetManager *p_tm)
{
	MainMenu *mm = MainMenu::GetInstance();
	game = NULL;
	tm = p_tm;
	totalFrame = 0;

	ts_shardContainer = tm->GetSizedTileset("Menu/shard_container_401x512.png");
	ts_sparkle = tm->GetSizedTileset("Menu/shard_sparkle_64x64.png");

	selectedIndex = 0;

	

	

	currShardText.setCharacterSize(20);
	currShardText.setFont(mm->arial);
	

	currShardNameText.setCharacterSize(36);
	currShardNameText.setFont(mm->arial);
	

	worldText.setCharacterSize(40);
	worldText.setFont(mm->arial);
	//worldText.setPosition(Vector2f(825 + 401 / 2, 594 + 93 / 2));
	

	//SetRectCenter(shardBGQuad, 744, 848, Vector2f(65 + 744 / 2, 66 + 848 / 2));

	SetRectColor(shardBGQuad, Color(0, 0, 0, 128));
	SetRectColor(shardTitleBGQuad, Color(0, 0, 0, 128));
	SetRectColor(descriptionBGQuad, Color(0, 0, 0, 128));
	SetRectColor(containerBGQuad, Color(0, 0, 0, 128));

	SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(0));

	numShardsTotal = 1;
	shardQuads = new Vertex[numShardsTotal * 4];
	imagePos = Vector2f(1243, 66);

	state = PAUSED;

	//this probably loads shard textures twice, is probably wasteful.
	for (int i = 0; i < 7; ++i)
	{
		ts_shards[i] = mm->GetSizedTileset("Shard/shards_w" + to_string(i+1) + "_192x192.png");
	}

	sparklePool = new EffectPool(EffectType::FX_REGULAR, 3, 1.f);
	sparklePool->ts = ts_sparkle;

	int waitFrames[3] = { 60, 20, 10 };
	int waitModeThresh[2] = { 2, 4 };
	int xSize = 4;
	int ySize = 4 + 1;

	int numWorlds = 8;

	shardInfo = new ShardDetailedInfo*[numWorlds];
	for (int i = 0; i < numWorlds; ++i)
	{
		shardInfo[i] = new ShardDetailedInfo[ShardInfo::MAX_SHARDS_PER_WORLD];
	}

	worldSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, numWorlds, 0);
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, xSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);

	LoadShardInfo();

	shardSelectQuads = new sf::Vertex[xSize * ySize * 4];

	SetTopLeft(Vector2f(50, 50));

	SetWorldMode();
}

void ShardMenu::SetTopLeft(sf::Vector2f &pos)
{
	float shardBGQuadWidth = 740;

	Vector2f pauseTexSize(1820, 980);

	int edgeMargin = 65;

	Vector2f shardBGQuadTopLeft = Vector2f(edgeMargin, edgeMargin) + pos;
	SetRectTopLeft(shardBGQuad, shardBGQuadWidth, pauseTexSize.y - edgeMargin * 2, shardBGQuadTopLeft);

	float rightOfShardBGQuad = shardBGQuadTopLeft.x + shardBGQuadWidth;

	float leftSideMiddle = shardBGQuadTopLeft.x + shardBGQuadWidth / 2;

	worldText.setPosition(leftSideMiddle, 80 + pos.y);

	float separationMargin = 15;
	float rightSideStartX = rightOfShardBGQuad + separationMargin;

	float rightSideWidth = (pauseTexSize.x - edgeMargin) - rightSideStartX;
	float rightSideCenterX = rightSideStartX + rightSideWidth / 2;

	float containerBGQuadHeight = 512;

	Vector2f containerBGQuadPos = Vector2f(rightSideStartX, edgeMargin + pos.y);

	SetRectTopLeft(containerBGQuad, rightSideWidth, containerBGQuadHeight, containerBGQuadPos);

	float containerQuadBottom = containerBGQuadPos.y + containerBGQuadHeight;

	Vector2f containerSize = Vector2f(ts_shardContainer->tileWidth,
		ts_shardContainer->tileHeight);
	containerCenter = Vector2f(rightSideCenterX, edgeMargin + containerSize.y / 2 + pos.y);

	SetRectCenter(largeShardContainer, containerSize.x, containerSize.y, containerCenter);
	SetRectCenter(largeShard, 192, 192, containerCenter);

	float nameQuadHeight = 93;
	float descriptionQuadHeight = 215;

	Vector2f shardTitleBGQuadPos = Vector2f(rightSideStartX, containerQuadBottom + separationMargin);
	Vector2f shardDescriptionBGQuadPos = Vector2f(rightSideStartX, containerQuadBottom + separationMargin + nameQuadHeight + separationMargin);

	SetRectTopLeft(shardTitleBGQuad, rightSideWidth, nameQuadHeight, shardTitleBGQuadPos);
	SetRectTopLeft(descriptionBGQuad, rightSideWidth, descriptionQuadHeight, shardDescriptionBGQuadPos);

	currShardNameText.setPosition(Vector2f(rightSideCenterX, shardTitleBGQuadPos.y + nameQuadHeight / 2));

	currShardText.setPosition(shardDescriptionBGQuadPos + Vector2f(edgeMargin, edgeMargin));

	int index = 0;
	int rectSize = 192 / 2;
	int xSpacing = 20 * 2;
	int ySpacing = 12 * 2;

	Vector2f gridStart(150, 200);

	gridStart += pos;

	for (int i = 0; i < ySelector->totalItems - 1; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = (i * xSelector->totalItems + j);

			SetRectCenter(shardSelectQuads + index * 4, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, i * rectSize + ySpacing * i) + gridStart);
		}
	}
}

void ShardMenu::UpdateWorld()
{
	worldText.setString("World " + to_string(worldSelector->currIndex + 1));
	sf::FloatRect localBounds = worldText.getLocalBounds();
	worldText.setOrigin(localBounds.left + localBounds.width / 2, 0);
	UpdateUnlockedShards();
	UpdateShardQuads();
}

void ShardMenu::UpdateShardQuads()
{
	for (int i = 0; i < ShardInfo::MAX_SHARDS_PER_WORLD; ++i)
	{
		if (shardInfo[worldSelector->currIndex][i].name == "")
		{
			SetRectSubRect(shardSelectQuads + i * 4, sf::FloatRect());
		}
		else
		{
			SetRectSubRect(shardSelectQuads + i * 4,
				ts_shards[worldSelector->currIndex]->GetSubRect(i));
		}
	}

	/*int index;
	for (int i = 0; i < ySelector->totalItems - 1; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = (i * xSelector->totalItems + j);

			if (index == ShardInfo::MAX_SHARDS_PER_WORLD)
			{
				return;
			}

			SetRectSubRect(shardSelectQuads + index * 4,
				ts_shards[worldSelector->currIndex]->GetSubRect(index));
		}
	}*/
}

ShardMenu::~ShardMenu()
{
	for (int i = 0; i < worldSelector->totalItems; ++i)
	{
		delete[] shardInfo[i];
	}
	delete[] shardInfo;

	delete xSelector;
	delete ySelector;
	delete worldSelector;

	delete[] shardQuads;
	delete[] shardSelectQuads;
	delete sparklePool;
}

void ShardMenu::SetGame(GameSession *p_game)
{
	game = p_game;
	UpdateWorld();
}

void ShardMenu::LoadShardInfo()
{
	for (int i = 0; i < 7; ++i)
	{
		ifstream is;
		is.open("Resources/Shard/shardinfo" + to_string( i + 1 ) + ".txt");

		if (is.is_open())
		{
			string lineString;
			string descriptionString;
			int index = 0;

			while (getline(is, lineString))
			{
				shardInfo[i][index].name = lineString;

				descriptionString = "";

				bool first = true;
				while (getline(is, lineString))
				{
					if (lineString == "")
					{
						break;
					}
					else
					{
						if (first)
						{
							first = false;
						}
						else
						{
							descriptionString += "\n";
						}

						descriptionString += lineString;

					}
				}

				shardInfo[i][index].desc = descriptionString;
				index++;
			}

			is.close();
		}
		else
		{
			cout << "couldnt open shard info file" << endl;
			assert(0);
		}
	}
	
}

bool ShardMenu::IsShardCaptured(int w, int li)
{
	if (shardInfo[w][li].name == "")
	{
		return false;
	}

	int shardType = Shard::GetShardTypeFromWorldAndIndex(w, li);

	assert(game != NULL);

	if (game->mainMenu->adventureManager != NULL)
	{
		SaveFile *saveFile = game->mainMenu->adventureManager->currSaveFile;
		if (saveFile != NULL)
		{
			return saveFile->IsShardCaptured(shardType);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return game->IsShardCaptured(shardType);
	}
}

bool ShardMenu::IsCurrShardCaptured()
{
	return IsShardCaptured(worldSelector->currIndex, selectedIndex);
}

void ShardMenu::UpdateUnlockedShards()
{
	int index = 0;

	int world = worldSelector->currIndex;
	for (int i = 0; i < ShardInfo::MAX_SHARDS_PER_WORLD; ++i)
	{
		if (IsShardCaptured(world, i))
		{
			SetRectColor(shardSelectQuads + i * 4, Color(Color::White));
			//SetRectSubRect(shardSelectQuads + index, ts_shards[0]->GetSubRect(i * xSelector->totalItems + j));
		}
		else
		{
			SetRectColor(shardSelectQuads + i * 4, Color(Color::Black));
			//SetRectSubRect(shardSelectQuads + index, FloatRect());
		}
	}

	
	//for (int i = 0; i < ySelector->totalItems - 1; ++i)
	//{
	//	for (int j = 0; j < xSelector->totalItems; ++j)
	//	{
	//		index = (i * xSelector->totalItems + j);
	//		//SetRectCenter(shardSelectQuads + index, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, i * rectSize + ySpacing * i) + gridStart);
	//		
	//		if (IsShardCaptured( worldSelector->currIndex, index))
	//		{
	//			SetRectColor(shardSelectQuads + index * 4, Color(Color::White));
	//			//SetRectSubRect(shardSelectQuads + index, ts_shards[0]->GetSubRect(i * xSelector->totalItems + j));
	//		}
	//		else
	//		{
	//			SetRectColor(shardSelectQuads + index * 4, Color(Color::Black));
	//			//SetRectSubRect(shardSelectQuads + index, FloatRect());
	//		}
	//		//ts_shards[0]->GetSubRect(i * xSelector->totalItems + j));
	//	}
	//}
}

bool ShardMenu::SetDescription( std::string &nameStr, std::string &destStr, const std::string &shardTypeStr)
{
	stringstream ss;
	ss << "Resources/Shard/Descriptions/" << shardTypeStr << ".sdesc";
	ifstream is;
	is.open(ss.str());

	nameStr = "";
	destStr = "";
	if( is.is_open())
	{
		getline(is, nameStr);
		destStr.assign((std::istreambuf_iterator<char>(is)),
			(std::istreambuf_iterator<char>()));
		/*char c;
		while (is.get(c))
		{
			
		}*/

		//getline(is, destStr);
		/*is.seekg(0, std::ios::end);
		destStr.reserve(is.tellg());
		is.seekg(0, std::ios::beg);

		destStr.assign((std::istreambuf_iterator<char>(is)),
			std::istreambuf_iterator<char>());*/
		/*if (!is.good())
		{
			assert(is.good());
		}*/
		
		//cout << "dest str: " << destStr << endl;
	}
	else
	{
		cout << "trying to get: " << shardTypeStr << " description" << endl;
		return false;
		assert(0);
	}
	return true;
}

void ShardMenu::UpdateShardSelectQuads()
{
	int index = 0;

	if (ySelector->currIndex == 0)
	{
		return;
	}

	//index = (xSelector->currIndex + xSelector->totalItems * (ySelector->currIndex-1));

	//SetRectColor(shardSelectQuads + index, currColor);
	int selectedShardTileIndex = -1;
	if (IsCurrShardCaptured())
	{
		SetRectSubRect(largeShard,
			ts_shards[worldSelector->currIndex]->GetSubRect(selectedIndex));
		//selectedShardTileIndex = index + 22;
	}
	else
	{
		SetRectSubRect(largeShard, FloatRect());
		//selectedShardTileIndex = index + 44;
	}

	SetRectCenter(selectedBGQuad, 192/2, 192/2,
		Vector2f((shardSelectQuads + selectedIndex * 4)->position + Vector2f(192/4, 192/4)));
	SetRectColor(selectedBGQuad, Color::White);
}

void ShardMenu::SetCurrentDescription( bool captured)
{
	if( captured )
	{
		cout << "worldindex: " << worldSelector->currIndex << ", select: " << selectedIndex << "\n";
		ShardDetailedInfo &currInfo = shardInfo[worldSelector->currIndex][selectedIndex];//GetCurrShardInfo();
		currShardNameText.setString(currInfo.name);
		FloatRect lBounds = currShardNameText.getLocalBounds();
		currShardNameText.setOrigin(lBounds.left + lBounds.width / 2, lBounds.top + lBounds.height / 2);
		
		currShardText.setString(currInfo.desc);
			
	}
	else
	{
		currShardNameText.setString("???");
		FloatRect lBounds = currShardNameText.getLocalBounds();
		currShardNameText.setOrigin(lBounds.left + lBounds.width / 2, lBounds.top + lBounds.height / 2);
		currShardText.setString("");
	}
	
}

const std::string & ShardMenu::GetShardDesc(int w, int li)
{
	return shardInfo[w][li].desc;
}

const std::string & ShardMenu::GetShardName(int w, int li)
{
	return shardInfo[w][li].name;
}

ShardDetailedInfo &ShardMenu::GetCurrShardInfo()
{
	return shardInfo[worldSelector->currIndex][selectedIndex];
}

void ShardMenu::SetCurrShard()
{
	int index = xSelector->currIndex + (ySelector->currIndex - 1) * xSelector->totalItems;
	selectedIndex = index;

	bool captured = IsCurrShardCaptured();

	SetCurrentDescription(captured);
}

void ShardMenu::SetWorldMode()
{
	currSelectMode = SM_WORLD;
	worldText.setOutlineColor(Color::Red);
	worldText.setOutlineThickness(2);
	SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(12));

	currShardNameText.setString("");
	currShardText.setString("");
}

void ShardMenu::Update( ControllerState &currInput, ControllerState &prevInput )
{
	int testIndex;

	if (currSelectMode == SM_WORLD)
	{
		int worldChanged = worldSelector->UpdateIndex(currInput.LLeft() || currInput.PLeft(), currInput.LRight() || currInput.PRight());
		int ychanged = ySelector->UpdateIndex(currInput.LUp() || currInput.PUp(), currInput.LDown() || currInput.PDown());

		if (worldChanged != 0)
		{
			xSelector->currIndex = 0;
			UpdateWorld();
		}

		if (ychanged != 0)
		{
			worldText.setOutlineThickness(0);

			int index = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;

			assert(index < ShardInfo::MAX_SHARDS_PER_WORLD);

			ShardDetailedInfo &currInfo = shardInfo[worldSelector->currIndex][index];



			if (currInfo.name == "")
			{
				for (int i = ySelector->totalItems - 1; i >= 1; --i)
				{
					ySelector->currIndex--;
					testIndex = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;
					ShardDetailedInfo &testInfo = shardInfo[worldSelector->currIndex][testIndex];
					if (testInfo.name != "")
					{
						break;
					}
				}
			}

			currSelectMode = SM_SHARD;
			SetCurrShard();

			UpdateShardSelectQuads();
		}
	}
	else if (currSelectMode == SM_SHARD)
	{
		int xchanged = xSelector->UpdateIndex(currInput.LLeft() || currInput.PLeft(), currInput.LRight() || currInput.PRight());
		int ychanged = ySelector->UpdateIndex(currInput.LUp() || currInput.PUp(), currInput.LDown() || currInput.PDown());

		bool currShardCap = IsCurrShardCaptured();

		int index = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;
		//ShardDetailedInfo &currInfo = shardInfo[worldSelector->currIndex][index];

		bool currShardValid = index < ShardInfo::MAX_SHARDS_PER_WORLD
			&&  shardInfo[worldSelector->currIndex][index].name != "";

		if (ychanged > 0)
		{
			if (!currShardValid)
			{
				ySelector->currIndex = 0;
			}
		}

		if (ySelector->currIndex == 0)
		{
			SetWorldMode();
			state = WAIT;
		}
		else
		{
			if (xchanged != 0 || ychanged != 0)
			{
				if (!currShardValid)
				{
					if (xchanged > 0)
					{
						xSelector->currIndex = 0;
					}
					else if (xchanged < 0)
					{
						for (int i = xSelector->totalItems - 1; i >= 0; --i)
						{
							xSelector->currIndex--;
							testIndex = (ySelector->currIndex - 1) * xSelector->totalItems + xSelector->currIndex;
							ShardDetailedInfo &testInfo = shardInfo[worldSelector->currIndex][testIndex];
							if (testInfo.name != "")
							{
								break;
							}
						}
					}
				}


				//state = PAUSED;
				/*if (!currShardCap)
				{
					
				}*/

				sparklePool->Reset();
				
				state = WAIT;
				SetCurrShard();
			}

			UpdateShardSelectQuads();


			if (currShardCap)
			{
				int lightningFactor = 7;//14;//8;
				SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect((totalFrame / lightningFactor) % 12));
			}
			else
			{
				SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(12));
			}

			sparklePool->Update();


			if (totalFrame % 120 == 0 && currShardCap)
			{
				Vector2f off(rand() % 101 - 50, rand() % 101 - 50);
				EffectInstance ei;

				int r = rand() % 3;
				if (r == 0)
				{
					ei.SetParams(containerCenter + off,
						Transform(Transform::Identity), 11, 5, 0);
				}
				else if (r == 1)
				{
					ei.SetParams(containerCenter + off,
						Transform(Transform::Identity), 10, 5, 11);
				}
				else if (r == 2)
				{
					ei.SetParams(containerCenter + off,
						Transform(Transform::Identity), 10, 5, 11);
				}

				//ei.pos = sparkleCenter;
				//ei.animFactor = 8;

				sparklePool->ActivateEffect(&ei);
			}
		}
	}

	++totalFrame;
}

void ShardMenu::SetShardTab()
{
	currSelectMode = SM_WORLD;
	ySelector->currIndex = 0;
	xSelector->ResetCounters();
	ySelector->ResetCounters();

	UpdateWorld();

	bool currShardCap = IsCurrShardCaptured();
	if (currShardCap)
	{
		int lightningFactor = 7;//8;
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect((totalFrame / lightningFactor) % 12));
	}
	else
	{
		SetRectSubRect(largeShardContainer, ts_shardContainer->GetSubRect(12));
	}
	//UpdateUnlockedShards();
	//UpdateShardSelectQuads();

	//shardMenu->SetCurrShard();
}

void ShardMenu::Draw(sf::RenderTarget *target)
{
	target->draw(shardBGQuad, 4, sf::Quads);
	target->draw(containerBGQuad, 4, sf::Quads);
	target->draw(descriptionBGQuad, 4, sf::Quads);
	target->draw(shardTitleBGQuad, 4, sf::Quads);

	target->draw(largeShardContainer, 4, sf::Quads, ts_shardContainer->texture);
	
	if (currSelectMode == SM_SHARD)
	{
		target->draw(largeShard, 4, sf::Quads, ts_shards[worldSelector->currIndex]->texture);
		sparklePool->Draw(target);

		target->draw(selectedBGQuad, 4, sf::Quads);
	}
	
	target->draw(shardSelectQuads , ShardInfo::MAX_SHARDS_PER_WORLD * 4,
		sf::Quads, ts_shards[worldSelector->currIndex]->texture);
	target->draw(worldText);
	
	if (currShardText.getString() != "")
	{
		target->draw(currShardText);
	}
	if (currShardNameText.getString() != "")
	{
		target->draw(currShardNameText);
	}
	//target->draw(currentMovie);
}

PNGSeq::PNGSeq(const
	std::string &p_seqName, list<Tileset*> &tList, bool p_singleImage )
	:seqName(p_seqName), singleImage( p_singleImage )
{
	if (singleImage)
	{
		numFrames = tList.size();
	}
	else
	{
		numFrames = tList.size() * 16;
	}
	
	numSets = tList.size();
	tSets = new Tileset*[numSets];
	int index = 0;
	for (auto it = tList.begin(); it != tList.end(); ++it)
	{
		tSets[index] = (*it);
		++index;
	}
	//load preview image

	Reset();
}

void PNGSeq::IncrementFrame()
{
	if (singleImage)
		return;

	tileIndex++;

	if (tileIndex == 16)
	{
		++setIndex;
		if (setIndex == numSets)
		{
			setIndex = 0;
		}
		tileIndex = 0;

		spr.setTexture(*tSets[setIndex]->texture);
	}

	spr.setTextureRect(tSets[setIndex]->GetSubRect(tileIndex));
}

void PNGSeq::DecrementFrame()
{
	if (singleImage)
		return;

	tileIndex--;

	if (tileIndex < 0)
	{
		--setIndex;
		if (setIndex < 0)
		{
			setIndex = numSets - 1;
		}

		int rem = (numFrames-1) % 16;
		tileIndex = rem;

		spr.setTexture(*tSets[setIndex]->texture);
	}

	spr.setTextureRect(tSets[setIndex]->GetSubRect(tileIndex));
}

PNGSeq::~PNGSeq()
{
	delete[] tSets;
}

void PNGSeq::Load()
{

}

void PNGSeq::Update()
{
	if (singleImage)
		return;

	if (tileIndex == 16)
	{
		++setIndex;
		if (setIndex == numSets)
		{
			setIndex = 0;
		}
		tileIndex = 0;

		spr.setTexture(*tSets[setIndex]->texture);
	}

	spr.setTextureRect(tSets[setIndex]->GetSubRect(tileIndex));

	tileIndex++;
}

void PNGSeq::Reset()
{
	tileIndex = 0;
	setIndex = 0;
	spr.setTexture( *tSets[0]->texture );
	spr.setTextureRect(tSets[0]->GetSubRect(0));
}

void PNGSeq::Draw(sf::RenderTarget *target)
{
	target->draw(spr);
}

//Sparkle::Sparkle(Tileset *p_ts, int type, Vector2f &p_pos)
//	:ts( p_ts ), pos( p_pos )
//{
//
//}