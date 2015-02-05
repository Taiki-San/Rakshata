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

@implementation RakSRContentManager

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		if(![self initData:&project :&cacheList :&activatedList :&filteredToSorted :&nbElemFull :&nbElemActivated])
			return nil;
		
		//Okay, we have all our data, we can register for updates
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		
		_sharedReference = [NSMutableArray array];
		
		for(uint i = 0; i < nbElemActivated; i++)
			[_sharedReference addObject:@(i)];
	}
	
	return self;
}

- (BOOL) initData : (PROJECT_DATA **) _project : (uint **) _cacheList : (BOOL **) _activatedList : (uint **) _filteredToSorted : (uint *) _nbElemFull : (uint *) _nbElemActivated
{
	//We get the full list
	*_project = getCopyCache(SORT_NAME, _nbElemFull);
	if(*_project == NULL)
		return NO;
	
	//We get the filtered list
	uint * filtered = getFilteredProject(_nbElemActivated);
	if(filtered == NULL)
	{
		freeProjectData(*_project);
		return NO;
	}
	
	//We alloc memory for everything
	*_cacheList = malloc(*_nbElemFull * sizeof(uint));
	*_activatedList = calloc(*_nbElemFull, sizeof(BOOL));
	*_filteredToSorted = malloc(*_nbElemActivated * sizeof(uint));
	
	if(*_cacheList == NULL || *_activatedList == NULL || *_filteredToSorted == NULL)
	{
		freeProjectData(*_project);
		free(filtered);
		free(*_cacheList);
		free(*_activatedList);
		free(*_filteredToSorted);
		return NO;
	}
	
	//We look for the highest value
	uint highestValue = *_nbElemFull;
	for(uint i = 0; i < *_nbElemFull; i++)
	{
		if((*_project)[i].cacheDBID > highestValue)
			highestValue = (*_project)[i].cacheDBID;
	}
	
	if(highestValue == UINT_MAX)	//We would get an overflow
	{
		freeProjectData(*_project);
		free(filtered);
		free(*_cacheList);
		free(*_activatedList);
		free(*_filteredToSorted);
		return NO;
	}
	
	//We create a buffer large enough to fit every data at the index of its value
	const uint invalidValue = UINT_MAX;
	uint * collector = malloc(++highestValue * sizeof(uint));
	if(collector == NULL)
	{
		freeProjectData(*_project);
		free(filtered);
		free(*_cacheList);
		free(*_activatedList);
		free(*_filteredToSorted);
		return NO;
	}
	memset(collector, invalidValue, highestValue * sizeof(uint));
	
	//We store the item position
	for(uint i = 0; i < *_nbElemFull; i++)
		collector[(*_project)[i].cacheDBID] = i;
	
	//We compact the large dataset in the reduced final structure
	uint orderedToSorted[*_nbElemFull];
	memset(orderedToSorted, invalidValue, *_nbElemFull * sizeof(uint));
	
	for(uint i = 0, pos = 0; i < highestValue; i++)
	{
		if(collector[i] != invalidValue)
		{
			(*_cacheList)[pos] = i;
			orderedToSorted[collector[i]] = pos++;
		}
	}
	
	free(collector);
	
	if(*_nbElemFull == *_nbElemActivated)
	{
		memset(*_activatedList, 1, *_nbElemFull);
		for(uint i = 0; i < *_nbElemActivated; i++)
			(*_filteredToSorted)[i] = i;
	}
	else
	{
		for(uint i = 0, filteredPos = 0; i < *_nbElemFull && filteredPos < *_nbElemActivated; i++)
		{
			if(filtered[filteredPos] == (*_cacheList)[i])
			{
				(*_activatedList)[orderedToSorted[i]] = YES;
			}
		}
		
		for(uint i = 0, filteredPos = 0; i < *_nbElemFull && filteredPos < *_nbElemActivated; i++)
		{
			if((*_activatedList)[i])
				(*_filteredToSorted)[filteredPos++] = i;
		}
	}
	
	free(filtered);
	return YES;
}

- (void) initViews
{
	previousFrame = _controlView.bounds;
	
	_grid = [[RakGridView alloc] initWithFrame : previousFrame : self];
	if(_grid != nil)
	{
		[_controlView addSubview : _grid.contentView];
	}
	
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
		
	return &(project[filteredToSorted[index]]);
}

@end
