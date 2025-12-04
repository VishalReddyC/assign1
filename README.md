# Storage Manager - Assignment 1
## CS 525 - Advanced Database Organization

---

## 📋 WHAT IS THIS?

This is a **Storage Manager** - a simple module that reads and writes blocks (pages) of data to/from disk files.

Think of it like a librarian:
- The librarian (storage manager) manages books (pages) on shelves (disk files)
- You can ask to read a specific book (read a page)
- You can ask to write/update a book (write a page)
- The librarian keeps track of which book you're currently reading (current page position)

---

## 🏗️ CODE STRUCTURE

### Files Created:

1. **storage_mgr.c** - Main implementation
   - All the functions that create, read, write files
   
2. **storage_mgr.h** - Header file
   - Function declarations (interface)
   - Data structure definitions
   
3. **dberror.c** - Error handling
   - Functions to print error messages
   
4. **dberror.h** - Error codes
   - Defines all error codes (RC_OK, RC_FILE_NOT_FOUND, etc.)
   - Defines PAGE_SIZE = 4096 bytes
   
5. **Makefile** - Build configuration
   - Compiles everything automatically
   
6. **README.txt** - This file
   - Explains the solution

---

## 💡 HOW IT WORKS

### Key Concepts:

**1. Pages (Blocks)**
- A page is a fixed-size chunk of data (4096 bytes)
- Files are divided into pages: [Page 0][Page 1][Page 2]...
- Like chapters in a book

**2. File Handle (SM_FileHandle)**
- Stores information about an open file:
  - `fileName` - name of the file
  - `totalNumPages` - how many pages in the file
  - `curPagePos` - which page we're currently at (like a bookmark)
  - `mgmtInfo` - stores the FILE pointer (the actual open file)

**3. Page Handle (SM_PageHandle)**
- Just a pointer to memory (char *)
- Points to a 4096-byte block in memory
- This is where we read/write page data

---

## 🔧 FUNCTIONS EXPLAINED

### File Management Functions:

**createPageFile(fileName)**
- Creates a new file with one empty page (4096 bytes of zeros)
- Like creating a new notebook with one blank page

**openPageFile(fileName, fHandle)**
- Opens an existing file
- Reads the file size to calculate total pages
- Sets current position to page 0 (first page)
- Stores FILE pointer in fHandle->mgmtInfo

**closePageFile(fHandle)**
- Closes the file
- Frees resources

**destroyPageFile(fileName)**
- Deletes the file from disk
- Like throwing away a notebook

---

### Reading Functions:

**readBlock(pageNum, fHandle, memPage)**
- Reads a specific page number
- Steps:
  1. Check if page number is valid
  2. Calculate file offset: pageNum × 4096
  3. Seek to that position
  4. Read 4096 bytes into memPage
  5. Update current position

**readFirstBlock()** - Reads page 0
**readLastBlock()** - Reads the last page
**readCurrentBlock()** - Reads the current page
**readNextBlock()** - Reads current + 1
**readPreviousBlock()** - Reads current - 1

**getBlockPos()** - Returns current page number

---

### Writing Functions:

**writeBlock(pageNum, fHandle, memPage)**
- Writes data from memory to a specific page
- Steps:
  1. Check if page number is valid
  2. Calculate file offset: pageNum × 4096
  3. Seek to that position
  4. Write 4096 bytes from memPage
  5. Flush to disk (ensure it's saved)

**writeCurrentBlock()** - Writes to current page

**appendEmptyBlock(fHandle)**
- Adds one new empty page at the end
- Increases totalNumPages by 1

**ensureCapacity(numberOfPages, fHandle)**
- Makes sure file has at least numberOfPages
- If not, appends empty pages until it does

---

## 🎯 IMPLEMENTATION DETAILS

1. **Using FILE pointer**
   - Stored in `fHandle->mgmtInfo`
   - Simpler than using POSIX file descriptors
   - Standard C library functions (fopen, fread, fwrite, etc.)

2. **Page Size**
   - Fixed at 4096 bytes (common page size)
   - Defined in dberror.h as PAGE_SIZE

3. **Error Handling**
   - Each function returns RC (return code)
   - RC_OK (0) = success
   - Other codes = specific errors

4. **Memory Management**
   - Functions allocate temporary memory for operations
   - Always free allocated memory before returning
   - Caller must allocate memory for memPage (4096 bytes)

---

## 📊 HOW PAGES ARE STORED

File on disk:
```
[Page 0: 4096 bytes][Page 1: 4096 bytes][Page 2: 4096 bytes]...
   Byte 0-4095         Byte 4096-8191       Byte 8192-12287
```

To read Page 1:
- Calculate offset: 1 × 4096 = 4096
- Seek to byte 4096
- Read next 4096 bytes

---

## 🔄 TYPICAL USAGE FLOW

```c
// 1. Initialize
initStorageManager();

// 2. Create a new file
createPageFile("mydata.bin");

// 3. Open the file
SM_FileHandle fh;
openPageFile("mydata.bin", &fh);

// 4. Allocate memory for a page
SM_PageHandle page = (SM_PageHandle)malloc(PAGE_SIZE);

// 5. Write some data
strcpy(page, "Hello, this is my data!");
writeBlock(0, &fh, page);

// 6. Read it back
memset(page, 0, PAGE_SIZE);  // Clear
readBlock(0, &fh, page);
printf("Data: %s\n", page);  // Prints: Hello, this is my data!

// 7. Clean up
free(page);
closePageFile(&fh);
```

---

## ⚠️ IMPORTANT NOTES

1. **Memory Allocation**
   - Before calling read functions, allocate 4096 bytes:
     ```c
     SM_PageHandle page = malloc(PAGE_SIZE);
     ```
   - Don't forget to free it later!

2. **Page Numbers**
   - Start at 0 (first page is page 0)
   - If file has 3 pages, valid numbers are: 0, 1, 2

3. **Current Position**
   - After reading/writing, current position updates
   - Used by readCurrentBlock, readNextBlock, etc.

4. **Error Checking**
   - Always check return codes:
     ```c
     RC rc = readBlock(5, &fh, page);
     if (rc != RC_OK) {
         // Handle error
     }
     ```

---

## 🧪 TESTING

The provided test file (test_assign1_1.c) tests:
- ✅ Creating files
- ✅ Opening and closing files
- ✅ Reading and writing pages
- ✅ Navigating (first, last, next, previous)
- ✅ Appending pages
- ✅ Ensuring capacity

To compile and run:
```bash
make clean
make
./test_assign1
```

Expected :
```
test single page content ... OK
test multiple page content ... OK
... [test_assign1_1.c-test single page content-L99-02:57:29] OK: finished test

output:
test_assign1_1.c-test single page content-L93-02:57:29] OK: expected true: character in page read from disk is the one we expected.
[test_assign1_1.c-test single page content-L93-02:57:29] OK: expected true: character in page read from disk is the one we expected.
reading first block
[test_assign1_1.c-test single page content-L99-02:57:29] OK: finished test


---

**END OF README**
