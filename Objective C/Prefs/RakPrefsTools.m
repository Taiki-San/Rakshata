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
	byte ID[] = {COLOR_EXTERNALBORDER_FAREST, COLOR_EXTERNALBORDER_MIDDLE, COLOR_EXTERNALBORDER_MIDDLE_NON_MAIN, COLOR_EXTERNALBORDER_CLOSEST, COLOR_INACTIVE, COLOR_SURVOL, COLOR_ACTIVE, COLOR_HIGHLIGHT, COLOR_READER_BAR, COLOR_READER_BAR_FRONT, COLOR_READER_BAR_PAGE_COUNTER, COLOR_BACKGROUND_READER_INTAB, COLOR_BACKGROUND_TABS, COLOR_BACKGROUND_DRAG_AND_DROP, COLOR_BORDER_TABS, COLOR_BACKGROUND_BACK_BUTTONS, COLOR_BACKGROUND_BACK_BUTTONS_ANIMATING, COLOR_TEXT_CT_SELECTOR_UNAVAILABLE, COLOR_TEXT_CT_SELECTOR_CLICKED, COLOR_TEXT_CT_SELECTOR_NONCLICKED, COLOR_BACKGROUND_CT_TVCELL, COLOR_BACKGROUND_COREVIEW, COLOR_PROGRESSCIRCLE_SLOT, COLOR_PROGRESSLINE_PROGRESS, COLOR_CLICKABLE_TEXT, COLOR_INSERTION_POINT, COLOR_SEARCHBAR_BACKGROUND, COLOR_SEARCHBAR_PLACEHOLDER_TEXT, COLOR_SEARCHBAR_SELECTION_BACKGROUND, COLOR_SEARCHBAR_SELECTION_TEXT, COLOR_SEARCHBAR_BORDER, COLOR_SEARCHTAB_BACKGROUND, COLOR_SEARCHTAB_BORDER_BAR, COLOR_SEARCHTAB_BORDER_COLLAPSED, COLOR_SEARCHTAB_BORDER_DEPLOYED, COLOR_SRPLACEHOLDER_TEXT, COLOR_BORDERS_COREVIEWS, COLOR_FONT_BUTTON_NONCLICKED, COLOR_FONT_BUTTON_HIGHLIGHT, COLOR_FONT_BUTTON_CLICKED, COLOR_FONT_BUTTON_UNAVAILABLE, COLOR_BORDER_BUTTONS, COLOR_BACKGROUND_BUTTON_UNSELECTED, COLOR_BACKGROUND_BUTTON_SELECTED, COLOR_FILTER_FORGROUND, COLOR_DANGER_POPOVER_BORDER, COLOR_DANGER_POPOVER_TEXT_COLOR, COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED, COLOR_BACKGROUND_TEXTFIELD, COLOR_CTHEADER_GRADIENT_START, COLOR_CTHEADER_GRADIENT_END, COLOR_CTHEADER_FONT, COLOR_TITLEBAR_BACKGROUND_MAIN, COLOR_TITLEBAR_BACKGROUND_GRADIENT_START, COLOR_TITLEBAR_BACKGROUND_GRADIENT_END, COLOR_TITLEBAR_BACKGROUND_STANDBY, COLOR_TAGITEM_BORDER, COLOR_TAGITEM_BACKGROUND, COLOR_TAGITEM_FONT, COLOR_BACKGROUND_GRID_FOCUS, COLOR_PREFS_BUTTONS_FOCUS, COLOR_BACKGROUND_PREFS_HEADER, COLOR_BORDER_PREFS_HEADER, COLOR_BACKGROUND_REPO_LIST, COLOR_BACKGROUND_REPO_LIST_ITEM, COLOR_PLACEHOLDER_REPO, COLOR_BACKGROUND_ADD_REPO, COLOR_BACKGROUND_SWITCH_BUTTON_OFF, COLOR_BACKGROUND_SWITCH_BUTTON_MIXED, COLOR_BACKGROUND_SWITCH_BUTTON_ON, COLOR_BORDER_SWITCH_BUTTON, COLOR_BACKGROUND_CT_LIST};
	
	char * names[] = {STRINGIZE(COLOR_EXTERNALBORDER_FAREST), STRINGIZE(COLOR_EXTERNALBORDER_MIDDLE), STRINGIZE(COLOR_EXTERNALBORDER_MIDDLE_NON_MAIN), STRINGIZE(COLOR_EXTERNALBORDER_CLOSEST), STRINGIZE(COLOR_INACTIVE), STRINGIZE(COLOR_SURVOL), STRINGIZE(COLOR_ACTIVE), STRINGIZE(COLOR_HIGHLIGHT), STRINGIZE(COLOR_READER_BAR), STRINGIZE(COLOR_READER_BAR_FRONT), STRINGIZE(COLOR_READER_BAR_PAGE_COUNTER), STRINGIZE(COLOR_BACKGROUND_READER_INTAB), STRINGIZE(COLOR_BACKGROUND_TABS), STRINGIZE(COLOR_BACKGROUND_DRAG_AND_DROP), STRINGIZE(COLOR_BORDER_TABS), STRINGIZE(COLOR_BACKGROUND_BACK_BUTTONS), STRINGIZE(COLOR_BACKGROUND_BACK_BUTTONS_ANIMATING), STRINGIZE(COLOR_TEXT_CT_SELECTOR_UNAVAILABLE), STRINGIZE(COLOR_TEXT_CT_SELECTOR_CLICKED), STRINGIZE(COLOR_TEXT_CT_SELECTOR_NONCLICKED), STRINGIZE(COLOR_BACKGROUND_CT_TVCELL), STRINGIZE(COLOR_BACKGROUND_COREVIEW), STRINGIZE(COLOR_PROGRESSCIRCLE_SLOT), STRINGIZE(COLOR_PROGRESSLINE_PROGRESS), STRINGIZE(COLOR_CLICKABLE_TEXT), STRINGIZE(COLOR_INSERTION_POINT), STRINGIZE(COLOR_SEARCHBAR_BACKGROUND), STRINGIZE(COLOR_SEARCHBAR_PLACEHOLDER_TEXT), STRINGIZE(COLOR_SEARCHBAR_SELECTION_BACKGROUND), STRINGIZE(COLOR_SEARCHBAR_SELECTION_TEXT), STRINGIZE(COLOR_SEARCHBAR_BORDER), STRINGIZE(COLOR_SEARCHTAB_BACKGROUND), STRINGIZE(COLOR_SEARCHTAB_BORDER_BAR), STRINGIZE(COLOR_SEARCHTAB_BORDER_COLLAPSED), STRINGIZE(COLOR_SEARCHTAB_BORDER_DEPLOYED), STRINGIZE(COLOR_SRPLACEHOLDER_TEXT), STRINGIZE(COLOR_BORDERS_COREVIEWS), STRINGIZE(COLOR_FONT_BUTTON_NONCLICKED), STRINGIZE(COLOR_FONT_BUTTON_CLICKED), STRINGIZE(COLOR_FONT_BUTTON_HIGHLIGHT), STRINGIZE(COLOR_FONT_BUTTON_UNAVAILABLE), STRINGIZE(COLOR_BORDER_BUTTONS), STRINGIZE(COLOR_BACKGROUND_BUTTON_UNSELECTED), STRINGIZE(COLOR_BACKGROUND_BUTTON_SELECTED), STRINGIZE(COLOR_FILTER_FORGROUND), STRINGIZE(COLOR_DANGER_POPOVER_BORDER), STRINGIZE(COLOR_DANGER_POPOVER_TEXT_COLOR), STRINGIZE(COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED), STRINGIZE(COLOR_BACKGROUND_TEXTFIELD), STRINGIZE(COLOR_CTHEADER_GRADIENT_START), STRINGIZE(COLOR_CTHEADER_GRADIENT_END), STRINGIZE(COLOR_CTHEADER_FONT), STRINGIZE(COLOR_TITLEBAR_BACKGROUND_MAIN), STRINGIZE(COLOR_TITLEBAR_BACKGROUND_GRADIENT_START), STRINGIZE(COLOR_TITLEBAR_BACKGROUND_GRADIENT_END), STRINGIZE(COLOR_TITLEBAR_BACKGROUND_STANDBY), STRINGIZE(COLOR_TAGITEM_BORDER), STRINGIZE(COLOR_TAGITEM_BACKGROUND), STRINGIZE(COLOR_TAGITEM_FONT), STRINGIZE(COLOR_BACKGROUND_GRID_FOCUS), STRINGIZE(COLOR_PREFS_BUTTONS_FOCUS), STRINGIZE(COLOR_BACKGROUND_PREFS_HEADER), STRINGIZE(COLOR_BORDER_PREFS_HEADER), STRINGIZE(COLOR_BACKGROUND_REPO_LIST), STRINGIZE(COLOR_BACKGROUND_REPO_LIST_ITEM), STRINGIZE(COLOR_PLACEHOLDER_REPO), STRINGIZE(COLOR_BACKGROUND_ADD_REPO), STRINGIZE(COLOR_BACKGROUND_SWITCH_BUTTON_OFF), STRINGIZE(COLOR_BACKGROUND_SWITCH_BUTTON_MIXED), STRINGIZE(COLOR_BACKGROUND_SWITCH_BUTTON_ON), STRINGIZE(COLOR_BORDER_SWITCH_BUTTON), STRINGIZE(COLOR_BACKGROUND_CT_LIST)};
	
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
