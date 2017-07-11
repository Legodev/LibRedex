/* main.hpp
 *
 * Copyright 2016-2017 Desolation Redux
 *
 * Author: Legodev <legodevgit@mailbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef SOURCE_MAIN_HPP_
#define SOURCE_MAIN_HPP_

#ifdef __MINGW32__
	#if _WIN64
			#define RVExtension __stdcall RVExtension
			#define RVExtensionArgs __stdcall RVExtensionArgs
	#else
			#define RVExtension __stdcall _RVExtension
			#define RVExtensionArgs __stdcall _RVExtensionArgs
	#endif
#endif
#ifdef _MSC_VER
		#define RVExtension __stdcall RVExtension
		#define RVExtensionArgs __stdcall RVExtensionArgs
#endif

#ifdef DEBUG
#include <fstream>
#include <mutex>
#endif

#include <sstream>
#include <string>
#include <string.h>            // strcmp, strncpy
#include <map>

#include "constants.hpp"
extern std::map<std::string, unsigned int> objectvariablemap;
std::map<std::string, unsigned int> objectvariablemap;

#include "redex.hpp"
redex * extension = 0;

#ifdef DEBUG
std::mutex ThreadMutex;
int attachedThreadCount = 0;
std::ofstream testfile("LibRedExLogFile.txt", std::ios::out | std::ios::trunc);
std::string escapeChars(std::string input) {
        std::stringstream outputstream;

        for (unsigned int i = 0; i < input.length(); i++) {
                switch(input[i]) {
                        case '-': if (input[i+1] > '0' && input[i+1] < '9') { outputstream << "-"; }; break;
                        case ';': break;
                        case '#': break;
                        case '"': if (i > 0 && i < input.length() - 1) { outputstream << "\\\""; }; break;
                        case '\\': outputstream << "\\\\"; break;
                        default: outputstream << input[i]; break;
                }
        }

        return outputstream.str();
}
#endif

static void init(void)
{
//#ifdef DEBUG
//    testfile.open("LibRedExLogFile.txt", std::ios::out | std::ios::trunc);
//    testfile << "starting init" << std::endl;
//    testfile.flush();
//#endif

    if (extension == 0) {
        extension = new redex();
    }

//#ifdef DEBUG
//    testfile << "done init" << std::endl;
//    testfile.flush();
//#endif
}

static void destroy(void)
{
#ifdef DEBUG
    testfile << "starting destroy" << std::endl;
#endif
    
    if (extension != 0) {
#ifdef DEBUG
        testfile << "deleting object" << std::endl;
#endif
        delete extension;
#ifdef DEBUG
        testfile << "resetting pointer" << std::endl;
#endif
        extension = 0;
    }

#ifdef DEBUG
    testfile << "done destroy" << std::endl;
    testfile.flush();
    testfile.close();
#endif
}

#if defined(__linux__)
        static void __attribute__((constructor))
        extensioninit(void)
        {
            init();
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

        BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
        {
                switch (ul_reason_for_call)
                {
                        case DLL_PROCESS_ATTACH:
				init();
                        break;
                        case DLL_THREAD_ATTACH:
#ifdef DEBUG
				ThreadMutex.lock();
				attachedThreadCount++;
				testfile << "done thread attach Threadcount: " << attachedThreadCount << std::endl;
				testfile.flush();
				ThreadMutex.unlock();
#endif
                        break;
                        case DLL_THREAD_DETACH:
#ifdef DEBUG
				ThreadMutex.lock();
				attachedThreadCount--;
				testfile << "done thread detach Threadcount: " << attachedThreadCount << std::endl;
				testfile.flush();
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

	__declspec (dllexport) void RVExtension(char *output, int outputSize, const char *function);
	__declspec (dllexport) void RVExtensionArgs(char *output, int outputSize, const char *function, const char **args, int argsCnt);

	#ifdef __cplusplus
		}
	#endif
#endif

#endif /* SOURCE_MAIN_HPP_ */
