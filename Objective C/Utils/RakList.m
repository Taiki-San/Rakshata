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

@implementation RakList

#pragma mark - Classical initialization

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		_scrollerWidth = [RakScroller width] + 2;
		_nbCoupleColumn = 1;
		_nbElemPerCouple = 1;
		selectedRowIndex = LIST_INVALID_SELECTION;
		selectedColumnIndex = LIST_INVALID_SELECTION;
		[Prefs getCurrentTheme:self];	//register for change
		_identifier = [NSString stringWithFormat:@"Mane 6 ~ %u", arc4random() % UINT_MAX];
	}
	
	return self;
}

- (void) applyContext : (NSRect) frame : (uint) activeRow : (long) scrollerPosition
{
	//Let the fun begin
	scrollView = [[RakListScrollView alloc] initWithFrame:[self getFrameFromParent:frame]];
	_tableView = [[RakTableView alloc] initWithFrame : scrollView.contentView.bounds];
	
	if(scrollView == nil || _tableView == nil)
	{
		NSLog(@"Luna refused to allocate this memory to us D:");
		return;
	}
	
	_tableView.wantsLayer = NO;
	_tableView.autoresizesSubviews = NO;
	scrollView.documentView = _tableView;
	
	//Customisation
	normal		= [self getTextColor];
	highlight	= [self getTextHighlightColor];
	
	_tableView.headerView = nil;
	_tableView.backgroundColor = [NSColor clearColor];
	_tableView.selectionHighlightStyle = NSTableViewSelectionHighlightStyleNone;
	_tableView.focusRingType = NSFocusRingTypeNone;
	_tableView.allowsMultipleSelection = NO;
	_tableView.intercellSpacing = NSMakeSize(0, _tableView.intercellSpacing.height);
	
	//End of setup
	[_tableView addTableColumn:[[NSTableColumn alloc] initWithIdentifier:RAKLIST_MAIN_COLUMN_ID]];
	[_tableView sizeLastColumnToFit];
	[_tableView setDelegate:self];
	[_tableView setDataSource:self];
	[_tableView reloadData];
	
	if(activeRow != LIST_INVALID_SELECTION)
	{
		[self tableView:_tableView shouldSelectRow:activeRow];		//Apply graphic changes
		self._selectionChangeComeFromClic = NO;
		[_tableView selectRowIndexes:[NSIndexSet indexSetWithIndex:activeRow] byExtendingSelection:NO];
	}
	
	if(scrollerPosition != -1)
	{
		[scrollView.contentView scrollToPoint:NSMakePoint(0, scrollerPosition)];
	}
	else
	{
		[_tableView scrollRowToVisible:0];
	}
}

- (void) failure
{
#ifdef DEV_VERSION
	NSLog(@"[%s] - Unrecoverable error, we need to abort!", __PRETTY_FUNCTION__);
#endif
}

- (NSScrollView*) getContent
{
	return scrollView;
}

- (void) setSuperview : (NSView *) superview
{
	[superview addSubview:scrollView];
}

- (NSView*) superview
{
	return scrollView.superview;
}

- (NSRect) frame
{
	return [scrollView frame];
}

- (void) setFrameOrigin : (NSPoint) origin
{
	[scrollView setFrameOrigin: origin];
}

- (void) setHidden : (BOOL) state
{
	[scrollView setHidden:state];
}

- (BOOL) isHidden
{
	return [scrollView isHidden];
}

- (void) setAlphaValue : (CGFloat) alphaValue : (BOOL) animated
{
	if(animated)
		scrollView.animator.alphaValue = alphaValue;
	else
		scrollView.alphaValue = alphaValue;
}

- (void) dealloc
{
	[_tableView removeFromSuperview];
	[scrollView setDocumentView:nil];
	
	[scrollView removeFromSuperview];
	
	free(_data);
}

#pragma mark - Resizing

- (void) setFrame : (NSRect) frameRect
{
	[self _resize:frameRect :NO : YES];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self _resize:frameRect :YES : YES];
}

- (void) reloadSize
{
	[self _resize:scrollView.frame :NO :NO];
}

- (void) _resize : (NSRect) frame : (BOOL) animate : (BOOL) immatureFrame
{
	NSSize oldTableviewSize = _tableView.bounds.size;
	NSRect scrollviewFrame = immatureFrame ? [self getFrameFromParent : frame] : frame;
	
	if(animate)
		[scrollView resizeAnimation:scrollviewFrame];
	else
		[scrollView setFrame:scrollviewFrame];
	
	scrollviewFrame.origin = NSZeroPoint;
	
	CGFloat scrollerWidth = scrollView.hasVerticalScroller ? _scrollerWidth : 0;
	scrollviewFrame.size.width -= scrollerWidth;
	oldTableviewSize.width = scrollviewFrame.size.width;
	
	[_tableView setFrameSize : oldTableviewSize];
	
	if(immatureFrame)
		[self updateMultiColumn : scrollviewFrame.size];
	else
		[self additionalResizing : scrollviewFrame.size : animate];
}

- (void) updateMultiColumn :(NSSize)scrollviewSize
{
	[self additionalResizing : scrollviewSize : NO];
}

- (void) additionalResizing : (NSSize) newSize : (BOOL) animated
{
	
}

- (NSRect) getFrameFromParent : (NSRect) superviewFrame
{
	return superviewFrame;
}

- (void) needUpdateTableviewHeight
{
	uint height = _tableView.numberOfRows * _tableView.rowHeight;
	
	if(height)
	{
		[_tableView setFrameSize:NSMakeSize(_tableView.bounds.size.width, height)];
	}
}

#pragma mark - Drag'n drop routines

- (void) enableDrop
{
	[_tableView registerForDraggedTypes:@[PROJECT_PASTEBOARD_TYPE]];
	[_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:YES];
	[_tableView setDraggingSourceOperationMask:NSDragOperationCopy forLocal:NO];
}

#pragma mark - Backup routine

- (uint) getSelectedElement
{
	return LIST_INVALID_SELECTION;
}

- (uint) getIndexOfElement : (int) element
{
	return LIST_INVALID_SELECTION;
}

- (float) getSliderPos
{
	if([scrollView hasVerticalScroller])
	{
		return (float) scrollView.contentView.bounds.origin.y;
	}
	
	return -1;
}

- (NSInteger) selectedRow
{
	return selectedRowIndex;
}

#pragma mark - Colors

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT:nil];
}

- (NSColor *) getTextColor : (uint) column : (uint) row
{
	return nil;
}

- (NSColor *) getTextHighlightColor
{
	return [Prefs getSystemColor:COLOR_ACTIVE:nil];
}

- (NSColor *) getTextHighlightColor : (uint) column : (uint) row
{
	return nil;
}

- (NSColor *) getBackgroundHighlightColor
{
	return [Prefs getSystemColor:COLOR_BACKGROUND_CT_TVCELL:nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	normal		= [self getTextColor];
	highlight	= [self getTextHighlightColor];
	
	[self updateTableElementsColor];
	[_tableView setNeedsDisplay:YES];
}

- (void) updateTableElementsColor
{
	RakText * element;
	NSColor * backgroundColor = [self getBackgroundHighlightColor];
	NSView * rowView;
	
	uint lastClickedColumn = _tableView.lastClickedColumn / _nbElemPerCouple;
	for(uint rowIndex = 0, column, max = [_tableView numberOfRows]; rowIndex < max; rowIndex++)
	{
		rowView = [_tableView rowViewAtRow:rowIndex makeIfNecessary:NO];
		column = 0;
		for(element in rowView.subviews)
		{
			if(element != nil && [element class] == [RakText class])
			{
				if(rowIndex == selectedRowIndex && column / _nbElemPerCouple == lastClickedColumn)
					element.textColor = highlight != nil ? highlight : [self getTextHighlightColor:column++ :rowIndex];
				else
					element.textColor = normal != nil ? normal : [self getTextColor:column++ :rowIndex];
				
				element.backgroundColor = backgroundColor;
			}
		}
	}
}

#pragma mark - Methods to deal with tableView

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return _data == NULL ? 0 : (_nbData / _nbCoupleColumn + (_nbData % _nbCoupleColumn != 0));
}

- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row
{
	NSTableRowView * output = [[NSTableRowView alloc] init];
	
	output.autoresizesSubviews = NO;
	output.wantsLayer = NO;
	
	return output;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	BOOL selected = row == selectedRowIndex;
	__block uint column = 0;
	if(_nbCoupleColumn > 1)	//We need to check if we are in the good column
	{
		[tableView.tableColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
			if(obj == tableColumn)
			{
				column = idx;
				*stop = YES;
			}
		}];
		
		if(selected)
			selected = tableView.lastClickedColumn / _nbElemPerCouple == column / _nbElemPerCouple;
	}
	
	// Get an existing cell with the identifier if it exists
	RakText *result = [tableView makeViewWithIdentifier : _identifier owner:self];
	
	if(result == nil)
	{
		result = [[RakText alloc] init];
		result.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13];
		result.identifier = _identifier;
	}
	
	_tmpColor = selected ? (highlight != nil ? highlight : [self getTextHighlightColor:column :row]) : (normal != nil ? normal : [self getTextColor:column :row]);
	result.backgroundColor = [self getBackgroundHighlightColor];
	
	[self graphicSelection : result : selected];
	
	return result;
}

- (BOOL) tableView : (RakTableView *) tableView shouldSelectRow:(NSInteger)rowIndex
{
	if(rowIndex == UINT_MAX)
		return NO;
	
	[self resetSelection:tableView];
	[tableView commitClic];
	
	selectedRowIndex = LIST_INVALID_SELECTION;
	
	uint baseColumn = tableView.lastClickedColumn, initialColumn = baseColumn / _nbElemPerCouple * _nbElemPerCouple;
	_tmpColor = (highlight != nil ? highlight : [self getTextHighlightColor:baseColumn / _nbElemPerCouple :rowIndex]);
	
	if(baseColumn != LIST_INVALID_SELECTION && rowIndex < [tableView numberOfRows] && initialColumn < [tableView numberOfColumns])
	{
		RakText * view;
		for(byte pos = 0; pos < _nbElemPerCouple; pos++)
		{
			view = [tableView viewAtColumn:initialColumn + pos row:rowIndex makeIfNecessary:NO];
			[self graphicSelection:view : YES];
		}
	}
	
	[self postProcessingSelection : rowIndex];
	return YES;
}

- (void) graphicSelection : (NSView *) view : (BOOL) select
{
	if(view != nil && [view class] == [RakText class])
	{
		((RakText *) view).drawsBackground = select;
		((RakText *) view).textColor = _tmpColor;
		[((RakText *) view) setNeedsDisplay];
	}
}

- (void) postProcessingSelection : (uint) row
{
	selectedRowIndex = row;
	selectedColumnIndex = _tableView.lastClickedColumn;
}

- (void) selectElement:(int)element
{
	[self selectIndex : [self getIndexOfElement:element]];
}

- (void) selectIndex : (uint) index
{
	if(_tableView == nil)
		return;
	
	uint column = LIST_INVALID_SELECTION;
	uint row = [self coordinateForIndex:index :&column];
	
	if(row != LIST_INVALID_SELECTION)
	{
		_tableView.preCommitedLastClickedColumn = column != LIST_INVALID_SELECTION ? column * _nbElemPerCouple : 0;
		[self tableView:_tableView shouldSelectRow:row];
		self._selectionChangeComeFromClic = NO;
	}
}

- (void) resetSelection : (NSTableView *) tableView
{
	if(tableView == nil)
	{
		if(_tableView != nil)
			tableView = _tableView;
		else
			return;
	}
	
	if(selectedRowIndex >= MIN(_nbData, [_tableView numberOfRows]))
		selectedRowIndex = LIST_INVALID_SELECTION;
	
	else if(selectedRowIndex != LIST_INVALID_SELECTION)
	{
		uint column = 0;
		NSView * rowView = [tableView rowViewAtRow:selectedRowIndex makeIfNecessary:NO];
		for(RakText * view in rowView.subviews)
		{
			if([view isKindOfClass:[self contentClass]])
			{
				_tmpColor = normal != nil ? normal : [self getTextColor:column++ :selectedRowIndex];
				[self graphicSelection:view :NO];
			}
		}
		
		CGFloat rowToDeselect = selectedRowIndex;
		selectedRowIndex = LIST_INVALID_SELECTION;	//Prevent any notification
		
		[tableView deselectRow : rowToDeselect];
	}
}

- (Class) contentClass
{
	return [RakText class];
}

#pragma mark - Model manipulation

- (uint) rowFromCoordinates : (uint) row : (uint) column
{
	return row;
}

- (uint) coordinateForIndex : (uint) index : (uint *) column
{
	if(column != NULL)
		*column = 0;
	
	return index >= [_tableView numberOfRows] ? LIST_INVALID_SELECTION : index;
}

#pragma mark - Smart reloading

//Ceci est l'algorithme naif en O(n^2)
//Il est viable sur < 1000 données, mais pourrait poser des problèmes à l'avenir
//Un algo alternatif, en O(n*log(n)) serait de faire des copies de _oldData et de _newData
//Les trier (avec un introsort (cf implé de g++), qsort est en n^2 dans notre cas générique)
//Retirer les doublons (vérifier que les positions collent, un déplacement doit être detecté)
//Regarder les positions de ce qu'il reste et voilà

- (void) smartReload : (SR_DATA*) oldData : (uint) nbElemOld : (SR_DATA*) newData : (uint) nbElemNew
{
	if(_tableView != nil)
	{
		if(oldData == NULL || newData == NULL || (_nbCoupleColumn > 1 && nbElemOld != nbElemNew))
		{
			[self fullAnimatedReload : oldData == NULL ? 0 : nbElemOld  : newData == NULL ? 0 : nbElemNew];
		}
		else
		{
			NSMutableIndexSet * new = [NSMutableIndexSet new], * old = [NSMutableIndexSet new];
			uint newElem = 0, oldElem = 0;
			uint current;
			BOOL tooMuchChanges = NO, singleColumn = _nbCoupleColumn == 1;
			
			for(uint posNew = 0, posOld = 0; posNew < nbElemNew && !tooMuchChanges; posNew++)
			{
				current = newData[posNew].data;
				
				if(posOld < nbElemOld)
				{
					if(oldData[posOld].data != current)
					{
						if(posOld < [_tableView numberOfRows])
						{
							for(uint max = MIN(nbElemOld, [_tableView numberOfRows]); posOld < max && oldData[posOld].data != current; oldElem++)
								[old addIndex : posOld++];
							
							if(posOld < nbElemOld && oldData[posOld].data != current)
								tooMuchChanges = YES;
							else
								posOld++;
						}
						else
							tooMuchChanges = YES;
						
					}
					else if(oldData[posOld].installed == newData[posNew].installed)
					{
						posOld++;
					}
					else
					{
						if(singleColumn)
						{
							[old addIndex : posOld];
							[new addIndex : posNew];
						}
						else
						{
							//Considering we refresh everything when anything is added/removed, if the row from the the old context is validated, the new one will also be
							//Also, considering they are the same, no need to compute them again
							uint row = [self coordinateForIndex:posOld : NULL];;
							if(![old containsIndex:row])
							{
								[old addIndex : row];
								[new addIndex : row];
							}
						}
						
						posOld++;
						oldElem++;	newElem++;
					}
				}
				else
				{
					if(singleColumn)
						[new addIndex:posNew];
					else
						tooMuchChanges = YES;
					
					newElem++;
				}
			}
			
			if(!tooMuchChanges)
			{
				[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
					
					[context setDuration:CT_TRANSITION_ANIMATION];
					
					if(oldElem != 0)
						[_tableView removeRowsAtIndexes:old withAnimation:NSTableViewAnimationSlideLeft];
					
					if(newElem != 0)
						[_tableView insertRowsAtIndexes:new withAnimation:NSTableViewAnimationSlideRight];
					
				} completionHandler:^{
					if(nbElemOld != nbElemNew)
						[_tableView noteNumberOfRowsChanged];
				}];
			}
			else
			{
				[self fullAnimatedReload : nbElemOld  : nbElemNew];
			}
		}
	}
	
	free(oldData);
	free(newData);
}

- (void) fullAnimatedReload : (uint) oldElem : (uint) newElem
{
	[self fullAnimatedReload:oldElem :newElem : NO];
}

//Reverted order allow to tweak in which direction the animation will go, for now used for fancy reloading
- (void) fullAnimatedReload : (uint) oldElem : (uint) newElem : (BOOL) revertedOrder
{
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		
		[context setDuration:CT_TRANSITION_ANIMATION];
		
		if(oldElem != 0)
			[_tableView removeRowsAtIndexes:[NSMutableIndexSet indexSetWithIndexesInRange:NSMakeRange(0, (NSUInteger)[_tableView numberOfRows])] withAnimation:revertedOrder ? NSTableViewAnimationSlideRight : NSTableViewAnimationSlideLeft];
		
		if(newElem != 0)
			[_tableView insertRowsAtIndexes:[NSMutableIndexSet indexSetWithIndexesInRange:NSMakeRange(0, newElem / _nbCoupleColumn + (newElem % _nbCoupleColumn != 0))] withAnimation: revertedOrder ? NSTableViewAnimationSlideLeft : NSTableViewAnimationSlideRight];
		
	} completionHandler:^{
		[scrollView updateScrollerState:scrollView.bounds];
	}];
}

#pragma mark - Drag'n drop control

- (void) fillDragItemWithData : (RakDragItem*) data : (uint) row
{
	
}

- (BOOL) receiveDrop : (PROJECT_DATA) project : (BOOL) isTome : (int) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation
{
#ifdef DEV_VERSION
	NSLog(@"Project: %@- isTome: %d - element: %d - sender: %d - row: %ld - operation: %lu", getStringForWchar(project.projectName), isTome, element, sender, (long)row, (unsigned long)operation);
#endif
	return YES;
}

- (BOOL) acceptDrop : (id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation source:(uint) source
{
	//Import task
	
	NSPasteboard * pasteboard = [info draggingPasteboard];
	
	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	
	if(item == nil || [item class] != [RakDragItem class])
		return NO;
	
	PROJECT_DATA localProject = getProjectByID(item.project.cacheDBID);	//We cannot trust the data from the D&D, as context may have changed during the D&D (end of DL)
	
	if(!localProject.isInitialized)
	{
		releaseCTData(item.project);
		return NO;
	}
	
	BOOL retVal = [self receiveDrop:localProject :item.isTome :item.selection :source :row :operation];
	
	releaseCTData(localProject);
	releaseCTData(item.project);
	
	return retVal;
}

- (void) cleanupDrag
{
	
}

#pragma mark - Drag'n drop support

- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
	if(rowIndexes == nil || [rowIndexes count] != 1)
		return NO;
	
	[RakDragResponder registerToPasteboard:pboard];
	[RakDragResponder patchPasteboardForFiledrop:pboard forType:isListOfRepo ? SOURCE_FILE_EXT : ARCHIVE_FILE_EXT];
	RakDragItem * item = [[RakDragItem alloc] init];
	
	if(item == nil)
		return NO;
	
	[self fillDragItemWithData : item : [rowIndexes firstIndex]];

	return [pboard setData:[item getData] forType:PROJECT_PASTEBOARD_TYPE];
}

+ (void) propagateDragAndDropChangeState : (BOOL) started : (BOOL) canDL
{
	NSView * view = [[NSApp delegate] serie];
	if(view != nil && view.superview != nil)
	{
		NSArray * views = [view.superview subviews];
		if(views != nil)
		{
			for(view in views)
			{
				if([view superclass] == [RakTabView class])
					[(RakTabView *) view dragAndDropStarted:started:canDL];
			}
		}
	}
}

- (void) tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes
{
	[self beginDraggingSession:session willBeginAtPoint:screenPoint forRowIndexes:rowIndexes withParent:tableView];
	draggingSession = session;
	[RakList propagateDragAndDropChangeState : YES : [RakDragItem canDL:[session draggingPasteboard]]];
}

- (NSArray *) tableView:(NSTableView *)tableView namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination forDraggedRowsWithIndexes:(NSIndexSet *)indexSet
{
	[RakExportController createArchiveFromPasteboard:[draggingSession draggingPasteboard] toPath:nil withURL:dropDestination];
	return nil;
}

- (void) tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
	//Need to cleanup once the drag is over
	draggingSession = nil;
	[RakList propagateDragAndDropChangeState : NO : [RakDragItem canDL:[session draggingPasteboard]]];
	[self cleanupDrag];
}

//Drop support, only used by MDL for now

- (NSDragOperation) tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
	return [self defineDropAuthorizations:info sender:[RakDragResponder getOwnerOfTV:[info draggingSource]] proposedRow:row operation:operation];
}

- (BOOL) tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
	//Does the actual work after the drop
	return [self acceptDrop:info  row:row dropOperation:operation source:[RakDragResponder getOwnerOfTV:[info draggingSource]]];
}

@end

@implementation RakTableView

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		self.preCommitedLastClickedColumn = LIST_INVALID_SELECTION;
		_lastClickedRow = _lastClickedColumn = _preCommitedLastClickedRow = LIST_INVALID_SELECTION;
	}
	
	return self;
}

- (NSColor *) _dropHighlightColor
{
	return [Prefs getSystemColor:COLOR_ACTIVE:nil];
}

- (void)setDropRow:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
	if(operation == NSTableViewDropOn)
		operation = NSTableViewDropAbove;
	
	[super setDropRow:row dropOperation:operation];
}

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context
{
	return NSDragOperationCopy;
}

#pragma mark - Selection

- (void) mouseDown:(NSEvent *)theEvent
{
	NSPoint localLocation = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	_preCommitedLastClickedRow = [self rowAtPoint:localLocation];
	self.preCommitedLastClickedColumn = [self columnAtPoint:localLocation];
	
	if(self.wantVerboseClick && _preCommitedLastClickedRow == self.selectedRow)
	{
		if([self.delegate tableView:self shouldSelectRow:_preCommitedLastClickedRow])
		{
			if([self.delegate isKindOfClass:[RakList class]])
				((RakList*) self.delegate)._selectionChangeComeFromClic = YES;
			
			[self.delegate tableViewSelectionDidChange:nil];
		}
	}
	
	[super mouseDown : theEvent];
}

- (void) commitClic
{
	_lastSelectionWasClic = YES;
	_lastSelectedRow = _preCommitedLastClickedRow;
	_lastSelectedColumn = self.preCommitedLastClickedColumn;
}

- (void) setLastClickedColumn : (uint) lastClickedColumn
{
	if(_lastSelectionWasClic)
	{
		_lastSelectedRow = [self selectedRow];
		_lastSelectionWasClic = NO;
	}
	
	_lastSelectedColumn = lastClickedColumn;
}

- (uint) lastClickedColumn
{
	return _lastSelectedColumn;
}

- (void) setLastClickedRow : (uint) lastClickedRow
{
	if(_lastSelectionWasClic)
	{
		_lastSelectedColumn = [self selectedColumn];
		_lastSelectionWasClic = NO;
	}
	_lastSelectedRow = lastClickedRow;
}

- (uint) lastClickedRow
{
	return _lastSelectedColumn;
}

@end