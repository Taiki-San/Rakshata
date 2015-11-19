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

@interface RakImportStatusController()
{
	RakMenuText * issueTitle;
	RakText * issueHeader;
	RakButton * replaceAll, * close;

	RakImportStatusList * outlineList;
	RakListScrollView * scrollview;

	NSArray * _dataSet;
}

@end

enum
{
	DELTA_WIDTH = 75,	//425 in total
	DELTA_HEIGHT = 306,

	LIST_HEIGHT = 350
};

@implementation RakImportStatusController

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Issue UI initialization

- (void) switchToIssueUI : (NSArray *) dataSet
{
	_dataSet = dataSet;

	[self setupIssueUI : dataSet];
	[self transition];
}

- (void) setupIssueUI : (NSArray *) dataSet
{
	NSView * superview = queryWindow.contentView;

	//Title of the new view
	issueTitle = [[RakMenuText alloc] initWithText:NSZeroRect :NSLocalizedString(@"IMPORT-REPORT", nil)];
	if(issueTitle != nil)
	{
		issueTitle.ignoreInternalFrameMagic = YES;
		issueTitle.barWidth = 0;
		issueTitle.haveBackgroundColor = NO;

		[superview addSubview:issueTitle];
	}
	
	if(issueHeader == nil)
	{
		issueHeader = [[RakText alloc] initWithText:[self secondaryHeaderText] :[self textColor]];
		if(issueHeader != nil)
			[superview addSubview:issueHeader];

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(refreshUI) name:NOTIFICATION_IMPORT_STATUS_UI object:nil];
	}
	else
		[self refreshUI];

	outlineList = [[RakImportStatusList alloc] initWithImportList:dataSet];
	if(outlineList != nil)
	{
		outlineList.controller = self;

		RakTreeView * view = [outlineList getContent];
		view.manualEventDispatching = YES;
				
		scrollview = [[RakListScrollView alloc] initWithFrame:NSMakeRect(0, 0, view.bounds.size.width, LIST_HEIGHT)];
		if(scrollview != nil)
		{
			scrollview.drawsBackground = YES;
			scrollview.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_COREVIEW :nil];
			scrollview.documentView = view;
			[superview addSubview:scrollview];

			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(replaceItem:) name:NOTIFICATION_IMPORT_REPLACE_ONE object:nil];
			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(replaceRootWithChild:) name:NOTIFICATION_IMPORT_REPLACE_ALL object:nil];
		}
	}

	replaceAll = [RakButton allocWithText:NSLocalizedString(@"REPLACE-ALL", nil)];
	if(replaceAll != nil)
	{
		replaceAll.hidden = !outlineList.haveDuplicate;
		replaceAll.target = self;
		replaceAll.action = @selector(replaceAll);
		[superview addSubview:replaceAll];
	}

	close = [RakButton allocWithText:NSLocalizedString(@"CLOSE", nil)];
	if(close != nil)
	{
		close.target = self;
		close.action = @selector(close);
		[superview addSubview:close];
	}

	[self setPositioningOfIssueUI];
}

- (void) setPositioningOfIssueUI
{
	//This method is first called to move all the UI out of the display in order for the animation to run smoothly.
	//However, this only happen once and the following calls only need to update the frame
	
	NSSize oldSize = queryWindow.contentView.bounds.size;
	CGFloat deltaWidth = updatedFrameOnce ? 0 : DELTA_WIDTH, deltaHeight = updatedFrameOnce ? 0 : DELTA_HEIGHT, offsetHeight;

	if(updatedFrameOnce)
	{
		deltaHeight = 0;
		deltaWidth = 0;
		offsetHeight = 0;
	}
	else
	{
		deltaHeight = DELTA_HEIGHT;
		deltaWidth = DELTA_WIDTH;
		offsetHeight = oldSize.height;
		updatedFrameOnce = YES;
	}
	
	CGFloat halfWidth = (oldSize.width + deltaWidth) / 2.0f, oldHeight = oldSize.height, currentY = oldHeight + deltaHeight;
	
	[issueTitle setFrameOrigin:NSMakePoint(halfWidth - issueTitle.bounds.size.width / 2, offsetHeight + (currentY -= issueTitle.bounds.size.height + 5))];
	[issueHeader setFrameOrigin:NSMakePoint(halfWidth - issueHeader.bounds.size.width / 2, offsetHeight + (currentY -= issueHeader.bounds.size.height + 5))];
	[scrollview setFrameOrigin:NSMakePoint(halfWidth - scrollview.bounds.size.width / 2, offsetHeight + (currentY -= scrollview.bounds.size.height + 10))];

	currentY /= 2;

	if(replaceAll != nil && !replaceAll.isHidden)
	{
		[replaceAll setFrameOrigin:NSMakePoint(halfWidth / 2 - replaceAll.bounds.size.width / 2, offsetHeight + currentY - replaceAll.bounds.size.height / 2)];
		[close setFrameOrigin:NSMakePoint(halfWidth + halfWidth / 2 - close.bounds.size.width / 2, offsetHeight + currentY - close.bounds.size.height / 2)];
	}
	else
		[close setFrameOrigin:NSMakePoint(halfWidth - close.bounds.size.width / 2, offsetHeight + currentY - close.bounds.size.height / 2)];
}

- (void) transition
{
	//First, we increase our width
	__block NSRect windowFrame = queryWindow.frame, frame;
	const NSSize oldWindowSize = windowFrame.size;

	[NSAnimationContext runAnimationGroup:^(NSAnimationContext * __nonnull context) {

		windowFrame.size.width += DELTA_WIDTH;
		[queryWindow setFrame:windowFrame display:YES animate:YES];
		
		//Animate the old entities
		for(NSView * view in @[title, progressBar, percentage, cancel])
		{
			frame = view.frame;
			frame.origin.x = windowFrame.size.width / 2 - frame.size.width / 2;
			view.animator.frame = frame;
		}

	} completionHandler:^{

		//Then, animate our height
		windowFrame.origin.y -= DELTA_HEIGHT / 2 - 4;
		windowFrame.size.height += DELTA_HEIGHT;
		[queryWindow setFrame:windowFrame display:YES animate:YES];

		[NSAnimationContext beginGrouping];

		NSView * views[] = {title, progressBar, percentage, cancel, issueTitle, issueHeader, replaceAll, close, scrollview};

		//Animate the old entities
		for(uint i = 0; i < 9; i++)
		{
			frame = views[i].frame;
			frame.origin.y -= oldWindowSize.height;
			views[i].animator.frame = frame;
		}
		
		//Try to fix the bug that made the window display the expired, midanimation, UI
		[[NSAnimationContext currentContext] setCompletionHandler:^{
			[queryWindow.contentView setNeedsDisplay:YES];
		}];

		[NSAnimationContext endGrouping];
	}];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	scrollview.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_COREVIEW :nil];
}

#pragma mark - Text management

- (NSString *) headerText
{
	return NSLocalizedString(@"IMPORTING", nil);
}

- (NSString *) secondaryHeaderText
{
	//Header, explaining what is going on
	uint nbIssues = 0;

	//We count the number of issues to show them in the UI
	for(RakImportItem * item in _dataSet)
	{
		if(item.issue != IMPORT_PROBLEM_NONE)
			nbIssues++;
	}

	if(nbIssues > 1)
		return [NSString localizedStringWithFormat:NSLocalizedString(@"IMPORT-%zu-PBM", nil), nbIssues];
	else if(nbIssues == 1)
		return NSLocalizedString(@"IMPORT-ONE-PBM", nil);

	return @"Yay, everything is okay (shouldn't appear)";
}

- (void) refreshUI
{
	issueHeader.stringValue = [self secondaryHeaderText];

	if(replaceAll.isHidden == outlineList.haveDuplicate)
	{
		replaceAll.hidden = !replaceAll.isHidden;
		[self setPositioningOfIssueUI];
	}
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil];
}

#pragma mark - Helper

- (NSData *) queryThumbOf : (RakImportItem *) item withIndex : (uint) index
{
	return [item queryThumbInWithIndex:index];
}

- (BOOL) reflectMetadataUpdate : (PROJECT_DATA) project withImages : (NSArray *) overridenImages forItem : (RakImportItem *) item
{
	char * referencePath = getPathForProject(item.projectData.data.project);
	if(referencePath == NULL)
		return NO;

	//Install the overriden images
	for(NSDictionary * dict in overridenImages)
	{
		byte retinaCode = [[dict objectForKey:@"code"] unsignedCharValue], standardCode = retinaCode - retinaCode % 2;
		NSImage * baseImage = [dict objectForKey:@"data"];

		NSSize retinaSize = thumbSizeForID(retinaCode), standardSize = thumbSizeForID(standardCode);

		exportImageToPath(baseImage, standardSize, retinaSize, [NSString stringWithUTF8String:getPathToIconsOfProject(project, retinaCode).string]);
		exportImageToPath(baseImage, standardSize, standardSize, [NSString stringWithUTF8String:getPathToIconsOfProject(project, standardCode).string]);
	}

	//We look for items from the same project
	for(RakImportItem * currentItem in _dataSet)
	{
		if(currentItem.issue != IMPORT_PROBLEM_METADATA)
			continue;

		char * currentPath = getPathForProject(currentItem.projectData.data.project);
		if(currentPath == NULL)
			continue;

		if(!strcmp(referencePath, currentPath))
			[currentItem updateProject:project];

		free(currentPath);
	}

	free(referencePath);

	[self postProcessUpdate];

	return YES;
}

#pragma mark - Button responder

- (void) replaceAll
{
	BOOL stillHaveAnError = NO;

	for(RakImportItem * item in _dataSet)
	{
		//If not a duplicate issue, we check if it's okay if we haven't anything
		//that would prevent us from getting away (missing metadata)
		if(![item overrideDuplicate] && !stillHaveAnError)
			stillHaveAnError |= item.issue != IMPORT_PROBLEM_NONE;
	}

	[RakImportStatusList refreshAfterPass];

	if(!stillHaveAnError)
		[self close];
}

- (void) replaceRootWithChild : (NSNotification *) notification
{
	RakImportItem * item = notification.object;
	if(![item isKindOfClass:[RakImportItem class]])
		return;

	char * referencePath = getPathForProject(item.projectData.data.project);
	if(referencePath == NULL)
		return;

	//We look for items from the same project
	BOOL updatedOne = NO;
	for(RakImportItem * currentItem in _dataSet)
	{
		if(currentItem.issue != IMPORT_PROBLEM_DUPLICATE)
			continue;

		char * currentPath = getPathForProject(currentItem.projectData.data.project);
		if(currentPath == NULL)
			continue;

		if(!strcmp(referencePath, currentPath))
		{
			updatedOne |= [currentItem overrideDuplicate];
		}

		free(currentPath);
	}

	free(referencePath);

	if(updatedOne)
		[self postProcessUpdate];
}

- (void) replaceItem : (NSNotification *) notification
{
	RakImportItem * item = notification.object;
	if(![item isKindOfClass:[RakImportItem class]])
		return;

	[item overrideDuplicate];

	[self postProcessUpdate];
}

- (void) postProcessUpdate
{
	[RakImportStatusList refreshAfterPass];
	[self checkIfStillHaveError];
}

- (void) checkIfStillHaveError
{
	BOOL stillHaveAnError = NO;

	for(RakImportItem * itemIter in _dataSet)
	{
		stillHaveAnError |= itemIter.issue != IMPORT_PROBLEM_NONE;

		if(stillHaveAnError)
			break;
	}
	if(!stillHaveAnError)
		[self close];
}

- (void) close
{
	outlineList.query = nil;
	[RakImportController postProcessingUI:self];
}

@end

