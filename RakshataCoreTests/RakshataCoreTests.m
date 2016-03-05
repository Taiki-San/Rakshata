//
//  RakshataCoreTests.m
//  RakshataCoreTests
//
//  Created by Taiki on 04/03/2016.
//  Copyright © 2016 Taiki. All rights reserved.
//

#import <XCTest/XCTest.h>

@interface RakshataCoreTests : XCTestCase

@end

@implementation RakshataCoreTests

//Perform the core initialization
+ (void) setUp
{
	if(!checkFileExist(SETTINGS_FILE".swp"))
		rename(SETTINGS_FILE, SETTINGS_FILE".swp");
	else
		remove(SETTINGS_FILE);
}

+ (void) tearDown
{
	remove(SETTINGS_FILE);
	rename(SETTINGS_FILE".swp", SETTINGS_FILE);
}

- (void) setUp
{
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
	remove(SETTINGS_FILE);

    [super tearDown];
}

- (void) testSettingsLoading
{
	//Create a dummy file
	const char * expectedSettingsFile = "<"SETTINGS_PROJECTDB_FLAG">\n===\n</"SETTINGS_PROJECTDB_FLAG">\n<"SETTINGS_EMAIL_FLAG">\ntest@yay.com\n</"SETTINGS_EMAIL_FLAG">\n<"SETTINGS_REPODB_FLAG">\n===\n</"SETTINGS_REPODB_FLAG">\n";
	AESEncrypt(SETTINGS_PASSWORD, expectedSettingsFile, SETTINGS_FILE, INPUT_IN_MEMORY);
	
	//Start testing
	char * settings = loadPrefFile();
	XCTAssert(settings != NULL, @"Coudn't load the settings file");
	
	XCTAssert(strlen(settings) == strlen(expectedSettingsFile) && !strncmp(settings, expectedSettingsFile, strlen(expectedSettingsFile)), @"Incorrect settings file loading");
	free(settings);
	
	bool retValue = loadEmailProfile();
	XCTAssert(retValue && COMPTE_PRINCIPAL_MAIL != NULL, @"Couldn't load email");
	XCTAssert(!strcmp(COMPTE_PRINCIPAL_MAIL, "test@yay.com"), @"Loaded incorrect email");
	
	[self measureBlock:^{
		deleteEmail();
		XCTAssert(COMPTE_PRINCIPAL_MAIL == NULL, @"Couldn't delete the email adress");
		
		bool retValue = loadEmailProfile();
		XCTAssert(!retValue, @"Couldn't delete the email adress");
		
		addToPref(SETTINGS_EMAIL_FLAG, "test@yay.com");
		retValue = loadEmailProfile();
		XCTAssert(retValue && COMPTE_PRINCIPAL_MAIL != NULL, @"Couldn't load email after update!");
	}];
}

@end
