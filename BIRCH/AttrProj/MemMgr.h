/*
  ========================================================================
  DEVise Data Visualization Software
  (c) Copyright 1992-1996
  By the DEVise Development Group
  Madison, Wisconsin
  All Rights Reserved.
  ========================================================================

  Under no circumstances is this software to be copied, distributed,
  or altered in any way without prior permission from the DEVise
  Development Group.
*/

/*
  $Id: MemMgr.h 2396 2014-03-13 21:13:27Z wkliao $

  $Log: MemMgr.h,v $
  Revision 1.2  1996/12/13 21:34:39  jussi
  Added checking of available semaphores and shared memory.

  Revision 1.1  1996/12/03 20:28:49  jussi
  Initial revision.
*/

#ifndef MemMgr_h
#define MemMgr_h

#include "DeviseTypes.h"
#include "DCE.h"

// Memory manager

class MemMgr {
  public:
    MemMgr(int numPages, int pageSize, int &status);
    ~MemMgr();

    // Page types
    enum PageType { Cache, Buffer };

    // Set maximum usage of a page type
    int SetMaxUsage(PageType type, int pages);

    // Allocate a memory buffer of one page; block until memory available
    int Allocate(PageType type, char *&buf) {
        int pages = 1;
        return Allocate(type, buf, pages);
    }

    // Allocate a memory buffer of given size; block until at least one
    // page available
    int Allocate(PageType type, char *&buf, int &pages, Boolean block = true);

    // Try to allocate a memory buffer of one page; return -1 if no
    // memory available
    int Try(PageType type, char *&buf) {
        int pages = 1;
        return Try(type, buf, pages);
    }        

    // Try to allocate a memory buffer of given size; return -1 if no
    // memory available
    int Try(PageType type, char *&buf, int &pages) {
        return Allocate(type, buf, pages, false);
    }

    // Deallocate a memory buffer of given size
    int Deallocate(PageType type, char *buf, int pages = 1);

    // Convert buffer memory to cache memory, or vice versa
    int Convert(char *buf, PageType oldType, PageType &newType);

    // Return number of free memory size in pages
    int NumFree() {
        AcquireMutex();
        int num = _count->free;
        ReleaseMutex();
        return num;
    }

    // Dump contents of free page table
    void Dump();

    int PageSize() { return _pageSize; }
    int NumPages() { return _numPages; }

    static MemMgr *Instance() { return _instance; }

  protected:
    // Allocation and initialization
    int SetupSharedMemory();
    int SetupLocalMemory();
    int Initialize();

    // Acquire and release mutex
    void AcquireMutex() { if (_sem) _sem->acquire(1); }
    void ReleaseMutex() { if (_sem) _sem->release(1); }

    // Acquire and release free semaphore
    void AcquireFree() { if (_free) _free->acquire(1); }
    void ReleaseFree() { if (_free) _free->release(1); }

    // Number of memory pages, free page table size, and page size
    const int _numPages;
    const int _tableSize;
    const int _pageSize;

    // Base address of memory
    char *_buf;

    char **_freePage;                   // table of free memory chunks
    int *_freePageCount;                // # of pages in each chunk
    struct CountStruct {
        int entries;                    // valid entries in free table
        int free;                       // # of free pages left
        int cache;                      // # of cache pages in use
        int buffer;                     // # of buffer pages in use
        int maxCache;                   // max # of cache pages allowed
        int maxBuffer;                  // max # of buffer pages allowed
    } *_count;

    // An instance of this class
    static MemMgr *_instance;

    // Mutex for synchronization
    SemaphoreV *_sem;
    SemaphoreV *_free;

    // Shared memory
    SharedMemory *_shm;
};

// Data types for I/O

typedef unsigned long long streampos_t;
typedef unsigned long long bytecount_t;
typedef unsigned long iosize_t;

// Data Pipe

class DataPipe {
  public:
    DataPipe(int maxSize, int &status);
    ~DataPipe();

    int Consume(char *&buf, streampos_t &offset, iosize_t &bytes);
    int Produce(char *buf, streampos_t offset, iosize_t bytes);
    int SetSize(int size);

    int NumData() {
        AcquireMutex();
        int num = _count->size - _count->free;
        ReleaseMutex();
        return num;
    }

  protected:
    // Initialize
    int Initialize(int maxSize);

    // Acquire and release mutex
    void AcquireMutex() { _sem->acquire(1); }
    void ReleaseMutex() { _sem->release(1); }

    // Acquire and release free semaphore
    void AcquireFree() { _free->acquire(1); }
    void ReleaseFree() { _free->release(1); }

    // Acquire and release data semaphore
    void AcquireData() { _data->acquire(1); }
    void ReleaseData() { _data->release(1); }

    SemaphoreV *_sem;                   // mutex for synchronization
    SemaphoreV *_free;
    SemaphoreV *_data;

    SharedMemory *_shm;                 // shared memory

    int _maxSize;                       // maximum pipe size

    char **_chunk;                      // pointers to data chunks
    streampos_t *_offset;               // offset of data chunks
    iosize_t *_bytes;                   // length of data chunks
    struct CountStruct {
        int head;                       // index of first data chunk
        int tail;                       // index of last data chunk
        int free;                       // number of free data chunks
        int size;                       // current pipe size
        int maxSize;                    // maximum pipe size
    } *_count;
};

// Multi Pipe

class MultiPipe {
  public:
    MultiPipe(int &status);
    ~MultiPipe();

    // Add and remove pipe from multipipe
    int AddPipe(DataPipe *pipe);
    int RemovePipe(DataPipe *pipe);

    // Produce and consume data
    int Consume(char *&buf, streampos_t &offset,
                iosize_t &bytes, DataPipe *&pipe);

  protected:
    // Acquire and release data semaphore
    void AcquireData() { _data->acquire(1); }
    void ReleaseData() { _data->release(1); }

    SemaphoreV *_data;

    static const int _maxPipes = 32;
    DataPipe *_pipes[_maxPipes];
    int _numPipes;
    int _hint;
};

#endif
