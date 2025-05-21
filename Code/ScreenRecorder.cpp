#include "ScreenRecorder.h"
#include <sstream>
#include <iostream>
#include "nlohmann\json.hpp"
#include <fstream>

using namespace sf;
using namespace std;
using namespace nlohmann;

#define RECFOLDER "seqrecord"

ScreenRecorder::ScreenRecorder( const std::string &p_animName )
	:animName( p_animName )
{
	recordSize = Vector2f(1920, 1080);
	cameraCenter = Vector2f(960, 540);
	scale = 1.f / 3.f;//1.0 / 8.0;//.25f;

	numTileX = 2048.f / (recordSize.x * scale);
	numTileY = 2048.f / (recordSize.y * scale);


	rt.create(2048, 2048);//recordSize.x * 4, recordSize.y * 4);
	recording = false;

	for (int i = 0; i < MAX_THREADS_RECORD; ++i)
	{
		t[i] = NULL;
	}
}

void ScreenRecorder::StopRecording()
{
	recording = false;
	auto nit = imNames.begin();
	for (auto it = ims.begin(); it != ims.end(); ++it, ++nit)
	{
		(*it).saveToFile((*nit));
	}

	json j;

	//json &myCurrJSON = j.emplace_back();
	j["totalFrames"] = numTileX * numTileY * ims.size(); //can be reduced later when you actually stop the recording
	j["recordSizeX"] = recordSize.x;
	j["recordSizeY"] = recordSize.y;
	j["numImages"] = ims.size();
	j["scale"] = scale;

	stringstream ss;

	ss << RECFOLDER << "/" << animName << ".json";

	ofstream o;
	o.open(ss.str());
	o << j.dump(4);
	o.close();

}

void ScreenRecorder::SaveImage( int index )
{
	std::stringstream ss;
	ss << RECFOLDER << "/" << animName << "_" << index << ".png";
	rt.display();
	ims.push_back( rt.getTexture().copyToImage() );//.saveToFile(ss.str());
	imNames.push_back(ss.str());
}

void ScreenRecorder::Update( const sf::Texture &drawnTexture)
{
	if (recording)
	{
		//if (tileIndex == 0 && imageIndex == 0)
		//{
		//	stringstream ss;
		//	ss << RECFOLDER << "/" << animName << "_preview.png";
		//	sf::Image im = drawnTexture.copyToImage();
		//	sf::Image im2;
		//	im2.create(recordSize.x, recordSize.y);
		//	im2.copy(im, 0, 0, IntRect(0, 0, recordSize.x, recordSize.y)); //IntRect(960 - 256, 540 - 256, 512, 512));
		//	bool save = im2.saveToFile(ss.str());
		//	assert(save);
		//}

		if (tileIndex == numTileX * numTileY)
		{
			//CreateSaveThread(this);
			Save();
			//SaveImage(imageIndex);
			//StopRecording();
			++imageIndex;
			rt.clear();
			tileIndex = 0;
			if (imageIndex == 10)
			{
				StopRecording();
			}
		}
		else
		{
			AddTileToRT(drawnTexture);
			++tileIndex;
		}
	}
}

void ScreenRecorder::Save()
{
	SaveImage(imageIndex);
}

void ScreenRecorder::CreateSaveThread(ScreenRecorder *sr)
{
	if (sr->currThread == MAX_THREADS_RECORD)
	{
		sr->currThread = 0;
	}

	if (sr->t[sr->currThread] != NULL)
	{
		sr->WaitForSave(sr->currThread);
	}

	sr->t[sr->currThread] = new boost::thread(boost::bind(&ScreenRecorder::Save, sr));
	sr->currThread++;

}

void ScreenRecorder::WaitForSave( int index )
{
	t[index]->join();
	delete t[index];
	t[index] = NULL;
}

void ScreenRecorder::StartRecording()
{
	if (!recording)
	{
		recording = true;
		tileIndex = 0;
		imageIndex = 0;
		currThread = 0;
	}
}

void ScreenRecorder::AddTileToRT( const Texture &drawnTexture )
{
	int x = tileIndex % numTileX;
	int y = tileIndex / numTileX;
	sf::Sprite spr;
	spr.setTexture(drawnTexture);

	spr.setScale(scale, scale);

	spr.setPosition(x * recordSize.x * scale, y * recordSize.y * scale);
	
	IntRect r;
	r.left = 0;
	r.top = 0;
	r.width = recordSize.x;
	r.height = recordSize.y;
	spr.setTextureRect(r);
	//spr.setOrigin(spr.getLocalBounds().width / 2.f, spr.getLocalBounds().height / 2.f);
	//spr.setPosition(center);
	rt.draw(spr);
}