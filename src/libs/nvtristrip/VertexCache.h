  
#ifndef VERTEX_CACHE_H

#define VERTEX_CACHE_H

class VertexCache
{
	
public:
	
	VertexCache(int size)
	{
		numEntries = size;
		
		entries = new int[numEntries];
		
		for(int i = 0; i < numEntries; i++)
			entries[i] = -1;
	}
		
	VertexCache() { VertexCache(16); }
	~VertexCache() { delete[] entries; entries = 0; }
	
	bool InCache(int entry)
	{
		bool returnVal = false;
		for(int i = 0; i < numEntries; i++)
		{
			if(entries[i] == entry)
			{
				returnVal = true;
				break;
			}
		}
		
		return returnVal;
	}
	
	int AddEntry(int entry)
	{
		int removed;
		
		removed = entries[numEntries - 1];
		
		//push everything right one
		for(int i = numEntries - 2; i >= 0; i--)
		{
			entries[i + 1] = entries[i];
		}
		
		entries[0] = entry;
		
		return removed;
	}

	void Clear()
	{
		memset(entries, -1, sizeof(int) * numEntries);
	}
	
	void Copy(VertexCache* inVcache);

	int At(int index) { return entries[index]; }
	void Set(int index, int value) { entries[index] = value; }

private:

  int *entries;
  int numEntries;

};

#include <algorithm>

inline void VertexCache::Copy(VertexCache* inVcache)
    {
        if(inVcache != this)
        {
            const int* source = entries;
            std::copy(source, source + numEntries, inVcache->entries);
        }
    }

#endif
