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
	CGContextRef contextBorder;
	
	RakText * title, * titleList;
	RakPrefsRepoList * subrepoList;
}

@end

enum
{
	WIDTH = 400,
	HEIGHT = 300,
	
	TOP_DARK_BORDER = 2,	//Dark window border have to be drawn at the top of the modal window
	
	TOP_BORDER_TITLE = 8,	//Border at the top of the title text
	
	TITLE_LIST_SEPARATOR = 4,	//Border below title
	LIST_SEPARATOR = (TITLE_LIST_SEPARATOR + 20),	//Border at the top of the list
	
	LIST_HEIGHT = 150,	//Usable height
	
	BORDER_WIDTH = 15,	//Border at the right or the left of the window
	WIDTH_BORDERED = (WIDTH - 2 * BORDER_WIDTH),	//Usable width
};

@implementation RakPrefsRepoAddView

- (instancetype) initWithRoot : (ROOT_REPO_DATA **) root nbElem : (uint) nbRoot
{
	self = [self initWithFrame:NSMakeRect(0, 0, WIDTH, HEIGHT)];
	
	if(self != nil)
	{
		_root = root;
		contextBorder = nil;

		//We craft a linearized list of repos
		for(_nbRoot = _nbRepo = 0; _nbRoot < nbRoot; _nbRoot++)
			_nbRepo += root[_nbRoot]->nombreSubrepo;

		if(!_nbRepo)
			return nil;

		linearizedRepo = malloc(_nbRepo * sizeof(REPO_DATA *));
		if (linearizedRepo == NULL)
			return nil;
		
		uint posLin = 0;
		for(uint posRoot = 0; posRoot < nbRoot && posLin < _nbRepo; posRoot++)
		{
			for (uint i = 0, max = root[posRoot]->nombreSubrepo && posLin < _nbRepo; i < max; i++)
				linearizedRepo[posLin++] = &(root[posRoot]->subRepo[i]);
		}
		
		if(posLin < _nbRepo)
			_nbRepo = posLin;
		
		[self generateUI];
	}
	
	return self;
}

#pragma mark - Generate data

- (NSString *) getTitleString
{
	NSString * definition = nil;

	//Localize the definition of the subrepo we are asking the user to select
	if(_nbRoot > 1)
		definition = NSLocalizedString(@"PREFS-ADD-REPO-SEVERAL-REPO-IN-SEVERAL-ROOTS", nil);
	else if(_nbRepo > 1)
		definition = NSLocalizedString(@"PREFS-ADD-REPO-SEVERAL-REPO-IN-ROOT", nil);
	else
		definition = NSLocalizedString(@"PREFS-ADD-REPO-ONE-REPO-IN-ROOT", nil);
	
	//We combine root's name
	NSString * names = getStringForWchar(_root[0]->name);
	
	if(_nbRoot > 1)
	{
		for(uint pos = 1; pos < _nbRoot - 1; pos++)
			names = [NSString stringWithFormat:@"%@, %@", names, getStringForWchar(_root[pos]->name)];
		
		names = [NSString stringWithFormat:@"%@ %@ %@", names, NSLocalizedString(@"PREFS-ADD-REPO-FINAL-LINK", nil), getStringForWchar(_root[_nbRoot - 1]->name)];
	}

	//We craft the final string
	return [NSString localizedStringWithFormat:NSLocalizedString(@"PREFS-ADD-REPO-CONFIRM-INSERT-%@-FROM-%@", nil), definition, names];
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

#pragma mark - Generate UI

- (void) generateUI
{
	//Title
	title = [[RakText alloc] initWithText:[self getTitleString] :[self getTitleColor]];
	if(title != nil)
	{
		title.alignment = NSCenterTextAlignment;
		
		[title.cell setWraps:YES];
		title.fixedWidth = WIDTH_BORDERED;
		
		[title setFrameOrigin:NSMakePoint(BORDER_WIDTH, TOP_BORDER_TITLE)];
		[self addSubview:title];
	}
	
	//Subtext for the list
	titleList = [[RakText alloc] initWithText:NSLocalizedString(@"PREFS-ADD-REPO-TITLE-LIST", nil) :[self getTitleColor]];
	if(titleList != nil)
	{
		[titleList setFrameOrigin:NSMakePoint(BORDER_WIDTH, NSMaxY(title.frame) + TITLE_LIST_SEPARATOR)];
		[self addSubview:titleList];
	}
	
	//List
	subrepoList = [RakPrefsRepoList alloc];
	if(subrepoList != nil)
	{
		subrepoList.responder = self;
		subrepoList.detailMode = YES;
		
		subrepoList = [subrepoList initWithFrame:[self listFrame]];
		
		[self addSubview:[subrepoList getContent]];
	}
}

- (void) craftBackgroundShape
{
	contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	
	NSSize currentSize = _bounds.size;
	const CGFloat radius = 2;
	
	CGContextBeginPath(contextBorder);
	CGContextMoveToPoint(contextBorder, 0, TOP_DARK_BORDER);
	CGContextAddLineToPoint(contextBorder, currentSize.width, TOP_DARK_BORDER);
	CGContextAddLineToPoint(contextBorder, currentSize.width, currentSize.height - radius);
	CGContextAddArc(contextBorder, currentSize.width - radius, currentSize.height - radius, radius, 0, M_PI_2, 0);
	CGContextAddLineToPoint(contextBorder, radius, currentSize.height);
	CGContextAddArc(contextBorder, radius, currentSize.height - radius, radius, M_PI_2, M_PI, 0);
	CGContextAddLineToPoint(contextBorder, 0, TOP_DARK_BORDER);
}

- (void) drawRect:(NSRect)dirtyRect
{
	[[self backgroundColor] setFill];
	
	if(contextBorder == nil)
		[self craftBackgroundShape];

	CGContextFillPath(contextBorder);

	[[Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_FAREST : nil] setFill];
	NSRectFill(NSMakeRect(0, 0, dirtyRect.size.width, TOP_DARK_BORDER));
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

- (NSColor *) getTitleColor
{
	return [Prefs getSystemColor:GET_COLOR_SURVOL :nil];
}

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_ADD_REPO :nil];
}

@end
