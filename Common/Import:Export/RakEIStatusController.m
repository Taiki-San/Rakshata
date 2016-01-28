/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

enum
{
	WINDOW_WIDTH = 350,
	WINDOW_HEIGHT = 150,

	BORDER_TOP = 10,
	BORDER_BUTTON = 15,

	PROGRESS_BASEX = 25,
	PROGRESS_WIDTH = 300,
	PROGRESS_HEIGHT = 20

};

@implementation RakEIStatusController

- (instancetype) init
{
	self = [super init];

	if(self != nil)
		[self startUI];

	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

- (void) startUI
{
	RakSheetView * mainView = [[RakSheetView alloc] initWithFrame:NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)];

	if(mainView != nil)
	{
		mainView.anchor = self;
		[Prefs registerForChange:self forType:KVO_THEME];

		mainView.backgroundColor = [self backgroundColor];
		[self setupBaseView:mainView];

		queryWindow = [[RakSheetWindow alloc] initWithContentRect:(NSRect) {{200, 200}, mainView.bounds.size} styleMask:0 backing:NSBackingStoreBuffered defer:YES];
		if(queryWindow != nil)
		{
			queryWindow.contentView = mainView;
			queryWindow.backgroundColor = [RakColor clearColor];

			[RakApp.window beginSheet:queryWindow completionHandler:^(NSModalResponse returnCode) {}];
			return;
		}
	}
}

- (void) closeUI
{
	[RakApp.window endSheet:queryWindow];
	queryWindow = nil;
}

- (void) setupBaseView : (RakView *) superview
{
	NSSize superviewSize = superview.bounds.size;
	CGFloat currentY = superviewSize.height;

	title = [[RakMenuText alloc] initWithText:NSZeroRect :[self headerText]];
	if(title != nil)
	{
		title.ignoreInternalFrameMagic = YES;
		title.barWidth = 0;
		title.haveBackgroundColor = NO;

		[title setFrameOrigin:NSMakePoint(superviewSize.width / 2 - title.bounds.size.width / 2, (currentY -= title.bounds.size.height + BORDER_TOP))];
		[superview addSubview:title];
	}

	progressBar = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(PROGRESS_BASEX, (currentY -= PROGRESS_HEIGHT + BORDER_TOP), PROGRESS_WIDTH, PROGRESS_HEIGHT)];
	if(progressBar != nil)
	{
		progressBar.usesThreadedAnimation = YES;
		progressBar.indeterminate = YES;

		[superview addSubview:progressBar];
	}

	percentage = [[RakText alloc] initWithText:NSLocalizedString(@"INITIALIZATION", nil) :[Prefs getSystemColor:COLOR_CLICKABLE_TEXT]];
	if(percentage != nil)
	{
		percentage.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:20];
		[percentage sizeToFit];
		percentage.alignment = NSTextAlignmentCenter;

		[percentage setFrameOrigin:NSMakePoint(superviewSize.width / 2 - percentage.bounds.size.width / 2, (currentY -= percentage.bounds.size.height + BORDER_TOP))];
		[superview addSubview:percentage];
	}

	cancel = [RakButton allocWithText:NSLocalizedString(@"CANCEL", nil)];
	if(cancel != nil)
	{
		cancel.target = self;
		cancel.action = @selector(cancel);

		[cancel setFrameOrigin:NSMakePoint(superviewSize.width / 2 - cancel.bounds.size.width / 2, (currentY -= cancel.bounds.size.height + BORDER_BUTTON))];
		[superview addSubview:cancel];
	}
}

- (NSString *) headerText
{
	return @"";
}

- (RakColor *) backgroundColor
{
	return [Prefs getSystemColor:COLOR_EXPORT_BACKGROUND];
}

#pragma mark - Refresh the status

- (void) setPosInEntry:(CGFloat)posInEntry
{
	_posInEntry = posInEntry;
	dispatch_async(dispatch_get_main_queue(), ^{
		[self refreshUI];
	});
}

- (void) setPosInExport:(CGFloat)posInExport
{
	_posInExport = posInExport;
	_posInEntry = 0;
	_nbElementInEntry = 1;
	dispatch_async(dispatch_get_main_queue(), ^{
		[self refreshUI];
	});
}

- (void) setNbElementToExport:(CGFloat)nbElementToExport
{
	_nbElementToExport = nbElementToExport;
	dispatch_async(dispatch_get_main_queue(), ^{	progressBar.indeterminate = NO;		});
}

- (void) refreshUI
{
	if(cachedPosInExport == _posInExport && cachedPosInEntry == _posInEntry)
		return;

	cachedPosInEntry = _posInEntry;
	cachedPosInExport = _posInExport;

	CGFloat percentageNumber = (_posInExport + _posInEntry / _nbElementInEntry) / _nbElementToExport, intermediaryPerc = round(percentageNumber * 1000);

	progressBar.doubleValue = intermediaryPerc / 10.0f;

	if(!_haveCanceled)
	{
		if(fmod(intermediaryPerc, 10.0f))
			percentage.stringValue = [NSString stringWithFormat:@"%.1f %%", intermediaryPerc / 10.0f];
		else
			percentage.stringValue = [NSString stringWithFormat:@"%d %%", (int) intermediaryPerc / 10];

		[percentage display];
	}

	[progressBar display];
}

- (void) finishing
{
	percentage.stringValue = NSLocalizedString(@"PROCESSING", nil);
	[self centeringPercentage];
}

- (void) cancel
{
	_haveCanceled = YES;

	progressBar.indeterminate = YES;
	percentage.stringValue = NSLocalizedString(@"CANCELED", nil);
	[self centeringPercentage];
}

#pragma mark UI

- (void) centeringPercentage
{
	[percentage sizeToFit];
	[percentage setFrameOrigin:NSMakePoint(percentage.superview.bounds.size.width / 2 - percentage.bounds.size.width / 2, percentage.frame.origin.y)];
	[percentage display];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	((RakSheetView *) queryWindow.contentView).backgroundColor = [self backgroundColor];
	percentage.textColor = [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
}

@end
