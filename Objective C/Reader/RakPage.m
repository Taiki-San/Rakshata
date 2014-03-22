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
 ********************************************************************************************/

@implementation RakPage

- (id) init : (Reader*)superView : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest
{
	if(![self initialLoading:dataRequest :elemRequest :isTomeRequest])
		return nil;
	
	if (![self craftPageAndSetupEnv:superView : READER_ETAT_DEFAULT])
		return nil;

	//We create the NSScrollview
	self = [self initWithFrame:frameReader];
	if(self != nil)
	{
		[self addPageToView];
		
		areSlidersHidden = YES;
		
		//We set preferences ~

		self.hasVerticalScroller =		pageTooHigh;
		self.hasHorizontalScroller =	pageTooLarge;
		self.borderType =				NSNoBorder;
		self.scrollerStyle =			NSScrollerStyleOverlay;
		self.drawsBackground =			NO;
		self.needsDisplay =				YES;
		self.autoresizesSubviews =		YES;
		
		if(areSlidersHidden)
		{
			self.verticalScroller.alphaValue =	0;
			self.horizontalScroller.alphaValue = 0;
		}
				
		[superView addSubview:self];
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	}
	return self;
}

- (BOOL) isEditable
{
	return NO;
}

- (BOOL) allowsCutCopyPaste
{
	return YES;
}

/*Handle the position of the whole thing when anything change*/

#pragma mark    -   Position manipulation

- (void) initialPositionning : (BOOL) canIHazSuperview : (NSRect) frameWindow
{
	if(canIHazSuperview)
		frameReader = self.superview.frame;
	else
		frameReader = frameWindow;
	
	//Largeur
	if(selfFrame.size.width > frameReader.size.width - 2 * READER_BORDURE_VERT_PAGE)	//	Page trop large
	{
		pageTooLarge = true;
		frameReader.size.width = frameReader.size.width - 2*READER_BORDURE_VERT_PAGE;
		frameReader.origin.x = READER_BORDURE_VERT_PAGE;
	}
	else
	{
		pageTooLarge = false;
		frameReader.origin.x = frameReader.size.width / 2 - selfFrame.size.width / 2;
		frameReader.size.width = selfFrame.size.width;
	}

	//Hauteur
	if (selfFrame.size.height > frameReader.size.height - READER_PAGE_BORDERS_HIGH)
	{
		pageTooHigh = true;
		frameReader.size.height = frameReader.size.height;
		frameReader.origin.y = 0;
	}
	else
	{
		pageTooHigh = false;
		frameReader.origin.y = (frameReader.size.height - READER_PAGE_BORDERS_HIGH) / 2 - selfFrame.size.height / 2;
		frameReader.size.height = selfFrame.size.height;
	}
}

- (void) setFrame:(NSRect)frameRect
{
	if(frameRect.size.width != frameReader.size.width || frameRect.size.height != frameReader.size.height)
	{
		[self initialPositionning:YES:frameRect];
		self.hasVerticalScroller = pageTooHigh;
		self.hasHorizontalScroller = pageTooLarge;
	}
	[super setFrame:frameReader];
}

/*Event handling*/

#pragma mark    -   Events

- (void)mouseDown:(NSEvent *)theEvent
{
	if (actionBeingProcessessed)
		return;
	
	actionBeingProcessessed = true;
	
	NSPoint mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];

	if(pageTooHigh)
		mouseLoc.y += [self.contentView documentRect].size.height - [self frame].size.height - [self.contentView documentVisibleRect].origin.y;
	
	if(pageTooLarge)
		mouseLoc.x += [self.contentView documentVisibleRect].origin.x;
	
	if(mouseLoc.y < READER_PAGE_TOP_BORDER || mouseLoc.y > [self.contentView documentRect].size.height - READER_PAGE_TOP_BORDER)
		[self prevPage];
	else
		[self nextPage];
	
	actionBeingProcessessed = false;
}

- (void) keyDown:(NSEvent *)theEvent
{
	if(actionBeingProcessessed)
		return;
	
	actionBeingProcessessed = true;
	
	NSString*   const   character   =   [theEvent charactersIgnoringModifiers];
    unichar     const   code        =   [character characterAtIndex:0];
	bool isModPressed = [theEvent modifierFlags] & (NSAlternateKeyMask | NSShiftKeyMask);
	
    switch (code)
    {
        case NSUpArrowFunctionKey:
        {
			if(isModPressed)
				[self moveSliderX:-PAGE_MOVE];
            else
				[self moveSliderY:PAGE_MOVE];
            break;
        }
        case NSDownArrowFunctionKey:
        {
			if(isModPressed)
				[self moveSliderX:PAGE_MOVE];
            else
				[self moveSliderY:-PAGE_MOVE];
            break;
        }
        case NSLeftArrowFunctionKey:
        {
            [self prevPage];
            break;
        }
        case NSRightArrowFunctionKey:
        {
            [self nextPage];
            break;
        }
    }
	
	actionBeingProcessessed = false;
}

/*Error management*/

#pragma mark    -   Errors

- (void) failure
{
	NSLog(@"Something went wrong delete?");
}

#pragma mark - High level API

- (void) nextPage
{
	[self changePage:READER_ETAT_NEXTPAGE];
}

- (void) prevPage
{
	[self changePage:READER_ETAT_PREVPAGE];
}

- (void) nextChapter
{
	[self changeChapter:true];
}

- (void) prevChapter
{
	[self changeChapter:false];
}

- (void) moveSliderX : (int) move
{
	if(!pageTooLarge)
		return;
	
	NSPoint point = [[self contentView] bounds].origin;
	
	if(move < 0 && point.x < -move)
		point.x = 0;
	
	else if(move > 0)
	{
		CGFloat basePos = [self.documentView frame].size.width - self.frame.size.width;
		if(point.x > basePos - move)
			point.x = basePos;
		else
			point.x += move;
	}
	else
		point.x += move;
	
	[self.contentView scrollToPoint:point];
}

- (void) moveSliderY : (int) move
{
	if(!pageTooHigh)
		return;
	
	NSPoint point = [[self contentView] bounds].origin;
	
	if(move < 0 && point.y < -move)
		point.y = 0;
	
	else if(move > 0)
	{
		CGFloat basePos = [self.documentView frame].size.height - self.frame.size.height;
		if(point.y > basePos - move)
			point.y = basePos;
		else
			point.y += move;
	}
	else
		point.y += move;
	
	[self.contentView scrollToPoint:point];
}

/*Active routines*/

#pragma mark    -   Active routines

- (BOOL) initialLoading : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest
{
	memcpy(&project, &dataRequest, sizeof(dataRequest));
	currentElem = elemRequest;
	isTome = isTomeRequest;
	loadTrad(texteTrad, 21);
	
	prevPage = pageData = nextPage = nil;
	cacheBeingBuilt = actionBeingProcessessed = false;
	
	updateIfRequired(&project, RDB_CTXLECTEUR);
	
	posElemInStructure = reader_getPosIntoContentIndex(project, currentElem, isTome);
	if(posElemInStructure == -1)
	{
		[self failure];
		return NO;
	}
	
	setLastChapitreLu(project, isTome, currentElem);
	if(reader_isLastElem(project, isTome, currentElem))
        startCheckNewElementInRepo(project, isTome, currentElem, false);
	
	//On met la page courante par défaut
	data.pageCourante = reader_getCurrentPageIfRestore(texteTrad);
	
	if(configFileLoader(project, isTome, currentElem, &data))
	{
		[self failure];
		return NO;
	}
	return YES;
}

- (NSData *) getPage : (uint) posData
{
	IMG_DATA * dataPage = loadSecurePage(data.path[data.pathNumber[posData]], data.nomPages[posData], data.chapitreTomeCPT[data.pathNumber[posData]], data.pageCouranteDuChapitre[posData]);
	
	if(dataPage == NULL)
	{
		[self failure];
		return NULL;
	}
	
	NSData *output = [NSData dataWithBytes:dataPage->data length:dataPage->length];
	
	free(dataPage->data);
	free(dataPage);

	return output;
}

- (void) buildCache
{
	cacheBeingBuilt = true;
	
	int localCurrentPage = data.pageCourante;
	
	if(localCurrentPage < 0 || localCurrentPage >= data.nombrePageTotale)	//Données hors de nos bornes
	{
		cacheBeingBuilt = false;
		return;
	}
	
	if(nextPage == nil)
	{
		if (localCurrentPage >= 0 && localCurrentPage < data.nombrePageTotale)
		{
			nextPage = [self getPage:localCurrentPage+1];
			[nextPage retain];
		}
	}

	if(prevPage == nil)
	{
		if(localCurrentPage > 0)
		{
			prevPage = [self getPage:localCurrentPage-1];
			[prevPage retain];
		}
	}
	
	cacheBeingBuilt = false;
}

- (void) changePage : (byte) switchType
{
	if(switchType == READER_ETAT_NEXTPAGE)
	{
		if(data.pageCourante+1 > data.nombrePageTotale)
		{
			[self changeChapter:true];
			return;
		}
		data.pageCourante++;
	}
	else if(switchType == READER_ETAT_PREVPAGE)
	{
		if(data.pageCourante < 1)
		{
			[self changeChapter:false];
			return;
		}
		data.pageCourante--;
	}
	else if(switchType != READER_ETAT_DEFAULT)
	{
		NSLog(@"Couldn't understand which direction I should move to");
		return;
	}
	
	[self craftPageAndSetupEnv:(Reader *)self.superview : switchType];
	[self addPageToView];	
}

- (void) changeChapter : (bool) goToNext
{
	uint newPosIntoStruct = posElemInStructure;
	if(!changeChapter(&project, isTome, &currentElem, &newPosIntoStruct, goToNext))
	{
		[self failure];
	}
	else
	{
		posElemInStructure = newPosIntoStruct;
		[self updateContext];
	}
}

- (void) updateContext
{
	[self flushCache];
	releaseDataReader(&data);
	
	updateIfRequired(&project, RDB_CTXLECTEUR);
	
	setLastChapitreLu(project, isTome, currentElem);
	if(reader_isLastElem(project, isTome, currentElem))
        startCheckNewElementInRepo(project, isTome, currentElem, false);
	
	data.pageCourante = 0;
	
	if(configFileLoader(project, isTome, currentElem, &data))
		[self failure];
	
	[self changePage:READER_ETAT_DEFAULT];
}

- (BOOL) craftPageAndSetupEnv : (Reader *) superView : (byte) switchType
{
	while(cacheBeingBuilt)
		usleep(25);
	
	if(switchType == READER_ETAT_DEFAULT)
	{
		//We rebuild the cache from scratch
		if(prevPage != nil)
		{
			[prevPage release];
			prevPage = nil;
		}
		
		if (pageData != nil)
		{
			[pageData release];
		}
		
		if(nextPage != nil)
		{
			[nextPage release];
			nextPage = nil;
		}

		pageData = [self getPage:data.pageCourante];
		[pageData retain];
	}
	else if(switchType == READER_ETAT_PREVPAGE)
	{
		if(nextPage != nil)
			[nextPage release];
		
		nextPage = pageData;
		
		if(prevPage == nil)
		{
			pageData = [self getPage:data.pageCourante];
			[pageData retain];
		}
		else
		{
			pageData = prevPage;
			prevPage = nil;
		}
	}
	else
	{
		if(prevPage != nil)
			[prevPage release];
		
		prevPage = pageData;
		
		if(nextPage == nil)
		{
			pageData = [self getPage:data.pageCourante];
			[pageData retain];
		}
		else
		{
			pageData = nextPage;
			nextPage = nil;
		}
	}
	
	page = [[NSImage alloc] initWithData:pageData];
	
	if(page == nil)
		return false;
	
	[page setCacheMode:NSImageCacheNever];
	[self performSelectorInBackground:@selector(buildCache) withObject:nil];

	//Work, we now craft the size of this view
	selfFrame = NSMakeRect(0.0, 0.0, page.size.width, page.size.height);
	[self initialPositionning:NO:[superView getCurrentFrame]];
	
	return true;
}

- (void) addPageToView
{
	//We create the view that si going to be displayed
	NSRect pageViewSize = selfFrame;
	pageViewSize.size.height += 2*READER_PAGE_TOP_BORDER;
	
	if(pageView != nil)
	{
		[pageView setFrame:pageViewSize];
		[pageView setImage:page];

	}
	else
	{
		pageView = [[NSImageView alloc] initWithFrame:pageViewSize];
		[pageView setImageAlignment:NSImageAlignCenter];
		[pageView setImageFrameStyle:NSImageFrameNone];
		[pageView setImage:page];
	
		self.documentView =	pageView;
		
		[pageView release];
	}
	
	[page release];
	
	NSPoint sliderStart;
	
	if (pageTooHigh)
		sliderStart.y = pageViewSize.size.height - frameReader.size.height;
	else
		sliderStart.y = 0;
	
	if(pageTooLarge)
		sliderStart.x = pageViewSize.size.width - frameReader.size.width;
	else
		sliderStart.x = 0;
	
	[self.contentView scrollToPoint:sliderStart];
	
	[self setFrameSize:frameReader.size];
	[self setFrameOrigin:frameReader.origin];
}

- (void) flushCache
{
	while (cacheBeingBuilt);
	
	[prevPage release];		prevPage = nil;
	[pageData release];		pageData = nil;
	[nextPage release];		nextPage = nil;
}

- (void) getTheFuckOut
{
	[self flushCache];
	[self removeFromSuperview];
	self.documentView = nil;
	releaseDataReader(&data);
}

@end
