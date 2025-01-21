#include "tt.hpp"

uint32_t hash_size;
HASHE* hash_table;

//Allocate hash table (size_mb is size of hash table in mb)
//Returns true if allocation succeeded, false if it failed
bool alloc_hash(uint32_t size_mb)
{
    size_mb *= 1000000 / sizeof(HASHE); //now size_mb is number of elements
    free(hash_table); //dont leak loads of mem when setoption!
    hash_table = (HASHE*)calloc(size_mb, sizeof(HASHE));
    if (hash_table != NULL) //check for success
    {
        hash_size = size_mb; //this has to be the number of elements
        return true;
    }
    return false; //failure: return false
}

//look at https://gitlab.com/mhouppin/stash-bot/-/blob/8ec0469cdcef022ee1bc304299f7c0e3e2674652/sources/tt/tt_probe.c
HASHE* ProbeHash(W_Board &board, int8_t ply)
{
    uint64_t curhash = board.hash();
    HASHE* phashe = &hash_table[curhash % hash_size];

    if (phashe->key == curhash) //hit
    {
        if (abs(phashe->val) > 32256) //mate score
        {
            //adjust based on ply
            if (phashe->val < 0)
                phashe->val += ply; //new_score = old_score + new_ply - old_ply
            else
                phashe->val -= ply; //same backwards
        }

        return phashe; //return entry
    }

    return nullptr; //miss (crashes if gets dereferenced)
}

//extremely basic always replace scheme (doesn't even check if it was the same node previously)
//look at https://gitlab.com/mhouppin/stash-bot/-/blob/8ec0469cdcef022ee1bc304299f7c0e3e2674652/sources/tt/tt_save.c
void RecordHash(W_Board &board, int8_t depth, int32_t val, uint8_t flags, const Move &best_move, int8_t ply)
{
    //important for persistent hash table
    if (val == INT32_MAX || val == -INT32_MAX) return; //don't store panic bogus in TT!

    //DO NOT STORE when close to a draw!
    if (board.halfMoveClock() > 60) return;

    if (abs(val) > 32256) //mate score
    {
        //adjust based on ply
        if (val < 0)
            val -= ply; //new_score = old_score + new_ply - old_ply
        else
            val += ply; //same backwards
    }

    uint64_t curhash = board.hash();
    HASHE* phashe = &hash_table[curhash % hash_size];

    //avoid replacing "better" search nodes with qs/eval ones
    // if (depth == 0 && phashe->depth >= 3) //1 or 2 depth can be replaced tho
    //     return;

    phashe->key = curhash;
    phashe->best = best_move.move();
    phashe->val = val;
    phashe->flags = flags; //TODO: tweak a bit
    phashe->depth = depth;
}
