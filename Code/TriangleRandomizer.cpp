//#include "TriangleRandomizer.h"
//#include "VectorMath.h"
//
//using namespace std;
//using namespace sf;
//
//PokeTriGroup::PokeTriGroup(int numTris)
//{
//	tris = new Vertex[numTris * 3];
//
//	Vector2f dir(1, 0);
//	RotateCCW(dir, (2 * PI) / numTris );
//	for (int i = 0; i < numTris; ++i)
//	{
//		PokeTri *newTri = new PokeTri(tris, i, Vector2f( 960, 540 ), dir * 200.f );
//		pokeTris.push_back(newTri);
//	}
//}
//
//void PokeTriGroup::Reset()
//{
//	for (int i = 0; i < pokeTris.size(); ++i)
//	{
//		pokeTris[i]->Reset();
//	}
//}
//
//void PokeTriGroup::ActivateTriangle(int index)
//{
//	pokeTris[i]->Reset();
//}
//
//void PokeTriGroup::Update()
//{
//	for (int i = 0; i < pokeTris.size(); ++i)
//	{
//		pokeTris[i]->Update();
//	}
//}
//
//
//void PokeTriGroup::Draw(sf::RenderTarget *target)
//{
//
//}
//
//PokeTri::PokeTri( sf::Vertex *v, int index, sf::Vector2f &base,
//	sf::Vector2f &offset)
//{
//	pokeAngle = GetVectorAngleCW(offset) / PI * 180.0;
//
//	stateLength[S_POKING] = 15;
//	stateLength[S_SHRINKING] = 15;
//
//	maxLength = 300;
//	startWidth = 50;
//	startColor = Color::Blue;
//	fadeColor = Color(40, 40, 40, 0);
//}
//
//void PokeTri::Reset()
//{
//	color = startColor;
//	length = 0;
//	frame = 0;
//	state = S_POKING;
//	angle = pokeAngle;
//	width = startWidth;
//	done = false;
//	Clear();
//	//UpdatePoints();
//}
//
//void PokeTri::Update()
//{
//	if (done)
//		return;
//
//	if (frame == stateLength[state])
//	{
//		frame = 0;
//		switch (state)
//		{
//		case S_POKING:
//			state = S_SHRINKING;
//			break;
//		case S_SHRINKING:
//			done = true;
//			Clear();
//			return;
//		}
//	}
//
//	double fac = (double)frame / stateLength[state];
//	switch (state)
//	{
//	case S_POKING:
//		length = fac * maxLength;
//		break;
//	case S_SHRINKING:
//		length = (1.0 - fac) * maxLength;
//		break;
//	}
//
//
//	UpdatePoints();
//	++frame;
//}
//
//void PokeTri::UpdatePoints()
//{
//	Vector2f dir(1, 0);
//	RotateCCW(dir, angle);
//
//	Vector2f norm(dir.y, -dir.x);
//
//	Vector2f end = basePos + dir * length;
//
//	points[0].position = end;
//	points[1].position = end;
//	points[2].position = basePos + norm * (width / 2.f);
//	points[3].position = basePos - norm * (width / 2.f);
//
//	SetColor(color);
//}