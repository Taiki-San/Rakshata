#!/bin/sh

TMP_DIR="tmp_lib_update"
OUTPUT_DIR="updated_lib"
DL_FILE="download_base"
ARCHIVE_FILE="amalgation.zip"
LIBRESSL_FILES=( "libcrypto.a" "libssl.a" "libtls.a" )

export MACOSX_DEPLOYMENT_TARGET=10.9

default=1
sqlite=1
libressl=1
libcurl=1

for request
do
	#If we have arguments, we parse them and overwrite the default
	if [ $default = 1 ]
	then
		default=0
		sqlite=0
		libressl=0
		libcurl=0
	fi

	if [ $request = 'sqlite' ]; then
		sqlite=1
	elif [ $request = 'libressl' ]; then
		libressl=1
	elif [ $request = 'libcurl' ]; then
		libcurl=1
	fi
done

#If we couldn't parse the input but yet, it overwrote us
if [ sqlite = 0 ] && [ libressl = 0 ] && [ libcurl = 0 ]
then
	echo "Sorry, couldn't understand your request :("
	exit
fi

#Should we delete the temporary directory?
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
fi

cd $TMP_DIR

#Main loop
for i in {1..3}
do
	#SQLite
	if [ $i = 1 ]
	then
		if [ $sqlite = 1 ]
		then
			BASE_DL_URL="https://www.sqlite.org/"
			DL_PAGE=$BASE_DL_URL"download.html"
			ARCHIVE_DIR="sqlite_uncompress"
			SQLITE_SOURCE_FILE="sqlite3.c"
			SQLITE_HEADER_FILE="sqlite3.h"
			OUTPUT_FILE="libsqlite.a"
			
			#Nuking the old library
			if [ -f "../$OUTPUT_DIR/$OUTPUT_FILE" ]
			then
				rm "../$OUTPUT_DIR/$OUTPUT_FILE"
			fi
		
			#Nuking the old header
			if [ -f "../$OUTPUT_DIR/$SQLITE_HEADER_FILE" ]
			then
				rm "../$OUTPUT_DIR/$SQLITE_HEADER_FILE"
			fi
		else
			continue
		fi
	elif [ $i = 2 ]
	then
		if [ $libressl = 1 ]
		then
			BASE_DL_URL="http://ftp.openbsd.org/pub/OpenBSD/LibreSSL"
			DL_PAGE="http://www.libressl.org"
			
			for file in "${LIBRESSL_FILES[@]}"
			do
				if [ -f "../$OUTPUT_DIR/$file" ]
				then
					rm "../$OUTPUT_DIR/$file"
				fi
			done
			
			if [ -d "../$OUTPUT_DIR/openssl" ]
			then
				rm -rf "../$OUTPUT_DIR/openssl"
			fi
		else
			continue
		fi
	else
		if [ $libcurl = 1 ]
		then
			BASE_DL_URL="https://github.com/bagder/curl/releases/download"
			DL_PAGE="https://github.com/bagder/curl/releases/latest"
			OUTPUT_FILE="libcurl.a"
			CRAFTED_LIBRESSL_DIR="libressl_crafted"
			
			if [ -f "../$OUTPUT_DIR/$OUTPUT_FILE" ]
			then
				rm "../$OUTPUT_DIR/$OUTPUT_FILE"
			fi
			
			if [ -d "../$OUTPUT_DIR/libcurl" ]
			then
				rm -rf "../$OUTPUT_DIR/libcurl"
			fi
		else
			continue
		fi
	fi

	#We download the download page (yeah...) to get the current version
	echo "Downloading $DL_PAGE"
	wget $DL_PAGE -O $DL_FILE -o "wget_"$i"_1.log"

	#Compute the last version
	if [ $i = 1 ]
	then
		#Quick sanity check
		FINAL_DL_URL="$(grep 'version' $DL_FILE)"
		if [ "$FINAL_DL_URL" = '' ]
		then
			echo "Couldn't download the sqlite download page from $DL_PAGE or couldn't recognize a patern :("
			echo "Aborting, you should tweak my calibration"
			continue
		fi

		#Extract the JS portion containing the download URL, in my testing "d391('a1','2015/sqlite-amalgamation-3090100.zip');"
		FINAL_DL_URL="$(grep "'a1'," $DL_FILE)"
		if [ "$FINAL_DL_URL" = '' ]
		then
			echo "Couldn't recognize a patern in the download page :("
			echo "Aborting, you should tweak my calibration"
			continue
		fi

		#We extract the URL from the string
		IFS="'"
		set -- $FINAL_DL_URL
		FINAL_DL_URL=$4
		if [ "$FINAL_DL_URL" = '' ]
		then
			echo "Couldn't recognize a patern in the download page :("
			echo "Aborting, you should tweak my calibration"
			continue
		fi
		
		FINAL_DL_URL="$BASE_DL_URL$FINAL_DL_URL"
	elif [ $i = 2 ]
	then
		FINAL_DL_URL="$(grep "The latest stable release" $DL_FILE)"
		if [ "$FINAL_DL_URL" = '' ]
		then
			echo "Couldn't download the libressl front page from $DL_PAGE or couldn't recognize a patern :("
			echo "Aborting, you should tweak my calibration"
			continue
		fi

		NEW_VERSION="$(echo $FINAL_DL_URL | sed -e 's/.*release is \(.*\)<br.*/\1/')"
		if [ "$FINAL_DL_URL" = '' ]
		then
			echo "Couldn't determine the libressl version from front page :("
			echo "Aborting, you should tweak my calibration"
			continue
		fi

		FINAL_DL_URL="$BASE_DL_URL/libressl-$NEW_VERSION.tar.gz"

		echo "Downloading the signature"		
		wget "$FINAL_DL_URL.asc" -O $ARCHIVE_FILE.asc -o "wget_"$i"_3.log"
	elif [ $i = 3 ]
	then
		FINAL_DL_URL="$(grep "<title>Release curl" $DL_FILE)"
		if [ "$FINAL_DL_URL" = '' ]
		then
			echo "Couldn't download the libcURL github page from $DL_PAGE or couldn't recognize a patern :("
			echo "Aborting, you should tweak my calibration"
			continue
		fi
		
		NEW_VERSION="$(echo $FINAL_DL_URL | sed -e 's/.*curl \(.*\) · bagder.*/\1/')"
		if [ "$FINAL_DL_URL" = '' ]
		then
			echo "Couldn't determine the libcURL version from github page :("
			echo "Aborting, you should tweak my calibration"
			continue
		fi
		
		echo "Downloading the signature"
		wget "http://curl.haxx.se/download/curl-$NEW_VERSION.tar.gz.asc" -O $ARCHIVE_FILE.asc -o  "wget_"$i"_3.log"
		
		FINAL_DL_URL=$BASE_DL_URL"/curl-${NEW_VERSION//./_}/curl-$NEW_VERSION.tar.gz"
	fi

	#Download everything
	echo "Downloading the actual archive from $FINAL_DL_URL"
	wget $FINAL_DL_URL -O $ARCHIVE_FILE -o "wget_"$i"_2.log"

	#Computing downloaded hash
	printf "Downloaded, file's hash is "
	FILE_HASH="$(shasum $ARCHIVE_FILE)"

	IFS=' '
	set -- $FILE_HASH
	FILE_HASH=$1

	echo "$FILE_HASH"
	
	if [ $i = 1 ]
	then
		DL_FILE_LINE="$(grep "amalgamation</a>, version" $DL_FILE)"
		NEW_VERSION="$(echo $DL_FILE_LINE | sed -e 's/.*version \(.*\)\.<br>.*/\1/')"
	
		#Checking the hash with the hash contained within the download page
		if [ "$(echo $DL_FILE_LINE | grep "(sha1: $FILE_HASH)" )" = '' ]
		then
			echo "Coudldn't verify the hash! WTFH!?!"
			continue
		fi
	
		echo "Properly checked the hash with the original page, moving on"
	elif [ $i = 2 ] || [ $i = 3 ]
	then
		echo "Checking signature status"
		gpg2 --verify $ARCHIVE_FILE.asc $ARCHIVE_FILE 
		
		if [ ! $? -eq 0 ]
		then
			echo "Exiting because of the invalid signature"
			continue
		fi
	fi
	
	#Decompressing the archive
	echo "Decompressing the archive"

	if [ $i = 1 ]
	then
		unzip -j $ARCHIVE_FILE -d $ARCHIVE_DIR > unzip_sqlite.log
	elif [ $i = 2 ]
	then
		tar -zxf $ARCHIVE_FILE
		ARCHIVE_DIR="libressl-$NEW_VERSION"
	elif [ $i = 3 ]
	then
		tar -zxf $ARCHIVE_FILE
		ARCHIVE_DIR="curl-$NEW_VERSION"
	fi
	cd $ARCHIVE_DIR
	
	if [ $i = 1 ]
	then
		if [ ! -f "$SQLITE_SOURCE_FILE" ] || [ ! -f "$SQLITE_HEADER_FILE" ]
		then
			echo "$SQLITE_SOURCE_FILE or $SQLITE_HEADER_FILE is missing from the archive!"
			continue
		fi
		
		echo "Found everything we needed, compiling!"
		
		#Compiling
		clang -O3 -c $SQLITE_SOURCE_FILE -o sqlite3.o
		
		#Packaging the library
		ar rcs ../../$OUTPUT_DIR/$OUTPUT_FILE sqlite3.o
		mv $SQLITE_HEADER_FILE ../../$OUTPUT_DIR/$SQLITE_HEADER_FILE
	elif [ $i = 2 ]
	then
		echo "Configuring the package"
		./config > ../config_libressl.log
		
		if [ ! $? -eq 0 ]
		then
			echo "Configuration error D:"
			continue
		fi
		
		echo "Compiling"
		make > ../make_libressl.log
		
		if [ ! $? -eq 0 ]
		then
			echo "Compile error D:"
			continue
		fi

		echo "Collecting the libraries"
		mkdir lib
		mv */.libs/*.a lib
		
		cp -R include/openssl ../../$OUTPUT_DIR/
		
		cd lib

		for file in "${LIBRESSL_FILES[@]}"
		do
			cp $file ../../../$OUTPUT_DIR/$file
		done
		
		cd ..
	elif [ $i = 3 ]
	then
		if [ $libressl = 0 ]
		then
			mkdir ../libressl_crafted
			mkdir ../libressl_crafted/lib
			mkdir ../libressl_crafted/include
			
			for file in "${LIBRESSL_FILES[@]}"
			do
				cp ../../$file ../libressl_crafted/lib/
			done
			
			cp -R ../../openssl ../libressl_crafted/include/
		else
			CRAFTED_LIBRESSL_DIR=$LIBRESSL_DIR
		fi
	
		echo "Configuring the package"
		./configure --disable-debug --enable-optimize --enable-static --disable-shared --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-manual --disable-smb --enable-ipv6 --enable-threaded-resolver --disable-verbose --disable-sspi --disable-ntlm-wb --disable-cookies --disable-soname-bump --without-darwinssl --with-ssl=$(pwd)/../$CRAFTED_LIBRESSL_DIR --without-ca-bundle --without-libssh2 --without-librtmp --without-winidn --without-libidn --disable-libcurl-option > ../config_libcurl.log
		
		if [ ! $? -eq 0 ]
		then
			echo "Configuration error D:"
			continue
		fi
		
		echo "Compiling"
		make > ../make_libcurl.log
		
		if [ ! $? -eq 0 ]
		then
			echo "Compile error D:"
			continue
		fi
		
		echo "Collecting the library"
		mv lib/.libs/$OUTPUT_FILE ../../$OUTPUT_DIR/
		mkdir ../../$OUTPUT_DIR/libcurl
		cp include/curl/*.h  ../../$OUTPUT_DIR/libcurl/
	fi

	#Cleaning up
	echo "Done working! Updated to $NEW_VERSION! Cleaning up"

	cd ..
	rm "wget_"$i"_1.log" "wget_"$i"_2.log" $ARCHIVE_FILE $DL_FILE
	
	if [ $i = 1 ]
	then
		rm -rf $ARCHIVE_DIR
		rm unzip.log
	elif [ $i = 2 ]
	then
		if [ $libcurl = 1 ]
		then
			LIBRESSL_DIR=$ARCHIVE_DIR
		else
			rm -rf $ARCHIVE_DIR
		fi

		rm $ARCHIVE_FILE.asc config_libressl.log make_libressl.log "wget_"$i"_3.log" 
	elif [ $i = 3 ]
	then
		if [ $libressl = 1 ]
		then
			rm -rf $LIBRESSL_DIR
		else
			rm -rf $CRAFTED_LIBRESSL_DIR
		fi
		
		rm -rf $ARCHIVE_DIR
		rm $ARCHIVE_FILE.asc config_libcurl.log make_libcurl.log "wget_"$i"_3.log" 	
	fi
done

#Final cleanup
cd ..

if [ ! "$(ls $TMP_DIR)" ]
then
	rm -rf $TMP_DIR
fi