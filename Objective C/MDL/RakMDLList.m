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
 *********************************************************************************************/

enum
{
	MDLLIST_CELL_HEIGHT = ((11 + 1) * 2),
	MDLLIST_CELL_HEIGHT_MULTI = 40
};

@implementation RakMDLList

- (instancetype) init : (NSRect) frame : (RakMDLController *) _controller
{
	if(_controller == nil)
		return nil;
	
	self = [super init];
	
	if(self != nil)
	{
		controller = _controller;
		dragInProgress = NO;
		wasSerie = controller.isSerieMainThread;
		
		[self applyContext:frame : -1 : -1];
		[scrollView setHasHorizontalScroller:NO];
		[self enableDrop];
		
		controller.list = self;
	}
	
	return self;
}

- (void) wakeUp
{
	[_tableView reloadData];
}

- (CGFloat) contentHeight
{
	if(scrollView == nil)
		return 0;
	
	CGFloat nbElem = [controller getNbElem:YES];
	
	if(!nbElem)
		return 0;
 
	return nbElem * ([self tableView:_tableView heightOfRow:0] + _tableView.intercellSpacing.height) + _tableView.intercellSpacing.height;
}

#pragma mark - Interface

- (void) checkIfShouldReload
{
	if(wasSerie != controller.isSerieMainThread)
	{
		wasSerie = !wasSerie;
		
		uint nbRow = [self numberOfRowsInTableView : _tableView];
		
		[self fullAnimatedReload:nbRow :nbRow];
	}
}

- (void) deleteElements : (uint*) indexes : (uint) length
{
	if(length == 0 || indexes == NULL)
		return;
	
	if(_tableView != nil)
	{
		NSMutableIndexSet * index = [NSMutableIndexSet new];
		
		for(uint i = 0; i < length; i++)
			[index addIndex:indexes[i]];
		
		[_tableView removeRowsAtIndexes:index withAnimation:NSTableViewAnimationSlideLeft];
	}
	
	MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate] MDL];
	if(tabMDL != nil)
	{
		NSRect lastFrame = [tabMDL lastFrame], newFrame = [tabMDL createFrame];
		
		if(!NSEqualRects(lastFrame, newFrame))
			[tabMDL fastAnimatedRefreshLevel : tabMDL.superview];
	}
	
	while(length--)
		[[NSNotificationCenter defaultCenter] postNotificationName: @"RakMDLListViewRowDeleted" object:self userInfo:@{ @"deletedRow" : @(indexes[length])}];
	
}

- (void) additionalResizing : (NSSize) newSize : (BOOL) animated
{
	if(animated)
		[_tableView.animator setFrameSize: NSMakeSize(_tableView.bounds.size.width, [self contentHeight])];
	else
		[_tableView setFrameSize: NSMakeSize(_tableView.bounds.size.width, [self contentHeight])];
}

#pragma mark - Methods to deal with tableView

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return [controller getNbElem : YES];
}

- (CGFloat) tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	if(wasSerie != controller.isSerieMainThread)
		[self checkIfShouldReload];
	
	return wasSerie ? MDLLIST_CELL_HEIGHT_MULTI : MDLLIST_CELL_HEIGHT;
}

- (NSNumber*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex >= [controller getNbElem : YES])
		return nil;
	
	return @(rowIndex);
}

- (void) tableView:(NSTableView *)tableView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	DATA_LOADED ** todoList = [controller getData:row : YES];
	
	if(todoList != NULL && *todoList != NULL)
	{
		(*todoList)->rowViewResponsible = (__bridge void *)([tableView viewAtColumn:0 row:row makeIfNecessary:NO]);
		[(__bridge RakMDLListView*) (*todoList)->rowViewResponsible updateContext];
	}
}

- (void) tableView:(NSTableView *)tableView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	if (row == -1)
		return;
	
	DATA_LOADED ** todoList = [controller getData:row : YES];
	
	if(todoList != NULL && *todoList != NULL)
	{
		(*todoList)->rowViewResponsible = NULL;
	}
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	// Get an existing cell with the MyView identifier if it exists
	RakMDLListView *result = [tableView makeViewWithIdentifier:@"Mane 6" owner:self];
	
	// There is no existing cell to reuse so create a new one
	if (result == nil)
	{
		result = [[RakMDLListView alloc] init :controller :row];
		
		if(result.invalidData)
			[_tableView reloadData];
		
		[result setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
		
		result.identifier = @"Mane 6";
	}
	else
	{
		[result updateData : row];
		
		if(result.invalidData)
			[_tableView reloadData];
	}
	
	return result;
}

- (BOOL) tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
	DATA_LOADED ** dataProject = [controller getData:row :YES];
	
	if(dataProject != NULL && (*dataProject)->datas != NULL)
	{
		[RakTabView broadcastUpdateContext:self :*(*dataProject)->datas :YES :VALEUR_FIN_STRUCT];
	}
	
	return NO;
}

#pragma mark - Drag support

- (PROJECT_DATA) getProjectDataForDrag : (uint) row
{
	DATA_LOADED ** dataProject = [controller getData:row :YES];
	
	if (dataProject == NULL || 	(*dataProject)->datas == NULL)
		return [super getProjectDataForDrag:row];
	
	return *((*dataProject)->datas);
}

- (NSString *) contentNameForDrag : (uint) row
{
	DATA_LOADED ** dataProject = [controller getData:row :YES];
	
	if (dataProject == NULL)
		return nil;
	
	if((*dataProject)->listChapitreOfTome == NULL)
	{
		int chapitre = (*dataProject)->identifier;
		
		if(chapitre % 10)
			return [NSString stringWithFormat:NSLocalizedString(@"CHAPTER-%d.%d", nil), chapitre / 10, chapitre % 10];
		return [NSString stringWithFormat:NSLocalizedString(@"CHAPTER-%d", nil), chapitre / 10];
	}
	else
	{
		if((*dataProject)->tomeName != NULL && (*dataProject)->tomeName[0] != 0)
			return [NSString stringWithUTF8String: (char*) (*dataProject)->tomeName];
		
		return [NSString stringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), (*dataProject)->identifier];
	}
}

- (void) fillDragItemWithData:(RakDragItem *)item :(uint)row
{
	row = row / _nbCoupleColumn + _tableView.preCommitedLastClickedColumn / _nbElemPerCouple;
	
	DATA_LOADED ** dataProject = [controller getData:row :YES];
	
	if(dataProject == NULL || *dataProject == NULL)
		return;
	
	BOOL isTome = ((*dataProject)->listChapitreOfTome != NULL);
	PROJECT_DATA project = getCopyOfProjectData(*(*dataProject)->datas);
	
	getUpdatedCTList(&project, true);
	getUpdatedCTList(&project, false);
	
	[item setDataProject : project isTome: isTome element: (*dataProject)->identifier];
	
	dragInProgress = YES;		draggedElement = row;
}

- (void) cleanupDrag
{
	dragInProgress = NO;
}

#pragma mark - Drop support

- (BOOL) supportReorder
{
	return YES;
}

- (BOOL) grantDropAuthorization : (BOOL) canDL
{
	return canDL;
}

- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow : (NSTableViewDropOperation) operation
{
	NSView * view = _tableView.superview;
	while (view != nil && [view superclass] != [RakTabView class])	{	view = view.superview;	}
	
	if(view != nil)
		return [(RakTabView *) view dropOperationForSender:sourceTab: [RakDragItem canDL:[item draggingPasteboard]]];
	
	return [super operationForContext:item :sourceTab :suggestedRow :operation];
}

- (uint) getSelfCode
{
	return TAB_MDL;
}

- (BOOL) receiveDrop : (PROJECT_DATA) project : (BOOL) isTome : (int) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation
{
	if (sender == [self getSelfCode])	//Reorder
	{
		if(project.repo == NULL || element == VALEUR_FIN_STRUCT || row > [self numberOfRowsInTableView:nil] || (operation != NSTableViewDropAbove && operation != NSTableViewDropOn))
			return NO;
		
		if(!dragInProgress)
			return NO;
		
		[controller reorderElements:draggedElement :draggedElement :row];
		[_tableView reloadData];
	}
	else
	{
		//Prevent the update from messing with memory that will get freed later
		project.chapitresFull = project.chapitresInstalled = NULL;
		project.tomesFull = project.tomesInstalled = NULL;
		project.chapitresPrix = NULL;
		getUpdatedCTList(&project, isTome);
		
		uint nbElemInjected = 1, nbElemStart = [controller getNbElem:YES], nbElemEnd;
		if(element != VALEUR_FIN_STRUCT)
			[controller addElement:project :isTome :element:NO];
		else
			nbElemInjected = [controller addBatch:project :isTome :YES];
		
		if(row != -1 && nbElemInjected != 0)
		{
			nbElemEnd = [controller getNbElem:YES];
			[controller reorderElements:nbElemStart : nbElemEnd - 1 :row];
		}
	}
	
	return YES;
}

@end
