#pragma once
#ifndef __PATCH_H
#define __PATCH_H

#include <vector>
#include <OGRE\Ogre.h>
#include <OgreLogManager.h>
#include <SdkTrays.h>

typedef enum PatchSide 
{
	RIGHT = 0,
	TOP = 1,
	LEFT = 2,
	BOTTOM = 3
} PatchSide;

class GridCell;
class Grid;

struct bestErrorOfPatch
{
//	Patch *p;
	PatchSide side;
	double error;
	GridCell* cell;
	std::vector<Ogre::Vector3> vertices;
	int patchId;
	Ogre::Quaternion orientation; 
};

class Patch 
{
public:
	//Constructors
	Patch(bool target, int numberOfPatch, Ogre::SceneManager*);
	Patch(bool, Ogre::Entity*); //isTarget, vertices of the patch
	
	//Destructor
	~Patch(void);

	//Others
	std::pair<std::vector<Ogre::Vector3>, std::vector<int>> getMeshInformation(const Ogre::MeshPtr* meshPtr,
									  size_t &vertex_count,
									  size_t &index_count, 
									  unsigned* &indices,
									  const Ogre::Vector3 &position,
									  const Ogre::Quaternion &orient,
									  const Ogre::Vector3 &scale);
	void getSideVertices(std::vector<Ogre::Vector3>, int, int); //sortVertices
	void computeError(Patch* target, int s1, int s2, OgreBites::ParamsPanel* mDetailsPanel, PatchSide, Patch* patch, GridCell* cell, int);									//s from p
	void becomesTarget();																		//Change when the patch becomes a target
	void rotatePatch(Ogre::SceneManager* mSceneMgr, int, int);
	void translatePatch(int, int, int, Ogre::SceneManager* mSceneMgr, Ogre::Root* mRoot);
	void translatePatchToOrigin(Ogre::SceneManager* mSceneMgr, int);
	void translatePatchDeffinitve(Ogre::SceneManager* mSceneMgr, bestErrorOfPatch, int, int);
	PatchSide getSideFromInt(int);
	std::pair<std::vector<Ogre::Vector3>, std::vector<Ogre::Vector3>> Patch::choseSide(Patch*, int pSide, int tSide);						//picks patch and template side 
	void removeFromErrorList(GridCell* cell);
	void deleteRepeatedVertices();

	std::vector<Ogre::Vector3> m_vertices;														//Four vertices per patch
	std::vector<int> m_indices;
	std::vector<Ogre::Vector3> m_leftside_vertices;												
	std::vector<Ogre::Vector3> m_rightside_vertices;											
	std::vector<Ogre::Vector3> m_bottomside_vertices;											
	std::vector<Ogre::Vector3> m_topside_vertices;	
	std::vector<int> m_leftSide_indices;
	std::vector<int> m_rightSide_indices;
	std::vector<int> m_bottomSide_indices;
	std::vector<int> m_topSide_indices;
	bool isTarget;
	bool available;
	Ogre::Real p_centerX, p_centerY;
	Ogre::Real p_width, p_height;
	std::vector<bestErrorOfPatch> _bestFitOfPatch;
	std::vector<bestErrorOfPatch> m_curError;
	
  PatchSide patch_side;
  Ogre::Quaternion m_orientation;

  Ogre::String nodeName;
  Ogre::String entName;
  Ogre::String mesh;
  Ogre::Entity* ent;
  Ogre::SceneNode* node;

  bestErrorOfPatch bestFit;

};

#endif