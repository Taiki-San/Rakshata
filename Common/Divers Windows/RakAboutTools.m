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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakAboutContent

- (void)drawRect:(NSRect)dirtyRect
{
	if(_haveAdditionalDrawing)
		_additionalDrawing(_bounds.size);
}

@end

@implementation RakAboutIcon

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [super initWithFrame : frame];
	
	if(self != nil)
	{
		NSImage * image = [[NSWorkspace sharedWorkspace] iconForFile:[[NSBundle mainBundle] bundlePath]];
		
		if(image == nil)
			return nil;
		
		self.image = image;
		self.imageScaling = NSImageScaleAxesIndependently;
	}
	
	return self;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if(_clicResponder != nil)
		[_clicResponder clicIcon];
}

@end