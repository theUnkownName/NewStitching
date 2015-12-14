#include "GridCell.h"

 ////////////////////////////
 ////////////////////////////
 /////////CELL///////////////
 ////////////////////////////
 ////////////////////////////

//GridCell::GridCell(void)
//{
//	c_height = 20;
//	c_width = 20;
//	m_gridproperty = FREE;
//	bestError = NULL;
//}

GridCell::GridCell(int i, int j)
{
	m_gridproperty = FREE;
	//bestError = NULL;	
	m_cellX = i;
	m_cellY = j;
	c_height = 20;
	c_width = 20;
	c_size = c_width + c_height;
	std::tie(c_centerX, c_centerY) = retrieveXY(i,j, c_height, c_width);
	cellUnchecked = true;
	numberOfPatchesTried = 0;
}

std::pair<int, int> GridCell::retrieveXY(int x, int y, int c_height, int c_width)											//Retrieve the x and ycoordinates from the i and j identifier of the grid
{
	switch (x)	{	
	case(0):x = 10;	break;
	case(1):x = 30; break;
	case(2):x = 50;	break;
	default:x= 0; break;
	}

	switch (y)	{
	case(0):y = 10;break;
	case(1):y = 30;break;
	case(2):y = 50;break;
	default:y = 0;break;
	}

	return std::make_pair(x, y);
}

bool GridCell::isFree()
{ 
	if (m_gridproperty == FREE)
		return true;
	return false;
}

 void GridCell::updateCell(Patch* p)	//Update cell when a patch is in that place
 {
	 m_gridproperty = OCCUPIED;
	 m_patch = p;
	 p->available = false;
	 p->becomesTarget();
 }
 


 void GridCell::updateSideVertices(Patch* p)								//Should i keep this?
 {
	p->m_rightside_vertices[0] = p->m_vertices[0];
	p->m_rightside_vertices[1] = p->m_vertices[1];
	p->m_topside_vertices[0] = p->m_vertices[1];
	p->m_topside_vertices[1] = p->m_vertices[2];
	p->m_leftside_vertices[0] = p->m_vertices[2];
	p->m_leftside_vertices[1] = p->m_vertices[3];
	p->m_bottomside_vertices[0] = p->m_vertices[3];
	p->m_bottomside_vertices[1] = p->m_vertices[0];
			
 }

 ////////////////////////////
 ////////////////////////////
 /////////GRID///////////////
 ////////////////////////////
 ////////////////////////////

 Grid::Grid(int width, int height)
 {
	centerCellX = floor(width/2);
	centerCellY = floor(height/2);
	std::tie(centerOfGridX, centerOfGridY) = retrieveXY(centerCellX, centerCellY);
	g_width = width;
	g_height = height;
	m_grid.resize(g_width);
	for (int i = 0; i < g_width; i++)	{
		m_grid[i].resize(g_height);
    }

	//Fill the grid with empty cells
	for (int i = 0; i < g_width; i++){
		for (int j = 0; j < g_height; j++)
		{
			GridCell* cell = new GridCell(i, j);
			m_grid[i][j] = cell;
		}
	}
 }

void Grid::updateGrid(Patch* p)
{
	m_grid[centerCellX][centerCellY]->updateCell(p);			//Update the cell of the grid with the new patch
	p->getSideVertices(p->m_vertices, centerOfGridX, centerOfGridY);
}

void Grid::updatePossibleCell(int i_grid, int j_grid, int centerX, int centerY, Patch* patch)
{
	m_grid[i_grid][j_grid]->m_patch = patch;														//Set the patch to a particular cell
	m_grid[i_grid][j_grid]->c_centerX = centerX;
	m_grid[i_grid][j_grid]->c_centerY = centerY;
	
	m_grid[i_grid][j_grid]->numberOfPatchesTried++;
	if (m_grid[i_grid][j_grid]->numberOfPatchesTried == 8)				//Change to false when all patches have been in that cell
		m_grid[i_grid][j_grid]->cellUnchecked = false;
}
 
std::pair<int, int> Grid::retrieveXY(int x, int y)			//Retrieve the centerx and centery coordinates from the i and j identifier of the grid
{
	/*switch (x)	{	
	case(0):x = 10;	break;
	case(1):x = 30; break;
	case(2):x = 50;	break;
	default:x= 0; break;
	}

	switch (y)	{
	case(0):y = 10;break;
	case(1):y = 30;break;
	case(2):y = 50;break;
	default:y = 0;break;
	}*/

	switch (x)	{	
	case(0):x = 10;	break;
	case(1):x = 30; break;
	case(2):x = 50;	break;
	default:x= 0; break;
	}

	switch (y)	{
	case(0):y = 10;break;
	case(1):y = 30;break;
	case(2):y = 50;break;
	default:y = 0;break;
	}


	return std::make_pair(x, y);
}

void Grid::rotate4Times(Patch* target, Patch* patch, int i_grid, int j_grid, OgreBites::ParamsPanel* mDetailsPanel, Ogre::SceneManager* mSceneMgr, int patchId, Ogre::Root* mRoot, int centerX, int centerY)
{
	int side;
	for (int rotationNumber = 0; rotationNumber < 4; rotationNumber++)
	{
		for (std::size_t neighbor = 0; neighbor < m_neighbours.size(); neighbor++)
		{
			
			target = std::get<2>(m_neighbours[neighbor]);																//Get the target from the neighbours vector
			patch->patch_side = std::get<0>(m_neighbours[neighbor]);
			
			if (rotationNumber == 0){																					//If there is no rotation yet, calculate error in first position
				side = patch->patch_side;
				patch->computeError(target, pSide[neighbor], tSide[neighbor], mDetailsPanel, patch->patch_side, patch, m_grid[i_grid][j_grid], patchId);	  //patchSide and targetSide for each neighbor
			}else
			{																									//Start rotation	
				if(neighbor == 0)																						//Rotate the patch only once for all the neighbours
				{	
					patch->rotatePatch(mSceneMgr, centerX, centerY);								//Rotate patch (Create new vertices after 90 degres rotation)
					mRoot->renderOneFrame();
					//Sleep(70);
				}
				side++;																					//Change the side, for each rotation
				if(side > 3){																			//If the side is BOTTOM (3) go to the RIGHT side that is zero
					side = RIGHT;}
				patch->patch_side = patch->getSideFromInt(side);
				patch->computeError(target, pSide[neighbor], tSide[neighbor], mDetailsPanel, patch->patch_side, patch, m_grid[i_grid][j_grid], patchId);			
			}
		}								
	}		
	patch->rotatePatch(mSceneMgr, centerX, centerY);												//Return to original position
}

void Grid::transverseGrid(Patch* patch, Patch* target, Ogre::SceneManager* mSceneMgr, Ogre::Root* mRoot, OgreBites::ParamsPanel* mDetailsPanel, int patchId, int numberOfCells)
{
	GridCell* cell;
	int centerX; //Center of the patch in X
	int centerY; //Center of the patch in y
	bestErrorOfPatch bestFitOverall;
	int z_position = 0;
	int patchCounter = patchId;

	updateGrid(target);																//Set the target into the cell and mark it as occupied																
	for (int i_grid = 0; i_grid < g_width; i_grid++){																//Go through all the grid
		for (int j_grid = 0; j_grid< g_height; j_grid++){
			if (stitchingIsPossible(i_grid,j_grid))																	//If that cell is free and its neighbours are occupied is a possible position
			{		
				if(m_grid[i_grid][j_grid]->cellUnchecked == true)													//If error hasnt been calculated for this cell
				{
					getNeighbours(i_grid, j_grid);																		//Get a vector of neighbours cells where stitching is possible
					std::tie(centerX, centerY) = retrieveXY(i_grid,j_grid);												//Get x and y coordinates of the patch according to the cell 	
					updatePossibleCell(i_grid, j_grid, centerX, centerY, patch);
					for (std::size_t translation_Number = 0; translation_Number < 3; translation_Number++)				//Translate in "Z" three times
					{ 
						z_position = getPositionInZ(translation_Number);		
						patch->translatePatch(centerX, centerY, z_position, mSceneMgr, mRoot);									//Translates the patch to the correct cell				
 						rotate4Times(target, patch, i_grid, j_grid, mDetailsPanel, mSceneMgr, patchId, mRoot, centerX, centerY);		//Rotate the patch 4 times and calculate the error
					}
				}
			}														
		}
	}
	_bestFitOfPatch.push_back(bestFitOfPatch(patch));					
	patch->translatePatchToOrigin(mSceneMgr, patchId);
	mRoot->renderOneFrame();													
}

bestErrorOfPatch Grid::bestFitOfPatch(Patch* p)
{
	int i = 0;
	while (p->m_curError[i].cell->m_gridproperty == OCCUPIED){i++;}

	PatchSide side = p->m_curError[i].side;
	double error = p->m_curError[i].error;
	GridCell* c = p->m_curError[i].cell;
	std::vector<Ogre::Vector3> bestError_vertices = p->m_curError[i].vertices;
	int patchId = p->m_curError[i].patchId;
	Ogre::Quaternion orientation = p->m_curError[i].orientation;
	
	for (std::size_t i = 0; i < p->m_curError.size(); i++){
		double minTmp = p->m_curError[i].error;
		if ( minTmp < error && p->m_curError[i].cell->m_gridproperty != OCCUPIED)									//If new error is lower than the last
		{
			error = minTmp;
			side = p->m_curError[i].side;
			c = p->m_curError[i].cell;
			bestError_vertices = p->m_curError[i].vertices;
			patchId = p->m_curError[i].patchId;
			orientation = p->m_curError[i].orientation;
		}
	}
	bestErrorOfPatch bestFitOfPatch;
	bestFitOfPatch.cell = c;
	bestFitOfPatch.error = error;
	bestFitOfPatch.side = side;
	bestFitOfPatch.vertices = bestError_vertices;
	bestFitOfPatch.patchId = patchId;										
	bestFitOfPatch.orientation = orientation;
	return bestFitOfPatch;
}


bestErrorOfPatch Grid::bestFitInGrid(std::vector<bestErrorOfPatch> bfit)
{
	PatchSide side;
	double error; 
	std::vector<Ogre::Vector3> bestError_vertices;
	GridCell* c;
	int patchId;
	Ogre::Quaternion orientation;
	
	int i = 0;
	while (bfit[i].cell->m_gridproperty == OCCUPIED){i++;}

	c = bfit[i].cell;
	side = bfit[i].side;
	error = bfit[i].error;
	bestError_vertices = bfit[i].vertices;
	patchId = bfit[i].patchId;
	orientation = bfit[i].orientation;
	
	for (std::size_t i = 0; i < bfit.size(); i++){
		double minTmp = bfit[i].error;
		if ( minTmp < error && bfit[i].cell->m_gridproperty != OCCUPIED)									//If new error is lower than the last
		{
			c = bfit[i].cell;
			side = bfit[i].side;
			error = bfit[i].error;
			bestError_vertices = bfit[i].vertices;
			patchId = bfit[i].patchId;
			orientation = bfit[i].orientation;		
		}
	}

	bestErrorOfPatch bestFit;
	bestFit.cell = c;
	bestFit.error = error;
	bestFit.side = side;
	bestFit.vertices = bestError_vertices;
	bestFit.patchId = patchId;
	bestFit.orientation = orientation;
	//bestFit.p = p;
	//bestfit.push_back(bestFit);
	bestErrors.clear();														//Delete the error for that iteration, and leave it clean for the next one
	return bestFit;
}

void Grid::getNeighbours(int i, int j)		
{
	m_neighbours.clear();
	pSide.clear();
	tSide.clear();
	 if (i+1 < g_width  && !m_grid[i+1][j]->isFree()){
		 pSide.push_back(RIGHT);
		 tSide.push_back(LEFT);
		 m_neighbours.push_back(std::make_tuple(RIGHT, LEFT,m_grid[i+1][j]->m_patch));			//Save in the neighbrous vector the side of the patch, the side of the target and the target itself
	 }
	 if (j+1 < g_height && !m_grid[i][j+1]->isFree()){
		 pSide.push_back(TOP);
		 tSide.push_back(BOTTOM);
		 m_neighbours.push_back(std::make_tuple(TOP, BOTTOM,m_grid[i][j+1]->m_patch));
	 }
	 if (i-1 >= 0 && !m_grid[i-1][j]->isFree()){
		 pSide.push_back(LEFT);
		 tSide.push_back(RIGHT);
		 m_neighbours.push_back(std::make_tuple(LEFT, RIGHT ,m_grid[i-1][j]->m_patch));
	 }
	 if (j-1 >= 0 && !m_grid[i][j-1]->isFree()){
		 pSide.push_back(BOTTOM);
		 tSide.push_back(TOP);
		 m_neighbours.push_back(std::make_tuple(BOTTOM, TOP,m_grid[i][j-1]->m_patch));
	 }
}

int Grid::getPositionInZ(std::size_t  translationNumber)
{
	int z_position;

	if (translationNumber == 1)																	//modify the "z" position 
		z_position = -1;
	else if (translationNumber == 2)
		z_position = 1;
	else z_position = 0;

	return z_position;
}

bool Grid::stitchingIsPossible(int i, int j)
{
	if(m_grid[i][j]->isFree())
	{
		if( i+1 < g_width  && !m_grid[i+1][j]->isFree()) //Check if its neighbours are occupied and if it is not outised of the grid
			return true;
		if (j+1 < g_height && !m_grid[i][j+1]->isFree())
			return true;
		if (i-1 >= 0 && !m_grid[i-1][j]->isFree())
			return true;
		if (j-1 >= 0 && !m_grid[i][j-1]->isFree()) 
			return true;
	}
	return false;
}

