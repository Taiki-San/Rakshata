/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakMDLList

- (id) init : (NSRect) frame
{
	self = [super init];
	
	if(self != nil)
	{
		amountData = 42;
		pause = [[RakButton allocForSeries : nil : @"pause" : NSMakePoint(0, 0) : nil : nil] retain];
		read = [[RakButton allocForSeries : nil : @"voir" : NSMakePoint(0, 0) : nil : nil] retain];
		remove = [[RakButton allocForSeries : nil : @"X" : NSMakePoint(0, 0) : nil : nil] retain];
		
		if(pause == nil || read == nil || remove == nil)
		{
			[self release];
			return nil;
		}
		
		controller = [[RakMDLController alloc] init];
		
		cellHeight = [remove frame].size.height;
		
		[self applyContext:frame : -1 : -1];
	}
	
	return self;
}

- (bool) didInitWentWell
{
	return true;
}

#pragma mark - Methods to deal with tableView

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return amountData;
}

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex >= amountData)
		return nil;
	
	NSString * output = @"Not so dummy request";
	
	return output;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    // Get an existing cell with the MyView identifier if it exists
    RakMDLListView *result = [tableView makeViewWithIdentifier:@"Mane 6" owner:self];
	
    // There is no existing cell to reuse so create a new one
    if (result == nil)
	{
		// Create the new NSTextField with a frame of the {0,0} with the width of the table.
		// Note that the height of the frame is not really relevant, because the row height will modify the height.
		result = [[RakMDLListView alloc] init:_tableView.frame.size.width :cellHeight :pause :read :remove :nil];

		[result setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
		
		result.identifier = @"Mane 6";
	}
	else
	{
		[result updateData : [self tableView:tableView objectValueForTableColumn:tableColumn row:row]];
	}
	
	// Return the result
	return result;
	
}

#pragma mark - Get result from NSTableView

- (BOOL) tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row;
{
	if(row < amountData)
	{
		NSLog(@"Great");
	}
	
	return NO;
}


@end
