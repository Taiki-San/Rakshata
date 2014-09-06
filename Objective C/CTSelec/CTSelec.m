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
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS : self].CGColor;
		self.layer.borderWidth = 2;
		
		backButton = [[RakBackButton alloc] initWithFrame:[self bounds]: true];
		[backButton setTarget:self];
		[backButton setAction:@selector(backButtonClicked)];
		[backButton setHidden:!readerMode];
		[self addSubview:backButton];
		[backButton release];

		if(state != nil && [state isNotEqualTo:STATE_EMPTY])
		{
			NSArray *componentsWithSpaces = [state componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
			NSArray *dataState = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
			
			if([dataState count] == 7)
			{
				do
				{
					int indexTeam = getIndexOfTeam((char*)[[dataState objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding]);
					if(indexTeam == -1)
					{
						NSLog(@"Couldn't find the repo to restore, abort :/");
						break;
					}

					uint projectID = [[dataState objectAtIndex:1] longLongValue];	//We want uint, however, only int is available, so we rather want an overflow than an overflow exception
					PROJECT_DATA * project = getDataFromSearch (indexTeam, projectID, RDB_CTXCT, false);
					
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
					free(project);
					
				} while (0);
			}
		}
		else
		{
			[self noContent];
		}
		
		if(coreView != nil)
		{
			[self addSubview:coreView];
			[coreView release];
		}
	}
    return self;
}

- (void) noContent
{

}

- (void) dealloc
{
	[backButton removeFromSuperview];	[backButton release];	backButton = nil;
	
	if(coreView != nil)	{	[coreView removeFromSuperview];		[coreView release];		coreView = nil;		}
	
	[super dealloc];
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

- (void) setUpViewForAnimation : (BOOL) reader
{
	[backButton setHidden:!reader];
}

- (NSString *) byebye
{
	NSString * string;
	
	if(coreView == nil || (string = [coreView getContextToGTFO]) == nil)
	{
		return [super byebye];
	}
	else
		[self removeFromSuperview];
	   
	return string;
}

- (void) updateProject : (PROJECT_DATA) project : (BOOL)isTome : (int) element
{
	PROJECT_DATA newProject = getCopyOfProjectData(project);	//Isole le tab des données
	updateIfRequired(&newProject, RDB_CTXCT);
	
	if(coreView != nil)
		[coreView updateContext:newProject];
	else
	{
		coreView = [[[RakChapterView alloc] initContent : [self calculateContentViewSize : [self frame] : backButton.frame.origin.y + backButton.bounds.size.height] : newProject : NO : (long [4]) {-1, -1, -1, -1}] autorelease];
		
		if(coreView != nil)
			[self addSubview:coreView];
	}
	
	//Coreview en fait aussi une copie, on doit donc release cette version
	releaseCTData(newProject);
}

#pragma mark - Reader code
/**		Reader		**/

- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_CT_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	NSRect frame = viewFrame;
	NSSize svSize = self.superview.bounds.size;
	[Prefs getPref:PREFS_GET_TAB_READER_POSX : &frame.size.width : &svSize];
	frame.size.width -= _lastFrame.origin.x;
	frame.origin.x = 0;
	
	MDL * tabMDL = [self getMDL : YES];
	
	if(tabMDL != nil)
	{
		CGFloat MDLHeight = [tabMDL lastFrame].size.height - [tabMDL frame].origin.y - viewFrame.origin.y;
		
		if(MDLHeight > 0)
		{
			frame.origin.y = MDLHeight;
			frame.size.height -= MDLHeight;
		}
		else
			frame.origin.y = 0;
	}
	else
		frame.origin.y = 0;
	
	return frame;
}

#pragma mark - Self code used in reader mode

- (NSRect) calculateContentViewSize : (NSRect) frame : (CGFloat) backButtonLowestY
{
	CGFloat previousHeight = frame.size.height;
	
	frame.size.height -= backButtonLowestY + RBB_TOP_BORDURE + CT_READERMODE_BOTTOMBAR_WIDTH;
	frame.origin.x = CT_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
	frame.origin.y = previousHeight - frame.size.height - CT_READERMODE_BOTTOMBAR_WIDTH;
	frame.size.width -= 2* frame.origin.x;	//Pas obligé de recalculer

	return frame;
}

- (BOOL) needToConsiderMDL
{
	return YES;
}

- (void) setFrame:(NSRect)frameRect
{
	if([self wouldFrameChange:frameRect])
	{
		[super setFrame : frameRect];
		
		frameRect.origin.x = frameRect.origin.y = 0;
		[backButton setFrame:frameRect];
		if (coreView != nil)
		{
			[coreView setFrame:[self calculateContentViewSize : [self lastFrame] : backButton.frame.origin.y + backButton.frame.size.height]];
		}
	}
}

- (void) resizeAnimation
{
	NSRect frame = [self createFrame];
	
	if([self wouldFrameChange:frame])
	{
		[self.animator setFrame:frame];

		frame.origin.x = frame.origin.y = 0;
		[backButton resizeAnimation:frame];

		if(coreView)
		{
			[coreView resizeAnimation:[self calculateContentViewSize : frame : RBB_TOP_BORDURE + RBB_BUTTON_HEIGHT]];
		}
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
	if(element == VALEUR_FIN_STRUCT && project.team != NULL)
	{
		Reader *readerTab = [[NSApp delegate] reader];
		MDL * MDLTab = [[NSApp delegate] MDL];
		
		if(readerTab != nil)
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
				
				[self updateProject : project : isTome : element];
				
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
			[coreView updateContext:project];
			if([Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
				[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
		}
	}
}

#pragma mark - Proxy

//Return NO some class were not instantiated
- (BOOL) refreshCT : (BOOL) checkIfRequired : (uint) ID
{
	if(coreView != nil)
		return [coreView refreshCT : checkIfRequired : ID];

	return NO;
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element
{
	if(coreView != nil)
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
	
	releaseCTData(data);
	
	return ret_value;
}

- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL
{
	if (sender == TAB_SERIES || sender == TAB_MDL)
		return NSDragOperationCopy;
	
	return [super dropOperationForSender:sender:canDL];
}

@end
