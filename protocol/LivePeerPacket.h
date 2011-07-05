//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

/**
* @file
* @brief Peer和Peer相关协议包相关的基类
*/

#ifndef LIVE_PEER_PACKET_H_
#define LIVE_PEER_PACKET_H_

#include "protocol/Packet.h"
#include "struct/LiveSubPieceInfo.h"
#include <util/serialization/stl/vector.h>

namespace protocol
{
    const boost::int8_t SUBPIECE_COUNT_IN_ONE_CHECK = 16;

    struct LivePeerPacket
        : Packet
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            Packet::serialize(ar);

            ar & protocol_version_;
            ar & packet_type_;
            ar & reserved_;
            ar & resource_id_;
        }

        virtual ~LivePeerPacket(){}

        LivePeerPacket()
        {
            reserved_ = 0;
            packet_type_ = LIVE_PACKET_TYPE;
            protocol_version_ = PEER_VERSION;
        }

        virtual boost::uint32_t length() const
        {
            return Packet::length() + 20;
        }

        boost::uint16_t protocol_version_;
        //  直播 packet_type_ = 128
        boost::uint8_t packet_type_;
        boost::uint8_t reserved_;
        Guid resource_id_;
    };

    template <boost::uint8_t action>
    struct LivePeerPacketT
        : PacketT<action>
        , LivePeerPacket
    {
    };

    /**
    *@brief  LiveRequestAnnounce 包
    */
    struct LiveRequestAnnouncePacket
        : LivePeerPacketT<0xC0>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            LivePeerPacket::serialize(ar);

            ar & request_block_id_;
            ar & upload_bandwidth_;
            ar & reserved2_;
        }

        LiveRequestAnnouncePacket()
        {
            request_block_id_ = 0;
            upload_bandwidth_ = 0;
            reserved2_ = 0;
        }

        LiveRequestAnnouncePacket(boost::uint32_t transaction_id, RID rid, boost::uint32_t request_block_id,
            boost::uint32_t upload_bandwidth, boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            request_block_id_ = request_block_id;
            upload_bandwidth_ = upload_bandwidth;
            reserved2_ = 0;
            end_point = endpoint_;
        }

        virtual boost::uint32_t length() const
        {
            return LivePeerPacket::length() + sizeof(request_block_id_) + sizeof(upload_bandwidth_)
                + sizeof(reserved2_);
        }

        boost::uint32_t request_block_id_;
        boost::uint32_t upload_bandwidth_;
        boost::uint32_t reserved2_;
    };

    struct LiveAnnounceMap
    {
        boost::uint16_t block_info_count_;
        boost::uint16_t live_interval_;
        std::map<boost::uint32_t, boost::dynamic_bitset<boost::uint8_t> > subpiece_map_;
        std::map<boost::uint32_t, boost::uint16_t> subpiece_nos_;
        boost::uint32_t request_block_id_;
    };

    template <typename Archive>
    void serialize(Archive & ar, LiveAnnounceMap & t)
    {
        util::serialization::split_free(ar, t);
    }

    template <typename Archive>
    void load(Archive & ar, LiveAnnounceMap & t)
    {
        boost::uint32_t start_block_info_id;

        ar & t.request_block_id_;
        ar & t.block_info_count_;
        std::vector<boost::uint16_t> subpiece_no;
        boost::uint16_t sn;

        
        for (boost::uint16_t i = 0; i < t.block_info_count_; ++i)
        {
            ar & sn;
            subpiece_no.push_back(sn);
        }
        ar & start_block_info_id;
        boost::uint32_t temp_block_info_id = start_block_info_id;

        ar & t.live_interval_;

        boost::uint16_t buffer_map_length;
        ar & buffer_map_length;

        // 首先把网络中传过来的uint8类型的buffer转成01串存在live_bits中
        boost::uint8_t buffer;
        std::deque<bool> live_bits;
        for (boost::uint16_t i = 0; i < buffer_map_length; ++i)
        {
            ar & buffer;
            for (int j = 7; j >= 0; --j)
            {
                live_bits.push_back((buffer >> j) % 2 == 1);
            }
        }

        boost::dynamic_bitset<boost::uint8_t> bit_set;
        t.subpiece_map_.clear();
        for (boost::uint16_t i = 0; i < t.block_info_count_; ++i)
        {
            // 计算这一个block含有多少位01串
            // 第一个subpiece单独占一位，之后每16个subpiece占一位，最后如果不足16个subpiece也占1位
            uint32_t bit_set_num = (subpiece_no[i] + SUBPIECE_COUNT_IN_ONE_CHECK - 2) / SUBPIECE_COUNT_IN_ONE_CHECK + 1;

            assert(live_bits.size() >= bit_set_num);

            bit_set.resize(bit_set_num);

            assert(live_bits.front());

            for (uint32_t j = 0; j < bit_set_num; ++j)
            {
                bit_set[j] = live_bits.front();
                live_bits.pop_front();
            }
            t.subpiece_map_.insert(std::make_pair(temp_block_info_id, bit_set));
            t.subpiece_nos_.insert(std::make_pair(temp_block_info_id, subpiece_no[i]));
            temp_block_info_id += t.live_interval_;
        }

        // 从live_bits中取完后，live_bits应该刚好变空，或者是剩下的都是0
        // 先考虑构造包的时候，假定我们总共有19个校验块（在这里把除第0个subpiece的每16个subpiece称为一个校验块，第0个subpiece自己是一个校验块），
        // 如果live_bits是 10110110 11011011 101，那么转化成uint8_t之后就是182,219,160(最后不足八位的在低位补0)
        // 收到包时，live_bits是10110110 11011011 10100000，因为不知道最后的0是不足补上去的，还是这个校验块不在，所以都存到了live_bits中
        // 那么也就是说live_bits中前19位是我所需要的，从第20位到最后是由于不足八位而补的0
        while (!live_bits.empty())
        {
            assert(live_bits.front() == 0);
            live_bits.pop_front();
        }
    };

    template <typename Archive>
    void save(Archive & ar, LiveAnnounceMap const & t)
    {
        assert(t.subpiece_map_.size() == t.subpiece_nos_.size());

        ar & t.request_block_id_;
        boost::uint16_t block_info_count = t.subpiece_map_.size();
        boost::uint32_t start_block_info_id;
        boost::uint16_t len = 0;
        ar & block_info_count;

        if (0 == block_info_count)
        {
            start_block_info_id = 0;
            ar & start_block_info_id;
            ar & t.live_interval_;
            ar & len;
            return ;
        }

        for (std::map<boost::uint32_t, boost::uint16_t>::const_iterator subpiece_no_iter = t.subpiece_nos_.begin();
            subpiece_no_iter != t.subpiece_nos_.end(); ++subpiece_no_iter)
        {
            ar & subpiece_no_iter->second;
        }

        std::map<boost::uint32_t, boost::dynamic_bitset<boost::uint8_t> >::const_iterator iter;
        iter = t.subpiece_map_.begin();
        start_block_info_id = iter->first;
        ar & start_block_info_id;
        ar & t.live_interval_;

        // 计算这些01串转成uint8后有多少位
        std::map<boost::uint32_t, boost::uint16_t>::const_iterator subpiece_no_iter = t.subpiece_nos_.begin();
        for (; iter != t.subpiece_map_.end(); ++iter)
        {
            len += iter->second.size();
            assert(iter->second[0]);
            assert((subpiece_no_iter->second + SUBPIECE_COUNT_IN_ONE_CHECK - 2) / SUBPIECE_COUNT_IN_ONE_CHECK + 1
                == iter->second.size());
            ++subpiece_no_iter;
        }
        len = (len + 7) / 8;
        ar & len;

        boost::dynamic_bitset<boost::uint8_t> buffer;
        int count = 0;
        boost::uint8_t sum = 0;
        for (iter = t.subpiece_map_.begin(); iter != t.subpiece_map_.end(); ++iter)
        {
            buffer = iter->second;

            // 通过左移操作将每8位转成一个uint8类型的
            for (uint32_t i = 0; i < buffer.size(); ++i)
            {
                sum = sum << 1;
                sum += buffer[i];
                count++;
                if (count == 8)
                {
                    ar & sum;
                    sum = 0;
                    count = 0;
                }
            }
        }

        // 最后不足8位的左移相应的位数，也序列化成一个uint8类型的
        if (count != 0)
        {
            sum = sum << (8 - count);
            ar & sum;
        }
    }

     /**
    *@brief  LiveAnnounce 包
    */
    struct LiveAnnouncePacket
        : LivePeerPacketT<0xC1>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            LivePeerPacket::serialize(ar);
            ar & live_announce_map_;
        }

        LiveAnnouncePacket(uint32_t transaction_id, RID rid,
            boost::uint16_t block_info_count,
            std::map<boost::uint32_t, boost::dynamic_bitset<boost::uint8_t> > subpiece_map,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            live_announce_map_.block_info_count_ = block_info_count;
            live_announce_map_.subpiece_map_ = subpiece_map;
            end_point = endpoint_;
        }

        LiveAnnouncePacket(uint32_t transaction_id, RID rid,
            LiveAnnounceMap live_announce_map,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            live_announce_map_ = live_announce_map;
            end_point = endpoint_;
        }

        LiveAnnouncePacket()
        {
            live_announce_map_.block_info_count_ = 0;
        }

        virtual boost::uint32_t length() const
        {
            return LivePeerPacket::length()
                + sizeof(boost::uint32_t)  // request_block_id_
                + sizeof(boost::uint16_t)  // piece_info_count_
                + sizeof(boost::uint16_t) * live_announce_map_.subpiece_map_.size()  // subpiece_no_
                + sizeof(boost::uint32_t)  // start_piece_info_id_
                + sizeof(boost::uint16_t)  // live_interval_
                + sizeof(boost::uint16_t)  // len
                + live_announce_map_.subpiece_map_.size() * sizeof(boost::uint8_t);
        }

        LiveAnnounceMap live_announce_map_;
    };

    /**
    *@brief  LiveRequestSubPiecePacket
    */
    struct LiveRequestSubPiecePacket
        : LivePeerPacketT<0xC2>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            LivePeerPacket::serialize(ar);

            ar & util::serialization::make_sized<boost::uint8_t>(sub_piece_infos_);
            request_sub_piece_count_ = sub_piece_infos_.size();
            ar & priority_;
        }

        LiveRequestSubPiecePacket()
        {
        }

        LiveRequestSubPiecePacket(
            boost::uint32_t  transaction_id, const RID& rid,
            const std::vector<LiveSubPieceInfo>& sub_piece_infos, boost::uint16_t priority, boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            sub_piece_infos_ = sub_piece_infos;
            priority_ = priority;
            end_point = endpoint_;
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t len = LivePeerPacket::length() + sizeof(request_sub_piece_count_) +
                sub_piece_infos_.size() * LiveSubPieceInfo::length() + sizeof(priority_);

            return len;
        }

        boost::uint8_t request_sub_piece_count_;
        std::vector<LiveSubPieceInfo> sub_piece_infos_;
        boost::uint16_t priority_;
        static const boost::uint16_t DEFAULT_LIVE_PRIORITY = 9;
    };

    /**
    *@brief  LiveSubPiece 包
    */
    struct LiveSubPiecePacket
        : LivePeerPacketT<0xC3>
    {
        LiveSubPiecePacket()
        {
            sub_piece_content_.reset(new LiveSubPieceContent);
            assert(sub_piece_content_);
        }

        LiveSubPiecePacket(
            boost::uint32_t transaction_id,
            RID rid,
            LiveSubPieceInfo sub_piece_info,
            boost::uint16_t sub_piece_length,
            LiveSubPieceBuffer sub_piece_content,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            sub_piece_info_ = sub_piece_info;
            sub_piece_length_ = sub_piece_length;
            sub_piece_content_ = sub_piece_content.GetSubPieceBuffer();
            end_point = endpoint_;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            if (!sub_piece_content_ || !*sub_piece_content_)
            {
                ar.fail();
                return;
            }
            LivePeerPacket::serialize(ar);
            ar & sub_piece_info_;
            ar & sub_piece_length_ ;

            for (size_t i = 0 ; i < sub_piece_length_ ; i++)
            {
                ar & *(sub_piece_content_->get_buffer() + i);
            }

        }

        virtual boost::uint32_t length() const
        {
            return LivePeerPacket::length() + sizeof(sub_piece_info_) + sizeof( sub_piece_length_ )
                + sub_piece_length_;
        }


        LiveSubPieceContent::pointer sub_piece_content_;
        boost::uint16_t sub_piece_length_;
        LiveSubPieceInfo sub_piece_info_;
    };

    template <typename PacketHandler>
    void register_live_peer_packet(
        PacketHandler & handler)
    {
        handler.template register_packet<LiveRequestAnnouncePacket>();
        handler.template register_packet<LiveAnnouncePacket>();
        handler.template register_packet<LiveRequestSubPiecePacket>();
        handler.template register_packet<LiveSubPiecePacket>();
    }
}

#endif // LIVE_PEER_PACKET_H_
