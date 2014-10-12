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

@implementation RakCTSelectionList

#pragma mark - Classical initialization

- (id) init : (NSRect) frame : (PROJECT_DATA) project : (bool) isTomeRequest : (long) elemSelected : (long) scrollerPosition
{
	self = [super init];

	if(self != nil)
	{
		NSInteger row = -1, tmpRow = 0;

		//We check we have valid data
		self.isTome = isTomeRequest;
		projectData = project;	//We don't protect chapter/volumen list but not really a problem as we'll only use it for drag'n drop
		
		if(self.isTome && project.tomesInstalled != NULL)
		{
			amountData = project.nombreTomesInstalled;
			data = malloc(amountData * sizeof(META_TOME));
			
			if(data != NULL)
			{
				memcpy(data, project.tomesInstalled, amountData * sizeof(META_TOME));
				if(elemSelected != -1)
				{
					for(; tmpRow < amountData && ((META_TOME*)data)[tmpRow].ID < elemSelected; tmpRow++);

					if(tmpRow < amountData && ((META_TOME*)data)[tmpRow].ID == elemSelected)
						row = tmpRow;
				}
			}
		}
		else if(!self.isTome && project.chapitresInstalled != NULL)
		{
			amountData = project.nombreChapitreInstalled;
			data = malloc(amountData * sizeof(int));
			
			if(data != NULL)
			{
				memcpy(data, project.chapitresInstalled, amountData * sizeof(int));
				if(elemSelected != -1)
				{
					for(; tmpRow < amountData && ((int*)data)[tmpRow] < elemSelected; tmpRow++);
					
					if(tmpRow < amountData && ((int*)data)[tmpRow] == elemSelected)
						row = tmpRow;
				}
			}
		}
		
		[self applyContext:frame :row :scrollerPosition];
		
		scrollView.wantsLayer = YES;
		scrollView.layer.backgroundColor = [NSColor whiteColor].CGColor;
		scrollView.layer.cornerRadius = 4;
	}
	
	return self;
}

- (bool) didInitWentWell
{
	return data != NULL;
}

- (BOOL) reloadData : (PROJECT_DATA) project : (int) nbElem : (void *) newData : (BOOL) resetScroller
{
	void * newDataBuf = NULL;
	
	NSInteger element = [self getSelectedElement];
	
	if(self.isTome)
	{
		newDataBuf = malloc(nbElem * sizeof(META_TOME));
		if(newDataBuf != NULL)
		{
			memcpy(newDataBuf, newData, nbElem * sizeof(META_TOME));
		}
	}
	else
	{
		newDataBuf = malloc(nbElem * sizeof(int));
		if(newDataBuf != NULL)
		{
			memcpy(newDataBuf, newData, nbElem * sizeof(int));
		}
	}
	
	if(newDataBuf == NULL)
		return NO;
		
	free(data);
	data = newDataBuf;
	amountData = nbElem;
	projectData = project;
	
	if(resetScroller)
		[_tableView scrollRowToVisible:0];
	
	[_tableView reloadData];
	
	if(element != -1)
		[self selectRow:[self getIndexOfElement:element]];
	
	return YES;
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

#pragma mark - Methods to deal with tableView

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex >= amountData)
		return nil;
	
	NSString * output;
	
	if(self.isTome)
	{
		META_TOME element = ((META_TOME *) data)[rowIndex];
		if(element.ID != VALEUR_FIN_STRUCT)
		{
			if(element.readingName[0])
				output = [[NSString alloc] initWithBytes:element.readingName length:sizeof(element.readingName) encoding:NSUTF32LittleEndianStringEncoding];
			else
				output = [NSString stringWithFormat:@"Tome %d", element.readingID];
		}
		else
			output = @"Error! Out of bounds D:";
		
	}
	else
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

@implementation RakCTSelectionListContainer

- (instancetype) initWithFrame : (NSRect) parentFrame : (BOOL) isCompact : (RakCTSelectionList*) content
{
	if(content == nil)
		return nil;
	
	_isCompact = isCompact;
	_content = content;
	
	self = [self initWithFrame : [self frameFromParent:parentFrame]];
	
	if(self != nil)
	{
		self.wantsLayer = YES;
		self.layer.backgroundColor = [self getBackgroundColor];
		self.layer.cornerRadius = isCompact ? 0 : 4.0;
		
		[Prefs getCurrentTheme:self];
		[content setSuperview:self];
	}
	
	return self;
}

#pragma mark - Properties

- (void) setCompactMode : (BOOL) compactMode
{
	if(compactMode)
	{
		_isCompact = compactMode;
		[self setHidden:_wasHidden];
		
		self.layer.cornerRadius = 0;
	}
	else
	{
		_wasHidden = self.isHidden;
		[self setHidden:NO];
		_isCompact = compactMode;

		self.layer.cornerRadius = 4.0f;
	}
	
	self.layer.backgroundColor = [self getBackgroundColor];
}

- (BOOL) compactMode
{
	return _isCompact;
}

- (void) setHidden : (BOOL) hidden
{
	if(!_isCompact)
		_wasHidden = hidden;

	else if(self.isHidden != hidden)
		[super setHidden : hidden];
}

#pragma mark - Sizing

- (NSRect) frameFromParent : (NSRect) parentFrame
{
	if(self.compactMode)
	{
		parentFrame.origin.x = CT_READERMODE_BORDER_TABLEVIEW;
		parentFrame.origin.y = 0;
		parentFrame.size.width -= 2 * CT_READERMODE_BORDER_TABLEVIEW;
		parentFrame.size.height -= CT_READERMODE_HEIGHT_CT_BUTTON + CT_READERMODE_HEIGHT_BORDER_TABLEVIEW;
	}
	else
	{
		parentFrame.origin.y = 10;
		parentFrame.size.height -= 20 + 8;

		parentFrame.size.width /= 2;
		parentFrame.size.width -= 20;
		
		if(_content.isTome)
			parentFrame.origin.x = parentFrame.size.width + 20;
		else
			parentFrame.origin.x = 10;
	}
	
	return parentFrame;
}

- (void) setFrame : (NSRect) parentFrame
{
	[super setFrame : [self frameFromParent:parentFrame]];
	
	[_content setFrame : self.bounds];
}

- (void) resizeAnimation : (NSRect) parentFrame
{
	NSRect frame = [self frameFromParent:parentFrame];
	[self.animator setFrame : frame];
	
	frame.origin = NSZeroPoint;
	
	[_content resizeAnimation : frame];
}

#pragma mark - Color

- (CGColorRef) getBackgroundColor
{
	if(self.compactMode)
		return [NSColor clearColor].CGColor;
	
	return [Prefs getSystemColor : GET_COLOR_BACKGROUD_CT_READERMODE : nil].CGColor;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] == [Prefs class])
		return;
	
	self.layer.backgroundColor = [self getBackgroundColor];
}

#pragma mark - Proxy

- (NSScrollView*) getContent	{	return [_content getContent];	}

- (NSInteger) getSelectedElement	{	return [_content getSelectedElement];	}
- (float) getSliderPos	{	return [_content getSliderPos];		}

- (BOOL) reloadData : (PROJECT_DATA) project : (int) nbElem : (void *) newData : (BOOL) resetScroller;
{
	return [_content reloadData : project : nbElem : newData : resetScroller];
}

- (NSInteger) getIndexOfElement : (NSInteger) element	{	return [_content getIndexOfElement:element];	}
- (void) selectRow : (int) row	{	[_content selectRow:row];	}
- (void) jumpScrollerToRow : (int) row	{	[_content jumpScrollerToRow:row];	}

- (void) resetSelection : (NSTableView *) tableView	{	[_content resetSelection:tableView];	}

@end