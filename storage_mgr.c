#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "storage_mgr.h"
#include "dberror.h"

// Page size constant (should be in dberror.h, but we define it here if needed)
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

/**
 * Initialize the storage manager
 * This is called before any other storage manager functions
 */
void initStorageManager(void) {
    // Nothing special to initialize for now
    printf("Storage Manager initialized.\n");
}

/**
 * Create a new page file with one empty page filled with '\0' bytes
 */
RC createPageFile(char *fileName) {
    // Open file for writing (create if doesn't exist, truncate if exists)
    FILE *file = fopen(fileName, "wb");
    
    if (file == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Create one page filled with zeros
    char *emptyPage = (char *)calloc(PAGE_SIZE, sizeof(char));
    
    if (emptyPage == NULL) {
        fclose(file);
        return RC_WRITE_FAILED;
    }
    
    // Write the empty page to file
    size_t written = fwrite(emptyPage, sizeof(char), PAGE_SIZE, file);
    
    // Free the allocated memory
    free(emptyPage);
    
    // Close the file
    fclose(file);
    
    // Check if write was successful
    if (written != PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }
    
    return RC_OK;
}

/**
 * Open an existing page file
 */
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    // Try to open the file for reading and writing
    FILE *file = fopen(fileName, "rb+");
    
    if (file == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Get the file size to calculate total number of pages
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);  // Go back to beginning
    
    // Calculate total number of pages
    int totalPages = fileSize / PAGE_SIZE;
    
    // Initialize the file handle
    fHandle->fileName = fileName;
    fHandle->totalNumPages = totalPages;
    fHandle->curPagePos = 0;  // Start at first page
    fHandle->mgmtInfo = file;  // Store FILE pointer for later use
    
    return RC_OK;
}

/**
 * Close an open page file
 */
RC closePageFile(SM_FileHandle *fHandle) {
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    // Get the FILE pointer
    FILE *file = (FILE *)fHandle->mgmtInfo;
    
    // Close the file
    int result = fclose(file);
    
    if (result != 0) {
        return RC_FILE_NOT_FOUND;
    }
    
    // Clear the file handle
    fHandle->mgmtInfo = NULL;
    
    return RC_OK;
}

/**
 * Delete a page file from disk
 */
RC destroyPageFile(char *fileName) {
    // Use unlink (POSIX) or remove (standard C) to delete file
    int result = remove(fileName);
    
    if (result != 0) {
        return RC_FILE_NOT_FOUND;
    }
    
    return RC_OK;
}

/**
 * Read a specific block (page) from the file
 */
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if file handle is valid
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    // Check if page number is valid
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    // Get the FILE pointer
    FILE *file = (FILE *)fHandle->mgmtInfo;
    
    // Calculate the byte offset for this page
    long offset = pageNum * PAGE_SIZE;
    
    // Seek to the page position
    int seekResult = fseek(file, offset, SEEK_SET);
    if (seekResult != 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    // Read the page into memory
    size_t bytesRead = fread(memPage, sizeof(char), PAGE_SIZE, file);
    
    if (bytesRead != PAGE_SIZE) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    // Update current page position
    fHandle->curPagePos = pageNum;
    
    return RC_OK;
}

/**
 * Get the current block position
 */
int getBlockPos(SM_FileHandle *fHandle) {
    if (fHandle == NULL) {
        return -1;
    }
    return fHandle->curPagePos;
}

/**
 * Read the first block in the file
 */
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

/**
 * Read the previous block relative to current position
 */
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    int prevPage = fHandle->curPagePos - 1;
    
    if (prevPage < 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    return readBlock(prevPage, fHandle, memPage);
}

/**
 * Read the current block
 */
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

/**
 * Read the next block relative to current position
 */
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    int nextPage = fHandle->curPagePos + 1;
    
    if (nextPage >= fHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    return readBlock(nextPage, fHandle, memPage);
}

/**
 * Read the last block in the file
 */
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    int lastPage = fHandle->totalNumPages - 1;
    
    if (lastPage < 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    return readBlock(lastPage, fHandle, memPage);
}

/**
 * Write a block to a specific page position
 */
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if file handle is valid
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    // Check if page number is valid
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_WRITE_FAILED;
    }
    
    // Get the FILE pointer
    FILE *file = (FILE *)fHandle->mgmtInfo;
    
    // Calculate the byte offset for this page
    long offset = pageNum * PAGE_SIZE;
    
    // Seek to the page position
    int seekResult = fseek(file, offset, SEEK_SET);
    if (seekResult != 0) {
        return RC_WRITE_FAILED;
    }
    
    // Write the page from memory to disk
    size_t bytesWritten = fwrite(memPage, sizeof(char), PAGE_SIZE, file);
    
    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }
    
    // Flush to ensure data is written to disk
    fflush(file);
    
    // Update current page position
    fHandle->curPagePos = pageNum;
    
    return RC_OK;
}

/**
 * Write a block at the current page position
 */
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

/**
 * Append an empty block to the end of the file
 */
RC appendEmptyBlock(SM_FileHandle *fHandle) {
    // Check if file handle is valid
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    // Get the FILE pointer
    FILE *file = (FILE *)fHandle->mgmtInfo;
    
    // Create an empty page (filled with zeros)
    char *emptyPage = (char *)calloc(PAGE_SIZE, sizeof(char));
    
    if (emptyPage == NULL) {
        return RC_WRITE_FAILED;
    }
    
    // Seek to the end of the file
    fseek(file, 0, SEEK_END);
    
    // Write the empty page
    size_t bytesWritten = fwrite(emptyPage, sizeof(char), PAGE_SIZE, file);
    
    // Free the allocated memory
    free(emptyPage);
    
    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }
    
    // Flush to ensure data is written to disk
    fflush(file);
    
    // Update total number of pages
    fHandle->totalNumPages++;
    
    return RC_OK;
}

/**
 * Ensure the file has at least numberOfPages pages
 */
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    // Check if file handle is valid
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    
    // If file already has enough pages, do nothing
    if (fHandle->totalNumPages >= numberOfPages) {
        return RC_OK;
    }
    
    // Calculate how many pages we need to add
    int pagesToAdd = numberOfPages - fHandle->totalNumPages;
    
    // Append empty pages until we reach the desired capacity
    for (int i = 0; i < pagesToAdd; i++) {
        RC result = appendEmptyBlock(fHandle);
        if (result != RC_OK) {
            return result;
        }
    }
    
    return RC_OK;
}