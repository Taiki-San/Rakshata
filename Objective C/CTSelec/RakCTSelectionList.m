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

#define IDENTIFIER_PRICE @"RakCTSelectionListPrice"

@implementation RakCTSelectionList

#pragma mark - Classical initialization

- (instancetype) initWithFrame : (NSRect) frame  isCompact : (BOOL) isCompact projectData : (PROJECT_DATA) project isTome : (bool) isTomeRequest selection : (long) elemSelected  scrollerPos : (long) scrollerPosition
{
	self = [self init];

	if(self != nil)
	{
		NSInteger row = -1, tmpRow = 0;

		//We check we have valid data
		_compactMode = isCompact;
		self.isTome = isTomeRequest;
		projectData = project;	//We don't protect chapter/volumen list but not really a problem as we'll only use it for drag'n drop
		chapterPrice = NULL;
		
		if(![self reloadData:projectData :NO])
		{
			self = nil;
			return nil;
		}
		
		if(elemSelected != -1)
		{
			if(self.isTome)
			{
				for(; tmpRow < amountData && ((META_TOME*)data)[tmpRow].ID < elemSelected; tmpRow++);
				
				if(tmpRow < amountData && ((META_TOME*)data)[tmpRow].ID == elemSelected)
					row = tmpRow;
			}
			else if(!self.isTome)
			{
				for(; tmpRow < amountData && ((int*)data)[tmpRow] < elemSelected; tmpRow++);
				
				if(tmpRow < amountData && ((int*)data)[tmpRow] == elemSelected)
					row = tmpRow;
			}
		}
		
		[self applyContext : frame : row :scrollerPosition];
		
		if(_tableView != nil && scrollView != nil)
		{
			_mainColumn = _tableView.tableColumns[0];
			[self updateColumnPrice : _compactMode];
			
			scrollView.wantsLayer = YES;
			scrollView.layer.backgroundColor = [NSColor whiteColor].CGColor;
			scrollView.layer.cornerRadius = 4;
		}
		else
		{
			free(data);
			free(chapterPrice);
			self = nil;
		}
	}
	
	return self;
}

- (bool) didInitWentWell
{
	return data != NULL;
}

- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller
{
	void * newDataBuf = NULL, *newData, *newPrices = NULL;
	uint nbElem, allocSize;
	
	NSInteger element = _tableView != nil ? [self getSelectedElement] : 0;
	
	if(self.isTome)
	{
		allocSize = sizeof(META_TOME);
		
		if(self.compactMode)
		{
			nbElem = project.nombreTomesInstalled;
			newData = project.tomesInstalled;
		}
		else
		{
			nbElem = project.nombreTomes;
			newData = project.tomesFull;
		}
		
		_nbChapterPrice = UINT_MAX;
	}
	else
	{
		allocSize = sizeof(int);
		
		if(self.compactMode)
		{
			nbElem = project.nombreChapitreInstalled;
			newData = project.chapitresInstalled;
		}
		else
		{
			nbElem = project.nombreChapitre;
			newData = project.chapitresFull;
		}
		
		if(projectData.isPaid && projectData.chapitresPrix != NULL)
		{
			newPrices = calloc(projectData.nombreChapitre, sizeof(uint));
			if(newPrices != NULL)
			{
				_nbChapterPrice = projectData.nombreChapitre;
				memcpy(newPrices, projectData.chapitresPrix, _nbChapterPrice * sizeof(uint));
			}
		}
	}
	
	if(newData == NULL)
	{
		free(newPrices);
		return NO;
	}
	
	newDataBuf = malloc(nbElem * allocSize);
	if(newDataBuf == NULL)
	{
		free(newPrices);
		return NO;
	}
	
	memcpy(newDataBuf, newData, nbElem * allocSize);
	
	free(data);
	data = newDataBuf;
	amountData = nbElem;
	projectData = project;
	
	free(chapterPrice);
	chapterPrice = newPrices;
	
	if(_tableView != nil)
	{
		if(resetScroller)
			[_tableView scrollRowToVisible:0];
		
		[_tableView reloadData];
		
		if(element != -1)
			[self selectRow:[self getIndexOfElement:element]];
	}
	
	return YES;
}

#pragma mark - Properties

- (uint) nbElem
{
	return amountData;
}

#pragma mark - Backup routine

- (NSInteger) getSelectedElement
{
	NSInteger row = selectedIndex;
	
	if(row < 0 || row > amountData)
		return -1;
	
	if(self.isTome)
		return ((META_TOME *) data)[row].ID;
	else
		return ((int *) data)[row];
}

- (void) jumpScrollerToRow : (int) row
{
	if(_tableView != nil && row != -1 && row < amountData)
		[_tableView scrollRowToVisible:row];
}
 
- (NSInteger) getIndexOfElement : (NSInteger) element
{
	if (data == NULL)
		return -1;
	
	if (self.isTome)
	{
		for (uint pos = 0; pos < amountData; pos++)
		{
			if(((META_TOME *) data)[pos].ID == element)
				return pos;
		}
	}
	else
	{
		for (uint pos = 0; pos < amountData; pos++)
		{
			if(((int *) data)[pos] == element)
				return pos;
		}
	}
	
	return -1;
}

#pragma mark - Switch state

- (BOOL) compactMode
{
	return _compactMode;
}

- (void) setCompactMode : (BOOL) compactMode
{
	if(compactMode != _compactMode)
	{
		[self updateColumnPrice:compactMode];
		_compactMode = compactMode;
	}
}

- (void) updateColumnPrice : (BOOL) isCompact
{
	if(isCompact)
	{
		[_tableView removeTableColumn:_detailColumn];
		_detailColumn = nil;
	}
	else
	{
		if(projectData.isPaid && (self.isTome || chapterPrice != NULL))
		{
			_detailColumn = [[NSTableColumn alloc] initWithIdentifier:IDENTIFIER_PRICE];
			[_tableView addTableColumn:_detailColumn];

			_mainColumn.width -= _detailColumn.width;
		}
	}
}

#pragma mark - Methods to deal with tableView

- (void) additionalResizingProxy
{
	[self additionalResizing : _tableView.bounds.size];
	_resizingQueued = NO;
}

- (void) additionalResizing : (NSSize) newSize
{
	_detailColumn.width = _detailWidth;
	_mainColumn.width = newSize.width - _detailWidth - (scrollView.hasVerticalScroller ? 10 : 0);
}

- (NSView*) tableView : (NSTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	RakText * output = (RakText *) [super tableView:tableView viewForTableColumn:tableColumn row:row];
	
	if(tableColumn == _detailColumn)
	{
		output.alignment = NSRightTextAlignment;

		output.stringValue = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];
		[output sizeToFit];
		
		if(output.bounds.size.width > _detailWidth)
		{
			_detailWidth = output.bounds.size.width;
			
			if(!_resizingQueued)
			{
				_resizingQueued = YES;
				[self performSelectorOnMainThread:@selector(additionalResizingProxy) withObject:nil waitUntilDone:NO];
			}
		}
	}
	else
		output.alignment = NSLeftTextAlignment;

	return output;
}

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	NSString * output = @"Error :(";
	
	if(rowIndex >= amountData)
		return output;
	
	if(self.isTome)
	{
		META_TOME element = ((META_TOME *) data)[rowIndex];
		if(element.ID != VALEUR_FIN_STRUCT)
		{
			if(aTableColumn != _detailColumn)
			{
				if(element.readingName[0])
					output = [[NSString alloc] initWithBytes:element.readingName length:sizeof(element.readingName) encoding:NSUTF32LittleEndianStringEncoding];
				else
					output = [NSString stringWithFormat:@"Tome %d", element.readingID];
			}
			else
				output = priceString(element.price);
		}
		else
			output = @"Error! Out of bounds D:";
		
	}
	else
	{
		if(aTableColumn != _detailColumn)
		{
			int ID = ((int *) data)[rowIndex];
			if(ID != VALEUR_FIN_STRUCT)
			{
				if(ID % 10)
					output = [NSString stringWithFormat:@"Chapitre %d.%d", ID / 10, ID % 10];
				else
					output = [NSString stringWithFormat:@"Chapitre %d", ID / 10];
			}
			else
				output = @"Error! Out of bounds D:";
		}
		else if(chapterPrice != NULL && rowIndex < _nbChapterPrice)
			output = priceString(chapterPrice[rowIndex]);
	}
	
	return output;
}

#pragma mark - Get result from NSTableView

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	if(selectedIndex != -1 && selectedIndex < amountData)
	{
		[(RakCTSelection*) scrollView.superview gotClickedTransmitData: self.isTome : selectedIndex];
	}
}

#pragma mark - Drag and drop support

- (uint) getSelfCode
{
	return TAB_CT;
}

- (PROJECT_DATA) getProjectDataForDrag : (uint) row
{
	return projectData;
}

- (NSString *) contentNameForDrag : (uint) row
{
	return [self tableView:nil objectValueForTableColumn:nil row:row];
}

- (void) fillDragItemWithData:(RakDragItem *)item :(uint)row
{
	int selection;
	
	if(self.isTome)
		selection = (((META_TOME *) data)[row]).ID;
	else
		selection = ((int *) data)[row];
	
	[item setDataProject:getCopyOfProjectData(projectData) isTome:self.isTome element:selection];
}

@end
