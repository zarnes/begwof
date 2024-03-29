#include "Fish.h"
#include "math.h"
#include "Guru.h"
using namespace cocos2d;

double const Fish::STEP = 20;
const double Fish::DISTANCE_MIN = 8;
const double Fish::DISTANCE_MIN_SQUARED = 64;
const double Fish::DISTANCE_MAX = 100;
const double Fish::DISTANCE_MAX_SQUARED = 10000;

Fish::Fish(cocos2d::Node * parent, double _X, double _Y, double _dir): Element(parent)
{
	velocity = Point(cos(_dir),sin(_dir));
	pos = Point(_X, _Y);
	lineLength = 10;
	lineThickness = 0.5;
	guru = nullptr;
}
Fish::Fish(): Element()
{

}
Fish::~Fish()
{
}

void Fish::Update(std::vector<std::shared_ptr<Fish>> &fishes, std::list<AreaToAvoid> &obstacles, double width, double length, float dt)
{
	if (!AvoidWall(0, 0, width, length))
	{
		if (!AvoidObstacle(obstacles))
		{
			if (!AvoidFish(fishes))
			{
				CalculateAverageDirection(fishes);
			}
		}
	}
	UpdatePosition(dt);
	Draw(dt);
}

void Fish::Draw(float dt)
{
	//PLACEHOLDER
	/*velocity.x = 5;
	velocity.y = std::sin(time) * 5;
	time += dt;
	
	// TODO LOGIC

	pos += velocity * dt;*/

	Point from = pos;
	Point to = from + velocity.getNormalized() * lineLength;
	
	draw->clear();
	draw->drawSegment(from, to, lineThickness, color);
	//draw->drawLine(from, to, color);
	draw->drawCircle(from, clickRadius, 360, 12, false, color);
}

double Fish::getVelocityX()
{
	return velocity.x;
}

double Fish::getVelocityY()
{
	return velocity.y;
}

void Fish::UpdatePosition(float dt)
{
	pos.x += Fish::STEP * velocity.x * dt;
	pos.y += Fish::STEP * velocity.y * dt;
}

bool Fish::InAlignment(Fish p)
{
	double squaredDistance = DistanceSquared(p);
	return (squaredDistance < DISTANCE_MAX_SQUARED && squaredDistance > DISTANCE_MIN_SQUARED);
}

double Fish::DistanceFromWall(double wallXMin, double wallYMin, double wallXMax, double wallYMax)
{
	double min = MIN(pos.x - wallXMin, pos.y - wallYMin);
	min = MIN(min, wallXMax - pos.x);
	min = MIN(min, wallYMax - pos.y);
	return min;
}

void Fish::NormalizeVelocity()
{
	double length = sqrt(velocity.x*velocity.x + velocity.y*velocity.y);
	velocity.x /= length;
	velocity.y /= length;
}

bool Fish::AvoidWall(double wallXMin, double wallYMin, double wallXMax, double wallYMax)
{
	if (pos.x < wallXMin) 
	{
		pos.x = wallXMin;
	}
	else if (pos.y < wallYMin)
	{
		pos.y = wallYMin;
	}
	else if (pos.x > wallXMax)
	{
		pos.x = wallXMax;
	}
	else if (pos.y > wallYMax)
	{
		pos.y = wallYMax;
	}

	double distance = DistanceFromWall(wallXMin, wallYMin, wallXMax, wallYMax);

	if (distance < DISTANCE_MIN)
	{
		if ((int)distance == (int)(pos.x - wallXMin))
		{
			velocity.x += 0.3;
		}
		else if ((int)distance == (int)(pos.y - wallYMin))
		{
			velocity.y += 0.3;
		}
		else if ((int)distance == (int)(wallXMax - pos.x))
		{
			velocity.x -= 0.3;
		}
		else if ((int)distance == (int)(wallYMax - pos.y))
		{
			velocity.y -= 0.3;
		}
		NormalizeVelocity();
		return true;
	}
	return false;
}

bool Fish::AvoidObstacle(std::list<AreaToAvoid> obstacles)
{
	if (obstacles.size() != 0)
	{
		//Recherche de l'obstacle le plus proche
		auto it = obstacles.begin();
		auto end = obstacles.end();
		double squaredDistance = DistanceSquared(*it);
		AreaToAvoid nearestObstacle = *it;
		while (it != end)
		{
			if (DistanceSquared(*it) < squaredDistance)
			{
				nearestObstacle = *it;
				squaredDistance = DistanceSquared(*it);
			}

			if (squaredDistance < (nearestObstacle.GetRadius()*nearestObstacle.GetRadius()))
			{
				//Si collision calcul du vecteur d'Úvitement
				double distance = sqrt(squaredDistance);
				double diffX = (nearestObstacle.pos.x - pos.x) / distance;
				double diffY = (nearestObstacle.pos.y - pos.y) / distance;
				velocity.x = velocity.x - diffX / 2;
				velocity.y = velocity.y - diffY / 2;
				NormalizeVelocity();
				return true;
			}
		}
	}
	return false;
}


bool Fish::AvoidFish(std::vector<std::shared_ptr<Fish>> &fishes)
{
	//recherche poisson le plus proche
	Fish* f= fishes.at(0).get();
	if (f == this)
		f = fishes.at(1).get();

	double distanceSquared = DistanceSquared(*f);
	for (int i = 0; i < fishes.size(); i++)
	{
		Fish* testing = fishes.at(i).get();
		bool isMe = testing == this;
		if (DistanceSquared(*fishes.at(i).get()) < distanceSquared && !(fishes.at(i).get() == this))
		{
			f = fishes.at(i).get();
			distanceSquared = DistanceSquared(*f);
		}
	}

	//Evitement
	if (distanceSquared < DISTANCE_MIN_SQUARED)
	{
		double distance = sqrt(distanceSquared);
		double diffX = (f->pos.x - pos.x) / distance;
		double diffY = (f->pos.y - pos.y) / distance;
		velocity.x = velocity.x - diffX;
		velocity.y = velocity.y - diffY;
		NormalizeVelocity();
		return true;
	}
	return false;
}

void Fish::CalculateAverageDirection(std::vector<std::shared_ptr<Fish>> fishes)
{
	double totalVelocityX = 0;
	double totalVelocityY = 0;
	int totalNb = 0;
	for (int i = 0; i < fishes.size(); i++)
	{
		Fish f = *fishes.at(i).get();
		if (teamId != f.teamId)
			continue;
		if (InAlignment(f))
		{
			totalVelocityX += f.velocity.x;
			totalVelocityY += f.velocity.y;
			totalNb++;
		}
	}
	if (totalNb >= 1)
	{
		velocity.x = (totalVelocityX / totalNb + velocity.x) / 2;
		velocity.y = (totalVelocityY / totalNb + velocity.y) / 2;
		NormalizeVelocity();
	}

}

void Fish::Influenced(Guru *_guru)
{
	if (isInInfluenceRadiusOfHisGuru())
		return;
	if(guru!=nullptr)guru->influenceRadius -= 1;
	guru = _guru;
	color = guru->color;
	teamId = guru->teamId;
	guru->influenceRadius += 1;
}


bool Fish::isInInfluenceRadiusOfHisGuru()
{
	if (guru == nullptr)
		return false;

	double distance = Distance((Element*)guru);
	if (distance <= guru->influenceRadius)
	{
		return true;
	}
	else return false;
}