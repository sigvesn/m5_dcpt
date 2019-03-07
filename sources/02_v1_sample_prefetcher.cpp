#include "interface.h"  // Do NOT edit interface .h
#include "sample_prefetcher.h"
#include <list>
#include <iostream>
#include <fstream>


// Prefetch table accounting:
//
// Bits per entry:
// 32 bit PC entry
// 27 bit last_adress (last 5 bits are always 0 - cachelines)
// 27 bit last_prefetch (last 5 bits are always 0 - cachelines)
// 19 12-bit deltas
// 5 bit pointer into circular buffer
//
// Total: 319 bits per entry
// 98 table entries = 31262 bits
// 
// Prefetchqueue: 32 entries * 27 bits = 864 bits
// Prefetch candidates : 17 * 27 bits = 459 bits  ( There can be a maximum 17 candidates)
//
// Total: 32585 bits
//



DCPrefetchTable *pref2;   // This is the L2 prefetcher

list<ADDRINT> mshr;           // 16-entry MSHR for L2
list<ADDRINT> prefetchQueue;  // 32 entry table that holds issued prefetches that have not yet
                              // been completed.

//  Function to initialize the prefetchers.  DO NOT change the prototype of this
//  function.  You can change the body of the function by calling your necessary
//  initialization functions.
//

void InitPrefetchers() // DO NOT CHANGE THE PROTOTYPE
{
    // Only L2 prefetching.
    pref2 = new DCPrefetchTable(98);
}

// This function checks if an prefetch has been issued for this adress, but
// not yet completed.

bool checkIfPresentInPrefetchQueue(ADDRINT Addr) {
  list<ADDRINT>::iterator it;
  for( it = prefetchQueue.begin(); it != prefetchQueue.end(); it++ ) {
    if (*it == Addr) {
      return true;
    }
  }
  return false;
}

// This function checks if a read request for this adress is present in the 
// MSHR.

bool checkIfPresentInMSHR(ADDRINT Addr) {
  list<ADDRINT>::iterator it;
  for( it = mshr.begin(); it != mshr.end(); it++ ) {
    if (*it == Addr) {
      return true;
    }
  }
  return false;
}

// Insert a request into the MSHR

bool insertIntoMSHR(ADDRINT missAddr) {
  // Mask out lowest bits
  missAddr &= ~63;
  // Error?
  if (missAddr == 0) {
    return false;
  }
  if (checkIfPresentInMSHR(missAddr)) {
    return false;
  }
  mshr.push_back(missAddr);
  // Enforce the size of the MSHR.
  if (mshr.size() > 16) {
    // This shouldn't happen unless there is no bandwidth constraints
    mshr.pop_front();
  }
  return true;
}

// Update the MSHR - Remove completed requests
// This function checks to see if some adresses in the MSHR are in the cache 
// and removes them if they are

void updateMSHR() {
  if (!mshr.empty()) {
    list<ADDRINT>::iterator it;
    for( it = mshr.begin(); it != mshr.end(); it++ ) {
      if ((GetPrefetchBit(1, *it) != -1) || (GetPrefetchBit(0, *it) != -1)) {
        // It is in the cache, remove it
        mshr.erase(it);
        // Check if there is more to remove
        updateMSHR();
        // To avoid having a non-functional list iterator
        break;
      }
    }
  }
}

// Update the Prefetchqueue
// Remove entries that have been completed.

void updatePrefetchQueue() {
  if (!prefetchQueue.empty()) {
    list<ADDRINT>::iterator it;
    for( it = prefetchQueue.begin(); it != prefetchQueue.end(); it++ ) {
      if ((GetPrefetchBit(1, *it) != -1) || (GetPrefetchBit(0, *it) != -1)) {
        // It is in the cache, remove it
        prefetchQueue.erase(it);
        // Check if there is more to remove
        updatePrefetchQueue();
        // To avoid having a non-functional list iterator
        break;
      }
    }
  }
}


// Issue the prefetches, but first check if it is
// useful.

bool insertPrefetch(COUNTER cycle, ADDRINT Addr) {
  //Check if it is present in L2
  if (GetPrefetchBit(1, Addr) != -1) {
    return false;
  }
  // Check if it is present in L1
  // TODO - is this really necessary?
  if (GetPrefetchBit(0, Addr) != -1) {
    return false;
  }
  // Check MSHR
  if (checkIfPresentInMSHR(Addr)) {
    return false;
  }
  if (checkIfPresentInPrefetchQueue(Addr)) {
    return false;
  }
  // Bandwidth check
  if ((prefetchQueue.size()) < 32) {
    // Issue prefetches
    IssueL2Prefetch(cycle,Addr);
    prefetchQueue.push_back(Addr);
  } else {
    return false;
  }
  return true;
}

//
//  Function that is called every cycle to issue prefetches should the
//  prefetcher want to.  The arguments to the function are the current cycle,
//  the demand requests to L1 and L2 cache.  Again, DO NOT change the prototype of this
//  function.  You can change the body of the function by calling your necessary
//  routines to invoke your prefetcher.
//

// DO NOT CHANGE THE PROTOTYPE
void IssuePrefetches( COUNTER cycle, PrefetchData_t *L1Data, PrefetchData_t * L2Data )
{    

    MemLogEntry *entry;
    list<ADDRINT> prefetchlist;  // Temporary storage for prefetch candidates
                                 // Maximum size is equal to the number of deltas - 2.

    updateMSHR();
    updatePrefetchQueue();
    
    // Insert load misses into the MSHR
    if((cycle == L2Data->LastRequestCycle)) {
      // Update the contents of the MSHR
      if (!L2Data->hit && (L2Data->AccessType == 1)) {
        // Insert into MSHR
        insertIntoMSHR( L2Data->DataAddr);
      }
    }

    

    // Issue L2 prefetches
    if((cycle == L2Data->LastRequestCycle)) {
      // Update the contents of the MSHR
      if (!L2Data->hit && (L2Data->AccessType == 1)) {
        // Insert into MSHR
        insertIntoMSHR( L2Data->DataAddr);
      }
      //insertIntoMissQueue(L2Data->DataAddr);
        entry = pref2->AccessEntry( 0, L2Data->LastRequestAddr, L2Data->DataAddr & ~63,L2Data->hit);
        if(entry) {
          if (entry->deltas.size() > 3) {
            // Find the last two deltas
            list<INT32>::iterator theIterator;
            theIterator = entry->deltas.begin();
            INT32 delta_1 = *theIterator;
            theIterator++;
            INT32 delta_2 = *theIterator;
            // Search by using a state machine, both deltas must be found
            bool delta_2_found = false;
            bool delta_1_found = false;
            ADDRINT prefetch_addr = entry->last_mem_addr;
            list<INT32>::reverse_iterator dc_iterator;
            // Clear old prefetch candidates
            prefetchlist.clear();
            // Traverse the delta buffer in reverse order.
            for(dc_iterator = entry->deltas.rbegin(); dc_iterator != entry->deltas.rend(); dc_iterator++ ) {
              
              // If dc_iterator is 0, there was an overflow, and thus everything up to 
              // this point is invalid

              if (*dc_iterator == 0) {
                delta_2_found = false;
                delta_1_found = false;
                prefetchlist.clear();
              }
              // If the pattern is found, start calculating prefetches.  
              if (delta_2_found && delta_1_found) {
                prefetch_addr += *dc_iterator * 64;
                prefetchlist.push_back(prefetch_addr);
                // If the adress calculated matches one allready issued, clear the prefetch list
                if (prefetch_addr == entry->last_prefetch_addr) {
                  prefetchlist.clear();
                }
                // Move on to the next delta
                continue;
              }
              // If delta2 is found, then the next one must be delta 1, or there is no match.
              if (delta_2_found && !delta_1_found) {
                if (*dc_iterator == delta_1) {
                  delta_1_found = true;
                } else {
                  delta_2_found = false;
                }
              }
              if (*dc_iterator == delta_2) {
                delta_2_found = true;
              }
            }
          // Issue prefetches
          if (delta_2_found && delta_1_found) {
            list<ADDRINT>::iterator prefetch_iterator;
            bool prefetchWasIssued;
            // Iterate over the prefetch candidates
            for(prefetch_iterator = prefetchlist.begin(); prefetch_iterator != prefetchlist.end(); prefetch_iterator++) {
              prefetchWasIssued = insertPrefetch(cycle, *prefetch_iterator);
              // Update last_prefetch_addr
              if (prefetchWasIssued) {
                entry->last_prefetch_addr = *prefetch_iterator;
              }
            }
          }
        }
      }
    }
}
