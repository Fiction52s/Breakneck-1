#include "ScreenRecorder.h"
#include <sstream>
#include <iostream>

using namespace sf;
using namespace std;

#define RECFOLDER "seqrecord"

ScreenRecorder::ScreenRecorder( const std::string &p_animName )
	:animName( p_animName )
{
	recordSize = Vector2f(512, 512);
	cameraCenter = Vector2f(960, 540);

	rt.create(recordSize.x * 4, recordSize.y * 4);
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
		if (tileIndex == 16)
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
	int h = recordSize.x / 2;
	View v;
	int xCenter = h + (tileIndex % 4) * recordSize.x;
	int yCenter = h + (tileIndex / 4) * recordSize.y;
	Vector2f center(xCenter, yCenter);
	//cout << "creating tilset index : " << tileIndex << " at " << center.x << ", " << center.y << endl;
	v.setCenter(center);
	v.setSize(Vector2f( recordSize.x, recordSize.y ));

	sf::Sprite spr;
	spr.setTexture(drawnTexture);
	
	IntRect r;
	r.left = 960 - h;
	r.top = 540 - h;
	r.width = recordSize.x;
	r.height = recordSize.y;
	spr.setTextureRect(r);
	spr.setOrigin(spr.getLocalBounds().width / 2.f, spr.getLocalBounds().height / 2.f);
	spr.setPosition(center);
	rt.draw(spr);
}