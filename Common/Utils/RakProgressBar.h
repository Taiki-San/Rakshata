/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@interface RakProgressBar : RakView
{
	NSSize currentSize;
	CGFloat _width;
	
	CGFloat _percentage;

	int64_t _speed;
	RakText * _speedText;
	
	RakColor * slotColor;
	RakColor * progressColor;
	
	uint _posX;
	
	BOOL wasOffsetSet;
}

@property (nonatomic) CGFloat offsetYSpeed;

+ (CGFloat) getLeftBorder;
+ (CGFloat) getRightBorder;

- (void) updatePercentage : (CGFloat) percentage : (size_t) downloadSpeed;
- (void) updatePercentageProxy : (NSArray*) percentage;

- (void) setRightTextBorder : (uint) posX;

- (void) notifyNeedDisplay;
- (void) centerText;

@end
