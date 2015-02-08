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

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = TAB_CT;
		self = [self initView:contentView : state];
		_initWithNoContent = NO;
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS : self].CGColor;
		self.layer.borderWidth = 2;
		
		backButton = [[RakBackButton alloc] initWithFrame:[self bounds]: true];
		[backButton setTarget : self];
		[backButton setAction : @selector(backButtonClicked)];
		[backButton setHidden : self.mainThread != TAB_READER];
		
		[self addSubview:backButton];

		BOOL initFailure = YES;
		if(state != nil && [state isNotEqualTo:STATE_EMPTY])
		{
			NSArray *componentsWithSpaces = [state componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
			NSArray *dataState = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
			
			if([dataState count] == 7)
			{
				do
				{
					int indexTeam = getRepoIndexFromURL((char*)[[dataState objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding]);
					if(indexTeam == -1)
					{
						NSLog(@"Couldn't find the repo to restore, abort :/");
						break;
					}

					uint projectID = [[dataState objectAtIndex:1] longLongValue];	//We want uint, however, only int is available, so we rather want an overflow than an overflow exception
					PROJECT_DATA * project = getDataFromSearch(indexTeam, projectID, false);
					
					if(project == NULL)
					{
						NSLog(@"Couldn't find the project to restore, abort :/");
						break;
					}
					else
					{
						checkChapitreValable(project, NULL);
						checkTomeValable(project, NULL);
					}
					
					//Perfect! now, all we have to do is to sanitize last few data :D
					bool isTome = [[dataState objectAtIndex:2] boolValue] != 0;
					long context[4];
					context[0] = [[dataState objectAtIndex:3] floatValue];		//elemSelectedChapter
					context[1] = [[dataState objectAtIndex:4] floatValue];		//scrollerPosChapter
					context[2] = [[dataState objectAtIndex:5] floatValue];		//elemSelectedVolume
					context[3] = [[dataState objectAtIndex:6] floatValue];		//scrollerPosVolume
						
					coreView = [[RakChapterView alloc] initContent:[self calculateContentViewSize : [self frame] : backButton.frame.origin.y + backButton.frame.size.height] : *project : isTome : context];
					
					releaseCTData(*project);
					free(project);
					
					initFailure = NO;
				} while (0);
			}
		}
		
		if(initFailure)
		{
			[self noContent];
		}
		
		if(coreView != nil)
			[self addSubview:coreView];
		else
		{
			[backButton removeFromSuperview];
			backButton = nil;
			self = nil;
		}
	}
    return self;
}

- (void) noContent
{
	PROJECT_DATA empty;
	empty.isInitialized = NO;
	
	coreView = [[RakChapterView alloc] initContent:[self calculateContentViewSize : [self frame] : backButton.frame.origin.y + backButton.frame.size.height] :empty : NO : (long[4]){-1, -1, -1, -1}];
}

- (void) dealloc
{
	[backButton removeFromSuperview];		backButton = nil;
	[coreView removeFromSuperview];			coreView = nil;
}

- (void) backButtonClicked
{
	noDrag = true;
	[self mouseUp:NULL];
}

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_TAB_CT_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_TAB_CT_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_TAB_CT_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_TAB_CT_WIDTH;
			break;
		}
			
		case CONVERT_CODE_FRAME:
		{
			output = PREFS_GET_TAB_CT_FRAME;
			break;
		}
		
		default:
			output = 0;
	}
	
	return output;
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
	
	[coreView focusViewChanged : mainThread];
	
	[super setUpViewForAnimation:mainThread];
}

- (void) refreshDataAfterAnimation
{
	if(backButton.alphaValue == 0)
		backButton.hidden = YES;
	
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

- (void) updateProject : (PROJECT_DATA) project : (BOOL)isTome : (int) element
{
	PROJECT_DATA newProject = getElementByID(project.cacheDBID);	//Isole le tab des données
	
	if(newProject.isInitialized)
	{
		_initWithNoContent = NO;
		[coreView updateContext:newProject];
		
		//Coreview en fait aussi une copie, on doit donc release cette version
		releaseCTData(newProject);
	}
}

#pragma mark - Reader code
/**		Reader		**/

- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_CT_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize
{
	NSRect frame = _bounds;
	NSSize svSize = self.superview.bounds.size;
	[Prefs getPref:PREFS_GET_TAB_READER_POSX : &frame.size.width : &svSize];

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

- (NSRect) calculateContentViewSize : (NSRect) frame : (CGFloat) backButtonLowestY
{
	if(self.mainThread == TAB_READER)
	{
		CGFloat previousHeight = frame.size.height;
		
		frame.size.height -= backButtonLowestY + RBB_TOP_BORDURE + CT_READERMODE_BOTTOMBAR_WIDTH;
		frame.origin.x = CT_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
		frame.origin.y = previousHeight - frame.size.height - CT_READERMODE_BOTTOMBAR_WIDTH;
		frame.size.width -= 2* frame.origin.x;	//Pas obligé de recalculer
	}
	else
		frame.origin = NSZeroPoint;

	return frame;
}

- (BOOL) needToConsiderMDL
{
	BOOL isReader;
	[Prefs getPref : PREFS_GET_IS_READER_MT : &isReader];
	
	return isReader;
}

- (void) setFrame:(NSRect)frameRect
{
	if([self wouldFrameChange:frameRect])
	{
		[super setFrame : frameRect];
		
		frameRect.origin.x = frameRect.origin.y = 0;
		[backButton setFrame:frameRect];
		[coreView setFrame:[self calculateContentViewSize : [self lastFrame] : backButton.frame.origin.y + backButton.frame.size.height]];
	}
}

- (void) resizeAnimation
{
	NSRect frame = [self createFrame];
	
	if([self wouldFrameChange:frame])
	{
		[self.animator setFrame:frame];

		frame.origin = NSZeroPoint;
		[backButton resizeAnimation:frame];
		[coreView resizeAnimation:[self calculateContentViewSize : frame : RBB_TOP_BORDURE + RBB_BUTTON_HEIGHT]];
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

- (void) updateContextNotification:(PROJECT_DATA) project : (BOOL)isTome : (int) element
{
	if(element == VALEUR_FIN_STRUCT && project.repo != NULL)
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
				
				else	//Fuck, we have to cancel the animation...
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
			else if(self.mainThread & TAB_SERIES)
			{
				[self mouseDown:nil];
				[self mouseUp:nil];
			}
		}
	}
}

#pragma mark - Proxy

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element
{
	[coreView selectElem : projectID : isTome : element];
}

#pragma mark - Drop

- (BOOL) receiveDrop : (PROJECT_DATA) data : (bool) isTome : (int) element : (uint) sender
{
	BOOL ret_value = NO;
	
	if(element == VALEUR_FIN_STRUCT || sender == TAB_MDL)
	{
		[coreView updateContext:data];
		ret_value = YES;
	}
	
	return ret_value;
}

- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL
{
	if (sender == TAB_SERIES || sender == TAB_MDL)
		return NSDragOperationCopy;
	
	return [super dropOperationForSender:sender:canDL];
}

@end
