#ifndef SAMPLE_PREFETCHER_H
#define SAMPLE_PREFETCHER_H

#include <stdio.h>

#include <map>
#include <list>
#include <fstream>
#include <iomanip>
#include <iostream>
#
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Basic Classes and Templates
///////////////////////////////////////////////////////////////////////////////

/// Nodes in the LRU-like queues
template<class ItemType>
class _List_Node
{
  public:
    ItemType _item;
    _List_Node* _next;
    _List_Node* _prev;
    
    _List_Node(ItemType item = ItemType())
        :_item(item), 
         _next(0), 
         _prev(0)
    {}

    _List_Node(_List_Node& node)
        :_item(node._item), 
         _next(0), 
         _prev(0)
    {}

    ~_List_Node(){}
};

/// template class for any LRU-like queues.
/// The queue is implemented in a form of double-linked list.
/// The template provides constructor/destructor, and a "bring to head" method.
/// Inherite from LRUList<ItemType> to create your new queue class,
/// where ItemType is the class of queue items. 
/// Requirement of ItemType:
/// 	o. It should provide default/copy constructors.
///		o. It should provide a Dump() routine for dumpping out its content to stdout. This is useful in debugging.
template<class ItemType>
class LRUList{
  public:
    typedef _List_Node<ItemType> Node;
  protected:
    Node* mListHead;
    Node* mListTail;
    UINT32 mListLength;

    /// Brings the node to the head of the queue.
    /// This makes it Most Recently Used
    void BringsToHead(Node* ptr){

        if( ptr->_prev != NULL ) 
        {
            // Pull the entry out chain previous and next elements
            // to each other
            ptr->_prev->_next = ptr->_next;
            if( ptr->_next ) 
            {
                ptr->_next->_prev = ptr->_prev;
            }
            else 
            {
                // If tail is being removed, set the tail to the
                // previous guy in the link-list
                mListTail = ptr->_prev;
            }

            // Since we are bringing this to the head, next
            // element is going to be the current head.  
            ptr->_next        = mListHead;
            ptr->_prev        = NULL;
                
            // set current head's previous to this
            mListHead->_prev   = ptr;
                
            // This is now the current head
            mListHead          = ptr;
        }
    }

    /// Brings the node to the tail of the queue.
    /// This makes it Least Recently Used
    void BringsToTail(Node* ptr){
        if( ptr->_next != NULL ) 
        {
            // Pull the entry out chain previous and next elements
            // to each other
            ptr->_next->_prev = ptr->_prev;
            if( ptr->_prev ) 
            {
                ptr->_prev->_next = ptr->_next;
            }
            else 
            {
                // If head is being removed, set the head to the
                // next guy in the link-list
                mListHead        = ptr->_next;
            }

            // Since we are bringing this to the tail, next
            // element is going to be the current tail.  
            ptr->_prev        = mListTail;
            ptr->_next        = NULL;

            mListTail->_next   = ptr;

            // This is now the current tail
            mListTail          = ptr;
        }
    }

  public:
    /// Create the list with specified length.
    LRUList(UINT32 listLength): 
        mListHead(0), 
        mListTail(0), 
        mListLength(listLength) 
    {
        Node* prevNode = 0;
        for (UINT32 i=0; i<listLength; i++){
            Node* node = new Node();
            if (mListHead == 0) mListHead = node;
            node->_prev = prevNode;
            if (prevNode) prevNode->_next = node;
            prevNode = node;
        }
        mListTail = prevNode;
    }

    ~LRUList()
    {
        while (mListHead)
        {
            Node* node = mListHead->_next;
            delete mListHead;
            mListHead = node;
        }
    }

    /// Utillity routine for debugging in gdb. Dump out the list.
    void Dump()
    {
        Node* node=mListHead;
        for (UINT32 i=0; i<mListLength; i++, node=node->_next)
        {
            cout<<i<<": ";
            if (node) node->_item.Dump();
            cout<<endl;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
// DCPT entry
///////////////////////////////////////////////////////////////////////////////

class MemLogEntry{
  public:
    ADDRINT ip; // IP of the mem instruction
    ADDRINT last_mem_addr; // the last mem addr
    list<INT32> deltas;   // Delta list
    ADDRINT last_prefetch_addr; // Last adress prefetched
    
    /// default constructor
    MemLogEntry(ADDRINT ip = 0)
        :last_mem_addr(0)
    { 
        this->ip = ip; 
        this->last_prefetch_addr = 0;
        this->deltas.clear();
    }

    ~MemLogEntry()
    {
        // Nothing
    }

};

class DCPrefetchTable
    : public LRUList<MemLogEntry> 
{
  private:
    map<ADDRINT, Node*> PClist;
    map<ADDRINT, Node*>::iterator iter;

    // These two values hold the maximum representable values
    // with a given amount of bits.
    int max_delta_val;
    int min_delta_val;

  public:
    static const UINT32 DEFAULT_STRIDE_PREFETCH_TABLE_SIZE = 98;
    static const UINT32 DELTAS_SIZE = 19;
    static const UINT32 BITS_PER_DELTA = 12;


    DCPrefetchTable(UINT32 tableSize=DEFAULT_STRIDE_PREFETCH_TABLE_SIZE)
        : LRUList<MemLogEntry>(tableSize){
        // Calculate the range representable with a certain amount of bits.
        max_delta_val = (1 << (BITS_PER_DELTA -1)) - 1;
        min_delta_val = -max_delta_val;
    }

    ~DCPrefetchTable(){
    }


    /// Look-up in the prefetch table. This leeds to a subsequent LRU update in the table.
    /// Returns the entry if hit.
    MemLogEntry* AccessEntry(UINT32 threadId, ADDRINT ip, ADDRINT memAddr, bool hit){

        // find the ip log in the table
        Node* node = mListHead;
        while (node && node->_item.ip!=ip) node=node->_next;

        MemLogEntry* entry;
        if ( node == NULL ) { 
            // Only make a new entry on a miss
            if (hit) {
              return 0;
            }
            /// Replace tail
            node = mListTail;

            entry = &(node->_item);

            DelFrmPClist( entry->ip );
            // Clear all old data, and replace with new.
            entry->ip = ip;
            entry->last_mem_addr = memAddr;
            entry->last_prefetch_addr = 0;
            entry->deltas.clear();
            AddToPClist( ip, node );
        }
        else         /// update
        {
            entry = &(node->_item);

            INT32 newstride      = memAddr - entry->last_mem_addr;
            entry->last_mem_addr = memAddr;

            // Strides is stores in units of cache lines to conserve
            // space

            newstride /=64;

            // Only store non-zero strides
            
            if (newstride) {

              // Check if there is overflow due to limited amount of bits per
              // delta
              if ((newstride > max_delta_val) || (newstride < min_delta_val)) {
                newstride = 0;
              }

              // Insert delta 
              entry->deltas.push_front(newstride);

              // If the delta table is full, remove old ones
              if (entry->deltas.size() > DELTAS_SIZE) {
                entry->deltas.pop_back();
              } 
            }
        }

        BringsToHead(node);

        return entry;
    }


    Node* FindPC( ADDRINT ip )
    {
        iter = PClist.find(ip);
        if( iter != PClist.end() ) 
        {
            return iter->second;
        }

        return NULL;
    }

    inline void AddToPClist( ADDRINT ip, Node *tmp ) 
    {
        PClist[ ip ] = tmp;
    }

    inline void DelFrmPClist( ADDRINT ip ) 
    {
            PClist.erase( ip );
    }

};

#endif
