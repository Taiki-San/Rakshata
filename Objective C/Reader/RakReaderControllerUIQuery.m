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

@implementation RakReaderControllerUIQuery

- (id) initWithData : (MDL*) tabMDL : (MANGAS_DATA) project : (BOOL) isTome : (int*) arraySelection : (uint) sizeArray
{
	if(tabMDL == nil || arraySelection == NULL || sizeArray == 0)
		return nil;
	
	self = [super initWithFrame : NSMakeRect(0, 0, 170, 175)];
	
	if(self != nil)
	{
		NSRect frame = NSZeroRect;
		_tabMDL = tabMDL;	_project = project;		_isTome = isTome;	_arraySelection = arraySelection;	_sizeArray = sizeArray;
		
		[self setWantsLayer:YES];
		[self.layer setCornerRadius:4];
		[self.layer setBorderWidth:1];
		[self.layer setBorderColor:[Prefs getSystemColor:GET_COLOR_BORDER_TABS].CGColor];
		[self.layer setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS].CGColor];
		
		[self setupView];
		
		popover = [[RakPopoverWrapper alloc] init:self];
		popover.anchor = _tabMDL;
		
		if ([_tabMDL isDisplayed])
			popover.direction = INPopoverArrowDirectionLeft;
		else
		{
			NSArray *subviews;
			popover.direction = INPopoverArrowDirectionDown;
			if(_tabMDL.superview != nil && (subviews = _tabMDL.superview.subviews) != nil)
			{
				NSView * view;
				for(view in subviews)
				{
					if([view class] == [Reader class])
					{
						frame.size.width = view.frame.origin.x;
						break;
					}
				}
			}
		}
		
		
		[popover additionalConfiguration:self :@selector(configurePopover:)];
		[popover togglePopover : frame];
		[popover setDelegate:self];
	}
	
	return self;
}

- (void) setupView
{
	NSString * string = nil, *complement = _isTome ? @"tome" : @"chapitre";
	
	if (_sizeArray == 1)
		string = [NSString stringWithFormat:@" J'ai remarqué %s y a un %@\nnon-téléchargé après\ncelui-là. Voulez vous\nque je le télécharge\npour vous?", _isTome ? "\nqu'il" : "qu'il\n", complement];
	else
		string = [NSString stringWithFormat:@" J'ai remarqué %s y a des %@s\nnon-téléchargés après\ncelui-là. Voulez vous\nque je les télécharge\npour vous?", _isTome ? "\nqu'il" : "qu'il\n", complement];
	
	RakText * contentText = [[RakText alloc] initWithText:self.frame :string :[Prefs getSystemColor : GET_COLOR_ACTIVE]];
	[contentText.cell setWraps:YES];
	[contentText setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
	[contentText sizeToFit];
	[self addSubview : contentText];
	[contentText setFrameOrigin:NSMakePoint(10 , self.frame.size.height - 10 - contentText.frame.size.height)];
}

- (void) configurePopover : (INPopoverController*) internalPopover
{
	internalPopover.borderColor = internalPopover.color = [[Prefs getSystemColor:GET_COLOR_INACTIVE] colorWithAlphaComponent:0.8];
	internalPopover.borderWidth = 4;
}

- (void)popoverDidClose:(INPopoverController *)discarded;
{
	[popover clearMemory];
	[popover release];
}

#pragma mark - Payload

- (void) confirmed
{
	for (uint pos = 0, handledArray = _sizeArray - 1; pos < handledArray; pos++)
	{
		[_tabMDL proxyAddElement: _project: _isTome : _arraySelection[pos] : NO];
	}

	[_tabMDL proxyAddElement: _project: _isTome: _arraySelection[_sizeArray - 1] : NO];
}
@end
