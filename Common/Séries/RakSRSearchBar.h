/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#define SEARCH_BAR_ID_NBCASES 2
enum
{
	SEARCH_BAR_ID_MAIN		= 0,
	SEARCH_BAR_ID_AUTHOR	= 1,
	SEARCH_BAR_ID_SOURCE	= 2,
	SEARCH_BAR_ID_TAG		= 3,
	SEARCH_BAR_ID_CAT		= 4,
	SEARCH_BAR_ID_EXTRA		= 5		//Unused in the search bar, but to stick to the scheme
};

enum
{
	SEARCH_BAR_ID_MAIN_TRIGGERED 	= 1,
	SEARCH_BAR_ID_MAIN_TYPED 		= 2,
	SEARCH_BAR_ID_AUTHOR_TRIGGERED	= 3,
	SEARCH_BAR_ID_AUTHOR_TYPED		= 4,
	SEARCH_BAR_ID_SOURCE_TRIGGERED	= 5,
	SEARCH_BAR_ID_SOURCE_TYPED		= 6,
	SEARCH_BAR_ID_TAG_TRIGGERED		= 7,
	SEARCH_BAR_ID_TAG_TYPED			= 8,
	SEARCH_BAR_ID_TYPE_TRIGGERED	= 9,
	SEARCH_BAR_ID_TYPE_TYPED		= 10,
	SEARCH_BAR_ID_FORCE_CLOSE
};

@interface RakSRSearchBar : NSSearchField <NSSearchFieldDelegate>
{
	BOOL _currentPlaceholderState, deletinChar, didAuthorizeSearch;
	byte _ID;
	
	BOOL noRecursive, normalKeyPressed;
	
	uint64_t * indexes;
	NSArray <NSString *> * data;
}

- (instancetype) initWithFrame : (NSRect) frameRect : (byte) ID;
- (instancetype) initWithFrame : (NSRect) frameRect ID : (byte) ID andData : (charType **) names ofSize : (uint) nbData andIndexes : (uint64_t *) listIndexes;

- (void) updateData : (charType **) names ofSize : (uint) nbData andIndexes : (uint64_t *) listIndexes;

+ (void) triggeringSearchBar : (BOOL) goingIn : (byte) ID;

- (void) resizeAnimation : (NSRect) frame;

- (void) updatePlaceholder : (BOOL) inactive;
- (void) willLooseFocus;

+ (RakColor *) getBackgroundColor;

@end

@interface RakSRSearchBarCell : NSSearchFieldCell

@end
