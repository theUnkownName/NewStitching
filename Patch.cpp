#include "Patch.h"

Patch::Patch(bool target, int numberOfPatch, Ogre::SceneManager* mSceneMgr)
{
	isTarget = false;
	Ogre::Vector3 bbsize = Ogre::Vector3::ZERO; //Boundingbox size
	Ogre::Vector3 scale(0.5, 0.5, 0.5);
	Ogre::Quaternion rotation(Ogre::Degree(180), Ogre::Vector3::UNIT_Y); 
	Ogre::Quaternion rotation2(Ogre::Degree(180), Ogre::Vector3::UNIT_Z); 

	available = true;
	nodeName = "node" + Ogre::StringConverter::toString(numberOfPatch);
	entName = Ogre::StringConverter::toString(numberOfPatch);
	mesh = entName + ".mesh";
	ent = mSceneMgr->createEntity(entName,mesh);										//Create Entity
	node = mSceneMgr->getSceneNode("grid")->createChildSceneNode(nodeName);				//Create Node

	translatePatchToOrigin(mSceneMgr, numberOfPatch);

	node->attachObject(ent);
	node->scale(scale);
	node->rotate(rotation, Ogre::Node::TransformSpace::TS_LOCAL);
	node->rotate(rotation2, Ogre::Node::TransformSpace::TS_LOCAL);
	Ogre::AxisAlignedBox bb = ent->getBoundingBox();
	bbsize = bb.getSize();
	p_centerX = bbsize.x/2;
	p_centerY = bbsize.y/2;
	p_width = bbsize.x;
	p_height = bbsize.y;
}

Patch::Patch(bool target, Ogre::Entity* targetPatch)						//Target
{
	isTarget = true;
	Ogre::Vector3 bbsize = Ogre::Vector3::ZERO; //Boundingbox size
	std::vector<Ogre::Vector3> temporalList;
	bool flag = false;
	
	std::vector<Ogre::Vector3> verticesList;
	size_t vertex_count,index_count;
	unsigned* indices;
	std::vector<Ogre::Vector3> vertices;														//Four vertices per patch
	std::vector<int> indice;
	
	Ogre::Vector3 pos = targetPatch->getParentSceneNode()->getPosition();
	Ogre::Vector3 scale = targetPatch->getParentSceneNode()->getScale();
	Ogre::Quaternion orientation = targetPatch->getParentSceneNode()->getOrientation();
	std::tie(m_vertices, m_indices) = getMeshInformation(&targetPatch->getMesh(),vertex_count,index_count,indices, pos, orientation, scale);

	deleteRepeatedVertices();
	getSideVertices(m_vertices, 30, 30);
	m_orientation = orientation;
	
	Ogre::AxisAlignedBox bb = targetPatch->getBoundingBox();
	bbsize = bb.getSize();
	p_centerX = bbsize.x/2;
	p_centerY = bbsize.y/2;
	p_width = bbsize.x;
	p_height = bbsize.y;
}

void Patch::getSideVertices(std::vector<Ogre::Vector3> verticesTemplate, int centerX, int centerY)
{
	Ogre::Vector3 firstVertex;
	m_rightside_vertices.clear();
	m_topside_vertices.clear();
	m_leftside_vertices.clear();
	m_bottomside_vertices.clear();

	//Offset is the center poing plus an arbitrary value 
	double offsetX1 = centerX + 9.8; //To the right from the center point
	double offsetX2 = centerX - 9.8;	//To the left
	double offsetY1 = centerY + 9.8;  //To the top
	double offsetY2 = centerY - 9.8;  //To the bottom

	//Apply offset here
	for (std::size_t i = 0; i < verticesTemplate.size(); i++)
	{
	//	if (verticesTemplate[i].z > 1)
	//	{
			if (verticesTemplate[i].x > offsetX1 )													//Bigger than originX, will be right side
				m_rightside_vertices.push_back(verticesTemplate[i]);
			else if(verticesTemplate[i].x < offsetX2)
				m_leftside_vertices.push_back(verticesTemplate[i]);									//Smaller than OriginX, will be left side
			if(verticesTemplate[i].y > offsetY1)												
				m_topside_vertices.push_back(verticesTemplate[i]);									//Bigger than originY, will be top side
			else if (verticesTemplate[i].y < offsetY2)	
				m_bottomside_vertices.push_back(verticesTemplate[i]);								//Smaller than originY, will be bottom side
	//	}

	}
}

void Patch::removeFromErrorList(GridCell* cell)
{
	double error = m_curError[0].error;
	GridCell* c = m_curError[0].cell;
	for (std::size_t i = 0; i < m_curError.size(); i++){
		double minTmp = m_curError[i].error;
		if ( minTmp < error)									//If new error is lower than the last
		{
			if (m_curError[i].cell == cell)
				m_curError.erase(m_curError.begin() + 1);
		}
	}
}

void Patch::computeError(Patch* target, PatchSide _patchSide, PatchSide _targetSide, OgreBites::ParamsPanel* mDetailsPanel, Patch* patch, GridCell* cell, int patchId, Ogre::SceneManager* mSceneMgr, Ogre::Root* mRoot)
{
	double error = 0.0;
	std::vector<Ogre::Real> temperror;
	std::vector<Ogre::Vector3> sideP;
	std::vector<Ogre::Vector3> sideT;
	int counter = 0;

	double offset = p_centerX / 2; //TODO; make it automatically with the size of the Patches

	std::tie(sideP, sideT) = choseSide(target, _patchSide, _targetSide);
	for(std::size_t vertexPatch = 0; vertexPatch < sideP.size(); vertexPatch++)
	{
		for (std::size_t vertexTemplate = 0; vertexTemplate < sideT.size(); vertexTemplate++ )
		{
			Ogre::Real distance = sideP[vertexPatch].distance(sideT[vertexTemplate]);
			temperror.push_back(Ogre::Math::Sqrt(distance * distance)); 
		}
	}
	for (std::size_t i = 0; i < temperror.size(); i++)
	{
		if ( std::abs((double)temperror[i]) <= (20/6))
		{
			error += temperror[i];	
			counter++;
		}
	}

	error = error/counter; //RMSE
	bestErrorOfPatch current_error;
	current_error.error = error;
	current_error.vertices = m_vertices;
	current_error.cell = cell;
	current_error.patchId = patchId;
	current_error.orientation = m_orientation;
	m_curError.push_back(current_error);

	mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(error).c_str());	
}

PatchSide Patch::getSideFromInt(int s)
{
	PatchSide side;

	switch(s)
	{
	case(0):
		side = RIGHT;
		break;
	case(1):
		side = TOP;
		break;
	case(2):
		side = LEFT;
		break;
	case(3):
		side = BOTTOM;
		break;
	}
	return side;
}

std::pair<std::vector<Ogre::Vector3>,std::vector<Ogre::Vector3>> Patch::choseSide(Patch* target, PatchSide pSide, PatchSide tSide)			//Retrieve the x and y coordinates from the i and j identifier of the grid
{
	std::vector<Ogre::Vector3> sideP;
	std::vector<Ogre::Vector3> sideT;
	double offset = p_centerX + p_centerX/2; //30
	
	switch (pSide)	{	
	case(RIGHT):sideP = m_rightside_vertices;	break;
	case(TOP):sideP = m_topside_vertices;		break;
	case(LEFT):sideP = m_leftside_vertices;		break;
	case(BOTTOM):sideP = m_bottomside_vertices;	break;
	}

	switch (tSide)	{
	case(RIGHT):sideT = target->m_rightside_vertices;	break;
	case(TOP):sideT = target->m_topside_vertices;		break;
	case(LEFT):sideT = target->m_leftside_vertices;	break;
	case(BOTTOM):sideT = target->m_bottomside_vertices;	break;
	}


	/*switch (pSide)	{	
		case(RIGHT):
			for (int i = 0; i <  m_rightside_vertices.size(); i++)
			{
				if(m_rightside_vertices[i].x > offset)	
					sideP.push_back(m_rightside_vertices[i]);
			}break;
		case(TOP):for (int i = 0; i <  m_topside_vertices.size(); i++)
			{
				if(m_topside_vertices[i].y > offset)	
					sideP.push_back(m_topside_vertices[i]);
			}break;
		case(LEFT):
			for (int i = 0; i <  m_leftside_vertices.size(); i++)
			{
				if(m_leftside_vertices[i].x < offset )	
					sideP.push_back(m_leftside_vertices[i]);
			}break;
		case(BOTTOM):
			for (int i = 0; i <  m_bottomside_vertices.size(); i++)
			{
				if(m_bottomside_vertices[i].y < offset)	
					sideP.push_back(m_bottomside_vertices[i]);
			}break;
		}

	switch (tSide)	{	
		case(RIGHT):
			for (int i = 0; i <  target->m_rightside_vertices.size(); i++)
			{
				if(target->m_rightside_vertices[i].x < offset + p_centerX)	
					sideT.push_back(target->m_rightside_vertices[i]);
			}break;
		case(TOP):for (int i = 0; i <  target->m_topside_vertices.size(); i++)
			{
				if(target->m_topside_vertices[i].y < offset + p_centerY)	
					sideT.push_back(target->m_topside_vertices[i]);
			}break;
		case(LEFT):
			for (int i = 0; i <  target->m_leftside_vertices.size(); i++)
			{
				if(target->m_leftside_vertices[i].x < offset - p_centerX)	
					sideT.push_back(target->m_leftside_vertices[i]);
			}break;
		case(BOTTOM):
			for (int i = 0; i <  target->m_bottomside_vertices.size(); i++)
			{
				if(target->m_bottomside_vertices[i].y < offset - p_centerY)	
					sideT.push_back(target->m_bottomside_vertices[i]);
			}break;
		}*/

	return std::make_pair(sideP, sideT);
}

void Patch::translatePatchToOrigin(Ogre::SceneManager* mSceneMgr, int patchId)
{
	switch (patchId)
	{
	case(0):
		mSceneMgr->getSceneNode(nodeName)->setPosition(0,-10,0);
		break;
	case(1):
		mSceneMgr->getSceneNode(nodeName)->setPosition(20,-10,0);
		break;
	case(2):
		mSceneMgr->getSceneNode(nodeName)->setPosition(40,-10,0);
		break;
	case(3):
		mSceneMgr->getSceneNode(nodeName)->setPosition(60,-10,0);
		break;
	case(4):
		mSceneMgr->getSceneNode(nodeName)->setPosition(80,-10,0);
		break;
	case(5):
		mSceneMgr->getSceneNode(nodeName)->setPosition(100,-10,0);
		break;
	case(6):
		mSceneMgr->getSceneNode(nodeName)->setPosition(120,-10,0);
		break;
	case(7):
		mSceneMgr->getSceneNode(nodeName)->setPosition(140,-10,0);
		break;
	default:
		break;
	}
}

void Patch::translatePatch(int centerX, int centerY, int z_position, Ogre::SceneManager* mSceneMgr,  Ogre::Root* mRoot)
{
	isTarget = false;													//Targets never are moved. If the patch is moving, is not a target
	mSceneMgr->getSceneNode(nodeName)->setPosition(centerX, centerY, z_position);
	mRoot->renderOneFrame();
	updateVertices(centerX, centerY, mSceneMgr);
}

void Patch::updateVertices(int centerX, int centerY, Ogre::SceneManager* mSceneMgr)
{
	Ogre::Vector3 originVertex;
	Ogre::Vector3 pos; 
	Ogre::Vector3 scale; 
	Ogre::Quaternion orientation; 
	size_t vertex_count,index_count;
	unsigned* indices;
	std::vector<Ogre::Vector3> vertices;														//Four vertices per patch
	std::vector<int> indice;
	m_vertices.clear();

	pos  = mSceneMgr->getSceneNode(nodeName)->getPosition();
	scale =  mSceneMgr->getSceneNode(nodeName)->getScale();
	orientation =  mSceneMgr->getSceneNode(nodeName)->getOrientation();
	std::tie(m_vertices, m_indices) = getMeshInformation(&mSceneMgr->getEntity(entName)->getMesh(),vertex_count,index_count,indices, pos, orientation, scale);
	deleteRepeatedVertices();
	getSideVertices(m_vertices, centerX, centerY);
	m_orientation = orientation;

}

void Patch::translatePatchDeffinitve(Ogre::SceneManager* mSceneMgr, bestErrorOfPatch bestFitOverall, int centerX, int centerY)
{
	isTarget = true;
	
	mSceneMgr->getSceneNode(nodeName)->setPosition(centerX, centerY, 0);
	mSceneMgr->getSceneNode(nodeName)->setOrientation(bestFitOverall.orientation);
	updateVertices(centerX, centerY, mSceneMgr);
}

void Patch::rotatePatch(Ogre::SceneManager* mSceneMgr, int centerX, int centerY)
{
	Ogre::Quaternion rotation(Ogre::Degree(-90), Ogre::Vector3::UNIT_Z); 
	Ogre::Matrix3 rotationM;
	rotation.ToRotationMatrix(rotationM);	
	mSceneMgr->getSceneNode(nodeName)->rotate(rotation, Ogre::Node::TransformSpace::TS_LOCAL);
	updateVertices(centerX, centerY, mSceneMgr);

}

void Patch::becomesTarget()
{
	isTarget = true;
}

void Patch::deleteRepeatedVertices()
{
	/////////////////////////////////////////
	//*eliminate duplicated vertices 
	//* returns 8 vertices, 4 per for the front face and 4 for the back face
	/////////////////////////////////////////

	std::vector<Ogre::Vector3> temporalList;
	std::vector<Ogre::Vector3> verticesList;
	bool flag = false;
	
	verticesList = m_vertices;
	temporalList.push_back(verticesList[0]);
	for (std::size_t i = 1; i < verticesList.size(); i++)
	{
		Ogre::Vector3 temp;
		temp = verticesList[i];													//start comparing from the first element
		for (int j = i-1; j >= 0; j--)
		{
			if (temp == verticesList[j])
			{
				flag = true;
			}
		}
		
		if (flag == false)
		{
			temporalList.push_back(temp);
		}
		flag = false;
	}

	m_vertices.clear();
	m_vertices = temporalList;
}

std::pair<std::vector<Ogre::Vector3>,std::vector<int>> Patch::getMeshInformation(const Ogre::MeshPtr* meshPtr,
									  size_t &vertex_count,
									  size_t &index_count, 
									  unsigned* &indices,
									  const Ogre::Vector3 &position = Ogre::Vector3::ZERO,
									  const Ogre::Quaternion &orient = Ogre::Quaternion::IDENTITY,
									  const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE)
{
	std::vector<Ogre::Vector3> verticesList;
	std::vector<int> indicesList;
  
	vertex_count = index_count = 0;
    bool added_shared = false;
    size_t current_offset = vertex_count;
    size_t shared_offset = vertex_count;
    size_t next_offset = vertex_count;
    size_t index_offset = index_count;
    size_t prev_vert = vertex_count;
    size_t prev_ind = index_count;
	Ogre::Mesh *mesh = meshPtr->getPointer();

	
    // Calculate how many vertices and indices we're going to need
    for(int i = 0;i < mesh->getNumSubMeshes();i++)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
 
        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if(!added_shared)
            {
                Ogre::VertexData* vertex_data = mesh->sharedVertexData;
                vertex_count += vertex_data->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            Ogre::VertexData* vertex_data = submesh->vertexData;
            vertex_count += vertex_data->vertexCount;
        }
 
        // Add the indices
        Ogre::IndexData* index_data = submesh->indexData;
        index_count += index_data->indexCount;
    }
 
    // Allocate space for the vertices and indices
   // vertices = new Ogre::Vector3[vertex_count];
    indices = new unsigned[index_count];
 
    added_shared = false;
 
    // Run through the submeshes again, adding the data into the arrays
    for(int i = 0;i < mesh->getNumSubMeshes();i++)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
 
        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
        if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }
 
            const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
            Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
            unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
            Ogre::Real* pReal;
 
            for(size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);
 
                Ogre::Vector3 pt;
 
                pt.x = (*pReal++);
                pt.y = (*pReal++);
                pt.z = (*pReal++);
 
                pt = (orient * (pt * scale)) + position;
				verticesList.push_back(Ogre::Vector3(pt.x, pt.y, pt.z));
            }
            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }
        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        unsigned short* pShort;
        unsigned int* pInt;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        if (use32bitindexes) pInt = static_cast<unsigned int*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        else pShort = static_cast<unsigned short*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
 
        for(size_t k = 0; k < numTris; ++k)
        {
            size_t offset = (submesh->useSharedVertices)?shared_offset:current_offset;
 
            unsigned int vindex = use32bitindexes? *pInt++ : *pShort++;
            indices[index_offset + 0] = vindex + offset;
            vindex = use32bitindexes? *pInt++ : *pShort++;
            indices[index_offset + 1] = vindex + offset;
            vindex = use32bitindexes? *pInt++ : *pShort++;
            indices[index_offset + 2] = vindex + offset;
 
            index_offset += 3;
        }
        ibuf->unlock();
        current_offset = next_offset;
	}

	//Create a list with all indices
	for (std::size_t i = 0; i < index_count; i++)
	{
		indicesList.push_back(indices[i]);
	}
	return std::make_pair(verticesList, indicesList);

}



void Patch::DestroyAllAttachedMovableObjects( Ogre::SceneNode* i_pSceneNode )
{

   // Destroy all the attached objects
   Ogre::SceneNode::ObjectIterator itObject = i_pSceneNode->getAttachedObjectIterator();

   while ( itObject.hasMoreElements() )
   {
      Ogre::MovableObject* pObject = static_cast<Ogre::MovableObject*>(itObject.getNext());
      i_pSceneNode->getCreator()->destroyMovableObject( pObject );
   }

   // Recurse to child SceneNodes
   Ogre::SceneNode::ChildNodeIterator itChild = i_pSceneNode->getChildIterator();

   while ( itChild.hasMoreElements() )
   {
      Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
      DestroyAllAttachedMovableObjects( pChildNode );
   }
}


