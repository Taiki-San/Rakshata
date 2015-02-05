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

#include "db.h"

@implementation RakSRSearchList

- (instancetype) init : (NSRect) frame : (byte) type
{
	self = [self init];
	
	if(self != nil)
	{
		_type = type;
		
		if(_type != RDBS_TYPE_SOURCE)
		{
			indexes = getSearchData(type, (charType ***) &_data, &_nbData);
			if(indexes == NULL)
				return nil;
		}
		else
		{
			BOOL fail = NO;
			uint nbElem;
			REPO_DATA ** repo = (REPO_DATA **) getCopyKnownRepo(&nbElem, NO);
			
			if(repo != NULL)
			{
				uint64_t * _indexes = malloc(nbElem * sizeof(uint64_t));
				charType ** output = malloc(nbElem * sizeof(charType *));
				if(output != NULL && _indexes != NULL)
				{
					for(uint i = 0; i < nbElem; i++)
					{
						output[i] = malloc(REPO_NAME_LENGTH * sizeof(charType));
						if(output[i] == NULL)
						{
							while (i-- > 0)
								free(output[i]);
							
							fail = YES;
							break;
						}
						wstrncpy(output[i], REPO_NAME_LENGTH, repo[i]->name);
						_indexes[i] = getRepoID(repo[i]);
					}
					
					if(fail)
						freeRepo(repo);
					else
					{
						_nbData = nbElem;
						_data = output;
						indexes = _indexes;
					}
				}
				else
				{
					free(_indexes);
					free(output);
					freeRepo(repo);
				}
			}
		}
		
		[self applyContext:frame : selectedRowIndex : -1];
		
		selection = [NSMutableArray array];
		
		_tableView.rowHeight = 14;
	}
	
	return self;
}

- (BOOL) didInitWentWell
{
	return YES;
}

#pragma mark - Data manipulation

- (SR_DATA *) getSmartReloadData : (PROJECT_DATA*) data : (uint) nbElem : (bool *) installed
{
	if(!nbElem)
		return NULL;
	
	SR_DATA * output = calloc(nbElem, sizeof(SR_DATA));
	
	if(output != NULL)
	{
		for(uint i = 0; i < nbElem; i++)
		{
			output[i].data = indexes[i];
		}
	}
	
	return output;
}

#pragma mark - tableview

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return _data == NULL ? 0 : _nbData;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	RakText * view = (RakText *) [super tableView:tableView viewForTableColumn:tableColumn row:row];
	
	if(view != nil)
	{
		if([selection indexOfObject:@(row)] != NSNotFound)
		{
			view.textColor = (highlight != nil ? highlight : [self getTextHighlightColor:0 :row]);
			view.drawsBackground = YES;
		}
		
		view.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:11];
	}
	
	return view;
}

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex < _nbData)
	{
		return getStringForWchar(((charType **) _data)[rowIndex]);
	}
	else
		return @"Error D:";
}

//Selection

- (BOOL) tableView:(RakTableView *)tableView shouldSelectRow:(NSInteger)rowIndex
{
	[tableView commitClic];
	
	RakText * view = [tableView viewAtColumn:0 row:rowIndex makeIfNecessary:NO];
	if(view != nil && [view class] == [RakText class])
	{
		lastWasSelection = !view.drawsBackground;
		if(lastWasSelection)
		{
			view.textColor = (highlight != nil ? highlight : [self getTextHighlightColor:0 :rowIndex]);
			view.drawsBackground = YES;
			[selection addObject:@(rowIndex)];
		}
		else
		{
			view.textColor = (normal != nil ? normal : [self getTextColor:0 :rowIndex]);
			view.drawsBackground = NO;
			[selection removeObject:@(rowIndex)];
		}

		selectedRowIndex = rowIndex;
		[view setNeedsDisplay];
		
		[self postProcessingSelection];
	}
	else
		selectedRowIndex = LIST_INVALID_SELECTION;
	
	return NO;
}

- (void) postProcessingSelection
{
	if(selectedRowIndex != LIST_INVALID_SELECTION)
	{
		NSString * notificationName;

		if(_type == RDBS_TYPE_AUTHOR)
			notificationName = SR_NOTIFICATION_AUTHOR;

		else if(_type == RDBS_TYPE_SOURCE)
			notificationName = SR_NOTIFICATION_SOURCE;
		
		else if(_type == RDBS_TYPE_TAG)
			notificationName = SR_NOTIFICATION_TAG;
		
		else if(_type == RDBS_TYPE_TYPE)
			notificationName = SR_NOTIFICATION_TYPE;
		
		else
			return;
		
		[[NSNotificationCenter defaultCenter] postNotificationName:notificationName object:getStringForWchar(((charType **) _data)[selectedRowIndex]) userInfo:@{SR_NOTIF_CACHEID : @(indexes[selectedRowIndex]), SR_NOTIF_OPTYPE : @(lastWasSelection)}];
	}
}

@end
