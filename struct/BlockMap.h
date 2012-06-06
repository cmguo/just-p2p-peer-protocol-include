//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

#ifndef _PEER_STORAGE_BLOCK_MAP_H_
#define _PEER_STORAGE_BLOCK_MAP_H_

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <iostream>

namespace protocol
{

    class BlockMap
#ifdef DUMP_OBJECT
        : public count_object_allocate<BlockMap>
#endif
    {
    public:
        BlockMap()
        {
        }

        BlockMap(
            boost::uint8_t* inbuf,
            uint32_t bytes_num,
            uint32_t bits_num)
        {
            try
            {
                if (inbuf == NULL)
                {
                    bitset_.resize(bits_num, false);
                }
                for (uint32_t i = 0;i<bytes_num;i++)
                {
                    bitset_.append(inbuf[i]);
                }
                if (bits_num>0)
                    bitset_.resize(bits_num);
            }
            catch(...)
            {

            }
        };

        typedef boost::shared_ptr<BlockMap> p;
        friend std::ostream & operator << (std::ostream & os, const BlockMap & bm);

        static BlockMap::p Create(
            uint32_t bitmap_size)
        {
            BlockMap::p pointer;
            pointer = BlockMap::p(new BlockMap(NULL, 0, bitmap_size));
            return pointer;
        };

        void Set(
            uint32_t index)
        {
            assert(index<bitset_.size());
            bitset_.set(index);
        }
        void Reset(
            uint32_t index)
        {
            assert(index<bitset_.size());
            bitset_.reset(index);
        }

        bool IsFull() const
        {
            return bitset_.count() == bitset_.size();
        }

        BlockMap::p Clone() const
        {
            BlockMap::p new_block_map =  BlockMap::p(new BlockMap());
            new_block_map->bitset_ = bitset_;
            return new_block_map;
        };

        bool HasBlock(
            uint32_t index) const
        {
            if (index < bitset_.size()) {
                return bitset_.test(index);
            }
            return false;
        };

        uint32_t GetCount() const
        {
            return bitset_.count();
        }

        template <typename OutIter>
        void GetBytes(
            OutIter & iter) const
        {
            boost::to_block_range(bitset_, iter);
        }

        void Resize(
            uint32_t num_bits,
            bool value = false)
        {
            bitset_.resize(num_bits, value);
        }

        void SetAll(
            bool value)
        {
            if (value)
                bitset_.set();
            else
                bitset_.reset();
        }

        void Set(
            int index)
        {
            assert((uint32_t)index<bitset_.size());
            bitset_.set(index);
        }

    private:
        boost::dynamic_bitset<boost::uint8_t> bitset_;
    };

    inline std::ostream & operator << (std::ostream & os, const BlockMap & bm)
    {
        return os << bm.bitset_;
    }
};

#endif  // _PEER_STORAGE_BLOCK_MAP_H_
