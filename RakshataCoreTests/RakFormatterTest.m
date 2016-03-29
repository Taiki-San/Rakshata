//
//  RakFormatterTest.m
//  Rakshata
//
//  Created by Taiki on 29/03/2016.
//  Copyright © 2016 Taiki. All rights reserved.
//

#import <XCTest/XCTest.h>

@interface RakFormatterTest : XCTestCase
{
	RakCTFormatter * formatter;
}

@end

@implementation RakFormatterTest

- (void) setUp
{
	formatter = [[RakCTFormatter alloc] init];
}

- (void) testCTFormatterParser
{
	NSArray * data = @[@[@"40"], @[@"0"], @[@"4,2", @"4.2"], @[@"4.2"]];
	
	for(NSArray * entry in data)
	{
		NSString * parsed = [formatter stringForObjectValue:getNumberForString([entry firstObject])];
		
		XCTAssert([parsed isEqualToString:[entry lastObject]], @"Couldn't parse %@ (had %@ instead)", [entry firstObject], parsed);
	}
}

- (void) testCTFormatterValidate
{
	NSArray * data = @[@[@"1", @(YES)], @[@"0", @(YES)],
					   @[@"-1", @(YES)], @[@"10000000000000", @(YES)],
					   @[@"1.0", @(YES)], @[@"-1.5", @(YES)],
					   @[@"1.42", @(NO)], @[@"1.00", @(NO)],
					   @[@"-", @(YES)], @[@"--", @(NO)]];
	
	for(NSArray * entry in data)
	{
		XCTAssert([RakCTFormatter isStringValid:[entry firstObject]] == [[entry lastObject] boolValue], @"Failed the validation test: was expecting %s for string %@", [[entry lastObject] boolValue] ? "validation" : "rejection", [entry firstObject]);
	}
}

@end