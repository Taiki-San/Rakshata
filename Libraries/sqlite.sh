#!/bin/sh

#SQLite compiler

#Setup the environment
TMP_DIR="tmp_lib_update"
BASE_DL_URL="https://www.sqlite.org/"
DL_PAGE=$BASE_DL_URL"download.html"
DL_FILE="download_sqlite"
ARCHIVE_FILE="amalgation.zip"
ARCHIVE_DIR="sqlite_uncompress"
SQLITE_SOURCE_FILE="sqlite3.c"
SQLITE_HEADER_FILE="sqlite3.h"
OUTPUT_DIR="updated_lib"
OUTPUT_FILE="libsqlite.a"

#should we delete the temporary directory?
if [ -d "$TMP_DIR" ]
then
	echo "tmp directory detected, should we delete it? [Y/n]"
	
	finish=0
	while [ "$finish" = 0 ]
	do
		finish=1
		read answer
		
		#default case
		if [ "$answer" = '' ]
		then
			answer='y'
		else
			case $answer in
				y | Y | yes | YES ) answer='y';;
				n | N | no | NO ) answer='n';;
				*) finish=0;
			esac
		fi
	done
	
	if [ "$answer" = 'y' ]
	then
		rm -rf $TMP_DIR
		mkdir $TMP_DIR
	fi
else
	mkdir $TMP_DIR
fi

#Tweaking the release directory
if [ ! -d "$OUTPUT_DIR" ]
then
	mkdir "$OUTPUT_DIR"
else
	#Nuking the old library
	if [ -e "$OUTPUT_DIR/$OUTPUT_FILE" ]
	then
		rm "$OUTPUT_DIR/$OUTPUT_FILE"
	fi

	#Nuking the old header
	if [ -e "$OUTPUT_DIR/$SQLITE_HEADER_FILE" ]
	then
		rm "$OUTPUT_DIR/$SQLITE_HEADER_FILE"
	fi
fi

#We download the download page (yeah...) to get the current version
cd $TMP_DIR
echo "Downloading $DL_PAGE"
wget $DL_PAGE -O $DL_FILE -o wget_sqlite_1.log

#Quick sanity check
FINAL_DL_URL="$(grep 'version' $DL_FILE)"
if [ "$FINAL_DL_URL" = '' ]
then
	echo "Couldn't download the sqlite download page from $DL_PAGE or couldn't recognize a patern :("
	echo "Aborting, you should tweak my calibration: $FINAL_DL_URL"
	exit
fi

#Extract the JS portion containing the download URL, in my testing "d391('a1','2015/sqlite-amalgamation-3090100.zip');"
FINAL_DL_URL="$(grep "'a1'," $DL_FILE)"
if [ "$FINAL_DL_URL" = '' ]
then
	echo "Couldn't recognize a patern in the download page :("
	echo "Aborting, you should tweak my calibration"
	exit
fi

#We extract the URL from the string
IFS="'"
set -- $FINAL_DL_URL
FINAL_DL_URL=$4
if [ "$FINAL_DL_URL" = '' ]
then
	echo "Couldn't recognize a patern in the download page :("
	echo "Aborting, you should tweak my calibration"
	exit
fi

#Add the begining of the URL and download everything
FINAL_DL_URL="$BASE_DL_URL$FINAL_DL_URL"
echo "Downloading the actual archive from $FINAL_DL_URL"
wget $FINAL_DL_URL -O $ARCHIVE_FILE -o wget_sqlite_2.log

#Computing downloaded hash
printf "Downloaded, file's hash is "
FILE_HASH="$(shasum $ARCHIVE_FILE)"

IFS=' '
set -- $FILE_HASH
FILE_HASH=$1

echo "$FILE_HASH"

DL_FILE_LINE="$(grep "amalgamation</a>, version" $DL_FILE)"
NEW_VERSION="$(echo $DL_FILE_LINE | sed -e 's/.*version \(.*\)\.<br>.*/\1/')"

#Checking the hash with the hash contained within the download page
if [ "$(echo $DL_FILE_LINE | grep "(sha1: $FILE_HASH)" )" = '' ]
then
	echo "Coudldn't verify the hash! WTFH!?!"
	exit
fi

echo "Properly checked the hash with the original page, moving on"

#Decompressing the archive
unzip -j $ARCHIVE_FILE -d $ARCHIVE_DIR > unzip.log
cd $ARCHIVE_DIR

if [ ! -e "$SQLITE_SOURCE_FILE" ] || [ ! -e "$SQLITE_HEADER_FILE" ]
then
	echo "$SQLITE_SOURCE_FILE or $SQLITE_HEADER_FILE is missing from the archive!"
	exit
fi

echo "Found everything we needed, compiling!"

#Setting up the compiler options
export MACOSX_DEPLOYMENT_TARGET=10.9

#Compiling
clang -O3 -c $SQLITE_SOURCE_FILE -o sqlite3.o

#Packaging the library
ar rcs ../../$OUTPUT_DIR/$OUTPUT_FILE sqlite3.o
mv $SQLITE_HEADER_FILE ../../$OUTPUT_DIR/$SQLITE_HEADER_FILE

#Cleaning up
echo "Done working! Updated to $NEW_VERSION! Cleaning up"

cd ..
rm -r $ARCHIVE_DIR
rm wget_sqlite_*.log unzip.log $ARCHIVE_FILE $DL_FILE
cd ..

if [ ! "$(ls $TMP_DIR)" ]
then
	rm -rf $TMP_DIR
fi