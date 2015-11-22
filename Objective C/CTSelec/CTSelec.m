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
 ********************************************************************************************/

@implementation CTSelec

- (instancetype) init : (NSView *) contentView : (NSString *) state
{
	self = [super init];
	if(self != nil)
	{
		flag = TAB_CT;
		[self initView:contentView : state];
		
		self.layer.borderColor = [Prefs getSystemColor:COLOR_TABS_BORDER].CGColor;
		self.layer.borderWidth = 2;
		
		[self initBackButton];
		
		if(state != nil && [state isNotEqualTo:STATE_EMPTY])
		{
			NSArray *componentsWithSpaces = [state componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
			NSArray *dataState = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
			
			if([dataState count] != 8 || ![self initCoreview : dataState])
				[self noContent];
		}
		else
			[self noContent];
		
		if(coreView != nil)
		{
			[self addSubview:coreView];
			[coreView focusViewChanged : self.mainThread];
			
			[self initMisc];
		}
		else
		{
			[backButton removeFromSuperview];
			backButton = nil;
			self = nil;
		}
	}
	return self;
}

- (void) initBackButton
{
	backButton = [[RakBackButton alloc] initWithFrame:_bounds: YES];
	if(backButton != nil)
	{
		[backButton setTarget : self];
		[backButton setAction : @selector(backButtonClicked)];
		backButton.alphaValue = self.mainThread == TAB_READER;
		backButton.hidden = self.mainThread != TAB_READER;
		
		[self addSubview:backButton];
	}
}

- (void) initMisc
{
	SRHeader = [[RakCTSerieHeader alloc] initWithFrame:[self headerFrame : _bounds]];
	if(SRHeader != nil)
	{
		if(self.mainThread != TAB_SERIES)
		{
			SRHeader.alphaValue = 0;
			SRHeader.hidden = YES;
		}
		
		[self addSubview:SRHeader];
	}
}

- (BOOL) initCoreview : (NSArray*) dataState
{
	PROJECT_DATA * project = getProjectFromSearch([getNumberForString([dataState objectAtIndex:0]) unsignedLongLongValue], [[dataState objectAtIndex:1] longLongValue], [[dataState objectAtIndex:2] boolValue], false);
	
	if(project == NULL)
	{
		NSLog(@"Couldn't find the project to restore, abort :/");
		return NO;
	}
	else
	{
		getCTInstalled(project, false);
		getCTInstalled(project, true);
	}
	
	//Perfect! now, all we have to do is to sanitize last few data :D
	BOOL isTome = [[dataState objectAtIndex:3] boolValue] != 0;
	long context[4];
	context[0] = [[dataState objectAtIndex:4] floatValue];		//elemSelectedChapter
	context[1] = [[dataState objectAtIndex:5] floatValue];		//scrollerPosChapter
	context[2] = [[dataState objectAtIndex:6] floatValue];		//elemSelectedVolume
	context[3] = [[dataState objectAtIndex:7] floatValue];		//scrollerPosVolume
	
	coreView = [[RakChapterView alloc] initContent:[self contentFrame : _bounds : backButton.frame.origin.y + backButton.frame.size.height] : *project : isTome : context];
	
	releaseCTData(*project);
	free(project);
	
	return YES;
}

- (void) noContent
{
	self.initWithNoContent = YES;
	coreView = [[RakChapterView alloc] initContent:[self contentFrame : _bounds : backButton.frame.origin.y + backButton.frame.size.height] :getEmptyProject() : NO : (long[4]){-1, -1, -1, -1}];
}

- (void) dealloc
{
	[backButton removeFromSuperview];		backButton = nil;
	[coreView removeFromSuperview];			coreView = nil;
}

- (void) backButtonClicked
{
	[self ownFocus];
}

- (uint) getFrameCode
{
	return PREFS_GET_TAB_CT_FRAME;
}

- (void) setUpViewForAnimation : (uint) mainThread
{
	if(mainThread != TAB_READER && !backButton.isHidden)
	{
		backButton.animator.alphaValue = 0;
	}
	else if(mainThread == TAB_READER && backButton.isHidden)
	{
		backButton.hidden = NO;
		backButton.animator.alphaValue = 1;
	}
	
	if(mainThread == TAB_SERIES)
	{
		SRHeader.hidden = NO;
		SRHeader.animator.alphaValue = 1;
	}
	else
	{
		SRHeader.animator.alphaValue = 0;
	}
	
	[coreView focusViewChanged : mainThread];
	
	[super setUpViewForAnimation:mainThread];
}

- (void) refreshDataAfterAnimation
{
	if(backButton.alphaValue == 0)
		backButton.hidden = YES;
	
	if(SRHeader.alphaValue == 0)
		SRHeader.hidden = YES;
	
	[coreView cleanupFocusViewChange];
	
	[super refreshDataAfterAnimation];
}

- (NSString *) byebye
{
	NSString * string;
	
	if((string = [coreView getContextToGTFO]) == nil)
	{
		return [super byebye];
	}
	else
		[self removeFromSuperview];
	
	return string;
}

- (void) updateProject : (PROJECT_DATA) project : (BOOL)isTome : (uint) element
{
	PROJECT_DATA newProject = getProjectByID(project.cacheDBID);	//Isole le tab des données
	
	if(newProject.isInitialized)
	{
		self.initWithNoContent = NO;
		[coreView updateContext:newProject];
		
		//Coreview en fait aussi une copie, on doit donc release cette version
		releaseCTData(newProject);
	}
}

- (void) resetTabContent
{
	self.initWithNoContent = YES;
	[coreView updateContext:getEmptyProject()];
}

#pragma mark - Reader code

- (BOOL) isStillCollapsedReaderTab
{
	uint state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_CT_FOCUS) == 0;
}

- (NSRect) generatedReaderTrackingFrame
{
	NSRect frame = [self lastFrame];
	NSSize svSize = self.superview.bounds.size;
	[Prefs getPref:PREFS_GET_TAB_READER_POSX : &frame.size.width : &svSize];
	
	frame.origin = NSZeroPoint;
	frame.size.width -= _lastFrame.origin.x;
	
	MDL * tabMDL = [self getMDL : YES];
	
	if(tabMDL != nil)
	{
		CGFloat MDLHeight = [tabMDL lastFrame].size.height - [tabMDL frame].origin.y - frame.origin.y;
		
		if(MDLHeight > 0)
		{
			frame.origin.y = MDLHeight;
			frame.size.height -= MDLHeight;
		}
	}
	
	return frame;
}

#pragma mark - Self code used in reader mode

- (NSRect) contentFrame : (NSRect) frame : (CGFloat) backButtonLowestY
{
	if(self.mainThread == TAB_READER)
	{
		CGFloat previousHeight = frame.size.height;
		
		frame.size.height -= backButtonLowestY + RBB_TOP_BORDURE + CT_READERMODE_BOTTOMBAR_WIDTH - TITLE_BALANCING_OFFSET;
		frame.origin.x = CT_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
		frame.origin.y = previousHeight - frame.size.height - CT_READERMODE_BOTTOMBAR_WIDTH;
		frame.size.width -= 2 * frame.origin.x;	//Pas obligé de recalculer
	}
	else if(self.mainThread == TAB_CT)
	{
		frame.origin = NSZeroPoint;
	}
	else
	{
		frame.origin.x = 0;
		frame.origin.y = SR_HEADER_HEIGHT_SINGLE_ROW - 3;
		frame.size.height -= frame.origin.y;
	}
	
	return frame;
}

- (NSRect) headerFrame : (NSRect) frame
{
	frame.origin = NSZeroPoint;
	frame.size.height = SR_HEADER_HEIGHT_SINGLE_ROW;
	
	return frame;
}

- (BOOL) needToConsiderMDL
{
	BOOL isReader;
	[Prefs getPref : PREFS_GET_IS_READER_MT : &isReader];
	
	return isReader;
}

- (void) resize : (NSRect) frame : (BOOL) animated
{
	frame.origin = NSZeroPoint;

	if(animated)
	{
		[backButton resizeAnimation:frame];
		[SRHeader setFrame:[self headerFrame : frame]];
		
		[coreView setFrame:[self contentFrame : frame : RBB_TOP_BORDURE + RBB_BUTTON_HEIGHT]];
	}
	else
	{
		[backButton setFrame:frame];
		[SRHeader setFrame:[self headerFrame:frame]];
		[coreView setFrame:[self contentFrame : [self lastFrame] : backButton.frame.origin.y + backButton.frame.size.height]];
	}
}

- (NSRect) getFrameOfNextTab
{
	NSRect output;
	NSSize sizeSuperview = self.superview.bounds.size;
	
	[Prefs getPref : PREFS_GET_TAB_READER_FRAME : &output : &sizeSuperview];
	
	return output;
}

#pragma mark - Communication with other tabs

- (void) updateContextNotification:(PROJECT_DATA) project : (BOOL)isTome : (uint) element
{
	if(element == INVALID_VALUE && project.isInitialized)
	{
		Reader *readerTab = [(RakAppDelegate*) [NSApp delegate] reader];
		MDL * MDLTab = [(RakAppDelegate*) [NSApp delegate] MDL];
		
		if(readerTab != nil && self.mainThread & TAB_READER)
		{
			__block NSRect readerFrame = readerTab.frame, MDLFrame = MDLTab.frame;
			CGFloat widthCTTab = readerFrame.origin.x - self.frame.origin.x;
			
			readerFrame.origin.x -= widthCTTab;
			readerFrame.size.width += widthCTTab;
			MDLFrame.size.width -= widthCTTab;
			
			[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
				[context setDuration:0.1];
				
				[readerTab.animator setFrame:readerFrame];
				
				if(MDLTab != nil)
					[MDLTab.animator setFrame:MDLFrame];
				
			} completionHandler:^{
				
				[CATransaction begin];
				[CATransaction setDisableActions:YES];
				
				[self updateProject : project : isTome : element];
				
				[CATransaction commit];
				
				if([Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
					[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
				
				else if(self.mainThread == TAB_READER)	//Fuck, we have to cancel the animation...
				{
					readerFrame.origin.x += widthCTTab;
					readerFrame.size.width -= widthCTTab;
					MDLFrame.size.width += widthCTTab;
					
					if(readerTab != nil)
						[readerTab setFrame:readerFrame];
					
					if(MDLTab != nil)
						[MDLTab setFrame:MDLFrame];
				}
			}];
			
		}
		
		else	//We bypass the fancy animation
		{
			[self updateProject : project : isTome : element];
			
			if(self.mainThread & TAB_READER && [Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
				[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
			
			//Oh, we got selected then, awesome :)
			else if(self.mainThread == TAB_SERIES)
			{
				[self ownFocus];
			}
		}
	}
}

#pragma mark - Proxy

- (void) selectElem : (uint) projectID : (BOOL) isTome : (uint) element
{
	[coreView selectElem : projectID : isTome : element];
}

- (PROJECT_DATA) activeProject
{
	return [coreView activeProject];
}

#pragma mark - Drop

- (BOOL) receiveDrop : (PROJECT_DATA) data : (BOOL) isTome : (uint) element : (uint) sender
{
	BOOL ret_value = NO;
	
	if(element == INVALID_VALUE || sender == TAB_MDL)
	{
		[coreView updateContext:data];
		ret_value = YES;
	}
	
	return ret_value;
}

- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL
{
	if(sender == TAB_SERIES || sender == TAB_MDL)
		return NSDragOperationCopy;
	
	return [super dropOperationForSender:sender:canDL];
}

@end
