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

- (void) addUnzFile : (unzFile *) unzipFile
{
	file = unzipFile;
}

- (NSString *) headerText
{
	return NSLocalizedString(@"IMPORTING", nil);
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

	//Header, explaining what is going on
	NSString * headerText;
	uint nbIssues = 0;

	//We count the number of issues to show them in the UI
	for(RakImportItem * item in dataSet)
	{
		if(item.issue != IMPORT_PROBLEM_NONE)
			nbIssues++;
	}

	if(nbIssues > 1)
		headerText = [NSString localizedStringWithFormat:NSLocalizedString(@"IMPORT-%zu-PBM", nil), nbIssues];
	else
		headerText = NSLocalizedString(@"IMPORT-ONE-PBM", nil);

	issueHeader = [[RakText alloc] initWithText:headerText :[self textColor]];
	if(issueHeader != nil)
		[superview addSubview:issueHeader];

	outlineList = [[RakImportStatusList alloc] initWithImportList:dataSet];
	if(outlineList != nil)
	{
		NSView * view = [outlineList getContent];
		scrollview = [[RakListScrollView alloc] initWithFrame:NSMakeRect(0, 0, view.bounds.size.width, LIST_HEIGHT)];
		if(scrollview != nil)
		{
			scrollview.drawsBackground = YES;
			scrollview.backgroundColor = [[NSColor blackColor] colorWithAlphaComponent:0.3];
			scrollview.documentView = view;
			[superview addSubview:scrollview];
		}
	}

	replaceAll = [RakButton allocWithText:NSLocalizedString(@"REPLACE-ALL", nil)];
	if(replaceAll != nil)
	{
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
	NSSize oldSize = queryWindow.contentView.bounds.size;
	CGFloat halfWidth = (oldSize.width + DELTA_WIDTH) / 2.0f, oldHeight = oldSize.height, currentY = oldHeight + DELTA_HEIGHT;

	[issueTitle setFrameOrigin:NSMakePoint(halfWidth - issueTitle.bounds.size.width / 2, oldHeight + (currentY -= issueTitle.bounds.size.height + 10))];
	[issueHeader setFrameOrigin:NSMakePoint(halfWidth - issueHeader.bounds.size.width / 2, oldHeight + (currentY -= issueHeader.bounds.size.height + 5))];
	[scrollview setFrameOrigin:NSMakePoint(halfWidth - scrollview.bounds.size.width / 2, oldHeight + (currentY -= scrollview.bounds.size.height + 10))];

	currentY /= 2;

	[replaceAll setFrameOrigin:NSMakePoint(halfWidth / 2 - replaceAll.bounds.size.width / 2, oldHeight + currentY - replaceAll.bounds.size.height / 2)];
	[close setFrameOrigin:NSMakePoint(halfWidth + halfWidth / 2 - close.bounds.size.width / 2, oldHeight + currentY - close.bounds.size.height / 2)];
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

		//Animate the old entities
		for(NSView * view in @[title, progressBar, percentage, cancel, issueTitle, issueHeader, replaceAll, close, scrollview])
		{
			frame = view.frame;
			frame.origin.y -= oldWindowSize.height;
			view.animator.frame = frame;
		}

		[NSAnimationContext endGrouping];
	}];
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil];
}

#pragma mark - Button responder

- (void) replaceAll
{

}

- (void) close
{
	[RakImportController postProcessing:file withUI:self];
}

@end

