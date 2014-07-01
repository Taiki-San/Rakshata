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

@implementation RakCTCoreContentView

#pragma mark - Classical initialization

- (id) init : (NSRect) frame : (MANGAS_DATA) project : (bool) isTomeRequest : (long) elemSelected : (long) scrollerPosition
{
	self = [super init];

	if(self != nil)
	{
		NSInteger row = -1, tmpRow = 0;

		//We check we have valid data
		isTome = isTomeRequest;
		projectData = project;	//We don't protect chapter/volumen list but not really a problem as we'll only use it for drag'n drop
		
		if(isTome && project.tomesInstalled != NULL)
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
		else if(!isTome && project.chapitresInstalled != NULL)
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
	}
	return self;
}

- (bool) didInitWentWell
{
	return data != NULL;
}

- (bool) reloadData : (MANGAS_DATA) project : (int) nbElem : (void *) newData : (BOOL) resetScroller
{
	void * newDataBuf = NULL;
	
	NSInteger element = [self getSelectedElement];
	
	if(isTome)
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
		return false;
		
	free(data);
	data = newDataBuf;
	amountData = nbElem;
	projectData = project;
	
	if(resetScroller)
		[_tableView scrollRowToVisible:0];
	
	[_tableView reloadData];
	
	if(element != -1)
		[self selectRow:[self getIndexOfElement:element]];
	
	return true;
}

- (NSRect) getTableViewFrame : (NSRect) superViewFrame
{
	NSRect frame = superViewFrame;
	
	frame.origin.x = CT_READERMODE_BORDER_TABLEVIEW;
	frame.size.width -= 2 * CT_READERMODE_BORDER_TABLEVIEW;
	frame.size.height -= CT_READERMODE_HEIGHT_CT_BUTTON + CT_READERMODE_HEIGHT_BORDER_TABLEVIEW;
	
	return frame;
}

#pragma mark - Backup routine

- (NSInteger) getSelectedElement
{
	NSInteger row = selectedIndex;
	
	if(row < 0 || row > amountData)
		return -1;
	
	if(isTome)
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
	
	if (isTome)
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
	
	if(isTome)
	{
		META_TOME element = ((META_TOME *) data)[rowIndex];
		if(element.ID != VALEUR_FIN_STRUCT)
		{
			if(element.readingName[0])
				output = [[NSString alloc] initWithBytes:element.readingName length:wstrlen(element.readingName) encoding:NSUTF32StringEncoding];
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
		[(RakCTContentTabView*) scrollView.superview gotClickedTransmitData: isTome : selectedIndex];
	}
}

#pragma mark - Drag and drop support

- (uint) getSelfCode
{
	return GUI_THREAD_CT;
}

- (MANGAS_DATA) getProjectDataForDrag : (uint) row
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
	
	if(isTome)
		selection = (((META_TOME *) data)[row]).ID;
	else
		selection = ((int *) data)[row];
	
	[item setDataProject:getCopyOfProjectData(projectData) isTome:isTome element:selection];
}

@end
