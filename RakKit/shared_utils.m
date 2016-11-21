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

#pragma mark - Icon loader

RakImage * getResImageWithName(NSString* baseName)
{
	return [[NSBundle mainBundle] imageForResource:baseName];
}
