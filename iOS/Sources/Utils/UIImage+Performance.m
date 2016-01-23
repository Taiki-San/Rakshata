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

//+imageByRenderingData is heavily based on the work available on NSProgrammer.com
//Follow the copyright of the original version (https://github.com/NSProgrammer/NSProgrammer/blob/master/code/NOBUILib/UIImage%2BASyncRendering.m)

/*
 
 Copyright (C) 2013 Nolan O'Brien
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 */

@implementation UIImage (Performance)

+ (UIImage *) imageByRenderingData : (NSData*) imageData
{
	if(imageData == nil)
		return nil;
	
	UIImage* imageObj = nil;
	BOOL isPng = isPNG(imageData.bytes);
	BOOL isJpg = isJPEG(imageData.bytes);
	
	if(!isPng && !isJpg)	//Not really sure about the format, we discard the optimization
		return [UIImage imageWithData:imageData];
	
	CGDataProviderRef dataProvider = CGDataProviderCreateWithCFData((__bridge CFDataRef)imageData);
	if (dataProvider != nil)
	{
		CGImageRef image = NULL;
		
		if (isPng)
			image = CGImageCreateWithPNGDataProvider(dataProvider, NULL, NO, kCGRenderingIntentDefault);
		else
			image = CGImageCreateWithJPEGDataProvider(dataProvider, NULL, NO, kCGRenderingIntentDefault);
		
		if (image != nil)
		{
			size_t width = CGImageGetWidth(image);
			size_t height = CGImageGetHeight(image);
			byte * imageBuffer = malloc(width * height * 4);
			if(imageBuffer != NULL)
			{
				CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
				if(colorSpace != nil)
				{
					CGContextRef imageContext = CGBitmapContextCreate(imageBuffer,
																	  width,
																	  height,
																	  8,
																	  width * 4,
																	  colorSpace,
																	  (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little));
					
					if (imageContext != nil)
					{
						CGContextDrawImage(imageContext, CGRectMake(0, 0, width, height), image);
						CGImageRef outputImage = CGBitmapContextCreateImage(imageContext);
						if (outputImage)
						{
							imageObj = [UIImage imageWithCGImage:outputImage
														   scale:[UIScreen mainScreen].scale
													 orientation:UIImageOrientationUp];
							
							CGImageRelease(outputImage);
						}
						
						CGContextRelease(imageContext);
					}
					
					CGColorSpaceRelease(colorSpace);
				}
				
				free(imageBuffer);
			}
			
			CGImageRelease(image);
		}
		
		CGDataProviderRelease(dataProvider);
	}
	
	return imageObj;
}

@end
