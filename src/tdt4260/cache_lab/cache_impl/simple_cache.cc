#include "tdt4260/cache_lab/cache_impl/simple_cache.hh"

#include <bits/stdc++.h>

#include "base/trace.hh"
#include "debug/AllAddr.hh"
#include "debug/AllCacheLines.hh"
#include "debug/TDTSimpleCache.hh"

namespace gem5
{

SimpleCache::SimpleCache(int size, int blockSize, int associativity,
                         statistics::Group *parent, const char *name)
    : size(size), blockSize(blockSize), associativity(associativity), cacheName(name),
      stats(parent, name)
{
    numEntries = this->size / this->blockSize;
    numSets = this->numEntries / this->associativity;

    // allocate entries for all sets and ways
    for (int i = 0; i < this->numSets; i++) {
        std::vector<Entry *> vec;

        // DONE: Associative: Allocate as many entries as there are ways
        // i.e. replace vector with vector of vector and build ways
        for (int i = 0; i < associativity; i++){
            vec.push_back(new Entry());
        }

        entries.push_back(vec);
    }
}

SimpleCache::SimpleCacheStats::SimpleCacheStats(
    statistics::Group *parent, const char *name)
    : statistics::Group(parent, name),
    ADD_STAT(reqsReceived, statistics::units::Count::get(),
        "Number of requests received from cpu side"),
    ADD_STAT(reqsServiced, statistics::units::Count::get(),
        "Number of requests serviced at this cache level"),
    ADD_STAT(respsReceived, statistics::units::Count::get(),
        "Number of responses received from mem side") {}

void
SimpleCache::recvReq(Addr req, int size)
{
    ++stats.reqsReceived;

    int index = calculateIndex(req);
    int tag = calculateTag(req);

    DPRINTF(TDTSimpleCache, "Debug: Addr: %#x, index: %d, tag: %d, in %s\n",
            req, index, tag, cacheName);
    DPRINTF(AllAddr, "%#x\n", req);
    DPRINTF(AllCacheLines, "%#x\n", req >> ((int) std::log2(blockSize)));

    // if cache line already in cache
    if (hasLine(index, tag)) {
        ++stats.reqsServiced;
        int way = lineWay(index, tag);
        DPRINTF(TDTSimpleCache, "Hit: way: %d\n", way);

        // TODO: Associative: Update LRU info for line in entries
        for(int i = 0; i < this->associativity; i++){
            if(i == way){
                this->entries.at(index).at(way)->lastUsed = 0;
            }
            else{
                this->entries.at(index).at(way)->lastUsed++;
            }
        }

        sendResp(req);
    } else{
        sendReq(req, size);
    }
}

void
SimpleCache::recvResp(Addr resp)
{
    ++stats.respsReceived;

    int index = calculateIndex(resp);
    int tag = calculateTag(resp);

    // there should never be a request (and thus a response) for a line already in the cache
    assert(!hasLine(index, tag));

    int way = oldestWay(index);
    DPRINTF(TDTSimpleCache, "Miss: Replaced way: %d\n", way);
    // DONE: Direct-Mapped: Record new cache line in entries
    this->entries.at(index).at(way)->tag = tag;

    // TODO: Associative: Record LRU info for new line in entries
    for(int i = 0; i < this->associativity; i++){
        if(i == way){
            this->entries.at(index).at(way)->lastUsed = 0;
        }
        else{
            this->entries.at(index).at(way)->lastUsed++;
        }
    }

    sendResp(resp);
}

int
SimpleCache::calculateTag(Addr req)
{
    // DONE: Direct-Mapped: Calculate tag
    // hint: req >> ((int)std::log2(...

    return req >> ((int(std::log2(blockSize))) + (int)(std::log2(numSets))); // right shifts away the block size and index bits
}

int
SimpleCache::calculateIndex(Addr req)
{
    // DONE: Direct-Mapped: Calculate index
    req >>= ((int)std::log2(this->blockSize)); // right shift away the block bits
    req &= (int)(this->numSets - 1); // only include the index bits

    return req;
}

bool
SimpleCache::hasLine(int index, int tag)
{
    // DONE: Direct-Mapped: Check if line is already in cache
    // return this->entries.at(index).at(0)->tag == tag;

    // DONE: Associative: Check all possible ways
    for(int i = 0; i < associativity; i++){
        if(this->entries.at(index).at(i)->tag == tag){
            return true;
        }
    }
    return false;
}

int
SimpleCache::lineWay(int index, int tag)
{
    // DONE: Associative: Find in which way a cache line is stored
    for(int i = 0; i < associativity; i++){
        if(this->entries.at(index).at(i)->tag == tag){
            return i;
        }
    }
    
    return -1; //error
}

int
SimpleCache::oldestWay(int index)
{
    // TODO: Associative: Determine the oldest way
    int toBeTested;
    int oldestWay = this->entries.at(index).at(0)->lastUsed;
    for(int way = 1; way < this->associativity; way++){
        toBeTested = this->entries.at(index).at(way)->lastUsed;
        if(toBeTested > oldestWay){
            oldestWay = toBeTested;
        }
    }
    return oldestWay;
}

void
SimpleCache::sendReq(Addr req, int size)
{
    memSide->recvReq(req, size);
}

void
SimpleCache::sendResp(Addr resp)
{
    cpuSide->recvResp(resp);
}

}
