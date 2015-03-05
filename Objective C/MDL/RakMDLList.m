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

@implementation RakMDLList

- (id) init : (NSRect) frame : (RakMDLController *) _controller
{
	if(_controller == nil)
		return nil;
	
	self = [super init];
	
	if(self != nil)
	{
		controller = _controller;
		cellHeight = MDLLIST_CELL_HEIGHT;
		dragInProgress = false;
		
		[self applyContext:frame : -1 : -1];
		[_tableView setRowHeight:cellHeight];
		[scrollView setHasHorizontalScroller:NO];
		[self enableDrop];
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
 
	return nbElem * (cellHeight + _tableView.intercellSpacing.height) + _tableView.intercellSpacing.height;
}

- (void) resizeAnimation: (NSRect) frameRect
{
	[scrollView.animator setFrame: [self getFrameFromParent:frameRect]];
}

- (void) setScrollerHidden : (BOOL) hidden
{
	if(scrollView != nil)
		[scrollView setHasVerticalScroller:!hidden];
}

#pragma mark - Methods to deal with tableView

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return [controller getNbElem : YES];
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
		result = [[RakMDLListView alloc] init:_tableView.frame.size.width :cellHeight :controller :row];

		[result setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
		
		result.identifier = @"Mane 6";
	}
	else
	{
		[result updateData : row : _tableView.frame.size.width];
	}
	
	// Return the result
	return result;
	
}

- (BOOL) tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
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
	
	dragInProgress = true;		draggedElement = row;
}

- (void) cleanupDrag
{
	dragInProgress = false;
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

- (BOOL) receiveDrop : (PROJECT_DATA) project : (bool) isTome : (int) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation
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
