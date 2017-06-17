#include "PlayerRecord.h"
#include "Actor.h"
#include <string>
#include <fstream>
#include <assert.h>
#include "Buf.h"
#include <iostream>
#include "GameSession.h"

using namespace sf;
using namespace std;

ReplayGhost::ReplayGhost(Actor *p_player)
	:player(p_player), sprBuffer(NULL)
{
	frame = 0;
	init = false;
}

void ReplayGhost::Draw(RenderTarget *target)
{
	if (!init)
		return;

	if (frame >= 0 && frame < numTotalFrames)
		target->draw(replaySprite);
}

bool ReplayGhost::OpenGhost(const std::string &fileName)
{
	ifstream is;

	is.open(fileName);
	if (is.is_open())
	{
		is >> numTotalFrames;
		sprBuffer = new SprInfo[numTotalFrames];

		init = true;

		for (int i = 0; i < numTotalFrames; ++i)
		{
			SprInfo &info = sprBuffer[i];
			is >> info.position.x;
			is >> info.position.y;

			is >> info.origin.x;
			is >> info.origin.y;

			is >> info.rotation;

			int fx;
			int fy;

			is >> fx;
			is >> fy;

			info.flipX = (bool)fx;
			info.flipY = (bool)fy;

			is >> info.action;
			is >> info.tileIndex;

			is >> info.speedLevel;

		}

		is.close();

		frame = 0;
		return true;
	}
	//return false on failure
	return false;

}

void ReplayGhost::UpdateReplaySprite()
{
	if (!init || frame == numTotalFrames)
		return;

	SprInfo &info = sprBuffer[frame];
	Tileset *ts = player->tileset[(Actor::Action)info.action];
	replaySprite.setTexture(*ts->texture);

	Actor::Action a = (Actor::Action)info.action;
	if (a == Actor::JUMPSQUAT)
	{
		cout << "setting jumpsquat in ghost. frame: " << frame << endl;
	}
	IntRect ir = ts->GetSubRect(info.tileIndex);

	replaySprite.setRotation(info.rotation);

	if (info.flipX)
	{
		ir.left += ir.width;
		ir.width = -ir.width;
	}
	if (info.flipY)
	{
		ir.top += ir.height;
		ir.height = -ir.height;
	}

	replaySprite.setTextureRect(ir);
	replaySprite.setOrigin(info.origin.x, info.origin.y);
	replaySprite.setRotation(info.rotation);
	replaySprite.setPosition(info.position.x, info.position.y);

	replaySprite.setColor(Color(255, 255, 255, 200));

	++frame;
}

RecordGhost::RecordGhost(Actor *p_player)
	:player(p_player)
{
	numTotalFrames = -1;
	frame = -1;
}

void RecordGhost::StartRecording()
{
	frame = 0;
	numTotalFrames = -1;
}

void RecordGhost::StopRecording()
{
	//cout << "stop recording: " << frame << endl;
	numTotalFrames = frame;
	frame = -1;
}

void RecordGhost::RecordFrame()
{
	if (frame < 0)
		return;
	//assert( frame >= 0 );
	if (frame >= MAX_RECORD)
		return;

	SprInfo &info = sprBuffer[frame];
	info.position = player->sprite->getPosition();
	info.origin = player->sprite->getOrigin();
	info.rotation = player->sprite->getRotation();
	info.flipX = player->flipTileX;
	info.flipY = player->flipTileY;
	info.action = (int)player->spriteAction;
	info.tileIndex = player->currTileIndex;
	info.speedLevel = player->speedLevel;

	//cout << "record frame: " << frame << ",action: " << info.action << ", t: " << info.tileIndex << endl;
	++frame;
}

void RecordGhost::WriteToFile(const std::string &fileName)
{
	assert(numTotalFrames > 0);

	ofstream of;
	of.open(fileName);
	if (of.is_open())
	{
		of << numTotalFrames << endl;
		for (int i = 0; i < numTotalFrames; ++i)
		{
			SprInfo &info = sprBuffer[i];
			of << info.position.x << " " << info.position.y << endl;
			of << info.origin.x << " " << info.origin.y << endl;
			of << info.rotation << endl;
			of << (int)info.flipX << " " << (int)info.flipY << endl;
			of << info.action << " " << info.tileIndex << endl;
			of << info.speedLevel << endl;
		}

		of.close();
	}
	else
	{
		cout << "recordghost fileName: " << fileName << endl;
		assert(false && "failed to open file to write to");
	}
}

RecordPlayer::RecordPlayer(Actor *p_player)
	:player(p_player)
{
	numTotalFrames = -1;
	frame = -1;
}

void RecordPlayer::StartRecording()
{
	frame = 0;
	numTotalFrames = -1;
}

void RecordPlayer::StopRecording()
{
	//cout << "stop recording: " << frame << endl;
	numTotalFrames = frame;
	frame = -1;
}

void RecordPlayer::RecordFrame()
{

	if (frame < 0)
		return;
	//assert( frame >= 0 );
	if (frame >= MAX_RECORD)
		return;

	//of << state.leftStickMagnitude << " " << state.leftStickRadians << " " << state.leftTrigger << " "
	//			<< state.rightTrigger 
	//			<< " " // << (int)state.start << " " << (int)state.back << " "
	//			<< (int)state.leftShoulder
	//			<< " " << (int)state.rightShoulder << " " << (int)state.A << " " << (int)state.B << " " 
	//			<< (int)state.X << " " << (int)state.Y << endl;
	ControllerState &s = player->owner->GetCurrInput(0);//currInput;
														//temporary hack^
														//ControllerState &state = inputBuffer[frame];
														//state = ;

	Buf & b = player->owner->testBuf;

	//b.Send( player->owner->totalGameFrames );
	b.Send(s.leftStickMagnitude);
	b.Send(s.leftStickRadians);
	b.Send(s.leftTrigger);
	b.Send(s.rightTrigger);

	b.Send(s.leftShoulder);
	b.Send(s.rightShoulder);
	b.Send(s.A);
	b.Send(s.B);
	b.Send(s.X);
	b.Send(s.Y);

	//cout << "record frame: " << frame << " buttons A: " << (int)s.A << ", B: " << (int)s.B << endl;

	//if( state.A )
	//	cout << "record frame: " << frame << " jump" << endl;
	/*info.position = player->sprite->getPosition();
	info.origin = player->sprite->getOrigin();
	info.rotation = player->sprite->getRotation();
	info.flipX = player->flipTileX;
	info.flipY = player->flipTileY;
	info.action = (int)player->spriteAction;
	info.tileIndex = player->currTileIndex;
	info.speedLevel = player->speedLevel;*/

	//cout << "record frame: " << frame << ",action: " << info.action << ", t: " << info.tileIndex << endl;
	++frame;
}

void RecordPlayer::WriteToFile(const std::string &fileName)
{
	return;
	assert(numTotalFrames > 0);

	ofstream of;
	of.open(fileName);
	if (of.is_open())
	{
		of << numTotalFrames << endl;
		for (int i = 0; i < numTotalFrames; ++i)
		{
			ControllerState &state = inputBuffer[i];
			of << state.leftStickMagnitude << " " << state.leftStickRadians << " " << state.leftTrigger << " "
				<< state.rightTrigger
				<< " " // << (int)state.start << " " << (int)state.back << " "
				<< (int)state.leftShoulder
				<< " " << (int)state.rightShoulder << " " << (int)state.A << " " << (int)state.B << " "
				<< (int)state.X << " " << (int)state.Y << endl;
			/*SprInfo &info = sprBuffer[i];
			of << info.position.x << " " << info.position.y << endl;
			of << info.origin.x << " " << info.origin.y << endl;
			of << info.rotation << endl;
			of << (int)info.flipX << " " << (int)info.flipY << endl;
			of << info.action << " " << info.tileIndex << endl;
			of << info.speedLevel << endl;*/
		}

		of.close();
	}
	else
	{
		cout << "recordghost fileName: " << fileName << endl;
		assert(false && "failed to open file to write to");
	}
}

ReplayPlayer::ReplayPlayer(Actor *p_player)
	:player(p_player), inputBuffer(NULL)
{
	frame = 0;
	init = false;
}

bool ReplayPlayer::OpenReplay(const std::string &fileName)
{
	//ifstream is;

	//is.open( fileName );
	//if( is.is_open() )
	//{
	//	init = true;

	//	is >> numTotalFrames;
	//	inputBuffer = new ControllerState[numTotalFrames];

	//	for( int i = 0; i < numTotalFrames; ++i )
	//	{
	//		ControllerState &state = inputBuffer[i];
	//		
	//		is >> state.leftStickMagnitude;
	//		is >> state.leftStickRadians;
	//		is >> state.leftTrigger;
	//		is >> state.rightTrigger;
	//		int start, back, leftShoulder, rightShoulder, A,B,X,Y;
	//		//is >> start;
	//		//is >> back;
	//		is >> leftShoulder;
	//		is >> rightShoulder;
	//		is >> A >> B >> X >> Y;

	//		//state.start = start;
	//		//state.back = back;
	//		state.leftShoulder = leftShoulder;
	//		state.rightShoulder = rightShoulder;
	//		state.A = A;
	//		state.B = B;
	//		state.X = X;
	//		state.Y = Y;
	//		/*SprInfo &info = sprBuffer[i];
	//		is >> info.position.x;
	//		is >> info.position.y;

	//		is >> info.origin.x;
	//		is >> info.origin.y;

	//		is >> info.rotation;

	//		int fx;
	//		int fy;

	//		is >> fx;
	//		is >> fy;

	//		info.flipX = (bool)fx;
	//		info.flipY = (bool)fy;

	//		is >> info.action;
	//		is >> info.tileIndex;

	//		is >> info.speedLevel;*/
	//	}

	//	is.close();

	//	frame = 0;
	//	return true;
	//}
	////return false on failure
	//return false;

	ifstream is;

	is.open(fileName, ios::binary | ios::in);
	if (is.is_open())
	{
		init = true;

		is.read((char*)&numTotalFrames, sizeof(numTotalFrames));
		cout << "reading num frames: " << numTotalFrames << endl;

		inputBuffer = new ControllerState[numTotalFrames];

		for (int i = 0; i < numTotalFrames; ++i)
		{
			ControllerState &state = inputBuffer[i];

			//int gameFrames = -1;

			//assert( !is.eof() );
			//is.read( (char*)&gameFrames, sizeof( int ) );


			//cout << "testing replay " << i << " : " << gameFrames << endl;
			is.read((char*)&state.leftStickMagnitude, sizeof(state.leftStickMagnitude));
			is.read((char*)&state.leftStickRadians, sizeof(state.leftStickRadians));
			is.read((char*)&state.leftTrigger, sizeof(state.leftTrigger));
			is.read((char*)&state.rightTrigger, sizeof(state.rightTrigger));

			int start, back, leftShoulder, rightShoulder, A, B, X, Y;

			is.read((char*)&state.leftShoulder, sizeof(bool));
			is.read((char*)&state.rightShoulder, sizeof(bool));
			is.read((char*)&state.A, sizeof(bool));
			is.read((char*)&state.B, sizeof(bool));
			is.read((char*)&state.X, sizeof(bool));
			is.read((char*)&state.Y, sizeof(bool));

			//cout << "replay frame: " << i << " buttons A: " << (int)state.A << ", B: " << (int)state.B << endl;
			//cout << "replay frame: " << i << " buttons A: " << (int)state.A << ", B: " << (int)state.B << endl;
			//cout << "replay frame: " << i << ", leftstickmag: " << state.leftStickMagnitude << endl;
			//is >> start;
			//is >> back;
			//is >> leftShoulder;
			//is >> rightShoulder;
			//is >> A >> B >> X >> Y;

			////state.start = start;
			////state.back = back;
			//state.leftShoulder = leftShoulder;
			//state.rightShoulder = rightShoulder;
			//state.A = A;
			//state.B = B;
			//state.X = X;
			//state.Y = Y;
		}

		is.close();

		frame = 0;
		return true;
	}
	//return false on failure
	return false;

}

void ReplayPlayer::UpdateInput(ControllerState &state)
{
	if (!init || frame == numTotalFrames)
		return;

	ControllerState &cs = inputBuffer[frame];

	state.leftStickMagnitude = cs.leftStickMagnitude;
	state.leftStickRadians = cs.leftStickRadians;

	float stickThresh = GameController::stickThresh;
	state.leftStickPad = 0;
	if (state.leftStickMagnitude > stickThresh)
	{
		//cout << "left stick radians: " << currInput.leftStickRadians << endl;
		float x = cos(state.leftStickRadians);
		float y = sin(state.leftStickRadians);

		if (x > stickThresh)
			state.leftStickPad += 1 << 3;
		if (x < -stickThresh)
			state.leftStickPad += 1 << 2;
		if (y > stickThresh)
			state.leftStickPad += 1;
		if (y < -stickThresh)
			state.leftStickPad += 1 << 1;
	}

	state.leftTrigger = cs.leftTrigger;
	state.rightTrigger = cs.rightTrigger;
	//state.start = cs.start;
	//state.back = cs.back;
	state.leftShoulder = cs.leftShoulder;
	state.rightShoulder = cs.rightShoulder;
	state.A = cs.A;
	state.B = cs.B;
	state.X = cs.X;
	state.Y = cs.Y;

	if (state.A)
		cout << "replay frame: " << frame << " pressing A" << endl;

	/*SprInfo &info = sprBuffer[frame];
	Tileset *ts = player->tileset[(Actor::Action)info.action];
	replaySprite.setTexture( *ts->texture );

	IntRect ir = ts->GetSubRect( info.tileIndex );

	replaySprite.setRotation( info.rotation );

	if( info.flipX )
	{
	ir.left += ir.width;
	ir.width = -ir.width;
	}
	if( info.flipY )
	{
	ir.top += ir.height;
	ir.height = -ir.height;
	}

	replaySprite.setTextureRect( ir );
	replaySprite.setOrigin( info.origin.x, info.origin.y );
	replaySprite.setRotation( info.rotation );
	replaySprite.setPosition( info.position.x, info.position.y );

	replaySprite.setColor( Color( 255, 255, 255, 150 ) );*/

	++frame;
}