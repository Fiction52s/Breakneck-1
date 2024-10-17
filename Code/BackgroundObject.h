#ifndef __BACKGROUND_OBJECT_H__
#define __BACKGROUND_OBJECT_H__


#include <SFML\Graphics.hpp>
#include <fstream>
#include "nlohmann\json.hpp"
#include <map>

struct Tileset;
struct BackgroundObject;
struct Background;

struct BackgroundLayer
{
	struct QuadInfo
	{
		QuadInfo(sf::Vertex *p_verts, int p_numQuads, Tileset *p_ts, sf::Shader *p_sh, const std::string &p_shaderName )
			:verts(p_verts), numQuads(p_numQuads), ts( p_ts ), sh(p_sh), shaderName( p_shaderName )
		{

		}
		~QuadInfo()
		{
			delete[] verts;
		}
		sf::Vertex *verts;
		int numQuads;
		Tileset *ts;
		sf::Shader *sh;
		std::string shaderName;
	};

	int drawLayer;
	std::vector<BackgroundObject*> objectVec;
	std::vector<QuadInfo*> quadPtrVec;
	std::vector<BackgroundObject*> selfDrawingObjects;
	sf::View oldView;
	sf::View newView;
	Background *bg;

	BackgroundLayer(Background *p_bg, int p_drawLayer);
	~BackgroundLayer();
	void Update(const sf::Vector2f &camPos);
	void Update(const sf::Vector2f &camPos, const int numFrames);
	void SetupQuads();
	void Draw(sf::RenderTarget *target);
};

struct BackgroundObject
{
	enum Type
	{
		BGO_WATERFALL,
	};


	Tileset *ts;
	sf::Shader *sh;
	std::string shaderName;
	sf::Vector2f myPos;
	sf::Vertex *quads;

	int loopWidth;
	int numQuads;
	int action;
	int frame;
	Background *bg;
	int depthLayer;
	float scrollSpeedX;
	sf::View oldView;
	sf::View newView;
	int repetitionFactor; //how often it repeats
	int scrollOffset;

	BackgroundObject(Background *p_bg, int p_layer );
	~BackgroundObject();
	virtual void Reset();
	virtual void Load(std::ifstream & is);
	virtual void DrawObject(sf::RenderTarget *target);
	virtual sf::IntRect GetSubRect();
	virtual void ProcessAction();

	virtual void UpdateQuads( float realX );
	void Update(const sf::Vector2f &camPos);
	void Update(const sf::Vector2f &camPos, const int numFrames);
	void Draw(sf::RenderTarget *target);
	void LayeredDraw(int p_depthLevel, sf::RenderTarget *target);
};

struct BackgroundTile : BackgroundObject
{
	sf::IntRect subRect;
	std::string folder;
	

	BackgroundTile(Background *p_bg, const std::string &p_folder, int p_layer );
	void Load(nlohmann::basic_json<> &jobj);
	sf::IntRect GetSubRect();
};

struct BackgroundWideSpread : BackgroundObject
{
	sf::IntRect subRect;
	std::string folder;
	int extraWidth;

	BackgroundWideSpread(Background *p_bg, const std::string &p_folder, int p_layer );
	void Load(nlohmann::basic_json<> &jobj);
	void UpdateQuads(float realX);
	sf::IntRect GetSubRect();
};

struct BackgroundWaterfall : BackgroundObject
{
	enum Action
	{
		A_IDLE,
		A_Count,
	};

	bool shortWaterfall;

	int actionLength[A_Count];
	int animFactor[A_Count];

	BackgroundWaterfall(Background *p_bg, int p_layer );
	~BackgroundWaterfall();
	void Load(nlohmann::basic_json<> &jobj);
	void DrawObject(sf::RenderTarget *target);
	void UpdateQuads(float realX);

	void ProcessAction();

};

struct BackgroundWaves : BackgroundObject
{
	enum Action
	{
		A_IDLE,
		A_Count,
	};

	Tileset *ts_0;
	Tileset *ts_1;

	int actionLength[A_Count];
	int animFactor[A_Count];

	BackgroundWaves(Background *p_bg, int p_layer);
	~BackgroundWaves();
	void Load(nlohmann::basic_json<> &jobj);

	void ProcessAction();
	sf::IntRect GetSubRect();
};

struct BackgroundTileTranscendGlow : BackgroundTile
{
	enum Action
	{
		A_IDLE,
		A_Count,
	};

	int actionLength[A_Count];
	int animFactor[A_Count];

	BackgroundTileTranscendGlow(Background *p_bg, const std::string &p_folder, int p_layer);
	~BackgroundTileTranscendGlow();
	void UpdateQuads(float realX);
	sf::IntRect GetSubRect();
	void ProcessAction();
};



#endif