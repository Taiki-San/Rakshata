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

#pragma mark - Preference size utilities

NSRect prefsPercToFrame(NSRect percentage, NSSize superview)
{
	percentage.origin.x *= superview.width / 100.0f;
	percentage.origin.y *= superview.height / 100.0f;
	percentage.size.width *= superview.width / 100.0f;
	percentage.size.height *= superview.height / 100.0f;
	
	return percentage;
}

CGFloat percToSize(CGFloat percentage, CGFloat superview, CGFloat max)
{
	percentage *= superview / 100.0f;
	
	if(max != -1 && percentage > max)
		percentage = max;
	
	return percentage;
}

#pragma mark - Custom color loader

#include "JSONParser.h"

NSArray * loadCustomColor(const char * file)
{
	//Load file content
	NSData * fileContent = [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:file]];
	
	if(fileContent == nil)
	{
		NSLog(@"Couldn't load the content of the custom color file, sorry!");
		return nil;
	}
	
	//Parse JSON
	NSError * parseError = nil;
	NSDictionary * parseData = [NSJSONSerialization JSONObjectWithData:fileContent options:0 error:&parseError];
	
	if(parseData == nil)
	{
		NSLog(@"JSON error... :%@", parseError);
		return nil;
	}
	
	//List all the available codes
	byte ID[] = {COLOR_INACTIVE, COLOR_SURVOL, COLOR_ACTIVE, COLOR_HIGHLIGHT, COLOR_CLICKABLE_TEXT, COLOR_INSERTION_POINT, COLOR_TEXTFIELD_BACKGROUND, COLOR_ICON_ACTIVE, COLOR_ICON_INACTIVE, COLOR_ICON_DISABLE, COLOR_EXTERNALBORDER_FAREST, COLOR_EXTERNALBORDER_MIDDLE, COLOR_EXTERNALBORDER_MIDDLE_NON_MAIN, COLOR_EXTERNALBORDER_CLOSEST, COLOR_TITLEBAR_BACKGROUND_MAIN, COLOR_TITLEBAR_BACKGROUND_GRADIENT_START, COLOR_TITLEBAR_BACKGROUND_GRADIENT_END, COLOR_TITLEBAR_BACKGROUND_STANDBY, COLOR_TABS_BACKGROUND, COLOR_TABS_BORDER, COLOR_BACKGROUND_DRAG_AND_DROP, COLOR_COREVIEW_BACKGROUND, COLOR_COREVIEW_BORDER, COLOR_BUTTON_BORDER, COLOR_BUTTON_BACKGROUND_UNSELECTED, COLOR_BUTTON_BACKGROUND_SELECTED, COLOR_BUTTON_TEXT_NONCLICKED, COLOR_BUTTON_TEXT_HIGHLIGHT, COLOR_BUTTON_TEXT_CLICKED, COLOR_BUTTON_TEXT_UNAVAILABLE, COLOR_BUTTON_SWITCH_BORDER, COLOR_BUTTON_SWITCH_BACKGROUND_OFF, COLOR_BUTTON_SWITCH_BACKGROUND_MIXED, COLOR_BUTTON_SWITCH_BACKGROUND_ON, COLOR_BUTTON_STATUS_BACKGROUND, COLOR_BUTTON_STATUS_OK, COLOR_BUTTON_STATUS_WARN, COLOR_BUTTON_STATUS_ERROR, COLOR_BACK_BUTTONS_BACKGROUND, COLOR_BACK_BUTTONS_BACKGROUND_ANIMATING, COLOR_LIST_SELECTED_BACKGROUND, COLOR_DANGER_POPOVER_BORDER, COLOR_DANGER_POPOVER_TEXT_COLOR, COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED, COLOR_GRID_FOCUS_BACKGROUND, COLOR_SR_PLACEHOLDER_TEXT, COLOR_FILTER_FOREGROUND, COLOR_TAGITEM_BORDER, COLOR_TAGITEM_BACKGROUND, COLOR_TAGITEM_FONT, COLOR_SEARCHBAR_BACKGROUND, COLOR_SEARCHBAR_PLACEHOLDER_TEXT, COLOR_SEARCHBAR_SELECTION_BACKGROUND, COLOR_SEARCHBAR_SELECTION_TEXT, COLOR_SEARCHBAR_BORDER, COLOR_SEARCHTAB_BACKGROUND, COLOR_SEARCHTAB_PEAK_BACKGROUND, COLOR_SEARCHTAB_BORDER_BAR, COLOR_SEARCHTAB_BORDER_COLLAPSED, COLOR_SEARCHTAB_BORDER_DEPLOYED, COLOR_CTHEADER_GRADIENT_START, COLOR_CTHEADER_GRADIENT_END, COLOR_CTHEADER_FONT, COLOR_CTLIST_BACKGROUND, COLOR_READER_BAR, COLOR_READER_BAR_FRONT, COLOR_READER_BAR_PAGE_COUNTER, COLOR_READER_BACKGROUND_INTAB, COLOR_PDF_BACKGROUND, COLOR_PROGRESSLINE_SLOT, COLOR_PROGRESSLINE_PROGRESS, COLOR_PREFS_HEADER_BACKGROUND, COLOR_PREFS_HEADER_BORDER, COLOR_PREFS_BUTTON_FOCUS, COLOR_REPO_LIST_BACKGROUND, COLOR_REPO_LIST_ITEM_BACKGROUND, COLOR_REPO_TEXT_PLACEHOLDER, COLOR_ADD_REPO_BACKGROUND, COLOR_DROP_AREA_BACKGROUND, COLOR_EXPORT_BACKGROUND};
	
	char * names[] = {STRINGIZE(COLOR_INACTIVE), STRINGIZE(COLOR_SURVOL), STRINGIZE(COLOR_ACTIVE), STRINGIZE(COLOR_HIGHLIGHT), STRINGIZE(COLOR_CLICKABLE_TEXT), STRINGIZE(COLOR_INSERTION_POINT), STRINGIZE(COLOR_TEXTFIELD_BACKGROUND), STRINGIZE(COLOR_ICON_ACTIVE), STRINGIZE(COLOR_ICON_INACTIVE), STRINGIZE(COLOR_ICON_DISABLE), STRINGIZE(COLOR_EXTERNALBORDER_FAREST), STRINGIZE(COLOR_EXTERNALBORDER_MIDDLE), STRINGIZE(COLOR_EXTERNALBORDER_MIDDLE_NON_MAIN), STRINGIZE(COLOR_EXTERNALBORDER_CLOSEST), STRINGIZE(COLOR_TITLEBAR_BACKGROUND_MAIN), STRINGIZE(COLOR_TITLEBAR_BACKGROUND_GRADIENT_START), STRINGIZE(COLOR_TITLEBAR_BACKGROUND_GRADIENT_END), STRINGIZE(COLOR_TITLEBAR_BACKGROUND_STANDBY), STRINGIZE(COLOR_TABS_BACKGROUND), STRINGIZE(COLOR_TABS_BORDER), STRINGIZE(COLOR_BACKGROUND_DRAG_AND_DROP), STRINGIZE(COLOR_COREVIEW_BACKGROUND), STRINGIZE(COLOR_COREVIEW_BORDER), STRINGIZE(COLOR_BUTTON_BORDER), STRINGIZE(COLOR_BUTTON_BACKGROUND_UNSELECTED), STRINGIZE(COLOR_BUTTON_BACKGROUND_SELECTED), STRINGIZE(COLOR_BUTTON_TEXT_NONCLICKED), STRINGIZE(COLOR_BUTTON_TEXT_HIGHLIGHT), STRINGIZE(COLOR_BUTTON_TEXT_CLICKED), STRINGIZE(COLOR_BUTTON_TEXT_UNAVAILABLE), STRINGIZE(COLOR_BUTTON_SWITCH_BORDER), STRINGIZE(COLOR_BUTTON_SWITCH_BACKGROUND_OFF), STRINGIZE(COLOR_BUTTON_SWITCH_BACKGROUND_MIXED), STRINGIZE(COLOR_BUTTON_SWITCH_BACKGROUND_ON), STRINGIZE(COLOR_BUTTON_STATUS_BACKGROUND), STRINGIZE(COLOR_BUTTON_STATUS_OK), STRINGIZE(COLOR_BUTTON_STATUS_WARN), STRINGIZE(COLOR_BUTTON_STATUS_ERROR), STRINGIZE(COLOR_BACK_BUTTONS_BACKGROUND), STRINGIZE(COLOR_BACK_BUTTONS_BACKGROUND_ANIMATING), STRINGIZE(COLOR_LIST_SELECTED_BACKGROUND), STRINGIZE(COLOR_DANGER_POPOVER_BORDER), STRINGIZE(COLOR_DANGER_POPOVER_TEXT_COLOR), STRINGIZE(COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED), STRINGIZE(COLOR_GRID_FOCUS_BACKGROUND), STRINGIZE(COLOR_SR_PLACEHOLDER_TEXT), STRINGIZE(COLOR_FILTER_FOREGROUND), STRINGIZE(COLOR_TAGITEM_BORDER), STRINGIZE(COLOR_TAGITEM_BACKGROUND), STRINGIZE(COLOR_TAGITEM_FONT), STRINGIZE(COLOR_SEARCHBAR_BACKGROUND), STRINGIZE(COLOR_SEARCHBAR_PLACEHOLDER_TEXT), STRINGIZE(COLOR_SEARCHBAR_SELECTION_BACKGROUND), STRINGIZE(COLOR_SEARCHBAR_SELECTION_TEXT), STRINGIZE(COLOR_SEARCHBAR_BORDER), STRINGIZE(COLOR_SEARCHTAB_BACKGROUND), STRINGIZE(COLOR_SEARCHTAB_PEAK_BACKGROUND), STRINGIZE(COLOR_SEARCHTAB_BORDER_BAR), STRINGIZE(COLOR_SEARCHTAB_BORDER_COLLAPSED), STRINGIZE(COLOR_SEARCHTAB_BORDER_DEPLOYED), STRINGIZE(COLOR_CTHEADER_GRADIENT_START), STRINGIZE(COLOR_CTHEADER_GRADIENT_END), STRINGIZE(COLOR_CTHEADER_FONT), STRINGIZE(COLOR_CTLIST_BACKGROUND), STRINGIZE(COLOR_READER_BAR), STRINGIZE(COLOR_READER_BAR_FRONT), STRINGIZE(COLOR_READER_BAR_PAGE_COUNTER), STRINGIZE(COLOR_READER_BACKGROUND_INTAB), STRINGIZE(COLOR_PDF_BACKGROUND), STRINGIZE(COLOR_PROGRESSLINE_SLOT), STRINGIZE(COLOR_PROGRESSLINE_PROGRESS), STRINGIZE(COLOR_PREFS_HEADER_BACKGROUND), STRINGIZE(COLOR_PREFS_HEADER_BORDER), STRINGIZE(COLOR_PREFS_BUTTON_FOCUS), STRINGIZE(COLOR_REPO_LIST_BACKGROUND), STRINGIZE(COLOR_REPO_LIST_ITEM_BACKGROUND), STRINGIZE(COLOR_REPO_TEXT_PLACEHOLDER), STRINGIZE(COLOR_ADD_REPO_BACKGROUND), STRINGIZE(COLOR_DROP_AREA_BACKGROUND), STRINGIZE(COLOR_EXPORT_BACKGROUND)};
	
	NSMutableArray * mutableOutput = [NSMutableArray arrayWithCapacity:sizeof(ID)];
	
	//Iterate the parsed data to correlate the content with our codes
	for(uint pos = 0; pos < sizeof(ID); pos++)
	{
		NSArray * currentEntry = objectForKey(parseData, [NSString stringWithFormat:@"%d", ID[pos]], [NSString stringWithUTF8String:names[pos]], [NSArray class]);
		
		if(currentEntry == nil)
		{
			NSLog(@"Hupf, couldn't find `%s` in the custom font file, aborting loading", names[pos]);
			return nil;
		}
		else if(![currentEntry count] || [currentEntry count] > 4 )
		{
			NSLog(@"Invalid entry for key `%s`", names[pos]);
			return nil;
		}
		
		//Check the content of the array is valid
		__block BOOL validData = YES;
		[currentEntry enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
			if(ARE_CLASSES_DIFFERENT(obj, [NSNumber class]))
			{
				validData = NO;
				*stop = YES;
			}
		}];
		
		if(!validData)
		{
			NSLog(@"Invalid content for key `%s`", names[pos]);
			return nil;
		}
		
		//Load the color

		NSColor * newColor = nil;
		switch ([currentEntry count])
		{
			case 1:
			{
				newColor = [NSColor colorWithDeviceWhite:[[currentEntry objectAtIndex:0] unsignedCharValue] / 255.0f alpha:1];
				break;
			}

			case 2:
			{
				newColor = [NSColor colorWithDeviceWhite:[[currentEntry objectAtIndex:0] unsignedCharValue] / 255.0f
												   alpha:[[currentEntry objectAtIndex:1] unsignedCharValue] / 255.0f];
				break;
			}

			case 3:
			{
				newColor = [NSColor colorWithSRGBRed:[[currentEntry objectAtIndex:0] unsignedCharValue] / 255.0f
												green:[[currentEntry objectAtIndex:1] unsignedCharValue] / 255.0f
												 blue:[[currentEntry objectAtIndex:2] unsignedCharValue] / 255.0f
												alpha:1];
				break;
			}

			case 4:
			{
				newColor = [NSColor colorWithSRGBRed:[[currentEntry objectAtIndex:0] unsignedCharValue] / 255.0f
											   green:[[currentEntry objectAtIndex:1] unsignedCharValue] / 255.0f
												blue:[[currentEntry objectAtIndex:2] unsignedCharValue] / 255.0f
											   alpha:[[currentEntry objectAtIndex:3] unsignedCharValue] / 255.0f];
				break;
			}
		}

		if(ID[pos] >= [mutableOutput count])
			[mutableOutput insertObject:newColor atIndex:ID[pos]];
		else
			[mutableOutput replaceObjectAtIndex:ID[pos] withObject:newColor];
	}
	
	return [NSArray arrayWithArray:mutableOutput];
}

#pragma mark - Icon loader

@implementation RakResPath

+ (NSImage *) getImage: (NSString*) baseName
{
	return [[NSBundle mainBundle] imageForResource:baseName];
}

@end