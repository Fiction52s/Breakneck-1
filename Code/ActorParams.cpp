#include "EditSession.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Physics.h"

using namespace std;
using namespace sf;

#define cout std::cout
#define V2d sf::Vector2<double>


EditSession * ActorParams::session = NULL;

ActorParams::ActorParams( ActorParams::PosType p_posType )
	:ISelectable( ISelectable::ACTOR ), boundingQuad( sf::Quads, 4 ), posType( p_posType ) //, ground( NULL ), groundQuantity( 420.69 ), 
{
	groundInfo = NULL;

	for( int i = 0; i < 4; ++i )
		boundingQuad[i].color = Color( 0, 255, 0, 100);
}

void ActorParams::SetSelected( bool select )
{
	cout << "------selected: " << select << endl;
	selected = select;
}

bool ActorParams::CanAdd()
{
	assert( false );
	return false; //shouldn't get called anyway
}

void ActorParams::Draw( sf::RenderTarget *target )
{
	//cout << "Selected: " << selected << endl;
	if( selected )
	{
		sf::RectangleShape rs;
		rs.setFillColor( Color::Transparent );
		rs.setOutlineColor( Color::Green );
		rs.setOutlineThickness( 3 * EditSession::zoomMultiple );
		rs.setPosition( image.getGlobalBounds().left, image.getGlobalBounds().top );
		rs.setSize( Vector2f( image.getGlobalBounds().width, image.getGlobalBounds().height ) );
		target->draw( rs );
		//cout << "selected draw" << endl;
	}
	target->draw( image );
}

void ActorParams::WriteFile( ofstream &of )
{
	//if( params.size() == 0 )
	//{
	//	assert( false && "no params" );
	//}
	
	//dont need number of params because the actortype determines that.
	of << type->name << " ";

	if( type->canBeGrounded && type->canBeAerial )
	{
		if( groundInfo != NULL )
		{
			int edgeIndex = groundInfo->GetEdgeIndex();
			of << "-air" << " " << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
		}
		else
		{
			of << "+air" << " " << position.x << " " << position.y << endl;
		}
	}
	else if( type->canBeGrounded )
	{
		assert( groundInfo != NULL );

		int edgeIndex = groundInfo->GetEdgeIndex();

		of << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
	}
	else if( type->canBeAerial )
	{
		of << position.x << " " << position.y << endl;
	}
	else
	{
		assert( false );
	}
	

	/*for( list<string>::iterator it = params.begin(); it != params.end(); ++it )
	{
		of << (*it) << endl;
	}*/
	WriteParamFile( of );
}

void ActorParams::DrawQuad( sf::RenderTarget *target )
{
	target->draw( boundingQuad );
}

GroundInfo::GroundInfo()
	:edgeStart( NULL ), groundQuantity( -1 ), ground( NULL )
{

}

int GroundInfo::GetEdgeIndex()
{
	int index = 0;
	for( TerrainPoint *curr = ground->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr == edgeStart )
			return index;
		++index;
	}

	
	//assert( false && "could not find correct edge index" );
	return -1;
}

void ActorParams::SetBoundingQuad()
{
	//float note
	if( type->canBeGrounded && groundInfo != NULL )
	{
		V2d v0( (*groundInfo->edgeStart).pos.x, (*groundInfo->edgeStart).pos.y );
		TerrainPoint *edgeEnd = groundInfo->edgeStart->next;
		if( edgeEnd == NULL )
			edgeEnd = groundInfo->ground->pointStart;
		V2d v1( edgeEnd->pos.x, edgeEnd->pos.y );
		V2d along = normalize( v1 - v0 );
		V2d other( along.y, -along.x );

		V2d startGround = v0 + along * groundInfo->groundQuantity;
		V2d leftGround = startGround - along * ( type->width / 2.0) + other * 1.0;
		V2d rightGround = startGround + along * ( type->width / 2.0) + other * 1.0;
		V2d leftAir = leftGround + other * (double)(type->height - 1) ;
		V2d rightAir = rightGround + other * (double)(type->height - 1 );

		boundingQuad[0].position = Vector2f( leftGround.x, leftGround.y );
		boundingQuad[1].position = Vector2f( leftAir.x, leftAir.y );
		boundingQuad[2].position = Vector2f( rightAir.x, rightAir.y );
		boundingQuad[3].position = Vector2f( rightGround.x, rightGround.y );
	}
	else
	{
		//patroller doesnt need a box because its not physical with the environment
		boundingQuad[0].position = Vector2f( position.x - type->width / 2, position.y - type->height / 2);
		boundingQuad[1].position = Vector2f( position.x + type->width / 2, position.y - type->height / 2);
		boundingQuad[2].position = Vector2f( position.x + type->width / 2, position.y + type->height / 2);
		boundingQuad[3].position = Vector2f( position.x - type->width / 2, position.y + type->height / 2);
	}
}

void ActorParams::UpdateGroundedSprite()
{	
	assert( groundInfo != NULL && groundInfo->ground != NULL );
	
	TerrainPoint *edge = groundInfo->edgeStart;
	TerrainPoint *next = edge->next;
	if( next == NULL )
		next = groundInfo->ground->pointStart;

	

	V2d pr( edge->pos.x, edge->pos.y );
	V2d cu( next->pos.x, next->pos.y );


	//this shouldn't remain here. i need more detailed checking.
	double groundLength = length( pr - cu );
	if( groundInfo->groundQuantity + type->width / 2 > groundLength )
	{
		groundInfo->groundQuantity = groundLength - type->width / 2;
	}
	else if( groundInfo->groundQuantity - type->width / 2 < 0 )
	{
		groundInfo->groundQuantity = type->width / 2;
	}

	V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
									(groundInfo->groundQuantity / length( cu - pr ) ) );

	double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

	image.setPosition( newPoint.x, newPoint.y );
	image.setRotation( angle );
}

void ActorParams::AnchorToGround( TerrainPolygon *poly, int edgeIndex, double quantity )
{
	assert( groundInfo == NULL );
	/*if( groundInfo != NULL )
	{
		cout << "unanchor" << endl;
		
		//UnAnchor( actor );
		cout << "end unanchor" << endl;
		
		//delete groundInfo;
		//groundInfo = NULL;
	}*/



	groundInfo = new GroundInfo;
	
	groundInfo->ground = poly;
	
	groundInfo->groundQuantity = quantity;
	
	int testIndex = 0;

	image.setTexture( type->imageTexture );	
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );

	Vector2i point;

	TerrainPoint *prev = groundInfo->ground->pointEnd;
	TerrainPoint *curr = groundInfo->ground->pointStart;

	for( ; curr != NULL; curr = curr->next )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( prev->pos.x, prev->pos.y );
			V2d cu( curr->pos.x, curr->pos.y );

			V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(groundInfo->groundQuantity / length( cu - pr ) ) );

			double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

			groundInfo->edgeStart = prev;

			
			UpdateGroundedSprite();
			SetBoundingQuad();

			break;
		}
		prev = curr;
		++testIndex;
	}

	
}

void ActorParams::UnAnchor( ActorPtr &actor )
{
	assert( groundInfo != NULL );
	if( groundInfo != NULL )
	{
		position = Vector2i( image.getPosition().x, image.getPosition().y );

		image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
		image.setRotation( 0 );
		//groundInfo->ground->enemies.clear();
		//i get deleted when i remove myself from the terrain polygon????

		//group->actors.front().use_count();

		//ActorPtr actor( this );
		//group->actors.
		//cout << "use count: " << actor.use_count() << endl;
		
		//groundInfo->ground->enemies.clear();
		
		//cout << 
		//SelectPtr select = boost::dynamic_pointer_cast<ISelectable>		
		groundInfo->ground->enemies[groundInfo->edgeStart].remove( actor );
		//cout << "After remove" << endl;

		//cout << "use count: " << actor.use_count() << endl;

		delete groundInfo;
		groundInfo = NULL;

		//cout << "About to bound" << endl;
		SetBoundingQuad();
		//cout << "just bounded" << endl;
		//image.setPosition( 
	}
}

bool ActorParams::ContainsPoint( sf::Vector2f test )
{ 
	sf::Transform trans = image.getTransform();


	/*V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );*/	

	
	V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );

	bool result = QuadContainsPoint( A,B,C,D, V2d( test.x, test.y ) );

	//cout << "result: " << result << endl;
	return result;
}

bool ActorParams::Intersects( sf::IntRect rect )
{
	V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );

	V2d rA( rect.left, rect.top );
	V2d rB( rect.left + rect.width, rect.top );
	V2d rC( rect.left + rect.width, rect.top + rect.height );
	V2d rD( rect.left, rect.top + rect.height );
	if( isQuadTouchingQuad( A, B, C, D, rA, rB, rC, rD ) )
	{
		return true;
	}
	else
		return false;
}

bool ActorParams::IsPlacementOkay()
{
	return false;
}

void ActorParams::Move( SelectPtr &me, sf::Vector2i delta )
{
	assert( groundInfo == NULL );

	//ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( me );
	//if( groundInfo != NULL )
	//{
	//	UnAnchor( actor );
	//}
	
	position.x += delta.x;
	position.y += delta.y;

	SetBoundingQuad();

	image.setPosition( position.x, position.y );
	
	/*if( session->worldPosGround.ground != NULL )
	{
		
		AnchorToGround( session->worldPosGround.ground, session->worldPosGround.GetEdgeIndex(), session->worldPosGround.groundQuantity );
		
		SetBoundingQuad();
		UpdateGroundedSprite();
		//session->editStartMove = false;

		//p->enemies[groundInfo->edgeStart].push_back( actor );
		//p->UpdateBounds();
		//cout << "new end" << endl;
	}
	else
	{
		SetBoundingQuad();

		image.setPosition( position.x, position.y );
	}*/
}

void ActorParams::BrushDraw( sf::RenderTarget *target, 
		bool valid )
{
	image.setColor( Color( 255, 255, 255, 100 ) );
	target->draw( image );
	image.setColor( Color::White );
}

void ActorParams::Deactivate( EditSession *edit, SelectPtr &select )
{
	ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( select );
	group->actors.remove( actor );
}

void ActorParams::Activate( EditSession *edit, SelectPtr &select )
{
	ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( select );
	group->actors.push_back( actor );
}

KeyParams::KeyParams( EditSession *edit, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop,
					 int p_stayFrames, bool p_teleport, GateInfo::GateTypes gType )
	:ActorParams( PosType::AIR_ONLY )
{	
	lines = NULL;
	position = pos;	
	gateType = gType;

	type = edit->types["key"];

	if( gateType == GateInfo::RED )
	{
		image.setTexture( type->imageTexture );
	}
	else if( gateType == GateInfo::GREEN )
	{
		image.setTexture( edit->types["greenkey"]->imageTexture );
	}
	else if( gateType == GateInfo::BLUE )
	{
		image.setTexture( edit->types["bluekey"]->imageTexture );
	}
	

	
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;
	stayFrames = p_stayFrames;
	teleport = p_teleport;


	SetBoundingQuad();
}

bool KeyParams::CanApply()
{
	return true;
	//for now can apply anywhere. should you only be able to apply when not on the same spot as other keys??
}

void KeyParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	if( lines != NULL )
	{
		delete lines;
		lines = NULL;
	}

	localPath.clear();
	if( globalPath.size() > 1 )
	{
		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}
}

void KeyParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		VertexArray &li = *lines;
	
	
			for( int i = 0; i < localPathSize+1; ++i )
			{
				li[i].position += Vector2f( position.x, position.y );
			}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	target->draw( image );
	if( selected )
	{
		sf::RectangleShape rs;
		rs.setFillColor( Color::Transparent );
		rs.setOutlineColor( Color::Green );
		rs.setOutlineThickness( 3 * EditSession::zoomMultiple );
		rs.setPosition( image.getGlobalBounds().left, image.getGlobalBounds().top );
		rs.setSize( Vector2f( image.getGlobalBounds().width, image.getGlobalBounds().height ) );
		target->draw( rs );
		//cout << "selected draw" << endl;
	}
}

std::list<sf::Vector2i> KeyParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void KeyParams::WriteParamFile( ofstream &of )
{
	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	of << (int)gateType << endl;

	if( loop )
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	of.precision( 5 );
	of << fixed << speed << endl;

	of << stayFrames << endl;
	
	if( teleport )
	{
		of << "+tele" << endl;
	}
	else
	{
		of << "-tele" << endl;
	}
}

PatrollerParams::PatrollerParams( EditSession *edit, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop )
	:ActorParams( PosType::AIR_ONLY)
{	
	lines = NULL;
	position = pos;	
	type = edit->types["patroller"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;

	SetBoundingQuad();
	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

PatrollerParams::PatrollerParams( EditSession *edit )
	:ActorParams( PosType::AIR_ONLY )
{	
	lines = NULL;
	//position = pos;	
	type = edit->types["patroller"];

	//image.setTexture( type->imageTexture );
	//image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	//image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	//SetPath( globalPath );

	//loop = p_loop;
	//speed = p_speed;

	//SetBoundingQuad();

	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

bool PatrollerParams::CanApply()
{
	return true;
	//see note for keyparams
}

void PatrollerParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	if( lines != NULL )
	{
		delete lines;
		lines = NULL;
	}
	
	
	

	localPath.clear();
	if( globalPath.size() > 1 )
	{

		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}

	
	
}

void PatrollerParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		VertexArray &li = *lines;
	
	
			for( int i = 0; i < localPathSize+1; ++i )
			{
				li[i].position += Vector2f( position.x, position.y );
			}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	target->draw( image );

	if( selected )
	{
		sf::RectangleShape rs;
		rs.setFillColor( Color::Transparent );
		rs.setOutlineColor( Color::Green );
		rs.setOutlineThickness( 3 * EditSession::zoomMultiple );
		rs.setPosition( image.getGlobalBounds().left, image.getGlobalBounds().top );
		rs.setSize( Vector2f( image.getGlobalBounds().width, image.getGlobalBounds().height ) );
		target->draw( rs );
		//cout << "selected draw" << endl;
	}
}

std::list<sf::Vector2i> PatrollerParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void PatrollerParams::WriteParamFile( ofstream &of )
{
	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	if( loop )
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	of.precision( 5 );
	of << fixed << speed << endl;
}

CrawlerParams::CrawlerParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, bool p_clockwise, float p_speed )
	:ActorParams( PosType::GROUND_ONLY )
{
	clockwise = p_clockwise;
	speed = p_speed;

	type = edit->types["crawler"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

CrawlerParams::CrawlerParams( EditSession *edit )
	:ActorParams( PosType::GROUND_ONLY ), clockwise( true ), speed( 0 )
{
	type = edit->types["crawler"];
}

bool CrawlerParams::CanApply()
{
	//hmm not sure about this now

	return true;
}

void CrawlerParams::WriteParamFile( ofstream &of )
{
	if( clockwise )
		of << "+clockwise" << endl;
	else
		of << "-clockwise" << endl;
	
	of.precision( 5 );
	of << fixed << speed << endl;
}

BasicTurretParams::BasicTurretParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, double p_bulletSpeed, int p_framesWait )
	:ActorParams( PosType::GROUND_ONLY )
{
	bulletSpeed = p_bulletSpeed;
	framesWait = p_framesWait;

	type = edit->types["basicturret"];
	
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool BasicTurretParams::CanApply()
{
	//hmm not sure about this now

	return true;
}

void BasicTurretParams::WriteParamFile( ofstream &of )
{
	of << bulletSpeed << endl;
	of << framesWait << endl;
}

FootTrapParams::FootTrapParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( PosType::GROUND_ONLY )	
{
	type = edit->types["foottrap"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool FootTrapParams::CanApply()
{
	//hmm not sure about this now

	return true;
}

void FootTrapParams::WriteParamFile( ofstream &of )
{
}

GoalParams::GoalParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( PosType::GROUND_ONLY )
{
	type = edit->types["goal"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool GoalParams::CanApply()
{
	//hmm not sure about this now

	return true;
}

void GoalParams::WriteParamFile( ofstream &of )
{
}