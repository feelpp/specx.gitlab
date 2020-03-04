///////////////////////////////////////////////////////////////////////////
// Spetabaru - Berenger Bramas MPCDF - 2017
// Under LGPL Licence, please you must read the LICENCE file.
///////////////////////////////////////////////////////////////////////////
#ifndef SPUTILS_HPP
#define SPUTILS_HPP

#include <thread>
#include <string>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifdef __APPLE__
#include "MacOSX.hpp"
#else    
#include <sched.h>
#endif

#include <cassert>

#include "SpDebug.hpp"

/**
 * Utils methods
 */
namespace SpUtils{
    /** Return the default number of threads using hardware's capacity */
    inline int DefaultNumThreads(){
        return static_cast<int>(std::thread::hardware_concurrency());
    }

    inline void BindToCore(const int inCoreId){
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(inCoreId, &mask);
        
        #ifdef __APPLE__
            [[maybe_unused]] int retValue = macosspecific::sched_setaffinity_np(pthread_self(), sizeof(mask), &mask);
            assert(retValue == 0);
        #else
            pid_t tid = static_cast<pid_t>(syscall(SYS_gettid));
            [[maybe_unused]] int retValue = sched_setaffinity(tid, sizeof(mask), &mask);
            assert(retValue == 0);
        #endif

        SpDebugPrint() << "Bind to " << inCoreId << " core";
    }

    /** Return the current binding (bit a position n is set to 1 when bind
     * to core n)
     */
    inline cpu_set_t GetBinding(){
        cpu_set_t mask;
        CPU_ZERO(&mask);
        
        #ifdef __APPLE__
            [[maybe_unused]] int retValue = macosspecific::sched_getaffinity_np(pthread_self(), sizeof(mask), &mask);
            assert(retValue == 0);
        #else
            pid_t tid = static_cast<pid_t>(syscall(SYS_gettid));
            // Get the affinity
            [[maybe_unused]] int retValue = sched_getaffinity(tid, sizeof(mask), &mask);
            assert(retValue == 0);
        #endif
        
        return mask;
    }

    /** Return the current thread id */
    long int GetThreadId();

    /** Return true if the code is executed from a task */
    inline bool IsInTask(){
        return GetThreadId() != 0;
    }

    /** Set current thread Id */
    void SetThreadId(const long int inThreadId);

    /** Replace all substring in a string */
    inline std::string ReplaceAllInString(std::string sentence, const std::string& itemToReplace, const std::string& inSubstitutionString){
        for(std::string::size_type idxFound = sentence.find(itemToReplace); idxFound != std::string::npos; idxFound = sentence.find(itemToReplace)){
            sentence.replace(idxFound, itemToReplace.size(), inSubstitutionString);
        }
        return sentence;
    }

    /** To perform some assert/check */
    inline void CheckCorrect(const char* test, const bool isCorrect, const int line, const char* file){
        if(!isCorrect){
            std::cout << "Error in file " << file << " line " << line << std::endl;
            std::cout << "Test was " << test << std::endl;
            exit(-1);
        }
    }
    
    template <template <typename...> typename Template, typename T>
    struct is_instantiation_of : std::false_type {};
    
    template <template <typename...> typename Template, typename... Args>
    struct is_instantiation_of<Template, Template<Args...> > : std::true_type {};
}

#define spetabaru_xstr(s) spetabaru_str(s)
#define spetabaru_str(s) #s
#define always_assert(X) SpUtils::CheckCorrect(spetabaru_str(X), (X), __LINE__, __FILE__)

#endif
