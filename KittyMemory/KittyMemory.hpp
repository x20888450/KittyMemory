//
//  KittyMemory.hpp
//
//  Created by MJ (Ruit) on 1/1/19.
//

#pragma once

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/mman.h>
#include <vector>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <libkern/OSCacheControl.h>
#endif

#define _SYS_PAGE_SIZE_ (sysconf(_SC_PAGE_SIZE))

#define _PAGE_START_OF_(x) ((uintptr_t)x & ~(uintptr_t)(_SYS_PAGE_SIZE_ - 1))
#define _PAGE_END_OF_(x, len) (_PAGE_START_OF_((uintptr_t)x + len - 1))
#define _PAGE_LEN_OF_(x, len) (_PAGE_END_OF_(x, len) - _PAGE_START_OF_(x) + _SYS_PAGE_SIZE_)
#define _PAGE_OFFSET_OF_(x) ((uintptr_t)x - _PAGE_START_OF_(x))

#define KITTY_LOG_TAG "KittyMemory"

#ifdef __ANDROID__
#include <android/log.h>

#ifdef kITTYMEMORY_DEBUG
#define KITTY_LOGD(fmt, ...) ((void)__android_log_print(ANDROID_LOG_DEBUG, KITTY_LOG_TAG, fmt, ##__VA_ARGS__))
#else
#define KITTY_LOGD(fmt, ...)
#endif

#define KITTY_LOGI(fmt, ...) ((void)__android_log_print(ANDROID_LOG_INFO, KITTY_LOG_TAG, fmt,  ##__VA_ARGS__))
#define KITTY_LOGE(fmt, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, KITTY_LOG_TAG, fmt, ##__VA_ARGS__))

#elif __APPLE__
#include <os/log.h>

#ifdef kITTYMEMORY_DEBUG
#define KITTY_LOGD(fmt, ...) os_log(OS_LOG_DEFAULT, KITTY_LOG_TAG ": " fmt, ##__VA_ARGS__)
#else
#define KITTY_LOGD(fmt, ...)
#endif

#define KITTY_LOGI(fmt, ...) os_log(OS_LOG_DEFAULT, KITTY_LOG_TAG ": " fmt, ##__VA_ARGS__)
#define KITTY_LOGE(fmt, ...) os_log_error(OS_LOG_DEFAULT, KITTY_LOG_TAG ": " fmt, ##__VA_ARGS__)

#endif

namespace KittyMemory
{

    /*
     * mprotect wrapper
     */
    int setAddressProtection(void *address, size_t length, int protection);

    /*
     * Reads an address content into a buffer
     */
    bool memRead(void *buffer, const void *address, size_t len);

#ifdef __ANDROID__

    class ProcMap {
    public:
        unsigned long long startAddress;
        unsigned long long endAddress;
        size_t length;
        int protection;
        bool readable, writeable, executable, is_private, is_shared, is_ro, is_rw, is_rx;
        unsigned long long offset;
        std::string dev;
        unsigned long inode;
        std::string pathname;

        ProcMap() : startAddress(0), endAddress(0), length(0), protection(0),
		            readable(false), writeable(false), executable(false),
                    is_private(false), is_shared(false),
                    is_ro(false), is_rw(false), is_rx(false),
                    offset(0), inode(0) {}

        inline bool isValid() const { return (length > 0); }
        inline bool isUnknown() const { return pathname.empty(); }
    };

    /*
     * Writes buffer content to an address
     */
    bool memWrite(void *address, const void *buffer, size_t len);

    /*
     * /proc/self/cmdline
     */
    std::string getProcessName();

    /*
     * Gets info of all maps in current process
     */
    std::vector<ProcMap> getAllMaps();

    /*
     * Gets info of all maps which contain "name" in current process
     */
    std::vector<ProcMap> getMapsByName(const std::string& name);

    /*
     * Gets map info of an address in self process
     */
    ProcMap getAddressMap(const void *address);

    /*
     * Gets the base map of a loaded shared object
     */
    ProcMap getBaseMapOf(const std::string& name);

#elif __APPLE__

    enum Memory_Status {
      KMS_FAILED = 0,
      KMS_SUCCESS,
      KMS_INV_ADDR,
      KMS_INV_LEN,
      KMS_INV_BUF,
      KMS_ERR_PROT,
      KMS_ERR_GET_PAGEINFO,
      KMS_ERR_MMAP,
      KMS_ERR_REMAP,
      KMS_ERR_VMCOPY,
    };

    class MemoryFileInfo {
    public:
      uint32_t index;
      const mach_header *header;
      const char *name;
      intptr_t address;

      MemoryFileInfo() : index(0), header(nullptr), name(nullptr), address(0) {}
    };

    /*
     * Writes buffer content to an address
     */
    Memory_Status memWrite(void *address, const void *buffer, size_t len);

    /*
     * vm_region_recurse_64 wrapper
     */
    kern_return_t getPageInfo(void *page_start, vm_region_submap_short_info_64 *info_out);

    /*
     * returns base executable info
     */
    MemoryFileInfo getBaseInfo();

    /*
     * find in memory file info by checking if target loaded object file ends with @fileName
     */
    MemoryFileInfo getMemoryFileInfo(const std::string& fileName);

    /*
     * returns the absolue address of a relative offset of a file in memory or NULL as fileName for base executable
     */
    uintptr_t getAbsoluteAddress(const char *fileName, uintptr_t address);

#endif

}