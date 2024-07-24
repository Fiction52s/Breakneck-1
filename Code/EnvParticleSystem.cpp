#include "EnvParticleSystem.h"
#include "Session.h"
#include "ParticleEffects.h"

using namespace sf;
using namespace std;

EnvParticleSystem::EnvParticleSystem()
{
	sess = Session::GetSession();

	//ShapeEmitter *testEmitter2 = new ShapeEmitter(6, DrawLayer::IN_FRONT);// PI / 2.0, 2 * PI, 1.0, 2.5);




	ShapeEmitter *testEmitter = new ForegroundTestEmitter(6, DrawLayer::BG_5);// PI / 2.0, 2 * PI, 1.0, 2.5);
	testEmitter->CreateParticles();
	testEmitter->SetIDAndAddToAllEmittersVec();
	emitters.push_back(testEmitter);


	ShapeEmitter *testEmitter2 = new ForegroundTestEmitter(6, DrawLayer::FG_5);// PI / 2.0, 2 * PI, 1.0, 2.5);
	testEmitter2->CreateParticles();
	testEmitter2->SetIDAndAddToAllEmittersVec();
	emitters.push_back(testEmitter2);

	/*testEmitter2->CreateParticles();
	testEmitter2->SetIDAndAddToAllEmittersVec();
	emitters.push_back(testEmitter2);
	testEmitter2->SetPos(Vector2f( 200, 0));
	testEmitter2->SetRatePerSecond(30);*/
	





	//testEmitter->SetPos(GetPositionF());
	//testEmitter->SetRatePerSecond(30);
}

EnvParticleSystem::~EnvParticleSystem()
{
	for (auto it = emitters.begin(); it != emitters.end(); ++it)
	{
		delete (*it);
	}
}

void EnvParticleSystem::Update()
{
	return; //testing

	if (sess->totalGameFrames == 0)
	{
		for (int i = 0; i < emitters.size(); ++i)
		{
			emitters[i]->Reset();
			sess->AddEmitter(emitters[i]);
		}
	}
}

//void EnvParticleSystem::LayeredDraw(int p_drawLayer, sf::RenderTarget *target)
//{
//
//}