#include "GoalFlow.h"
#include <iostream>
#include <assert.h>
#include "Session.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

GoalFlow::GoalFlow(Vector2f &gPos, list<list<pair<V2d, bool>>> &infoList)
{
	flowFrameCount = 60;
	flowFrame = 0;
	maxFlowRadius = 10000;
	radDiff = 100;
	flowSpacing = 600;
	maxFlowRings = 40;
	goalEnergyFlowVA = NULL;

	sess = Session::GetSession();

	ts_veinEnd = sess->GetSizedTileset("Env/vein_end_16x4.png");

	if (!flowShader.loadFromFile("Resources/Shader/flow.frag", sf::Shader::Fragment))
	{
		cout << "flow SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}

	Color fColor(0, 0x66, 0xCC);
	Color fCloseColor(0, 0xEE, 0xFF);

	flowShader.setUniform("radDiff", radDiff);
	flowShader.setUniform("Resolution", Vector2f(1920, 1080));// window->getSize().x, window->getSize().y);
	flowShader.setUniform("flowSpacing", flowSpacing);
	flowShader.setUniform("maxFlowRings", maxFlowRadius / maxFlowRings);
	
	SetWorld(0);

	Setup(gPos, infoList);
}

GoalFlow::~GoalFlow()
{
	delete goalEnergyFlowVA;

	delete [] veinEndQuads;
}

void GoalFlow::Setup( sf::Vector2f &gPos, list<list<pair<V2d, bool>>> &infoList)
{
	SetGoalPos(gPos);

	int totalPoints = 0;
	for (auto it = infoList.begin(); it != infoList.end(); ++it)
	{
		list<pair<V2d, bool>> &pointList = (*it);
		totalPoints += pointList.size();
	}

	numFlowQuads = totalPoints / 2;
	//cout << "number of quads: " << totalPoints / 2 << endl;

	veinEndQuads = new Vertex[numFlowQuads * 4 * 2];

	goalEnergyFlowVA = new VertexArray(sf::Quads, numFlowQuads * 4);
	VertexArray &va = *goalEnergyFlowVA;
	int extra = 0;
	double width = 16;

	int wI = width;
	double currW = 0;
	int minWidth = 8;
	double awayFromEdgeAmount = 32;

	for (auto it2 = infoList.begin(); it2 != infoList.end(); ++it2)
	{
		list<pair<V2d, bool>> &pointList = (*it2);
		for (list<pair<V2d, bool>>::iterator it = pointList.begin(); it != pointList.end(); ++it)
		{
			V2d startPoint = (*it).first;
			++it;
			V2d endPoint = (*it).first;

			V2d along = normalize(endPoint - startPoint);
			V2d other(along.y, -along.x);

			currW = rand() % (wI - minWidth) + minWidth;

			V2d startLeft = startPoint - other * currW / 2.0 + along * awayFromEdgeAmount;
			V2d startRight = startPoint + other * currW / 2.0 + along * awayFromEdgeAmount;
			V2d endLeft = endPoint - other * currW / 2.0 - along * awayFromEdgeAmount;
			V2d endRight = endPoint + other * currW / 2.0 - along * awayFromEdgeAmount;

			va[extra + 0].color = Color::Red;
			va[extra + 1].color = Color::Red;
			va[extra + 2].color = Color::Red;
			va[extra + 3].color = Color::Red;

			va[extra + 0].position = Vector2f(startLeft.x, startLeft.y);
			va[extra + 1].position = Vector2f(startRight.x, startRight.y);
			va[extra + 2].position = Vector2f(endRight.x, endRight.y);
			va[extra + 3].position = Vector2f(endLeft.x, endLeft.y);

			ts_veinEnd->SetQuadSubRect(veinEndQuads + extra * 2, 0);
			ts_veinEnd->SetQuadSubRect(veinEndQuads + extra * 2 + 4, 0);

			double ang = GetVectorAngleCW(other);
			SetRectRotation(veinEndQuads + extra * 2, ang, currW + 2, 4, Vector2f( startPoint + along * awayFromEdgeAmount ));
			SetRectRotation(veinEndQuads + extra * 2 + 4, ang + PI, currW + 2, 4, Vector2f(endPoint - along * awayFromEdgeAmount));

				
				//veinEndQuads[extra *4]

			extra += 4;
		}
	}
}

void GoalFlow::SetGoalPos(sf::Vector2f &gPos)
{
	goalPos = gPos;
	flowShader.setUniform("goalPos", goalPos );
}

void GoalFlow::SetWorld(int worldIndex)
{
	Color fColor;
	Color fCloseColor;

	switch (worldIndex)
	{
	case 0:
		fColor = Color(0, 0x44, 0xCC);
		fCloseColor = Color(0, 0xEE, 0xFF);
		break;
	case 1:
		fColor = Color(0, 0x88, 0x88);
		fCloseColor = Color(0, 0xFF, 0x44);
		break;
	case 2:
		fColor = Color(0xCC, 0x77, 0);
		fCloseColor = Color(0xFF, 0xFF, 0);
		break;
	case 3:
		fColor = Color(0xCC, 0x33, 0);
		fCloseColor = Color(0xFF, 0x99, 0);
		break;
	case 4:
		fColor = Color(0x88, 0, 0x44);
		fCloseColor = Color(0xFF, 0, 0);
		break;
	case 5:
		fColor = Color(0x88, 0, 0xBB);
		fCloseColor = Color(0xFF, 0, 0xDD);
		break;
	case 6:
		fColor = Color(0x33, 0x77, 0x99);
		fCloseColor = Color(0x99, 0xCC, 0xCC);
		break;
	case 7:
		fColor = Color(0x55, 0, 0xFF);
		fCloseColor = Color(0xAA, 0x55, 0xFF);
		break;
	default:
		fColor = Color(0xa9, 0x00, 0x00);//Color::Red;//Color(0, 0x66, 0xCC);
		fCloseColor = Color(0xff, 0x24, 0x24); //Color::Yellow;//Color(0, 0xEE, 0xFF);
		break;
	}

	flowShader.setUniform("flowColor", ColorGL(fColor));
	flowShader.setUniform("closeFlowColor", ColorGL(fCloseColor));
}

void GoalFlow::Update( float camZoom, sf::Vector2f &topLeft, float camAngleRad  )
{
	V2d pPos = sess->GetPlayerPos(0);
	flowRadius = (maxFlowRadius - (maxFlowRadius / flowFrameCount) * flowFrame);

	flowShader.setUniform("radius", flowRadius / maxFlowRings);
	flowShader.setUniform("zoom", camZoom);
	flowShader.setUniform("playerPos", Vector2f(pPos));

	flowShader.setUniform("topLeft", topLeft );

	flowShader.setUniform("u_cameraAngle", camAngleRad);

	++flowFrame;
	if (flowFrame == flowFrameCount)
	{
		flowFrame = 0;
	}
}

void GoalFlow::Draw(sf::RenderTarget *target)
{
	target->draw(*goalEnergyFlowVA, &flowShader);
	target->draw(veinEndQuads, numFlowQuads * 2 * 4, sf::Quads, ts_veinEnd->texture);
}