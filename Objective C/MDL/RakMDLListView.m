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
	REQUEST_OFFSET_WIDTH = 50,
	Y_OFFSET_MULTILINE_TEXT = 5,
	Y_OFFSET_MULTILINE_BUTTON = 2,
};

@implementation RakMDLListView

- (instancetype) init : (id) controller : (uint) rowID
{
	self = [self initWithFrame: NSZeroRect];
	
	if(self != nil)
	{
		isSecondTextHidden = YES;
		_controller = controller;
		_row = rowID;
		_invalidData = NO;
		wasMultiLine = _controller.isSerieMainThread;	//We want to trigger the code updating wasMultiLine
		
		todoList = [_controller getData: _row : YES];
		if(todoList == NULL || *todoList == NULL)
		{
			_invalidData = YES;
			return nil;
		}

		previousStatus = MDL_CODE_UNUSED;
		
		self.autoresizesSubviews = NO;
		
		requestName = [[RakText alloc] initWithText:self.bounds : @"Dangos are awesome" : [Prefs getSystemColor:GET_COLOR_INACTIVE : self]];
		if(requestName != nil)		[self addSubview:requestName];
		
		statusText = [[RakText alloc] initWithText:self.bounds : NSLocalizedString(@"MDL-INSTALLING", nil) : [Prefs getSystemColor:GET_COLOR_ACTIVE : nil]];
		if(statusText != nil)		{		[statusText sizeToFit];			[self addSubview:statusText];		}
		
		[self initIcons];
		
		DLprogress = [[RakProgressBar alloc] initWithFrame: NSMakeRect([RakProgressBar getLeftBorder], 0, self.frame.size.width - ([RakProgressBar getLeftBorder] + [RakProgressBar getRightBorder]), self.frame.size.height)];
		if(DLprogress != nil){	[self addSubview:DLprogress];	[DLprogress setHidden:YES];	}
		
		iconWidth = [_remove frame].size.width;
		
		[self multilineStateUpdated];	//Update everything depending of the context
		
		self.postsFrameChangedNotifications = YES;
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(rowDeleted:) name:@"RakMDLListViewRowDeleted" object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(frameChanged) name:NSViewFrameDidChangeNotification object:self];
	}
	
	return self;
}

- (void) dealloc
{
	[_pause removeFromSuperview];
	[_read removeFromSuperview];
	[_remove removeFromSuperview];
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) initIcons
{
	if(_pause == nil)
	{
		_pause = [RakButton allocImageWithoutBackground : @"pause" : RB_STATE_STANDARD : self : @selector(sendPause)];
		if(_pause != nil)
		{
			[_pause setButtonType:NSMomentaryChangeButton];
			[self addSubview:_pause];
			[_pause setHidden:YES];
		}
	}
	
	if(_read == nil)
	{
		_read = [RakButton allocImageWithoutBackground : @"voir" : RB_STATE_STANDARD : self : @selector(sendRead)];
		if(_read != nil)
		{
			[_read setButtonType:NSMomentaryChangeButton];
			[self addSubview:_read];
			[_read setHidden:YES];
			[_read.cell setHighlightAllowed: NO];
		}
	}
	
	if(_remove == nil)
	{
		_remove = [RakButton allocImageWithoutBackground : @"X" : RB_STATE_STANDARD : self : @selector(sendRemove)];
		if(_remove != nil)
		{
			[_remove setButtonType:NSMomentaryChangeButton];
			[self addSubview:_remove];
			[_remove setHidden:NO];
			[_remove.cell setHighlightAllowed: NO];
		}
	}
}

- (NSString *) getName
{
	NSString * localized;
	
	if((*todoList)->listChapitreOfTome == NULL)
	{
		if((*todoList)->identifier % 10)
			localized = [NSString stringWithFormat:NSLocalizedString(@"CHAPTER-%d.%d", nil), (*todoList)->identifier / 10, (*todoList)->identifier % 10];
		else
			localized = [NSString stringWithFormat:NSLocalizedString(@"CHAPTER-%d", nil), (*todoList)->identifier / 10];
	}
	else
	{
		if((*todoList)->tomeName != NULL && (*todoList)->tomeName[0] != 0)
		{
			localized = getStringForWchar((*todoList)->tomeName);
		}
		else
			localized = [NSString stringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), (*todoList)->identifier];
	}
	
	if(wasMultiLine)
		return [NSString stringWithFormat:@"%@\n%@", getStringForWchar((*todoList)->datas->projectName), localized];

	return [NSString stringWithFormat:@"%@ - %@", getStringForWchar((*todoList)->datas->projectName), localized];
}

- (void) setFont : (NSFont*) font
{
	[requestName setFont:font];		[requestName sizeToFit];
	[statusText setFont:font];		[statusText sizeToFit];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[requestName setTextColor:[Prefs getSystemColor:GET_COLOR_INACTIVE :nil]];
	[statusText setTextColor:[Prefs getSystemColor:GET_COLOR_ACTIVE :nil]];
	
	[_pause removeFromSuperview];	_pause = nil;
	[_read removeFromSuperview];	_read = nil;
	[_remove removeFromSuperview];	_remove = nil;
	
	[self initIcons];
}

#pragma mark - Data update

- (void) multilineStateUpdated
{
	if(wasMultiLine)
		DLprogress.offsetYSpeed = Y_OFFSET_MULTILINE_BUTTON + 1;
	else
		DLprogress.offsetYSpeed = -1;
	
	[requestName setStringValue : [self getName]];
	[requestName sizeToFit];
}

- (void) setPositionsOfStuffs
{
	NSRect frame = _bounds, curFrame;
	NSPoint newPoint;
	
	if(wasMultiLine != (frame.size.height > 30 && frame.size.width < 300))
	{
		wasMultiLine = !wasMultiLine;

		[self multilineStateUpdated];
	}
	
	//Text at extreme left
	if (requestName != nil)
	{
		curFrame = requestName.frame;
		
		if(NSEqualRects(curFrame, NSZeroRect))
		{
			[requestName sizeToFit];
			curFrame = requestName.frame;
		}
		
		curFrame.size.width = frame.size.width - REQUEST_OFFSET_WIDTH;
		
		if(wasMultiLine)
			curFrame.origin.y = Y_OFFSET_MULTILINE_TEXT;
		else
			curFrame.origin.y = frame.size.height / 2 - curFrame.size.height / 2;
		
		curFrame.origin.x = 5;
		
		[requestName setFrame:curFrame];
	}
	
	newPoint.x = frame.size.width - 3;
	
	
	//Icon at extreme right
	if (_remove != nil)
	{
		curFrame = _remove.frame;
		
		if(wasMultiLine)
		{
			newPoint.y = Y_OFFSET_MULTILINE_BUTTON + _remove.frame.size.height;
			newPoint.x = frame.size.width - curFrame.size.width - 6;
		}
		else
		{
			newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
			newPoint.x -= 5 + curFrame.size.width;
		}
		
		[_remove setFrameOrigin:newPoint];
	}
	
	// Complementary icon
	if(_read != nil)
	{
		curFrame = _read.frame;

		if(wasMultiLine)
		{
			newPoint.y = Y_OFFSET_MULTILINE_BUTTON;
			newPoint.x = frame.size.width - curFrame.size.width - 5;
		}
		else
		{
			newPoint.x -= 5 + curFrame.size.width;
			newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		}
		
		
		[_read setFrameOrigin:newPoint];
	}
	
	
	if (_pause != nil)
	{
		curFrame = _pause.frame;
		
		if(wasMultiLine)
			newPoint.y = Y_OFFSET_MULTILINE_BUTTON;
		else
			newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		
		if(_read == nil)
		{
			if(wasMultiLine)
				newPoint.x = frame.size.width - curFrame.size.width - 5;
			else
				newPoint.x -= 5 + curFrame.size.width;
		}
		
		[_pause setFrameOrigin:newPoint];
	}
	
	if(DLprogress != nil)
	{
		curFrame = frame;
		
		curFrame.origin.x = [RakProgressBar getLeftBorder];
		curFrame.size.width -= [RakProgressBar getLeftBorder] + [RakProgressBar getRightBorder];

		[DLprogress setFrame:curFrame];
		[DLprogress setRightTextBorder: newPoint.x - 10];
		[DLprogress centerText];
	}
	
	if(statusText != nil)
	{
		curFrame = statusText.frame;
		
		if(wasMultiLine)
			newPoint.y = Y_OFFSET_MULTILINE_TEXT;
		else
			newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;

		newPoint.x = (frame.size.width - 3) - (_remove != nil ? (5 + _remove.frame.size.width) : 0) - (5 + curFrame.size.width);
		
		if(requestName == nil || requestName.frame.size.width + requestName.frame.origin.x + 25 < newPoint.x)
			[statusText setFrameOrigin:newPoint];
		else if([statusText isHidden] == NO)
			[statusText setHidden:YES];
	}
}

- (void) frameChanged
{
	[self setPositionsOfStuffs];
}

- (void) updateData : (uint) data
{
	_row = data;
	
	todoList = [_controller getData : _row : YES];
	if(todoList == NULL || *todoList == NULL)
	{
		_invalidData = YES;
		return;
	}
	else
		_invalidData = NO;
	
	[requestName setStringValue : [self getName]];

	[_pause.cell setState: ((*todoList)->downloadSuspended & DLSTATUS_SUSPENDED ? RB_STATE_HIGHLIGHTED : RB_STATE_STANDARD)];
	[_pause setNeedsDisplay : YES];
	[_remove.cell setState:RB_STATE_STANDARD];
	
	previousStatus = MDL_CODE_UNUSED;
	
	[self updateContext];
	
	[self setPositionsOfStuffs];
}

#pragma mark - Delete row

- (void) removeRowFromList
{
	NSView * tableView = self;
	while (tableView != nil && [tableView class] != [RakTableView class])
		tableView = tableView.superview;
	
	if(tableView != nil)
		[(RakTableView*) tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndex : _row] withAnimation:NSTableViewAnimationSlideLeft];
	
	MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate] MDL];
	if(tabMDL != nil)
	{
		NSRect lastFrame = [tabMDL lastFrame], newFrame = [tabMDL createFrame];
		
		if(!NSEqualRects(lastFrame, newFrame))
			[tabMDL fastAnimatedRefreshLevel : tabMDL.superview];
	}
	
	//Now, we can send a notification to update _row counters
	[[NSNotificationCenter defaultCenter] postNotificationName: @"RakMDLListViewRowDeleted" object:self userInfo:@{ @"deletedRow" : @(_row)}];
}

- (void) rowDeleted : (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];

	NSNumber * deletedRow = [userInfo objectForKey:@"deletedRow"];

	if(deletedRow != nil && _row > [deletedRow unsignedIntValue])
		_row--;
}

#pragma mark - View management

- (void) updateContext
{
	int8_t newStatus = [_controller statusOfID : _row : YES];
	
	if(newStatus == previousStatus)
		return;
	
	if(![statusText isHidden])		[statusText setHidden:YES];
	if(![_pause isHidden])			[_pause setHidden:YES];
	if(![_read isHidden])			[_read setHidden:YES];
	if(![DLprogress isHidden])		[DLprogress setHidden:YES];
	
	previousStatus = newStatus;

	switch (newStatus)
	{
		case MDL_CODE_DL:
		{
			[_pause setHidden:NO];				[_pause display];
			[DLprogress setHidden:NO];
			[DLprogress updatePercentage:0 :0];	[DLprogress display];
			break;
		}
			
		case MDL_CODE_INSTALL:
		{
			[statusText setHidden:NO];
			[self setPositionsOfStuffs];
			[statusText display];
			break;
		}
			
		case MDL_CODE_INSTALL_OVER:
		{
			[_read setHidden:NO];
			[_read display];
			break;
		}
	}

	//Force them to refresh, otherwise, we may end up with an outdated, pressed button
	[_remove setNeedsDisplay];
	[_read setNeedsDisplay];
	[_pause setNeedsDisplay];
	
	[self setNeedsDisplay:YES];
}

- (void) requestReloadData : (NSTableView *) tableView
{
	[tableView reloadData];
}

#pragma mark - Proxy

- (void) updatePercentage : (CGFloat) percentage : (size_t) speed
{
	if(DLprogress != nil)
		[DLprogress performSelectorOnMainThread:@selector(updatePercentageProxy:) withObject:@[@(percentage), @(speed)] waitUntilDone:YES];
}

//We will abort the download, notify the controller, notify the main dispatcher, and disappear from the table
//If we were already installed, we won't uninstall

- (BOOL) abortProcessing
{
	if(todoList == nil)
		return false;
	
	int8_t status = [_controller statusOfID:_row :YES];

	(*todoList)->downloadSuspended |= DLSTATUS_ABORT;	//Send the code to stop the download
	
	if((*todoList)->downloadSuspended & DLSTATUS_SUSPENDED && (*todoList)->curlHandler != NULL)
	{
		curl_easy_pause((*todoList)->curlHandler, CURLPAUSE_CONT);
	}
	
	[_controller setStatusOfID: _row : YES : MDL_CODE_ABORTED];
	(*todoList)->rowViewResponsible = NULL;
	
	return status == MDL_CODE_DL;
}

- (void) sendRemove
{
	int8_t status = [_controller statusOfID:_row :YES];

	[self abortProcessing];
	
	[_controller discardElement: _row withSimilar: status == MDL_CODE_INSTALL_OVER];

	if(status == MDL_CODE_DL)
		MDLDownloadOver(false);
}

- (void) sendPause
{
	if((*todoList)->curlHandler != NULL)
	{
		if((*todoList)->downloadSuspended & DLSTATUS_SUSPENDED)
		{
			curl_easy_pause((*todoList)->curlHandler, CURLPAUSE_CONT);
			[_pause.cell setState:RB_STATE_STANDARD];
		}
		else
		{
			curl_easy_pause((*todoList)->curlHandler, CURLPAUSE_ALL);
			[_pause.cell setState:RB_STATE_HIGHLIGHTED];
		}
		
		(*todoList)->downloadSuspended ^= DLSTATUS_SUSPENDED;
	}
}

- (void) sendRead
{
	[[(RakAppDelegate*) [NSApp delegate] MDL] propagateContextUpdate:*(*todoList)->datas :(*todoList)->listChapitreOfTome != NULL :(*todoList)->identifier];

	[_controller discardElement: _row withSimilar: YES];
}

@end
