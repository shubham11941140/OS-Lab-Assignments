#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "alloc.h"
#include<bits/stdc++.h>
using namespace std;

char *ptr;
map<char*, int> free_space; // size of each free block
map<char*, int> alloc_space; // size of each allocated block

int init_alloc()
{
    ptr = (char *)mmap(NULL, PAGESIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);// -1
    free_space.clear();
    alloc_space.clear();
    free_space[ptr] = PAGESIZE;
    if(ptr==MAP_FAILED)
		return -1;
    else
		return 0; // success
}

int cleanup()
{
    int v = munmap(ptr, PAGESIZE);
    free_space.clear();
    alloc_space.clear();
    if(v==0)
		return 0; // success
    else
		return -1;
}

char *alloc(int sz)
{
    if(sz % MINALLOC != 0)
		return NULL;
    // first fit method
    for(auto it = free_space.begin(); it!=free_space.end(); it++)
	{
        if(it->second >= sz)
		{
			// found
            char * rptr = it->first + it->second - sz;
            it->second -= sz; // splitting
            if(it->second == 0)
			{
                free_space.erase(it);
            }
            alloc_space[rptr] = sz;
            return rptr;
        }
    }
    return NULL; // no space found
}

void dealloc(char * ch_ptr)
{
    if(alloc_space.find(ch_ptr) == alloc_space.end())
	{
        return; // address not allocated before
    }
    int sz = alloc_space[ch_ptr];
    alloc_space.erase(ch_ptr);

    // merge with chunk on right
    auto it = free_space.find(ch_ptr + sz);
    if(it != free_space.end())
	{
        sz += it->second;
        free_space.erase(it);
    }
    free_space[ch_ptr] = sz;

    // merge with chunk on left
    it = free_space.find(ch_ptr);
    if(it != free_space.begin())
	{
        it--;
        if(it->first+it->second == ch_ptr)
		{
            it->second += free_space[ch_ptr];
            free_space.erase(ch_ptr);
        }
    }

}