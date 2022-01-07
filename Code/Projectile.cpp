//#include "Projectile.h"
//#include "Session.h"
//
//
//using namespace std;
//using namespace sf;
//
//ProjectileLauncher::ProjectileLauncher( int projType )
//{
//	Session *sess = Session::GetSession();
//
//	ts = NULL;
//	//numBullets = 10;
//	//bulletVec.resize(numBullets);
//	//verts = new Vertex[numBullets * 4];
//	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
//	/*for (int i = 0; i < numBullets; ++i)
//	{
//		bulletVec[i] = new Projectile(verts + 4 * i, this);
//	}*/
//}
//
//void ProjectileLauncher::SetTileset(Tileset *p_ts)
//{
//	ts = p_ts;
//}
//
//
////ProjectileLauncher::~ProjectileLauncher()
////{
////	for (int i = 0; i < numBullets; ++i)
////	{
////		delete bulletVec[i];
////	}
////
////	delete[] verts;
////}
////
////void ProjectileLauncher::Reset()
////{
////	for (int i = 0; i < numBullets; ++i)
////	{
////		bulletVec[i]->Reset();
////	}
////}
////
////Projectile * ProjectileLauncher::Throw(V2d &pos, V2d &dir)
////{
////	Projectile *bs = NULL;
////	for (int i = 0; i < numBullets; ++i)
////	{
////		bs = bulletVec[i];
////		if (!bs->spawned)
////		{
////			bs->Throw(pos, dir);
////			break;
////		}
////	}
////	return bs;
////}
////
////void ProjectileLauncher::Draw(sf::RenderTarget *target)
////{
////	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
////}
//
//Projectile::Projectile(ProjectileLauncher *p_launcher, int p_index )
//	:Enemy(EnemyType::EN_PROJECTILE, NULL)
//{
//	SetNumActions(A_Count);
//	SetEditorActions(IDLE, 0, 0);
//
//	actionLength[IDLE] = 1;
//	animFactor[IDLE] = 1;
//
//	//actionLength[GRIND] = 10;
//	//animFactor[GRIND] = 1;
//
//	launcher = p_launcher;
//	index = p_index;
//	quad = launcher->quads + index * 4;
//
//	CreateSurfaceMover(startPosInfo, 12, this);
//
//	hitboxInfo = new HitboxInfo;
//	hitboxInfo->damage = 18;
//	hitboxInfo->drainX = 0;
//	hitboxInfo->drainY = 0;
//	hitboxInfo->hitlagFrames = 0;
//	hitboxInfo->hitstunFrames = 10;
//	hitboxInfo->knockback = 4;
//
//	BasicCircleHitBodySetup(16);
//	hitBody.hitboxInfo = hitboxInfo;
//
//	//flySpeed = 10;
//
//	ResetEnemy();
//}
//
//void Projectile::ResetEnemy()
//{
//	ClearRect(quad);
//
//	facingRight = true;
//
//	//action = THROWN;
//	frame = 0;
//
//	//firePool.Reset();
//
//	DefaultHitboxesOn();
//
//	UpdateHitboxes();
//}
//
//void Projectile::HitTerrainAerial(Edge *e, double quant)
//{
//	//action = GRIND;
//	frame = 0;
//	surfaceMover->SetSpeed(10);
//}
//
//void Projectile::SetLevel(int lev)
//{
//	level = lev;
//	switch (level)
//	{
//	case 1:
//		scale = 1.0;
//		break;
//	case 2:
//		scale = 2.0;
//		maxHealth += 2;
//		break;
//	case 3:
//		scale = 3.0;
//		maxHealth += 5;
//		break;
//	}
//}
//
//void Projectile::Launch(ProjectileParams &params)
//{
//	Reset();
//	sess->AddEnemy(this);
//
//	currPosInfo.position = params.position;
//	currPosInfo.ground = NULL;
//
//	if (surfaceMover != NULL)
//	{
//		surfaceMover->Set(currPosInfo);
//	}
//
//	//surfaceMover->velocity = dir * flySpeed;
//}
//
//bool Projectile::IsEnemyMoverAction(int a)
//{
//	return false;
//}
//
//void Projectile::FrameIncrement()
//{
//}
//
//void Projectile::ProcessState()
//{
//	if (frame == actionLength[action] * animFactor[action])
//	{
//		switch (action)
//		{
//		case IDLE:
//			break;
//		case LINEAR_MOVE:
//			break;
//		}
//		frame = 0;
//	}
//
//	/*if (action == GRIND)
//	{
//		firePool.Create(GetPosition(), surfaceMover->ground, surfaceMover->edgeQuantity);
//	}*/
//}
//
//void Projectile::IHitPlayer(int index)
//{
//}
//
//void Projectile::UpdateSprite()
//{
//	//ts->SetQuadSubRect(quad, 0);
//	//SetRectCenter(quad, ts->tileWidth, ts->tileWidth, GetPositionF());
//}
//
//void Projectile::EnemyDraw(sf::RenderTarget *target)
//{
//	//firePool.Draw(target);
//}
//
//void Projectile::HandleHitAndSurvive()
//{
//}