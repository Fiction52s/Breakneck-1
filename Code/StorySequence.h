#ifndef __STORYSEQUENCE_H__
#define __STORYSEQUENCE_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
#include "Input.h"
#include "EffectLayer.h"

struct MusicInfo;
struct StoryMusic
{
	std::string musicName;
	MusicInfo *musicInfo;
	sf::Time startTime;
	float transitionSeconds;
};

struct StoryText
{
	StoryText( sf::Font &font, const std::string &p_str, sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
	bool Update( ControllerState &prev, ControllerState &curr);
	void Reset();
	std::string textStr;
	sf::Text text;
	bool done;
};

struct Conversation;
struct ConversationGroup;
struct StorySequence;
struct StoryPart
{
	StoryPart(StorySequence *seq);
	void Reset();
	Tileset *ts;
	sf::Sprite spr;
	sf::Vector2f pos;
	bool hasIntro;
	StorySequence *seq;

	enum OutroType
	{
		O_NONE,
		O_FADE
	};

	OutroType outType;
	sf::Color fadeOutColor;
	int fadeOutFrames;
	int startOutroFadeFrame;

	int layer;
	float time;
	int frame;
	int totalFrames;
	bool Update(ControllerState &prev, ControllerState &curr);
	void Draw(sf::RenderTarget *target);
	std::string imageName;
	Conversation *text;
	//ConversationGroup *textGroup;
	//StoryText *text;
	StoryPart *sub;
	bool musicStarted;

	EffectLayer effectLayer;

	StoryMusic *music;
	bool blank;
};

struct GameSession;
struct StorySequence
{
	StorySequence( sf::Font &font, TilesetManager *tm);
	StorySequence(GameSession *owner);
	bool Load( const std::string &sequenceName );
	void Reset();
	bool Update( ControllerState &prev, ControllerState &curr);
	bool UpdateLayer( int layer, ControllerState &prev, ControllerState &curr);
	void Draw( sf::RenderTarget *target );
	void DrawLayer(sf::RenderTarget *target, EffectLayer eLayer);
	void EndSequence();

	const static int NUM_LAYERS = 16;
	std::list<StoryPart*> parts[NUM_LAYERS];
	bool pUpdate[NUM_LAYERS];
	std::list<StoryPart*>::iterator currPartIt[NUM_LAYERS];
	TilesetManager *tm;
	sf::Font &myFont;
	GameSession *owner;
	std::string seqName;
	std::map<std::string, ConversationGroup*> convGroups;
	Conversation* GetConv(const std::string &name, 
		int index);
	void AddConvGroup(const std::string &name);
	bool HasConvGroup(const std::string &name);

	MusicInfo *oldMusic;
};



struct StoryImage
{
	enum ImageIntro
	{
		I_NONE
	};

	enum ImageOutro
	{
		O_NONE
	};

	ImageIntro intro;
	ImageOutro outro;
	StoryImage();
	void SetIntro(ImageIntro iType, int frameLength);
	void SetOutro(ImageOutro iType, int frameLength);
	void SetImage(Tileset *t, int tileIndex);
	void Reset();
	void SetScale(sf::Vector2f &scale);
	void SetPositionTopLeft(sf::Vector2f &pos);
	void SetPositionCenter(sf::Vector2f &pos);
	void SetRotation(float rot);
	void SetFlip(bool x, bool y);
	void UpdateImage();
	bool flipx;
	bool flipy;
	
	void SetConfirmNeeded( bool cn );
	bool isConfirmNeeded;
	bool Update( bool confirm = false );
	void UpdateIntro();
	void UpdateActive();
	void UpdateOutro();

	void Draw(sf::RenderTarget *target);
	
	sf::Sprite spr;
	Tileset *ts;
	int tile;
	int frame;
	
	int introLength;
	int activeLength;
	int outroLength;
};

struct StoryImageSequence
{

};

#endif