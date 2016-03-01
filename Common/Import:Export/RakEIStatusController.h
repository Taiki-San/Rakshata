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

#if TARGET_OS_IPHONE
@interface RakEIStatusController : UIViewController
{
#else
@interface RakEIStatusController : NSObject
{
	RakSheetWindow * queryWindow;

	RakMenuText * title;
	NSProgressIndicator * progressBar;
	RakText * percentage;
	RakButton * cancel;
#endif
	CGFloat cachedPosInExport, cachedPosInEntry;
}

@property BOOL haveCanceled;

@property (nonatomic) CGFloat nbElementToExport, posInExport;
@property (nonatomic) CGFloat nbElementInEntry, posInEntry;

#if TARGET_OS_IPHONE
	
@property NSURL * fileURL;
@property IBOutlet UINavigationItem * header;
@property IBOutlet UILabel * archiveLabel;

	
- (void) deleteImportedFile;
- (IBAction)closeUI;

#else

- (void) startUI;
- (void) closeUI;

#endif

- (void) refreshUI;
- (void) finishing;

- (NSString *) headerText;
- (RakColor *) backgroundColor;

@end
