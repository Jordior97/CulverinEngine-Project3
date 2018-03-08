#pragma once
#include <list>
#include <vector>
#include "Geometry\AABB.h"
#include "Globals.h"
//#define Octree_MAX_ITEMS 2
//#define Octree_MIN_SIZE 10.0f

class GameObject;

struct OctreeLimits
{
	uint octreeMaxItems = 2;
	float octreeMinSize = 10.0f;
};

class OctreeNode
{
public:
	OctreeNode(const AABB& box);
	~OctreeNode();

	bool isLeaf() const;

	void Insert(GameObject* obj, OctreeLimits& limits);
	void Remove(GameObject* obj);

	void DebugDraw();

	void CreateChilds();

	//template<typename TYPE>
	int CollectIntersections(std::list<GameObject*>& nodes, const Frustum& frustum) const;

public:
	AABB box;
	std::list<GameObject*> objects;
	OctreeNode* parent = nullptr;
	OctreeNode* childs[8];

};

class Octree
{
public:
	Octree();
	~Octree();

	void Boundaries(AABB limits);
	void Clear(bool fullclear = true);
	void Remove(GameObject* mesh);
	void Insert(GameObject* obj);

	void DebugDraw();

	//template<typename TYPE>
	int CollectIntersections(std::list<GameObject*>& nodes, const Frustum& frustum) const;

public:
	
	OctreeLimits limits;

private:
	OctreeNode* root_node = nullptr;
};