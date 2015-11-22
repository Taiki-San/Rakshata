/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

#ifdef _WIN32
	typedef struct _UNICODE_STRING
	{
      USHORT Length;
      USHORT MaximumLength;
      PWSTR  Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;

    typedef struct _OBJECT_ATTRIBUTES {
      ULONG Length;
      HANDLE RootDirectory;
      PUNICODE_STRING ObjectName;
      ULONG Attributes;
      PVOID SecurityDescriptor;
      PVOID SecurityQualityOfService;
    } OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

    typedef int(__stdcall *FUNC)(HANDLE* hThread,int DesiredAccess,OBJECT_ATTRIBUTES* ObjectAttributes, HANDLE ProcessHandle,void* lpStartAddress,void* lpParameter,unsigned long CreateSuspended_Flags,unsigned long StackZeroBits,unsigned long SizeOfStackCommit,unsigned long SizeOfStackReserve,void* lpBytesBuffer);
    FUNC ZwCreateThreadEx;

    #ifdef DEV_VERSION
        #define SECURE_THREADS 0x0
    #else
        #define SECURE_THREADS 0x4
    #endif
#else

    struct thread_info /* Used as argument to thread_start() */
    {
        pthread_t thread_id;        /* ID returned by pthread_create() */
        int       thread_num;       /* Application-defined thread # */
        char     *argv_string;      /* From command-line argument */
    };

    #include <errno.h>
#endif

void createNewThread(void *function, void *arg)
{
#ifdef _WIN32
    if(ZwCreateThreadEx == NULL)
    {
        ZwCreateThreadEx = (FUNC)GetProcAddress(GetModuleHandle("ntdll.dll"), "ZwCreateThreadEx");
        if(ZwCreateThreadEx == NULL)
			alertExit("Failed at export primitives");
    }
    if(ZwCreateThreadEx != NULL)
    {
        HANDLE hThread = 0;
        ZwCreateThreadEx(&hThread, GENERIC_ALL, 0, GetCurrentProcess(), function, arg, SECURE_THREADS, 0, 0, 0, 0);
        CloseHandle(hThread);
    }

#else
    pthread_t thread;

    if(pthread_create(&thread, NULL, function, arg))
        alertExit("Failed at create thread");
#endif
}

THREAD_TYPE createNewThreadRetValue(void *function, void *arg)
{
    THREAD_TYPE threadID;

#ifdef _WIN32
    if(ZwCreateThreadEx == NULL)
    {
        ZwCreateThreadEx = (FUNC)GetProcAddress(GetModuleHandle("ntdll.dll"),"ZwCreateThreadEx");
        if(ZwCreateThreadEx == NULL)
			alertExit("Failed at create thread");
    }
    if(ZwCreateThreadEx != NULL)
    {
        ZwCreateThreadEx(&threadID, GENERIC_ALL, 0, GetCurrentProcess(), function, arg, SECURE_THREADS, 0, 0, 0, 0);
    }

#else

    if(pthread_create(&threadID, NULL, function, arg))
        alertExit("Failed at create thread");

#endif

    return threadID;
}

bool isThreadStillRunning(THREAD_TYPE hThread)
{
#ifdef _WIN32
    return WaitForSingleObject( hThread, 0) != WAIT_OBJECT_0;
#else
    return pthread_kill(hThread, 0) != ESRCH;
#endif // _WIN32
}
