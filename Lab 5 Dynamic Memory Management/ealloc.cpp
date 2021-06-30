#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "ealloc.h"
#include<bits/stdc++.h>
using namespace std;

int max_pages = 4;
vector<char *> ptr;
vector< map<char*, int> > free_space, alloc_space; // size of each free and allocated block


int init_page(int pg)
{
    ptr[pg] = (char *)mmap(NULL, PAGESIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
    free_space[pg].clear();
    alloc_space[pg].clear();
    free_space[pg][ptr[pg]] = PAGESIZE;
    if(ptr[pg]==MAP_FAILED)
		return -1;
    else
		return 0; // success
}

char *alloc_page(int sz, int pg)
{
    // first fit method
    for(auto it = free_space[pg].begin(); it!=free_space[pg].end(); it++)
	{
        if(it->second >= sz)
		{
			// found
            char * rptr = it->first + it->second - sz;
            it->second -= sz; // splitting
            if(it->second == 0)
			{
                free_space[pg].erase(it);
            }
            alloc_space[pg][rptr] = sz;
            return rptr;
        }
    }
    return NULL; // no space found
}

int dealloc_page(char * ch_ptr, int pg)
{
    if(alloc_space[pg].find(ch_ptr) == alloc_space[pg].end())
	{
        return -1; // address not allocated on this page before
    }
    int sz = alloc_space[pg][ch_ptr];
    alloc_space[pg].erase(ch_ptr);

    // merge with chunk on right
    auto it = free_space[pg].find(ch_ptr + sz);
    if(it != free_space[pg].end())
	{
        sz += it->second;
        free_space[pg].erase(it);
    }
    free_space[pg][ch_ptr] = sz;

    // merge with chunk on left
    it = free_space[pg].find(ch_ptr);
    if(it != free_space[pg].begin())
	{
        it--;
        if(it->first+it->second == ch_ptr)
		{
            it->second += free_space[pg][ch_ptr];
            free_space[pg].erase(ch_ptr);
        }
    }
    return 0;

}

void init_alloc()
{
    ptr.clear(); free_space.clear(); alloc_space.clear();
    ptr.resize(max_pages, NULL);
    free_space.resize(max_pages);
    alloc_space.resize(max_pages);
    return;
}

void cleanup()
{
    ptr.clear();
    free_space.clear();
    alloc_space.clear();
    return;
}

char *alloc(int sz)
{
    if(sz % MINALLOC != 0)
		return NULL;

    for(int pg=0; pg<max_pages; pg++)
	{
        if(ptr[pg] == NULL)
		{
            int v = init_page(pg);
            if(v==-1)
			{
                return NULL;
            }
        }
        char * rptr = alloc_page(sz, pg);
        if(rptr != NULL)
		{
			// found space
            return rptr;
        }
    }

    return NULL; // no space found
}

void dealloc(char * ch_ptr)
{
    for(int pg=0; pg<max_pages; pg++)
	{
        if(ptr[pg] == NULL)
		{
            break;
        }
        int v = dealloc_page(ch_ptr, pg);
        if(v == 0)
		{
			// deallocation complete
            return;
        }
    }

}

