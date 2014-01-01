/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#import "superHeader.h"

@implementation NSViewCustom

- (NSView *) setUpView: (NSView *)superView : (int)type
{
	NSRect frame = NSMakeRect([self getRequestedViewPosX: superView.frame.size.width : type], 0, [self getRequestedViewWidth: superView.frame.size.width : type], superView.frame.size.height);
	
	NSView * newTab = [[NSView alloc] initWithFrame:frame];
	[superView addSubview:newTab];
	
	/*		All the shit between this point and return is for debugging purposes	*/
	if(type == CREATE_CUSTOM_VIEW_TAB_SERIE)
	{
		frame.origin.y = frame.size.height * 0.25;
		frame.size.height *= 0.75;
	}
	else
		frame.origin.x = 0;
	
	NSColorWell *background = [[NSColorWell alloc] initWithFrame:frame];
	
	if(type == CREATE_CUSTOM_VIEW_TAB_SERIE)
		[background setColor:[NSColor redColor]];
	else if(type == CREATE_CUSTOM_VIEW_TAB_CT)
		[background setColor:[NSColor blueColor]];
	else
		[background setColor:[NSColor greenColor]];
		
	[background setBordered:NO];
	[newTab addSubview:background];
	[background release];
	
	return newTab;
}

- (int) convertTypeToPrefArg : (int) type : (bool) getX
{
	int arg;
	
	switch(type)
	{
		case CREATE_CUSTOM_VIEW_TAB_SERIE:
		{
			arg = PREFS_GET_TAB_SERIE_WIDTH;
			break;
		}
			
		case CREATE_CUSTOM_VIEW_TAB_CT:
		{
			arg = PREFS_GET_TAB_CT_WIDTH;
			break;
		}
			
		case CREATE_CUSTOM_VIEW_TAB_READER:
		{
			arg = PREFS_GET_TAB_READER_WIDTH;
			break;
		}
	}
	
	if(getX)
	{
		arg += PREFS_GET_TAB_SERIE_POSX - PREFS_GET_TAB_SERIE_WIDTH;
	}
	return arg;
}

- (int) getRequestedViewPosX: (int) widthWindow : (int) source
{
	return widthWindow * (int) [Prefs getPref:[self convertTypeToPrefArg:source:YES]] / 100;
}

- (int) getRequestedViewWidth:(int) widthWindow : (int) source
{
	return widthWindow * (int) [Prefs getPref:[self convertTypeToPrefArg:source:NO]] / 100;
}

@end