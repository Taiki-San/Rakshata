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
 ********************************************************************************************/

//Implementation massively based on Richard Heard's work

//  Copyright (c) 2013 Richard Heard. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//  1. Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//  3. The name of the author may not be used to endorse or promote products
//  derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
//  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

//define the private CGS methods we need to exclude a window from being captured.
// ------ PRIVATE API ------
typedef int CGSConnectionID;
typedef int CGSWindowID;
typedef CFTypeRef CGSRegionRef;

CG_EXTERN CGSConnectionID CGSMainConnectionID(void);

CG_EXTERN CGError CGSGetWindowBounds(CGSConnectionID cid, CGSWindowID wid, CGRect *rectOut);

CG_EXTERN CGError CGSNewRegionWithRect(const CGRect *rect, CGSRegionRef *regionOut);
CG_EXTERN BOOL CGSRegionIsEmpty(CGSRegionRef region);
CG_EXTERN CGError CGSReleaseRegion(CGSRegionRef region);

CG_EXTERN CGError CGSSetWindowCaptureExcludeShape(CGSConnectionID cid, CGSWindowID wid, CGSRegionRef region);
// ------ END PRIVATE API ------


//implementation

BOOL preventWindowCaptureForWindow(NSWindow *window)
{
#ifndef DEV_VERSION
	CGSConnectionID cid = CGSMainConnectionID();
	CGSWindowID wid = (int)window.windowNumber;
	
	CGRect windowRectOut;
	CGError err = CGSGetWindowBounds(cid, wid, &windowRectOut);
	if(err != kCGErrorSuccess)
	{
#ifdef EXTENSIVE_LOGGING
		NSLog(@"Error: failed to get windows bounds with error %i", err);
#endif
		return NO;
	}
	windowRectOut.origin = CGPointZero;
	
	CGSRegionRef regionOut;
	err = CGSNewRegionWithRect(&windowRectOut, &regionOut);
	if(err != kCGErrorSuccess)
	{
#ifdef EXTENSIVE_LOGGING
		NSLog(@"Error: failed to get create CGSRegion with error %i", err);
#endif
		return NO;
	}
	
	err = CGSSetWindowCaptureExcludeShape(cid, wid, regionOut);
	if(err != kCGErrorSuccess)
	{
#ifdef EXTENSIVE_LOGGING
		NSLog(@"Error: failed to set capture exclude shape for window with error %i", err);
#endif
		CGSReleaseRegion(regionOut);
		return NO;
	}
	
	CGSReleaseRegion(regionOut);
#endif
	return YES;
}
