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

#ifdef DEV_VERSION
//	#define PERF_ANALYSIS
#endif

@implementation RakPage

- (id) init : (Reader*)superView : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage
{
	alreadyRefreshed = false;
	readerMode = superView->readerMode;
	
	if(![self initialLoading:dataRequest :elemRequest :isTomeRequest : startPage])
		return nil;
	
	if (![self craftPageAndSetupEnv:superView : READER_ETAT_DEFAULT])
		return nil;

	//We create the NSScrollview
	self = [self initWithFrame:frameReader];
	if(self != nil)
	{
		[self addPageToView];
		
		//We set preferences ~

		self.hasVerticalScroller =		pageTooHigh;
		self.hasHorizontalScroller =	pageTooLarge;
		self.borderType =				NSNoBorder;
		self.scrollerStyle =			NSScrollerStyleOverlay;
		self.drawsBackground =			NO;
		self.needsDisplay =				YES;
		self.autoresizesSubviews =		YES;
		
		self.verticalScroller.alphaValue =	0;
		self.horizontalScroller.alphaValue = 0;
				
		[superView addSubview:self];
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	}
	return self;
}

- (void) bottomBarInitialized
{
	//And we update the bar with data
	[(Reader*)[self superview] updatePage:data.pageCourante : data.nombrePageTotale];
}

- (BOOL) isEditable
{
	return NO;
}

- (BOOL) allowsCutCopyPaste
{
	return NO;
}

- (NSString *) getContextToGTFO
{
	NSPoint sliders = [[self contentView] bounds].origin;
	return [NSString stringWithFormat:@"%s\n%s\n%d\n%d\n%d\n%.0f\n%.0f", project.team->URL_depot, project.mangaNameShort, currentElem, isTome ? 1 : 0, data.pageCourante, sliders.x, sliders.y];
}

/*Handle the position of the whole thing when anything change*/

#pragma mark    -   Position manipulation

- (void) initialPositionning : (BOOL) canIHazSuperview : (NSRect) frameWindow
{
	if(canIHazSuperview)
		frameReader = self.superview.frame;
	else
		frameReader = frameWindow;
	
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
		frameReader.size.height = selfFrame.size.height + READER_PAGE_BORDERS_HIGH;
	}
	
	if(!readerMode)	//Dans ce contexte, les calculs de largeur n'ont aucune importance
		return;
	
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
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self setFrameInternal: frameRect : YES];
}

- (void) setFrame:(NSRect)frameRect
{
	[self setFrameInternal: frameRect : NO];
}

- (void) setFrameInternal : (NSRect) frameRect : (BOOL) isAnimated
{
	if((frameRect.size.width != frameReader.size.width && readerMode) || frameRect.size.height != frameReader.size.height)
	{
		if(!readerMode)
		{
			frameRect.size.width = self.frame.size.width;
			frameRect.origin.x = self.frame.origin.x;
			frameRect.origin.y = self.frame.origin.y;
		}
		
		[self initialPositionning : !isAnimated : frameRect];
		self.hasVerticalScroller = pageTooHigh;
		self.hasHorizontalScroller = pageTooLarge;
	}
	else if(!readerMode)
		return;
	
	if(isAnimated)
		[self.animator setFrame:frameReader];
	else
		[super setFrame:frameReader];
}

- (void) leaveReaderMode
{
	readerMode = false;
}

- (void) startReaderMode
{
	readerMode = true;
}

/*Event handling*/

#pragma mark    -   Events

- (void) mouseDown:(NSEvent *)theEvent
{
	noDrag = true;
}

- (void) mouseDragged:(NSEvent *)theEvent
{
	noDrag = false;
}

- (void)mouseUp:(NSEvent *)theEvent
{
	bool fail = false;

	if(!readerMode || !noDrag)
		fail = true;
	else
	{
		NSPoint mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
		
		if(pageTooHigh)
		{
			mouseLoc.y += [self.contentView documentRect].size.height - [self frame].size.height - [self.contentView documentVisibleRect].origin.y;
			if(mouseLoc.y < READER_PAGE_TOP_BORDER || mouseLoc.y > [(NSView*) self.documentView frame].size.height - READER_PAGE_BOTTOM_BORDER)
				fail = true;
		}
		
		if(pageTooLarge)
		{
			mouseLoc.x += [self.contentView documentRect].size.width - [self frame].size.width - [self.contentView documentVisibleRect].origin.x;
			if(mouseLoc.x < READER_BORDURE_VERT_PAGE || mouseLoc.x > [(NSView*) self.documentView frame].size.width - READER_BORDURE_VERT_PAGE)
				fail = true;
		}
	}
	
	if(fail)
	{
		[self.superview mouseDown:NULL];
		[self.superview mouseUp:theEvent];
	}
	else
		[self nextPage];
}

- (void) keyDown:(NSEvent *)theEvent
{
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

- (void) setSliderPos : (NSPoint) newPos
{
	NSPoint point = [[self contentView] bounds].origin;
	
	[self moveSliderY : newPos.y - point.y];
	[self moveSliderX : newPos.x - point.x];
}

/*Active routines*/

#pragma mark    -   Active routines

- (BOOL) initialLoading : (MANGAS_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage
{
	memcpy(&project, &dataRequest, sizeof(dataRequest));
	currentElem = elemRequest;
	isTome = isTomeRequest;
	loadTrad(texteTrad, 21);
	
	prevPage = pageData = nextPage = nil;
	cacheBeingBuilt = false;
	
	updateIfRequired(&project, RDB_CTXLECTEUR);

	checkChapitreValable(&project, NULL);
	checkTomeValable(&project, NULL);
	
	posElemInStructure = reader_getPosIntoContentIndex(project, currentElem, isTome);
	if(posElemInStructure == -1)
	{
		[self failure];
		return NO;
	}
	
	setLastChapitreLu(project, isTome, currentElem);
	if(reader_isLastElem(project, isTome, currentElem))
	{
		[self performSelectorInBackground:@selector(checkIfNewElements) withObject:nil];
	}
	
	if(configFileLoader(project, isTome, currentElem, &data))
	{
		[self failure];
		return NO;
	}
	
	if(startPage < 0)
		data.pageCourante = 0;
	else if(startPage < data.nombrePageTotale)
		data.pageCourante = startPage;
	else
		data.pageCourante = data.nombrePageTotale - 1;
	
	return YES;
}

#ifdef DEV_VERSION
	void updateChapter(DATA_LECTURE * dataLecteur, int numeroChapitre);
#endif

- (NSData *) getPage : (uint) posData
{
	if(data.path == NULL)
	{
		[self failure];
		return nil;
	}
	
#ifdef PERF_ANALYSIS
	struct timeval t1, t2;
    double elapsedTime;
	gettimeofday(&t1, NULL);
#endif
	
	IMG_DATA * dataPage = loadSecurePage(data.path[data.pathNumber[posData]], data.nomPages[posData], data.chapitreTomeCPT[data.pathNumber[posData]], data.pageCouranteDuChapitre[posData]);
	
#ifdef PERF_ANALYSIS
	gettimeofday(&t2, NULL);
	
    // compute and print the elapsed time in millisec
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000;
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
	NSLog(@"Loading time: %f", elapsedTime);
#endif
	
	if(dataPage == NULL)
	{
#ifdef DEV_VERSION
		updateChapter(&data, currentElem);
		dataPage = loadSecurePage(data.path[data.pathNumber[posData]], data.nomPages[posData], data.chapitreTomeCPT[data.pathNumber[posData]], data.pageCouranteDuChapitre[posData]);
		
		if(dataPage == NULL)
		{
			[self failure];
			return NULL;
		}
#else
		[self failure];
		return NULL;
#endif
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
	
	if([self craftPageAndSetupEnv:(Reader *)self.superview : switchType])
	{
		[self addPageToView];
		
		//And we update the bar
		[(Reader*)[self superview] updatePage:data.pageCourante : data.nombrePageTotale];
	}
	else
		[self failure];
}

- (void) jumpToPage : (uint) newPage
{
	if (newPage == data.pageCourante || newPage >= data.nombrePageTotale)
		return;
	
	int pageCourante = data.pageCourante;
	
	if(newPage == pageCourante - 1)
		[self changePage:READER_ETAT_PREVPAGE];
	else if(newPage == pageCourante + 1)
		[self changePage:READER_ETAT_NEXTPAGE];
	else
	{
		data.pageCourante = newPage;
		[self changePage:READER_ETAT_DEFAULT];
	}
}

- (void) changeChapter : (bool) goToNext
{
	uint newPosIntoStruct = posElemInStructure;
	if(changeChapter(&project, isTome, &currentElem, &newPosIntoStruct, goToNext))
	{
		posElemInStructure = newPosIntoStruct;
		[self updateContext];
	}
}

- (void) changeProject : (MANGAS_DATA) projectRequest : (int) elemRequest : (bool) isTomeRequest : (int) startPage
{
	if(projectRequest.cacheDBID != project.cacheDBID)
		alreadyRefreshed = false;
	else if(elemRequest == currentElem && isTomeRequest == isTome)
	{
		[self jumpToPage:startPage];
		return;
	}
	
	[self flushCache];
	releaseDataReader(&data);
	
	if([self initialLoading:projectRequest :elemRequest :isTomeRequest : startPage])
		[self changePage:READER_ETAT_DEFAULT];
	
	addRecentEntry(project, false);
}

- (void) updateContext
{
	[self flushCache];
	releaseDataReader(&data);
	
	if(updateIfRequired(&project, RDB_CTXLECTEUR))
	{
		checkChapitreValable(&project, NULL);
		checkTomeValable(&project, NULL);
	}
	
	setLastChapitreLu(project, isTome, currentElem);
	if(reader_isLastElem(project, isTome, currentElem))
        [self checkIfNewElements];
	
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
	{
#ifdef DEV_VERSION
		FILE * lol = fopen("lol.png", "wb");
		void * buffer = malloc([pageData length]);
		if(lol != NULL && buffer != NULL)
		{
			[pageData getBytes:buffer length:[pageData length]];
			fwrite(buffer, [pageData length], 1, lol);
			fclose(lol);
		}
		else if(lol != NULL)
			fclose(lol);
		else
			free(buffer);
#endif
		return false;
	}
	
	[page setCacheMode:NSImageCacheNever];
	[self performSelectorInBackground:@selector(buildCache) withObject:nil];

	//Work, we now craft the size of this view
	selfFrame = NSMakeRect(0.0, 0.0, page.size.width, page.size.height);
	[self initialPositionning:NO:[superView getCurrentFrame]];
	
	return true;
}

- (void) deleteElement
{
	NSAlert * alert = [[NSAlert alloc] init];
	
	[alert setAlertStyle:NSInformationalAlertStyle];
	[alert setMessageText:[NSString stringWithFormat:@"Suppression d'un %s", isTome ? "tome" : "chapitre"]];
	[alert setInformativeText :[NSString stringWithFormat:@"Attention: vous vous apprêtez à supprimer définitivement un %s, pour le relire, vous aurez à le télécharger de nouveau, en êtes vous sûr?", isTome ? "tome" : "chapitre"]];
	
	[alert addButtonWithTitle:@"NON!"];
	NSButton * firstButton = [[alert buttons] objectAtIndex:0];
	[firstButton setTitle:@"I want it dead"];
	[alert addButtonWithTitle:@"NON!"];
	
	if([alert runModal] == NSAlertFirstButtonReturn)
	{
		[alert release];
		while (cacheBeingBuilt);
		internalDeleteCT(project, isTome, currentElem);
		
		if(posElemInStructure != isTome ? project.nombreTomes : project.nombreChapitre)
			[self nextChapter];
		else if(posElemInStructure > 0)
			[self prevChapter];
		else
			[self failure];
	}
	else
		[alert release];
}

- (void) addPageToView
{
	//We create the view that si going to be displayed
	NSRect pageViewSize = selfFrame;
	pageViewSize.size.height += READER_PAGE_BORDERS_HIGH; 
	
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

#pragma mark - Checks if new elements to download

- (void) checkIfNewElements
{
	if(alreadyRefreshed)
		return;
	else
		alreadyRefreshed = true;
	
	MANGAS_DATA localProject;
	memcpy(&localProject, &project, sizeof(MANGAS_DATA));
	
	uint nbElemToGrab = checkNewElementInRepo(&localProject, isTome, currentElem);
	
	if(!nbElemToGrab)
		return;
	
	RakArgumentToRefreshAlert * argument = [RakArgumentToRefreshAlert alloc];
	argument.data = &localProject;
	argument.nbElem = nbElemToGrab;
	
	[self performSelectorOnMainThread:@selector(promptToGetNewElems:) withObject:argument waitUntilDone:YES];
	
	[argument release];
}

- (void) promptToGetNewElems : (RakArgumentToRefreshAlert *) arguments
{
	MANGAS_DATA localProject = *arguments.data;
	uint nbElemToGrab = arguments.nbElem;
	
	if(project.cacheDBID != localProject.cacheDBID)	//The active project changed meanwhile
		return;
	
	bool onlyOneElementAvailable = nbElemToGrab == 1;
	char * element = isTome ? "tome" : "chapitre", * particule = onlyOneElementAvailable ? "" : "s";
	
	NSAlert * alert = [[NSAlert alloc] init];
	
	[alert setAlertStyle:NSInformationalAlertStyle];
	[alert setMessageText:[NSString stringWithFormat:@"%s %s%s!", onlyOneElementAvailable ? "Un" : "Des", element, onlyOneElementAvailable ? " est disponible" : "s sont disponibles"]];
	[alert setInformativeText :[NSString stringWithFormat:@"J'ai remarqué qu'il y a %s %s%s non-téléchargé%s après celui-là. Voulez vous que je le%s télécharge pour vous?", onlyOneElementAvailable ? "un" : "quelques", element, particule, particule, particule]];
	
	[alert addButtonWithTitle:@"Eyup!"];
	[alert addButtonWithTitle:@"Nope"];
	
	if([alert runModal] == NSAlertFirstButtonReturn)
	{
		addtoDownloadListFromReader(localProject, nbElemToGrab, isTome);
		//Still need to warn the MDL to start to work
	}
	
	[alert release];
}

#pragma mark - Quit

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
	releaseDataReader(&data);
	[self removeFromSuperview];
	self.documentView = nil;
}

@end

//Shitty class only used because performSelectorOnMainThread want a class, an not two perfectly fine arguments
@implementation RakArgumentToRefreshAlert

- (void) setData : (MANGAS_DATA *) newData
{
	data = newData;
}

- (MANGAS_DATA *) data
{
	return data;
}

- (void) setNbElem : (uint) newData
{
	nbElem = newData;
}

- (uint) nbElem
{
	return nbElem;
}

@end
