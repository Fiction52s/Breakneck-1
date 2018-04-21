//sfe::Movie m;
//assert(m.openFromFile("Movie/testvid.ogv"));
//m.fit(sf::FloatRect(0, 0, 1920, 1080));
//
//
//m.play();

#include "ShardMenu.h"
#include <sstream>
#include <list>
#include <assert.h>
#include <iostream>

using namespace sf;
using namespace std;

ShardMenu::ShardMenu()
{
	//currentMovie.scale(.5, .5);

	numShardsTotal = 1;
	shardQuads = new Vertex[numShardsTotal * 4];

	testSeq = GetSequence("testanim");
	testSeq->spr.setPosition(600, 600);
}

PNGSeq * ShardMenu::GetSequence(const std::string &str)
{
	stringstream ss;
	int counter = 0;
	list<Tileset*> tList;
	while (true)
	{
		ss.str("");
		ss << "seqrecord/" << str << "_" << counter << ".png";
		cout << "test: " << ss.str() << endl;
		Tileset *t = tMan.GetTileset(ss.str(), 512, 512);
		if (t == NULL)
		{
			break;
		}
		else
		{
			tList.push_back(t);
		}
		++counter;
	}

	assert(counter > 0);
	
	if (counter == 0)
	{
		return NULL;
	}
	else
	{
		PNGSeq *newSeq = new PNGSeq(str, tList);

		return newSeq;
	}
}

void ShardMenu::SetupShardImages()
{

}

void ShardMenu::Update()
{
	testSeq->Update();
}

void ShardMenu::Draw(sf::RenderTarget *target)
{
	testSeq->Draw(target);
	//target->draw(currentMovie);
}

bool ShardMenu::LoadPNGSequences()	
{
	return false;
}

PNGSeq::PNGSeq(const
	std::string &p_seqName, list<Tileset*> &tList )
	:seqName(p_seqName)
{
	numFrames = tList.size() * 16;
	numSets = tList.size();
	tSets = new Tileset*[numSets];
	int index = 0;
	for (auto it = tList.begin(); it != tList.end(); ++it)
	{
		tSets[index] = (*it);
		++index;
	}
	Reset();
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