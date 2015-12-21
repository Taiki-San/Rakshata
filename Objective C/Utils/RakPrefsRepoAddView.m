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

@interface RakPrefsRepoAddView()
{
	ROOT_REPO_DATA ** _root;
	uint _nbRoot;
	
	REPO_DATA ** linearizedRepo;
	
	uint _nbRepo;
	
	//UI elements
	RakText * title;
	RakListScrollView * scrollview;
	RakAddRepoList * subrepoList;
	RakButton * cancelButton, * confirmButton;
}

@end

enum
{
	WIDTH = 400,
	HEIGHT = 300,
	
	TOP_BORDER_TITLE = 12,	//Border at the top of the title text
	
	LIST_SEPARATOR = 12,	//Border at the top of the list
	
	LIST_HEIGHT = 200,	//Usable height
	
	BORDER_WIDTH = 15,	//Border at the right or the left of the window
	WIDTH_BORDERED = (WIDTH - 2 * BORDER_WIDTH),	//Usable width
	
	OFFSET_BUTTONS = 20,
	
	SCROLL_VIEW_RADIUS = 3,
};

@implementation RakPrefsRepoAddView

- (instancetype) initWithRoot : (ROOT_REPO_DATA **) root nbElem : (uint) nbRoot
{
	self = [self initWithFrame:NSMakeRect(0, 0, WIDTH, HEIGHT)];
	
	if(self != nil)
	{
		_root = root;
		
		//We craft a linearized list of repos
		for(_nbRoot = _nbRepo = 0; _nbRoot < nbRoot; _nbRoot++)
			_nbRepo += root[_nbRoot]->nbSubrepo;
		
		if(!_nbRepo)
			return nil;
		
		linearizedRepo = malloc(_nbRepo * sizeof(REPO_DATA *));
		if(linearizedRepo == NULL)
			return nil;
		
		uint posLin = 0;
		for(uint posRoot = 0; posRoot < nbRoot && posLin < _nbRepo; posRoot++)
		{
			for (uint i = 0, max = root[posRoot]->nbSubrepo && posLin < _nbRepo; i < max; i++)
				linearizedRepo[posLin++] = &(root[posRoot]->subRepo[i]);
		}
		
		if(posLin < _nbRepo)
			_nbRepo = posLin;
		
		[self generateUI];
	}
	
	return self;
}

- (void) dealloc
{
	if(_root != NULL)
	{
		for(uint i = 0; i < _nbRoot; i++)
			freeSingleRootRepo(_root[i]);
		
		free(_root);
	}
	
	free(linearizedRepo);
}

#pragma mark - Generate data

- (NSString *) getTitleString
{
	NSString * definition = nil;
	
	//Localize the definition of the subrepo we are asking the user to select
	if(_nbRepo > 1)
		definition = NSLocalizedString(@"PREFS-ADD-REPO-SEVERAL-REPO", nil);
	else
		definition = NSLocalizedString(@"PREFS-ADD-REPO-ONE-REPO", nil);
	
	//We craft the final string
	return [NSString localizedStringWithFormat:NSLocalizedString(@"PREFS-ADD-REPO-CONFIRM-INSERT-%@", nil), definition];
}

- (void *) dataForMode : (BOOL) rootMode index : (uint) index
{
	if(index >= _nbRepo)
		return NULL;
	
	return linearizedRepo[index];
}

- (uint) sizeForMode : (BOOL) rootMode
{
	return _nbRepo;
}

- (void) statusTriggered : (id) responder : (REPO_DATA *) repoData
{
	repoData->active = [responder getButtonState];
}

- (void) selectionUpdate : (BOOL) isRoot : (uint) index	{}

- (void) cancelClicked
{
	[RakApp.window endSheet:self.window];
}

- (void) confirmClicked
{
	uint nbRejected = 0;
	ROOT_REPO_DATA * collector[_nbRoot];
	
	//We check which repo are at least partially selected
	for(uint posRoot = 0; posRoot < _nbRoot; posRoot++)
	{
		uint posSub = _root[posRoot]->nbSubrepo;
		
		while(posSub-- > 0)
		{
			if(_root[posRoot]->subRepo[posSub].active)
			{
				posSub = 0;
				break;
			}
		}
		
		//We couldn't find a single valid item
		if(posSub != 0)
		{
			collector[nbRejected++] = _root[posRoot];
			_root[posRoot] = NULL;
		}
	}
	
	//Perform the heavy task: insert in the DB and refresh in order to get the projects
	addRootRepoToDB(_root, _nbRoot);
	
	//We remove the window
	[RakApp.window endSheet:self.window];
	
	//We update the context so dealloc will only deallocate what have to be
	if(nbRejected)
	{
		memcpy(_root, collector, nbRejected * sizeof(ROOT_REPO_DATA *));
	}
	
	_nbRoot = nbRejected;
}

#pragma mark - Generate UI

- (void) generateUI
{
	//Title
	title = [[RakText alloc] initWithText:[self getTitleString] :[self getTitleColor]];
	if(title != nil)
	{
		title.alignment = NSTextAlignmentCenter;
		
		[title.cell setWraps:YES];
		title.fixedWidth = WIDTH_BORDERED;
		
		[title setFrameOrigin:NSMakePoint(BORDER_WIDTH, TOP_BORDER_TITLE)];
		[self addSubview:title];
	}
	
	//List
	subrepoList = [RakAddRepoList alloc];
	if(subrepoList != nil)
	{
		NSRect frame = [self listFrame];
		frame.size.width -= [RakScroller width];
		
		subrepoList = [subrepoList initWithRepo: _root : _nbRoot : frame];
		
		scrollview = [[RakListScrollView alloc] initWithFrame:[self listFrame]];
		if(scrollview != nil)
		{
			scrollview.wantsLayer = YES;
			
			scrollview.layer.cornerRadius = SCROLL_VIEW_RADIUS;
			
			scrollview.documentView = [subrepoList getContent];
			[self addSubview:scrollview];
		}
	}
	
	//Buttons
	cancelButton = [RakButton allocWithText:NSLocalizedString(@"CANCEL", nil)];
	if(cancelButton != nil)
	{
		cancelButton.target = self;
		cancelButton.action = @selector(cancelClicked);
		
		[cancelButton setFrameOrigin:NSMakePoint(_bounds.size.width / 3 - cancelButton.bounds.size.width / 2, NSMaxY(scrollview.frame) + OFFSET_BUTTONS)];
		
		[self addSubview:cancelButton];
	}
	
	confirmButton = [RakButton allocWithText:NSLocalizedString(@"PREFS-ADD-REPO-CONFIRM", nil)];
	if(confirmButton != nil)
	{
		confirmButton.target = self;
		confirmButton.action = @selector(confirmClicked);
		
		[confirmButton setFrameOrigin:NSMakePoint(_bounds.size.width * 2 / 3 - confirmButton.bounds.size.width / 2, NSMaxY(scrollview.frame) + OFFSET_BUTTONS)];
		
		[self addSubview:confirmButton];
	}
}

- (void) drawBackground
{
	[[self backgroundColor] setFill];
	
	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	
	NSSize currentSize = _bounds.size;
	const CGFloat radius = 2;
	
	CGContextBeginPath(contextBorder);
	CGContextMoveToPoint(contextBorder, 0, 0);
	CGContextAddLineToPoint(contextBorder, currentSize.width, 0);
	CGContextAddLineToPoint(contextBorder, currentSize.width, currentSize.height - radius);
	CGContextAddArc(contextBorder, currentSize.width - radius, currentSize.height - radius, radius, 0, M_PI_2, 0);
	CGContextAddLineToPoint(contextBorder, radius, currentSize.height);
	CGContextAddArc(contextBorder, radius, currentSize.height - radius, radius, M_PI_2, M_PI, 0);
	CGContextAddLineToPoint(contextBorder, 0, 0);
	
	CGContextFillPath(contextBorder);
}

- (void) drawRect:(NSRect)dirtyRect
{
	[self drawBackground];
	
	[[self listBackgroundColor] setFill];
	[[NSBezierPath bezierPathWithRoundedRect:scrollview.frame xRadius:SCROLL_VIEW_RADIUS yRadius:SCROLL_VIEW_RADIUS] fill];
}

- (BOOL) isFlipped
{
	return YES;
}

#pragma mark - Color and sizing management

- (NSRect) listFrame
{
	return NSMakeRect(BORDER_WIDTH, NSMaxY(title.frame) + LIST_SEPARATOR, WIDTH_BORDERED, LIST_HEIGHT);
}

- (RakColor *) getTitleColor
{
	return [Prefs getSystemColor:COLOR_SURVOL];
}

- (RakColor *) backgroundColor
{
	return [Prefs getSystemColor:COLOR_ADD_REPO_BACKGROUND];
}

- (RakColor *) listBackgroundColor
{
	return [Prefs getSystemColor:COLOR_LIST_SELECTED_BACKGROUND];
}

@end
