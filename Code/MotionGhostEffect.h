#ifndef __MOTIONGHOSTEFFECT_H__
#define __MOTIONGHOSTEFFECT_H__

#include "Movement.h"



struct Tileset;
struct VertexBuf;

struct MotionGhostEffect
{
	enum GhostType
	{
		GT_DRAG,
		GT_PAST,
	};

	MotionGhostEffect(int maxGhosts, int gt );
	~MotionGhostEffect();
	void SetParams();
	void AddPosition(sf::Vector2f &pos);
	void SetShader(sf::Shader *pShad);
	void SetRootPos(sf::Vector2f &pos);
	void SetDistInBetween(float dist);
	void SetSpread(int numGhosts, sf::Vector2f &dir, float angle);
	void SetVibrateParams(CubicBezier &vBez, float maxVibrate, int startGhost = 0);
	void SetTileset(Tileset *ts);
	void SetTile(int tileIndex);
	void SetColor(sf::Color &c);

	void SetColorGradient(sf::Color &c0,
		sf::Color &c1, CubicBezier &bez);
	void Draw(sf::RenderTarget *target);
	void SetFacing(bool facingRight, bool reversed);
	void SetScaleParams(CubicBezier &sBez, float p_maxScaleUp, float p_maxScaleDown, int startScaleGhost = 0);
	void ApplyUpdates();
	void Reset();


	int maxGhosts;
	GhostType gType;
	VertexBuf *motionGhostBuffer;
	sf::Color rootColor;
	sf::Color tailColor;
	CubicBezier colorBez; //starts at root
	bool facingRight;
	float angle;
	bool reversed;

	int tileIndex;
	Tileset *ts;
	sf::Vector2f dir;
	sf::Vector2f rootPos;
	int numGhosts;
	float distInBetween;
	sf::Shader *shader;

	CubicBezier vibrateBez;
	int startVibrateGhost;
	CubicBezier scaleBez;
	int startScaleGhost;
	float maxScaleUp;
	float maxScaleDown;
	float maxVibrate;

	std::vector<sf::Vector2f> oldPositions;
	//settileset and settile
	//void SetParams( )
	//setrotation
	//setrootposition, direction, spread dist,offset
	//set scale if i >= 10
	//setnumactivemembers
	//setghostcolor
	//updateverticesg
	//draw

};

#endif