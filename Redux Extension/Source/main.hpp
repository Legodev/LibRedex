/* main.hpp
 *
 * Copyright 2016-2020 Desolation Redux
 *
 * Author: Legodev <legodevgit@mailbox.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 */

#ifndef SOURCE_MAIN_HPP_
#define SOURCE_MAIN_HPP_

#ifdef __MINGW32__
	#if _WIN64
			#define RVExtensionVersion __stdcall RVExtensionVersion
			#define RVExtension __stdcall RVExtension
			#define RVExtensionArgs __stdcall RVExtensionArgs
			#define RVExtensionRegisterCallback __stdcall RVExtensionRegisterCallback
	#else
			#define RVExtensionVersion __stdcall _RVExtensionVersion
			#define RVExtension __stdcall _RVExtension
			#define RVExtensionArgs __stdcall _RVExtensionArgs
			#define RVExtensionRegisterCallback __stdcall _RVExtensionRegisterCallback
	#endif
#endif
#ifdef _MSC_VER
		#define RVExtensionVersion __stdcall RVExtensionVersion
		#define RVExtension __stdcall RVExtension
		#define RVExtensionArgs __stdcall RVExtensionArgs
		#define RVExtensionRegisterCallback __stdcall RVExtensionRegisterCallback
#endif

#include "logger.hpp"

#include <sstream>
#include <string>
#include <cstring>
#include <map>

#include "constants.hpp"

#include "redex.hpp"

redex * extension = nullptr;
Logger * logfile = nullptr;
std::ofstream * logfilehandler = nullptr;

int(*callbackPtr)(char const *name, char const *function, char const *data) = nullptr;

#ifdef DEBUG
std::mutex ThreadMutex;
int attachedThreadCount = 0;
#endif

#if defined(__linux__)
static void init(const char* filepath)
#else
static void init(TCHAR* filepath)
#endif
{
    if (extension == nullptr) {
        std::stringstream basepath;
        basepath << filepath;
        std::string string = basepath.str();
        fprintf(stderr, "starting libredex with string: %s \n", string.c_str());

        if (logfile == nullptr) {
          boost::filesystem::path LibRedexFilePath = string;
          LibRedexFilePath = LibRedexFilePath.parent_path() / "libredex.log";
          logfilehandler = new std::ofstream(LibRedexFilePath.string(), std::ios::out | std::ios::trunc);
          logfile = new Logger(*logfilehandler);
        }
        (*logfile) << "starting libredex with string: " << string << std::endl;
        logfile->flush();
#ifndef DEBUG
        (*logfile) << "DEBUG output is disabled, use debug libredex to enable the debug output!" << std::endl;
        logfile->flush();
#endif
        extension = new redex(string);
    }
}

static void destroy(void)
{
     (*logfile) << "starting libredex shutdown" << std::endl;
    
     if (extension != nullptr) {
#ifdef DEBUG
        (*logfile) << "deleting extension object" << std::endl;
#endif
        delete extension;
#ifdef DEBUG
        (*logfile) << "resetting extension pointer" << std::endl;
#endif
        extension = nullptr;
     }

    (*logfile) << "finished libredex shutdown" << std::endl;
    logfile->flush();
    logfile->close();
    delete logfile;
    delete logfilehandler;
}

#if defined(__linux__)
#include <dlfcn.h>
        static void __attribute__((constructor))
        extensioninit(void)
        {
            Dl_info dl_info;
            dladdr((void *)extensioninit, &dl_info);
            fprintf(stderr, "module %s loaded\n", dl_info.dli_fname);
            init(dl_info.dli_fname);
        }

        static void __attribute__((destructor))
        extensiondestroy(void)
        {
            destroy();
        }
#endif

#ifndef __linux__
	#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers
	#include <windows.h>           // Windows Header Files
	#ifdef _MSC_VER
		#include <boost/config/compiler/visualc.hpp>
	#endif
	TCHAR LibRedexFilePath[512 + 1] = { 0 };

        BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
        {
                switch (ul_reason_for_call)
                {
                        case DLL_PROCESS_ATTACH:
				GetModuleFileNameA(hModule, LibRedexFilePath, 512);
				init(LibRedexFilePath);
                        break;
                        case DLL_THREAD_ATTACH:
#ifdef DEBUG
				ThreadMutex.lock();
				attachedThreadCount++;
				 (*logfile) << "done thread attach Threadcount: " << attachedThreadCount << std::endl;
				 (*logfile) << "dll path is: " << LibRedexFilePath << std::endl;
				logfile->flush();
				ThreadMutex.unlock();
#endif
                        break;
                        case DLL_THREAD_DETACH:
#ifdef DEBUG
				ThreadMutex.lock();
				attachedThreadCount--;
				 (*logfile) << "done thread detach Threadcount: " << attachedThreadCount << std::endl;
				logfile->flush();
				ThreadMutex.unlock();
#endif
                        break;
                        case DLL_PROCESS_DETACH:
				destroy();
                        break;
                }
                return TRUE;
        }


	#ifdef __cplusplus
		extern "C" {
	#endif
	__declspec (dllexport) void RVExtensionVersion(char *output, int outputSize);
	__declspec (dllexport) void RVExtension(char *output, int outputSize, const char *function);
	__declspec (dllexport) void RVExtensionArgs(char *output, int outputSize, const char *function, const char **args, int argsCnt);
	__declspec (dllexport) void RVExtensionRegisterCallback(int(*callbackProc)(char const *name, char const *function, char const *data));

	#ifdef __cplusplus
		}
	#endif
#endif

#endif /* SOURCE_MAIN_HPP_ */
