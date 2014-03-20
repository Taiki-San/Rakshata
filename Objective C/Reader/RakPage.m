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
	
	if (![self craftPageAndSetupEnv:superView])
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
	else
	{
		[page release];
	}
	return self;
}

- (BOOL) craftPageAndSetupEnv : (Reader *) superView
{
	page = [[NSImage alloc] initWithData:[self getPage:data.pageCourante]];
	if(page == nil)
		return false;
	
	//Worked, we now craft the size of this view
	selfFrame = NSMakeRect(0.0, 0.0, page.size.width, page.size.height);
	[self initialPositionning:NO:[superView getCurrentFrame]];
	
	return true;
}

- (void) addPageToView
{
	//We create the view that si going to be displayed
	NSRect pageViewSize = selfFrame;
	pageViewSize.size.height += 2*READER_PAGE_TOP_BORDER;
	pageView = [[NSImageView alloc] initWithFrame:pageViewSize];
	
	[pageView setImageAlignment:NSImageAlignCenter];
	[pageView setImageFrameStyle:NSImageFrameNone];
	[pageView setImage:page];
	
	self.documentView =	pageView;
	
	if (pageTooHigh)
		[self.contentView scrollToPoint:NSMakePoint(0, pageViewSize.size.height - frameReader.size.height)];
}

- (BOOL) isEditable
{
	return NO;
}

- (BOOL) allowsCutCopyPaste
{
	return YES;
}

- (void) dealloc
{
	[pageView.image release];
	[pageView release];
	[super dealloc];
}

/*Handle the position of the whole thing when anything change*/

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

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];

	if(pageTooHigh)
		mouseLoc.y += [self.contentView documentRect].size.height - [self frame].size.height - [self.contentView documentVisibleRect].origin.y;
	
	if(pageTooLarge)
		mouseLoc.x += [self.contentView documentVisibleRect].origin.x;
	
	if(mouseLoc.y < READER_PAGE_TOP_BORDER || mouseLoc.y > [self.contentView documentRect].size.height - READER_PAGE_TOP_BORDER)
		[self.superview mouseDown:theEvent];
	else
		[self nextPage];
}

/*Error management*/

- (void) failure
{
	NSLog(@"Something went wrong delete?");
}


/*Active routines*/

- (BOOL) initialLoading : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest
{
	memcpy(&project, &dataRequest, sizeof(dataRequest));
	currentElem = elemRequest;
	isTome = isTomeRequest;
	loadTrad(texteTrad, 21);
	
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
	IMG_DATA * pageData = loadSecurePage(data.path[data.pathNumber[posData]], data.nomPages[posData], data.chapitreTomeCPT[data.pathNumber[posData]], data.pageCouranteDuChapitre[posData]);
	
	if(pageData == NULL)
	{
		[self failure];
		return NULL;
	}
	
	NSData *output = [NSData dataWithBytes:pageData->data length:pageData->length];
	
	free(pageData);

	return output;
}

- (void) nextPage
{
	if(data.pageCourante+1 >= data.nombrePageTotale)
		return;
	
	data.pageCourante++;
	[pageView.image release];
	[pageView release];
	
	[self craftPageAndSetupEnv:(Reader *)self.superview];
	[self addPageToView];	
}

@end
