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

void logR(const char *error, ...)
{
	va_list args;
	va_start(args, error);
	
#ifdef __APPLE__
	sendToLog(error, args);
#else
    FILE* logFile = fopen("log", "a+");
    if(logFile != NULL)
    {
        if(error != NULL && error[0] != '0')
        {
			vfprintf(logFile, error, args);
			fputc('\n', logFile);
        }
        fclose(logFile);
    }
#endif
	
	va_end(args);
}

int libcurlErrorCode(CURLcode code)
{
    int ret_value = CODE_RETOUR_DL_CLOSE;
	
    switch(code)
    {
        case CURLE_FAILED_INIT :
        {
            logR("Initialization failed");
            break;
        }

        case CURLE_URL_MALFORMAT:
        {
            logR("URL is malformated");
            break;
        }
        case CURLE_COULDNT_RESOLVE_PROXY:
        {
            logR("Failed at resolve the proxy");
            break;
        }
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        {
            ret_value = CODE_FAILED_AT_RESOLVE;
            break;
        }
			
		case CURLE_RECV_ERROR:
        case CURLE_PARTIAL_FILE:
        {
            logR("Partial file");
            ret_value = CODE_RETOUR_PARTIAL;
            break;
        }
        case CURLE_OUT_OF_MEMORY:
        {
			logR("Everything is screwed up...");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_OPERATION_TIMEDOUT:
        {
            logR("Request timed out");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_ABORTED_BY_CALLBACK:
		case CURLE_WRITE_ERROR:
        {
            ret_value = CODE_RETOUR_DL_CLOSE;
            break;
        }
		case CURLE_SSL_CONNECT_ERROR:
        case CURLE_SSL_CACERT_BADFILE:
        {
			logR("TLS error");
            ret_value = CODE_RETOUR_INTERNAL_FAIL;
            break;
        }
        case CURLE_SSL_CACERT:
        {
            break;
        }

        default:
        {
            logR("Unknown libcURL error: %d (%s)", code, curl_easy_strerror(code));
            break;
        }
    }
	
    return ret_value;
}

void memoryError(size_t size)
{
#ifndef __APPLE__
    logR("Failed at allocate memory for : %d bytes", size);
#else
    logR("Failed at allocate memory for : %ld bytes", size);
#endif
}

