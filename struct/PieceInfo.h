//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

// protocol::PieceInfo.h

#ifndef _STORAGE_PIECE_INFO_H_
#define _STORAGE_PIECE_INFO_H_

namespace protocol
{
    struct PieceInfo
    {
        uint32_t block_index_;
        uint32_t piece_index_;

        explicit PieceInfo(
            uint32_t block_index = 0,
            uint32_t piece_index = 0)
            : block_index_(block_index)
            , piece_index_(piece_index)
        {
        }

        static void MakeByPosition(
            uint32_t position,
            uint32_t block_size,
            PieceInfo & piece_info)
        {
            piece_info.block_index_ = (block_size == 0 ? 0 : position / block_size);
            piece_info.piece_index_ = (block_size == 0 ? 0 : (position % block_size) / PIECE_SIZE);
        }

        bool operator < (const PieceInfo& n) const
        {
            if (block_index_ != n.block_index_)
                return block_index_ < n.block_index_;
            else return piece_index_ < n.piece_index_;
        }

        int operator - (const PieceInfo& n) const
        {
            return block_index_ * 16 + piece_index_ -
                (n.block_index_ * 16 + n.piece_index_);
        }

        bool operator == (const PieceInfo& n) const
        {
            return block_index_ == n.block_index_ && piece_index_ == n.piece_index_;
        }

        uint32_t GetPieceIndexInFile(
            uint32_t block_size) const
        {
            assert(block_size % PIECE_SIZE == 0);
            uint32_t piece_count_in_block = block_size / PIECE_SIZE;
            return block_index_*piece_count_in_block + piece_index_;
        }

        uint32_t GetPosition(
            uint32_t block_size) const
        {
            return block_index_*block_size + piece_index_*PIECE_SIZE;
        }

        uint32_t GetEndPosition(
            uint32_t block_size) const
        {
            assert(block_size % PIECE_SIZE == 0);
            return block_index_*block_size + piece_index_*PIECE_SIZE + PIECE_SIZE;
        }

        uint32_t GetBlockEndPosition(
            uint32_t block_size) const
        {
            assert(block_size % PIECE_SIZE == 0);
            return (block_index_ + 1)*block_size;
        }

        friend std::ostream& operator << (
            std::ostream& os,
            const PieceInfo& n)
        {
            os << "(" << n.block_index_ << "|p:" << n.piece_index_ << ")";
            return os;
        }
    };

    struct PieceInfoEx
        : PieceInfo
    {
        boost::uint16_t subpiece_index_;
        // piece下载的终点，默认为127
        boost::uint16_t subpiece_index_end_;

        explicit PieceInfoEx(
            uint32_t block_index = 0,
            uint32_t piece_index = 0,
            boost::uint16_t subpiece_index = 0)
            : PieceInfo(block_index, piece_index)
            , subpiece_index_(subpiece_index)
            , subpiece_index_end_(127)
        {
        }

        uint32_t GetPosition(
            uint32_t block_size) const
        {
            return PieceInfo::GetPosition(block_size) + subpiece_index_*SUB_PIECE_SIZE;
        }

        uint32_t GetEndPosition(
            uint32_t block_size) const
        {
            // 计算piece最后的position的时候，需要额外加上最后一片的1024字节
            return PieceInfo::GetPosition(block_size) + (subpiece_index_end_+1) * SUB_PIECE_SIZE;
        }

        PieceInfo GetPieceInfo() const
        {
            return PieceInfo(block_index_, piece_index_);
        }

        friend std::ostream& operator << (
            std::ostream & os,
            const PieceInfoEx & n)
        {
            os << "(" << n.block_index_ << "|p:" << n.piece_index_ << "|sp:" << n.subpiece_index_ << ")";
            return os;
        }
    };
}

#endif  // _STORAGE_PIECE_INFO_H_
