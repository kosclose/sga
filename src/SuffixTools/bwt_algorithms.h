//-----------------------------------------------
// Copyright 2009 Wellcome Trust Sanger Institute
// Written by Jared Simpson (js18@sanger.ac.uk)
// Released under the GPL license
//-----------------------------------------------
//
// bwt_algorithms.h - Algorithms for aligning to a bwt structure
//
#ifndef BWT_ALGORITHMS_H
#define BWT_ALGORITHMS_H

#include "STCommon.h"
#include "BWT.h"
#include <queue>
#include <list>

#define LEFT_INT_IDX 0
#define RIGHT_INT_IDX 1

// types
enum ExtendDirection
{
	ED_LEFT,
	ED_RIGHT
};

struct BWTInterval
{
	int64_t lower;
	int64_t upper;
	inline bool isValid() const { return lower <= upper; }
	
	static inline bool compare(const BWTInterval& a, const BWTInterval& b)
	{
		if(a.lower == b.lower)
			return a.upper < b.upper;
		else
			return a.lower < b.lower;
	}

	static inline bool equal(const BWTInterval& a, const BWTInterval& b)
	{
		return a.lower == b.lower && a.upper == b.upper;
	}
};

struct BWTIntervalPair
{
	BWTInterval& get(unsigned int idx) { return interval[idx]; }
	BWTInterval interval[2];
};

struct BWTAlign
{
	inline int length() const { return right_index - left_index + 1; }
	inline bool isSeed() const { return length() < seed_len; }
	inline bool isIntervalValid(int idx) { return ranges.interval[idx].isValid(); }

	int left_index; // inclusive
	int right_index; // inclusive
	int seed_len;
	ExtendDirection dir; // the direction that this alignment is being extended in
	int z;

	BWTIntervalPair ranges; // ranges.interval[0] is the left interval, 1 is the right 
	
	// Sort the alignments based on their r_lower/r_upper
	static inline bool compareLeftRange(const BWTAlign& a, const BWTAlign& b)
	{
		return BWTInterval::compare(a.ranges.interval[0], b.ranges.interval[0]);
	}

	// Compare for equality based on the left range
	// If the length of the alignment is equal, then if the left ranges
	// are a match, the two alignment objects are redundant and one can be removed
	static inline bool equalLeftRange(const BWTAlign& a, const BWTAlign& b)
	{
#ifdef VALIDATE
		if(BWTInterval::equal(a.ranges.inteval[0], b.ranges.interval[0]))
			assert(a.length() == b.length() && a.z == b.z);
#endif
		return BWTInterval::equal(a.ranges.interval[0], b.ranges.interval[0]);
	}
	

	void print() const
	{
		printf("li: %d ri: %d sl: %d dir: %d z: %d lrl: %d lru: %d rlr: %d rlu: %d\n", 
		        left_index, right_index, seed_len, dir, z, 
				(int)ranges.interval[0].lower, (int)ranges.interval[0].upper, 
				(int)ranges.interval[1].lower, (int)ranges.interval[1].upper);
	}


	void print(const std::string& w) const
	{
		printf("sub: %s li: %d ri: %d sl: %d dir: %d z: %d lrl: %d lru: %d rlr: %d rlu: %d\n", 
		        w.substr(left_index, length()).c_str(), left_index, right_index,
				seed_len, dir, z, (int)ranges.interval[0].lower, (int)ranges.interval[0].upper, 
				(int)ranges.interval[1].lower, (int)ranges.interval[1].upper);
	}
};

typedef std::queue<BWTAlign> BWTAlignQueue;
typedef std::list<BWTAlign> BWTAlignList;

// functions

//
// Update both the left and right intervals using pRevBWT
// This assumes that the left/right ranges in ipair are for string S
// It returns the updated left/right ranges for string Sb (appending b)
// using the pRevBWT to update both
inline void updateBothR(BWTIntervalPair& pair, char b, const BWT* pRevBWT)
{
	// Update the left index using the difference between the AlphaCounts in the reverse table
	AlphaCount diff = pRevBWT->getOccDiff(pair.interval[1].lower - 1, pair.interval[1].upper);
	pair.interval[0].lower = pair.interval[0].lower + diff.getLessThan(b);
	pair.interval[0].upper = pair.interval[0].lower + diff.get(b) - 1;

	// Update the right index directly
	size_t pb = pRevBWT->getC(b);
	pair.interval[1].lower = pb + pRevBWT->getOcc(b, pair.interval[1].lower - 1);
	pair.interval[1].upper = pb + pRevBWT->getOcc(b, pair.interval[1].upper) - 1;

}

// Update the left interval in pair using pB
// This assumes the left interval is for string S
// and returns the interval for bS (prepend b)
inline void updateLeft(BWTIntervalPair& pair, char b, const BWT* pB)
{
	size_t pb = pB->getC(b);
	pair.interval[LEFT_INT_IDX].lower = pb + pB->getOcc(b, pair.interval[LEFT_INT_IDX].lower - 1);
	pair.interval[LEFT_INT_IDX].upper = pb + pB->getOcc(b, pair.interval[LEFT_INT_IDX].upper) - 1;

}

int alignSuffixInexact(const std::string& w, const BWT* pBWT, const BWT* pRevBWT, 
                       double error_rate, int minOverlap, Hit& hitTemplate, HitVector* pHits);

int alignSuffixInexactExhaustive(const std::string& w, const BWT* pBWT, const BWT* pRevBWT, 
                       double error_rate, int minOverlap, Hit& hitTemplate, HitVector* pHits);

int alignSuffixMaxDiff(const std::string& w, const BWT* pBWT, const BWT* pRevBWT, int maxDiff, int minOverlap, Hit& hitTemplate, HitVector* pHits);

int _alignBlock(const std::string& w, int block_start, int block_end,
                const BWT* pBWT, const BWT* pRevBWT, int maxDiff, Hit& hitTemplate, HitVector* pHits);




#endif