#ifndef __EDITORDECORINFO_H__
#define __EDITORDECORINFO_H__

#include "ISelectable.h"

struct TransformTools;
struct Tileset;
struct EditorDecorInfo : ISelectable
{
	enum DrawMode
	{
		D_NORMAL,
		D_TRANSFORM
	};

	EditorDecorInfo(const std::string &dName,
		Tileset *ts, int p_tile,int lay,
		sf::Vector2f &centerPos, float rot, sf::Vector2f &sc );
	EditorDecorInfo(EditorDecorInfo &edi );

	void CancelTransformation();
	DecorPtr CompleteTransformation();
	void UpdateTransformation(TransformTools *tr);
	void StartTransformation();

	bool ContainsPoint(sf::Vector2f test);
	bool Intersects(sf::IntRect rect);
	void Move(sf::Vector2i delta);
	void MoveTo(sf::Vector2i &pos);
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	void Deactivate();
	void Activate();
	void SetSelected(bool select);
	void WriteFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);

	//sf::Vector2f currScale;
	float rotation;

	void UpdateQuad();
	sf::IntRect GetAABB();
	sf::Vector2f center;
	sf::Vector2f scale;
	sf::Vector2f tileSize;
	DrawMode dMode;
	int layer;
	std::string decorName;
	sf::Vertex quad[4];
	Tileset *ts;
	int tile;
	std::list<DecorPtr> *myList;
	static bool CompareDecorInfoLayer(EditorDecorInfo &di0, EditorDecorInfo &di1);
};

#endif