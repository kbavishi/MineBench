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
  $Id: MemMgr.C,v 1.3 1996/12/13 22:59:39 jussi Exp $

  $Log: MemMgr.C,v $
  Revision 1.3  1996/12/13 22:59:39  jussi
  Added missing return statement.

  Revision 1.2  1996/12/13 21:34:38  jussi
  Added checking of available semaphores and shared memory.

  Revision 1.1  1996/12/03 20:28:48  jussi
  Initial revision.
*/

#include <memory.h>

#include "MemMgr.h"
#include "Exit.h"

#define DEBUGLVL 0

MemMgr *MemMgr::_instance = 0;

MemMgr::MemMgr(int numPages, int pageSize, int &status) :
	_numPages(numPages), _tableSize(numPages), _pageSize(pageSize)
{
    _instance = this;

    status = SetupSharedMemory();
    if (status < 0)
        status = SetupLocalMemory();
    if (status >= 0)
        status = Initialize();
}

int MemMgr::SetupSharedMemory()
{
    if (SemaphoreV::numAvailable() < 2) {
        fprintf(stderr,
                "Unable to use shared memory. Using local memory instead.\n");
        return -1;
    }

    // We need space for page and also address in _freePage
    int size = _numPages * _pageSize
               + _tableSize * (sizeof(char *) + sizeof(int))
               + sizeof(CountStruct);

    key_t _shmKey = SharedMemory::newKey();
    int created = 0;
    _shm = new SharedMemory(_shmKey, size, _buf, created);
    if (!_shm || !_buf) {
      fprintf(stderr, "Cannot create shared memory\n");
      return -1;
    }
    if (!created)
        printf("Warning: pre-existing shared memory initialized\n");
#if DEBUGLVL >= 1
    //printf("Created a %d-byte shared memory segment at 0x%p\n", size, _buf);
#endif

    int status;
    _sem = new SemaphoreV(Semaphore::newKey(), status, 1);
    if (!_sem || status < 0) {
      fprintf(stderr, "Cannot create semaphore\n");
      delete _shm;
      return -1;
    }
    _sem->setValue(1);

    _free = new SemaphoreV(Semaphore::newKey(), status, 1);
    if (!_free || status < 0) {
      fprintf(stderr, "Cannot create semaphore\n");
      delete _shm;
      _sem->destroy();
      delete _sem;
      return -1;
    }
    _free->setValue(0);

    return 0;
}

int MemMgr::SetupLocalMemory()
{
    // Make sure we don't reference (deleted) shared memory structures
    _shm = 0;
    _sem = 0;
    _free = 0;

    // We need space for page and also address in _freePage
    int size = _numPages * _pageSize
               + _tableSize * (sizeof(char *) + sizeof(int))
               + sizeof(CountStruct);

    _buf = new char [size];
    if (!_buf) {
      fprintf(stderr, "Cannot allocate local memory\n");
      return -1;
    }
#if DEBUGLVL >= 1
    //printf("Created a %d-byte local memory buffer at 0x%p\n", size, _buf);
#endif

    return 0;
}

int MemMgr::Initialize()
{
#if DEBUGLVL >= 1
    //printf("Initializing memory manager\n");
#endif

    memset(_buf, 0, _numPages * _pageSize);

#if DEBUGLVL >= 1
    //printf("Creating free, cache, and buffer lists\n");
#endif

    _freePage = (char **)(_buf + _numPages * _pageSize);
    _freePageCount = (int *)(_freePage + _tableSize);
    _count = (CountStruct *)(_freePageCount + _tableSize);

    // Initially, there is one contiguous free memory area
    _freePage[0] = _buf;
    _freePageCount[0] = _numPages;

    for(int i = 1; i < _tableSize; i++) {
        _freePage[i] = 0;
        _freePageCount[i] = 0;
    }

    _count->entries = 1;
    _count->free = _numPages;
    _count->cache = 0;
    _count->buffer = 0;
    _count->maxCache = _numPages;
    _count->maxBuffer = _numPages;

    return 0;
}

MemMgr::~MemMgr()
{
    delete _shm;

    if (_sem)
        _sem->destroy();
    delete _sem;

    if (_free)
        _free->destroy();
    delete _free;
}

int MemMgr::SetMaxUsage(PageType type, int pages)
{
    AcquireMutex();
    if (type == Cache)
        _count->maxCache = pages;
    else
        _count->maxBuffer = pages;
    ReleaseMutex();

    return 0;
}

int MemMgr::Allocate(PageType type, char *&buf, int &pages, Boolean block)
{
    // Return immediately if no pages requested
    if (pages <= 0)
        return -1;

    AcquireMutex();

    // Return immediately if no free pages available in non-blocking mode,
    // or if no pages of given type are available.
    if (!block) {
        if (!_count->free
            || (type == Cache && _count->cache >= _count->maxCache)
            || (type == Buffer && _count->buffer >= _count->maxBuffer)) {
            ReleaseMutex();
            return -1;
        }
    }

    while (!_count->free) {
#if DEBUGLVL >= 5
        //printf("Out of free pages: %d cache, %d buffer\n",
         //      _count->cache, _count->buffer);
#endif
        ReleaseMutex();
        AcquireFree();
        AcquireMutex();
#if DEBUGLVL >= 5
        //printf("Woke up from sleep, %d free pages, %d cache, %d buffer\n",
         //      _count->free, _count->cache, _count->buffer);
#endif
    }
           
    // Find first contiguous area that is at least the size requested.
    // Make note of largest contiguous area smaller than requested size.

    int pick = -1;
    int i = 0;
    for(; i < _count->entries; i++) {
        if (_freePageCount[i] >= pages) {
#if DEBUGLVL >= 3
            //printf("Found %d-page contiguous block at index %d\n",
             //      _freePageCount[i], i);
#endif
            pick = i;
            break;
        }
        if (pick < 0 || _freePageCount[i] > _freePageCount[pick])
            pick = i;
    }

    DOASSERT(pick >= 0 && pick < _count->entries, "Invalid index");

    if (_freePageCount[pick] < pages) {
#if DEBUGLVL >= 3
        //printf("Reducing %d-page request to largest available %d\n",
         //      pages, _freePageCount[pick]);
#endif
        pages = _freePageCount[pick];
        DOASSERT(pages > 0, "Invalid page count");
    }

    DOASSERT(_count->free >= pages, "Invalid free count");
    _count->free -= pages;
    buf = _freePage[pick];
    DOASSERT(buf, "Invalid page");
    _freePageCount[pick] -= pages;

    // If nothing left of memory chunk, move another chunk into this
    // table position. Otherwise, adjust memory chunk location.

    if (!_freePageCount[pick]) {
        _count->entries--;
        if (pick < _count->entries) {
            _freePage[pick] = _freePage[_count->entries];
            _freePageCount[pick] = _freePageCount[_count->entries];
            _freePage[_count->entries] = 0;
            _freePageCount[_count->entries] = 0;
        } else {
            _freePage[pick] = 0;
        }
    } else {
        _freePage[i] = buf + pages * _pageSize;
    }

    if (type == Buffer)
        _count->buffer += pages;
    else
        _count->cache += pages;

#if DEBUGLVL >= 3
    //printf("Allocated %d %s page(s) at 0x%p\n",
    //       pages, (type == Cache ? "cache" : "buffer"), buf);
#endif

#if DEBUGLVL >= 5
    //printf("Memory allocation table now:\n");
    Dump();
#endif

    ReleaseMutex();

    return 0;
}

int MemMgr::Deallocate(PageType type, char *buf, int pages)
{
    AcquireMutex();

    DOASSERT(buf, "Invalid page");
    DOASSERT(_count->free + pages <= _numPages, "Invalid page");

    // Find free page area that merges perfectly with deallocated range.

    char *endBuf = buf + pages * _pageSize;
    int mergeLeft = -1;
    int mergeRight = -1;
    for(int i = 0; i < _count->entries; i++) {
        DOASSERT(_freePageCount[i] > 0, "Invalid free page count");
        DOASSERT(_freePage[i], "Invalid free memory area");
        char *endFreeBuf = _freePage[i] + _freePageCount[i] * _pageSize;
        if (_freePage[i] == endBuf)
            mergeLeft = i;
        if (buf == endFreeBuf)
            mergeRight = i;
    }

    if (mergeLeft >= 0 && mergeRight >= 0) {
        // Freed area sits perfectly between two previously freed areas
        DOASSERT(mergeLeft != mergeRight, "Impossible merge");
        _freePageCount[mergeRight] += pages + _freePageCount[mergeLeft];
        _count->entries--;
        if (mergeLeft < _count->entries) {
            _freePage[mergeLeft] = _freePage[_count->entries];
            _freePageCount[mergeLeft] = _freePageCount[_count->entries];
            _freePage[_count->entries] = 0;
            _freePageCount[_count->entries] = 0;
        } else {
            _freePageCount[mergeLeft] = 0;
            _freePage[mergeLeft] = 0;
        }
    } else if (mergeLeft >= 0) {
        // Freed area is just to the left of a previously freed area
        _freePage[mergeLeft] -= pages * _pageSize;
        _freePageCount[mergeLeft] += pages;
    } else if (mergeRight >= 0) {
        // Freed area is just to the right of a previously freed area
        _freePageCount[mergeRight] += pages;
    } else {
        // Freed area is not adjacent to any previously freed area
        int freeEntry = _count->entries;
        DOASSERT(freeEntry <= _tableSize, "Inconsistent state");
        DOASSERT(!_freePage[freeEntry] && !_freePageCount[freeEntry],
                 "Entry not free");
        _freePage[freeEntry] = buf;
        _freePageCount[freeEntry] = pages;
        _count->entries++;
    }

    _count->free += pages;

#if DEBUGLVL >= 3
    //printf("Deallocated %d %s page(s) at 0x%p\n",
     //      pages, (type == Cache ? "cache" : "buffer"), buf);
#endif

    if (type == Buffer) {
        _count->buffer -= pages;
#if DEBUGLVL >= 3
        //printf("Now %d buffer pages remain in use\n", _count->buffer);
#endif
    } else {
        _count->cache -= pages;
#if DEBUGLVL >= 3
        //printf("Now %d cache pages remain in use\n", _count->cache);
#endif
    }

#if DEBUGLVL >= 5
    //printf("Memory allocation table now:\n");
    Dump();
#endif

    DOASSERT(_count->cache >= 0 && _count->buffer >= 0, "Inconsistent state");

    // If someone is waiting for a free page, signal it
    if (_count->free == pages)
        ReleaseFree();

    ReleaseMutex();

    return 0;
}

int MemMgr::Convert(char *page, PageType oldType, PageType &newType)
{
    AcquireMutex();

    page = page;

    if (oldType != newType) {
        if (oldType == Cache) {
            _count->cache--;
            _count->buffer++;
        } else {
            _count->cache++;
            _count->buffer--;
        }

#if DEBUGLVL >= 3
        //printf("Memory manager now has %d cache pages, %d buffer pages\n",
      //         _count->cache, _count->buffer);
#endif
    }

    ReleaseMutex();

    return 0;
}

void MemMgr::Dump()
{
    //printf("Free %d, buffer %d (%d max), cache %d (%d max), entries %d\n",
    //       _count->free, _count->buffer, _count->maxBuffer,
    //       _count->cache, _count->maxCache, _count->entries);
/*    for(int i = 0; i < (_tableSize > 10 ? 10 : _tableSize); i++) {
        printf("memory[%d] = 0x%p, %d pages\n", i, _freePage[i],
               _freePageCount[i]);
    }
*/
}

DataPipe::DataPipe(int maxSize, int &status)
{
    status = Initialize(maxSize);
}

int DataPipe::Initialize(int maxSize)
{
#if DEBUGLVL >= 1
    //printf("Creating new data pipe, maximum size %d\n", maxSize);
#endif

    if (maxSize < 2) {
        maxSize = 2;
        fprintf(stderr, "Adjusting maximum size to %d\n", maxSize);
    }

    int status;
    _sem = new SemaphoreV(Semaphore::newKey(), status, 1);
    if (!_sem || status < 0) {
      fprintf(stderr, "Cannot create semaphore\n");
      return -1;
    }
    _sem->setValue(1);

    _free = new SemaphoreV(Semaphore::newKey(), status, 1);
    if (!_free || status < 0) {
      fprintf(stderr, "Cannot create semaphore\n");
      return -1;
    }
    _free->setValue(0);

    _data = new SemaphoreV(Semaphore::newKey(), status, 1);
    if (!_data || status < 0) {
      fprintf(stderr, "Cannot create semaphore\n");
      return -1;
    }
    _data->setValue(0);

    int size = maxSize * (sizeof(char *) + sizeof(streampos_t)
                          + sizeof(iosize_t)) + sizeof(CountStruct);

    key_t _shmKey = SharedMemory::newKey();
    int created = 0;
    char *buf;
    _shm = new SharedMemory(_shmKey, size, buf, created);
    if (!_shm) {
      fprintf(stderr, "Cannot create shared memory\n");
      return -1;
    }
    if (!created)
        printf("Warning: pre-existing shared memory initialized\n");
#if DEBUGLVL >= 1
    printf("Created a %d-byte shared memory segment (key %d) at 0x%p\n",
           size, _shmKey, buf);
#endif

    if (!buf) {
      fprintf(stderr, "Failed to get shared memory\n");
      return -1;
    }

    _chunk = (char **)buf;
    _offset = (streampos_t *)(_chunk + maxSize);
    _bytes = (iosize_t *)(_offset + maxSize);
    _count = (CountStruct *)(_bytes + maxSize);

    for(int i = 0; i < maxSize; i++) {
        _chunk[i] = 0;
        _offset[i] = 0;
        _bytes[i] = 0;
    }

    _count->head = 0;
    _count->tail = 0;
    _count->free = maxSize;
    _count->size = maxSize;
    _count->maxSize = maxSize;

    return 0;
}

DataPipe::~DataPipe()
{
    delete _shm;

    _data->destroy();
    delete _data;

    _free->destroy();
    delete _free;

    _sem->destroy();
    delete _sem;
}

int DataPipe::Produce(char *buf, streampos_t offset, iosize_t bytes)
{
    AcquireMutex();

    // Free count may be negative if pipe size was recently reduced.

    while (_count->free <= 0) {
#if DEBUGLVL >= 3
//        printf("Pipe 0x%p has to wait for consumer's free space (%d)\n",
               this, _count->free);
#endif
        ReleaseMutex();
        AcquireFree();
        AcquireMutex();
    }

    DOASSERT(!_chunk[_count->head], "Inconsistent state");

    _chunk[_count->head] = buf;
    _offset[_count->head] = offset;
    _bytes[_count->head] = bytes;

    _count->head = (_count->head + 1) % _count->maxSize;
    _count->free--;

#if DEBUGLVL >= 3
    //printf("Pipe 0x%p has %d free space left\n", this, _count->free);
#endif

    if (_count->free == _count->size - 1) {
#if DEBUGLVL >= 3
        //printf("Pipe 0x%p signaling consumer about new data (%d,%d)\n",
               this, _count->free, _count->size);
#endif
        ReleaseData();
    }

    ReleaseMutex();

    return 0;
}

int DataPipe::Consume(char *&buf, streampos_t &offset, iosize_t &bytes)
{
    AcquireMutex();

    while (_count->free == _count->size) {
#if DEBUGLVL >= 3
        //printf("Pipe 0x%p has to wait for producer's data (%d,%d)\n",
               this, _count->free, _count->size);
#endif
        ReleaseMutex();
        AcquireData();
        AcquireMutex();
    }

    buf = _chunk[_count->tail];
    offset = _offset[_count->tail];
    bytes = _bytes[_count->tail];

    _chunk[_count->tail] = 0;
    _offset[_count->tail] = 0;
    _bytes[_count->tail] = 0;
    _count->tail = (_count->tail + 1) % _count->maxSize;
    _count->free++;

#if DEBUGLVL >= 3
    //printf("Pipe 0x%p has %d free space left\n", this, _count->free);
#endif

    if (_count->free == 1) {
#if DEBUGLVL >= 3
        //printf("Pipe 0x%p signaling producer about free space\n", this);
#endif
        ReleaseFree();
    }

    ReleaseMutex();

    return 0;
}

int DataPipe::SetSize(int size)
{
    AcquireMutex();

    if (size < 2) {
        size = 2;
        fprintf(stderr, "Adjusting pipe size to %d\n", size);
    }

    if (size >= _count->maxSize) {
        fprintf(stderr, "Invalid pipe size %d reduced to %d\n",
                size, _count->maxSize);
        size = _count->maxSize;
    }

    // The following adjustment to free count may leave if negative.
    // That's fine, and just means that Produce() will not be able
    // to append data to the pipe until that many chunks have been
    // consumed from the pipe.

    _count->free += size - _count->size;
    _count->size = size;

    ReleaseMutex();

    return size;
}

MultiPipe::MultiPipe(int &status) : _numPipes(0), _hint(0)
{
    _data = new SemaphoreV(Semaphore::newKey(), status, 1);
    if (!_data || status < 0) {
      fprintf(stderr, "Cannot create semaphore\n");
      status = -1;
      return;
    }
    _data->setValue(0);
}

MultiPipe::~MultiPipe()
{
    _data->destroy();
    delete _data;
}

int MultiPipe::AddPipe(DataPipe *pipe)
{
    // See if pipe already in multipipe
    for(int i = 0; i < _numPipes; i++) {
        if (_pipes[i] == pipe)
            return -1;
    }

    if (_numPipes >= _maxPipes)
      return -1;

    _pipes[_numPipes++] = pipe;

#if DEBUGLVL >= 3
    //printf("Added pipe 0x%p to multipipe 0x%p\n", pipe, this);
#endif

    return 0;
}

int MultiPipe::RemovePipe(DataPipe *pipe)
{
    for(int i = 0; i < _numPipes; i++) {
        if (_pipes[i] == pipe) {
            for(int j = i; j < _numPipes - 1; j++)
                _pipes[j] = _pipes[j + 1];
            _numPipes--;
            if (_hint >= _numPipes)
                _hint = 0;
#if DEBUGLVL >= 3
            //printf("Removed pipe 0x%p from multipipe 0x%p\n", pipe, this);
#endif
            return 0;
        }
    }

    return -1;
}

int MultiPipe::Consume(char *&buf, streampos_t &offset,
                       iosize_t &bytes, DataPipe *&pipe)
{
    for(int j = 0; j < _numPipes; j++) {
        pipe = _pipes[_hint];
        if (pipe->NumData() > 0) {
#if DEBUGLVL >= 3
            //printf("Consuming data from pipe 0x%p of multipipe 0x%p\n",
                   pipe, this);
#endif
            return pipe->Consume(buf, offset, bytes);
        }
        _hint++;
    }

    // Block on first pipe but change this later once a smarter
    // synchronization mechanism is designed.

    pipe = _pipes[_hint];
#if DEBUGLVL >= 3
    //printf("Blocking and consuming data from pipe 0x%p of multipipe 0x%p\n",
           pipe, this);
#endif
    return pipe->Consume(buf, offset, bytes);
}
