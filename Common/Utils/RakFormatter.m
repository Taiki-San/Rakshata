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
 ********************************************************************************************/

@implementation RakCTFormatter

+ (BOOL) isStringValid : (NSString *) string
{
	if([string length] == 0)
		return YES;
	
	NSString *expression = @"^([0-9]+)?([,.]([0-9])?)?$";
	NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:expression
																		   options:NSRegularExpressionCaseInsensitive
																			 error:nil];
	if([regex numberOfMatchesInString:string
							  options:0
								range:NSMakeRange(0, [string length])] == 0)
		return NO;

	return YES;
}

- (BOOL) isPartialStringValid : (NSString *) partialString newEditingString : (NSString**) newString errorDescription : (NSString**) error
{
	return [[self class] isStringValid:partialString];
}

@end