#ifndef __SCREENRECORDER_H__
#define __SCREENRECORDER_H__

#include <SFML/Graphics.hpp>
#include <boost\thread.hpp>

struct ScreenRecorder
{
	ScreenRecorder( const std::string &animName );
	void SaveImage( int index );
	void Save();
	sf::Vector2f recordSize;
	sf::Vector2f cameraCenter;
	sf::RenderTexture rt;
	std::string animName;
	void StartRecording();
	void StopRecording();
	void AddTileToRT( const sf::Texture &texture );
	void Update( const sf::Texture &drawnTexture);

	void WaitForSave( int index );
	bool recording;
	int tileIndex;
	int imageIndex;
	const static int MAX_THREADS_RECORD = 12;
	boost::thread *t[MAX_THREADS_RECORD];
	std::list<sf::Image> ims;
	std::list<std::string> imNames;
	int currThread;
	static void CreateSaveThread( ScreenRecorder *sr );
};

#endif