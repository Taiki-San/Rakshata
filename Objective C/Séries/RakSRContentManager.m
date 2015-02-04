/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 ********************************************************************************************/

#include "db.h"

@implementation RakSRContentManager

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		//We get the full list
		project = getCopyCache(SORT_NAME, &nbElemFull);
		if(project == NULL)
			return nil;
		
		//We get the filtered list
		uint * filtered = getFilteredProject(&nbElemActivated);
		if(filtered == NULL)
		{
			freeProjectData(project);
			return nil;
		}
		
		//We alloc memory for everything
		cacheList = malloc(nbElemFull * sizeof(uint));
		activatedList = calloc(nbElemFull, sizeof(BOOL));
		sortedToOrdered = malloc(nbElemFull * sizeof(uint));
		orderedToSorted = malloc(nbElemActivated * sizeof(uint));
		
		if(cacheList == NULL || activatedList == NULL || sortedToOrdered == NULL || orderedToSorted == NULL)
		{
			freeProjectData(project);
			free(filtered);
			free(cacheList);
			free(activatedList);
			free(sortedToOrdered);
			free(orderedToSorted);
			return nil;
		}
		
		//We look for the highest value
		uint highestValue = nbElemFull;
		for(uint i = 0; i < nbElemFull; i++)
		{
			if(project[i].cacheDBID > highestValue)
				highestValue = project[i].cacheDBID;
		}
		
		if(highestValue == UINT_MAX)	//We would get an overflow
		{
			freeProjectData(project);
			free(filtered);
			free(cacheList);
			free(activatedList);
			free(sortedToOrdered);
			free(orderedToSorted);
			return nil;
		}
		
		//We create a buffer large enough to fit every data at the index of its value
		const uint invalidValue = UINT_MAX;
		uint * collector = malloc(++highestValue * sizeof(uint));
		if(collector == NULL)
		{
			freeProjectData(project);
			free(filtered);
			free(cacheList);
			free(activatedList);
			free(sortedToOrdered);
			free(orderedToSorted);
			return nil;
		}
		memset(collector, invalidValue, highestValue * sizeof(uint));

		//We store the item position
		for(uint i = 0; i < nbElemFull; i++)
			collector[project[i].cacheDBID] = i;
		
		//We compact the large dataset in the reduced final structure
		for(uint i = 0, pos = 0; i < highestValue; i++)
		{
			if(collector[i] != invalidValue)
			{
				cacheList[pos] = i;
				sortedToOrdered[pos++] = collector[i];
			}
		}
		
		free(collector);
		
		if(nbElemFull == nbElemActivated)
		{
			memset(activatedList, 1, nbElemFull * sizeof(uint));
			for(uint i = 0; i < nbElemActivated; i++)
				orderedToSorted[i] = i;
		}
		else
		{
			for(uint i = 0, filteredPos = 0; i < nbElemFull && filteredPos < nbElemActivated; i++)
			{
				if(filtered[filteredPos] == cacheList[i])
				{
					activatedList[i] = YES;
					orderedToSorted[filteredPos++] = i;
				}
			}
		}
		
		free(filtered);
	}
	
	return self;
}

- (void) initViews
{
	previousFrame = _controlView.bounds;
	
	_grid = [[RakGridView alloc] initWithFrame : previousFrame : self];
	if(_grid != nil)
		[_controlView addSubview : _grid.contentView];
	
	_grid.hidden = _activeView != SR_CELLTYPE_GRID;
	_initialized = YES;
}

#pragma mark - Property

- (BOOL) hidden
{
	return _hidden;
}

- (void) setHidden : (BOOL) hidden
{
	_hidden = hidden;
	
	if(_activeView == SR_CELLTYPE_GRID)
		_grid.hidden = hidden;
	
	else if(_activeView == SR_CELLTYPE_REPO)
	{
		
	}
	
	else if(_activeView == SR_CELLTYPE_LIST)
	{
		
	}
}

- (void) setFrame : (NSRect) frame
{
	previousFrame = frame;
	
	if(_activeView == SR_CELLTYPE_GRID)
		[_grid setFrame:frame];
	
	else if(_activeView == SR_CELLTYPE_REPO)
	{
		
	}
	
	else if(_activeView == SR_CELLTYPE_LIST)
	{
		
	}
}

- (void) resizeAnimation : (NSRect) frame
{
	previousFrame = frame;
	
	if(_activeView == SR_CELLTYPE_GRID)
		[_grid resizeAnimation:frame];
	
	else if(_activeView == SR_CELLTYPE_REPO)
	{
		
	}
	
	else if(_activeView == SR_CELLTYPE_LIST)
	{
		
	}
}

- (byte) activeView
{
	return _activeView;
}

- (void) setActiveView : (byte) activeView
{
	[self setActiveView:activeView :YES];
}

- (void) setActiveView : (byte) activeView : (BOOL) animated
{
	if(!_initialized || activeView == _activeView)
		return;
	
	//We resize the new view
	if(activeView == SR_CELLTYPE_GRID)
	{
		[_grid setFrame:previousFrame];
		if(animated)
			[_grid contentView].alphaValue = 0;
	}
	else if(activeView == SR_CELLTYPE_REPO)
	{}
	else if(activeView == SR_CELLTYPE_LIST)
	{}

	if(animated)
		[NSAnimationContext beginGrouping];
	
	//We hide the previous view
	if(_activeView == SR_CELLTYPE_GRID)
	{
		if(animated)
			[_grid contentView].animator.alphaValue = 0;
		else
			[_grid setHidden:YES];
	}
	else if(_activeView == SR_CELLTYPE_REPO)
	{}
	else if(_activeView == SR_CELLTYPE_LIST)
	{}
	
	__block byte oldView = _activeView;
	_activeView = activeView;

	//We reveal the new view
	if(activeView == SR_CELLTYPE_GRID)
	{
		if(animated)
			[_grid contentView].animator.alphaValue = 1;
		else
			[_grid setHidden:NO];
	}
	else if(activeView == SR_CELLTYPE_REPO)
	{}
	else if(activeView == SR_CELLTYPE_LIST)
	{}
	
	if(animated)
	{
		[[NSAnimationContext currentContext] setCompletionHandler:^{
			if(oldView == SR_CELLTYPE_GRID)
			{
				_grid.hidden = YES;
				[_grid contentView].alphaValue = 1;
			}
			else if(oldView == SR_CELLTYPE_REPO)
			{}
			else if(oldView == SR_CELLTYPE_LIST)
			{}
		}];
		[NSAnimationContext endGrouping];
	}
}

#pragma mark - Interface

- (uint) nbElement
{
	return nbElemActivated;
}

- (PROJECT_DATA *) getDataAtIndex : (uint) index
{
	if(index >= nbElemActivated)
		return NULL;
		
	return &(project[orderedToSorted[index]]);
}

@end
