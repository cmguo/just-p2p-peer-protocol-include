//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

// PeerPacketT.h

/**
* @file
* @brief Peer和Peer相关协议包相关的基类
*/

#ifndef _PROTOCOL_PEER_PACKET_H_
#define _PROTOCOL_PEER_PACKET_H_

#include "protocol/Packet.h"
#include "struct/serialization.h"
#include "struct/Base.h"
#include "struct/UdpBuffer.h"

#include <util/serialization/stl/string.h>
#include <util/serialization/stl/vector.h>
#include <util/archive/ArchiveBuffer.h>
#include <util/archive/LittleEndianBinaryIArchive.h>

namespace protocol
{
    struct PeerPacket
        : Packet
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            Packet::serialize(ar);
            ar & *this;
        }

        PeerPacket()
        {
            sequece_id_ = PEER_VERSION;
            reserve_ = 0;
            packet_type_ = VOD_PACKET_TYPE;
            reserve2_ = 0;
        }

        virtual ~PeerPacket(){}

        virtual boost::uint32_t length() const
        {
            return Packet::length() + sizeof(sequece_id_) + sizeof(reserve_) + sizeof(resource_id_) + sizeof(peer_guid_);
        }

        boost::uint16_t sequece_id_;
        boost::uint16_t reserve_;
        RID resource_id_;
        Guid peer_guid_;
        boost::uint16_t protocol_version_;
        boost::uint8_t packet_type_;
        boost::uint8_t reserve2_;

        friend bool operator == (
            PeerPacket const & l,
            PeerPacket const & r)
        {
            // live
            if (l.packet_type_ == LIVE_PACKET_TYPE && r.packet_type_ == LIVE_PACKET_TYPE)
            {
                return l.protocol_version_ == r.protocol_version_ && l.reserve2_ == r.reserve2_
                    && l.resource_id_ == r.resource_id_ && l.peer_guid_ == r.peer_guid_
                    && l.end_point == r.end_point && l.PacketAction == r.PacketAction && l.transaction_id_ == r.transaction_id_;
            }
            else
            {
                return l.sequece_id_ == r.sequece_id_ && l.reserve_ == r.reserve_ && l.resource_id_ == r.resource_id_ && l.peer_guid_ == r.peer_guid_
                    && l.end_point == r.end_point && l.PacketAction == r.PacketAction && l.transaction_id_ == r.transaction_id_;
            }
        }
    };

    template <typename Archive>
    void serialize(Archive & ar, PeerPacket & t)
    {
        util::serialization::split_free(ar, t);
    }

    template <typename Archive>
    void load(Archive & ar, PeerPacket & t)
    {
        boost::uint8_t temp[4];
        ar & framework::container::make_array(temp);

        util::archive::ArchiveBuffer<> buf((char*)temp, 4, 4);
        util::archive::LittleEndianBinaryIArchive<> ia(buf);

        //          点播                                直播
        // ____________________________         ________________________________
        // |             |             |        |                   |          |
        // |sequece_id_  | uint16_t    |        |protocol_version_  | uint16_t |
        // |             |             |        |                   |          |
        // |_____________|_____________|        |___________________|__________|
        // |             |             |        |packet_type_       | uint8_t  |
        // |reserve_     | uint16_t    |        |___________________|__________|
        // |             |             |        |reserve2_          | uint8_t  |
        // |_____________|_____________|        |___________________|__________|

        // 判断packet_type_字段，如果为128代表是直播
        // 这里我们要求reserve_的前8位必须不能是128

        if (LIVE_PACKET_TYPE == temp[2])  // live
        {
            ia >> t.protocol_version_;
            ia >> t.packet_type_;
            ia >> t.reserve2_;
        }
        else  // vod
        {
            ia >> t.sequece_id_;
            ia >> t.reserve_;
        }

        ar & t.resource_id_;
        ar & t.peer_guid_;
    };

    template <typename Archive>
    void save(Archive & ar, PeerPacket const & t)
    {
        if (128 == t.packet_type_)  // live
        {
            ar & t.protocol_version_;
            ar & t.packet_type_;
            ar & t.reserve2_;
        }
        else  // vod
        {
            ar & t.sequece_id_;
            ar & t.reserve_;
        }

        ar & t.resource_id_;
        ar & t.peer_guid_;
    }

    template <boost::uint8_t action>
    struct PeerPacketT
        : PacketT<action>
        , PeerPacket
    {
        virtual ~PeerPacketT(){}
    };

    /**
    *@brief  Error 包
    */
    struct ErrorPacket
        : PeerPacketT<0x51>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
            ar & error_code_;
            ar & util::serialization::make_sized<boost::uint16_t>(error_info_);
        }

        template <typename PeerPacketType>
        ErrorPacket(PeerPacketType const &packet)
        {
            transaction_id_ = packet.transaction_id_;
            resource_id_ = packet.resource_id_;
            error_info_length_ = 0;
            error_info_ = "";
            end_point = packet.end_point;
            // PacketAction = Action;
        }

        ErrorPacket(boost::uint32_t transaction_id, RID rid, Guid peer_guid,
            boost::uint16_t error_code, boost::uint16_t error_info_length, string error_info, boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            error_code_ = error_code;
            error_info_length_ = error_info_length;
            error_info_ = error_info;
            end_point = endpoint_;
            // PacketAction = Action;
        }

        ErrorPacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            return PeerPacket::length() + sizeof(error_code_) + sizeof(error_info_length_) + error_info_.length();
        }

        static const boost::uint16_t PPV_EXCHANGE_NO_RESOURCEID = 0x0011;
        static const boost::uint16_t PPV_EXCHANGE_NOT_DOWNLOADING = 0x0012;
        static const boost::uint16_t PPV_CONNECT_NO_RESOURCEID = 0x0021;
        static const boost::uint16_t PPV_CONNECT_CONNECTION_FULL = 0x0022;
        static const boost::uint16_t PPV_ANNOUCE_NO_RESOURCEID = 0x0031;
        static const boost::uint16_t PPV_SUBPIECE_NO_RESOURCEID = 0x0041;
        static const boost::uint16_t PPV_SUBPIECE_SUBPIECE_NOT_FOUND = 0x0042;
        static const boost::uint16_t PPV_SUBPIECE_UPLOAD_BUSY = 0x0043;
        static const boost::uint16_t PPV_RIDINFO_NO_RESOURCEID = 0x0051;

        boost::uint16_t error_code_;
        boost::uint16_t error_info_length_;
        std::string error_info_;
    };

    /**
    *@brief  Connect 包
    */
    struct ConnectPacket
        : PeerPacketT<0x52>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
            ar & basic_info_;
            ar & send_off_time_;
            ar & peer_version_;
            ar & peer_info_;
            ar & reserve3_;
            ar & peer_download_info_;
            ar & ip_pool_size_;
        }

        ConnectPacket(boost::uint32_t transaction_id,
            boost::uint8_t connect_type, RID rid, Guid peer_guid, boost::uint16_t sequece_id_or_protocol_version,
            boost::uint8_t basic_info, boost::uint32_t send_off_time, boost::uint32_t peer_version,
            CandidatePeerInfo peer_info, PEER_DOWNLOAD_INFO download_info, boost::asio::ip::udp::endpoint endpoint_, boost::uint16_t ip_pool_size = 0)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            
            if (LIVE_PACKET_TYPE == connect_type)
            {
                protocol_version_ = sequece_id_or_protocol_version;
                packet_type_ = LIVE_PACKET_TYPE;
                reserve2_ = 0;
            }
            else
            {
                sequece_id_ = sequece_id_or_protocol_version;
                reserve_ = 0;
            }
            peer_guid_ = peer_guid;
            basic_info_ = basic_info;
            send_off_time_ = send_off_time;
            peer_version_ = peer_version;
            peer_info_ = peer_info;
            peer_download_info_ = download_info;
            end_point = endpoint_;
            ip_pool_size_ = ip_pool_size;
        }

        ConnectPacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            return PeerPacket::length() + sizeof(basic_info_) + sizeof(send_off_time_) + sizeof(peer_version_)
                + sizeof(peer_info_) + sizeof(reserve3_) + sizeof(peer_download_info_) + sizeof(ip_pool_size_);
        }

        inline bool IsRequest() const{ if (basic_info_ % 2 == 0) return true; return false;}

        boost::uint8_t basic_info_;
        boost::uint32_t send_off_time_;
        boost::uint32_t peer_version_;
        CandidatePeerInfo peer_info_;
        boost::uint8_t reserve3_;
        PEER_DOWNLOAD_INFO peer_download_info_;
        boost::uint16_t ip_pool_size_;
    };

    /**
    *@brief  RequestAnnounce 包
    */
    struct RequestAnnouncePacket
        : PeerPacketT<0x53>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
        }

        RequestAnnouncePacket(boost::uint32_t transaction_id, RID rid, Guid peer_guid, boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            end_point = endpoint_;
            // PacketAction = Action;
        }

        RequestAnnouncePacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            return PeerPacket::length();
        }
    };

    /**
    *@brief  Announce 包
    */
    struct AnnouncePacket
        : PeerPacketT<0x54>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
            ar & peer_download_info_;
            ar & block_map_;
        }

        AnnouncePacket(uint32_t transaction_id, RID rid, Guid peer_guid,
            PEER_DOWNLOAD_INFO peer_download_info, BlockMap block_map,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            peer_download_info_ = peer_download_info;
            block_map_ = block_map;
            end_point = endpoint_;
            // PacketAction = Action;
        }

        AnnouncePacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            boost::uint8_t buf[32];
            boost::uint8_t * p = buf;

            block_map_.GetBytes(p);

            return PeerPacket::length() + sizeof(peer_download_info_)
                + sizeof(boost::uint32_t)  // bitset_size
                + (p - buf) * block_map_.GetCount();
        }

        PEER_DOWNLOAD_INFO peer_download_info_;
        BlockMap block_map_;
    };

    /**
    *@brief  RequestSubPiecePacketOld 包, 请求报文中包含GUID
    */
    struct RequestSubPiecePacketOld
        : PeerPacketT<0x55>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
            ar & util::serialization::make_sized<boost::uint16_t>(subpiece_infos_);
        }

        RequestSubPiecePacketOld() {}
        RequestSubPiecePacketOld(RequestSubPiecePacketOld const & packet)
        {
            transaction_id_ = packet.transaction_id_;
            resource_id_ = packet.resource_id_;
            peer_guid_ = packet.peer_guid_;
            subpiece_infos_ = packet.subpiece_infos_;
            end_point = packet.end_point;
            // PacketAction = Action;
        }

        RequestSubPiecePacketOld(
            boost::uint32_t  transaction_id,
            RID rid,
            Guid peer_guid,
            std::vector<SubPieceInfo> sub_piece_info,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            subpiece_infos_ = sub_piece_info;
            end_point = endpoint_;
            // PacketAction = Action;
        }

        RequestSubPiecePacketOld(
            boost::uint32_t  transaction_id,
            RID rid,
            Guid peer_guid,
            SubPieceInfo const & sub_piece_info,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            subpiece_infos_.push_back(sub_piece_info);
            end_point = endpoint_;
            // PacketAction = Action;
        }

        virtual boost::uint32_t length() const
        {
            return PeerPacket::length() + sizeof(subpiece_infos_.size()) + subpiece_infos_.size() * sizeof(SubPieceInfo);
        }

        std::vector<SubPieceInfo> subpiece_infos_;
    };

    /**
    *@brief  RequestSubPiecePacket 包, 请求报文中不包含GUID。直接继承PacketT
    */
    struct RequestSubPiecePacket
        : PacketT<0x5B>
        , Packet
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            Packet::serialize(ar);
            ar & sequece_id_;
            ar & reserve_;
            ar & resource_id_;
            ar & util::serialization::make_sized<boost::uint16_t>(subpiece_infos_);
            if (Archive::is_saving::value)
            {
                if (priority_ != INVALID_PRIORITY)
                {
                    // priority == INVALID_PRIORITY表示对方peer不支持新协议
                    // 应用层构造报文时使用INVALID_PRIORITY
                    ar & priority_;
                }
            }
            else
            {
                if (sequece_id_ >= PEER_VERSION_V4)
                {
                    // 新协议直接序列化得到priority_
                    ar & priority_;
                }
                else
                {
                    // 老协议没有带priority, 使用默认优先级50
                    priority_ = DEFAULT_PRIORITY;
                }
            }
        }

        RequestSubPiecePacket()
        {
            sequece_id_ = PEER_VERSION;
            reserve_ = 0;
        }

        RequestSubPiecePacket(RequestSubPiecePacket const & packet)
        {
            transaction_id_ = packet.transaction_id_;
            resource_id_ = packet.resource_id_;
            subpiece_infos_ = packet.subpiece_infos_;
            sequece_id_ = PEER_VERSION;
            end_point = packet.end_point;
            priority_ = packet.priority_;
            reserve_ = packet.reserve_;
            // PacketAction = Action;
        }

        RequestSubPiecePacket(
            boost::uint32_t  transaction_id,
            RID rid,
            std::vector<SubPieceInfo> sub_piece_info,
            boost::asio::ip::udp::endpoint endpoint_,
            boost::uint16_t priority,
            boost::uint16_t reserve = 0)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            subpiece_infos_ = sub_piece_info;
            sequece_id_ = PEER_VERSION;
            end_point = endpoint_;
            priority_ = priority;
            reserve_ = reserve;
            // PacketAction = Action;
        }

        RequestSubPiecePacket(
            boost::uint32_t  transaction_id,
            RID rid,
            SubPieceInfo const & sub_piece_info,
            boost::asio::ip::udp::endpoint endpoint_,
            boost::uint16_t priority,
            boost::uint16_t reserve = 0)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            subpiece_infos_.push_back(sub_piece_info);
            sequece_id_ = PEER_VERSION;
            end_point = endpoint_;
            priority_ = priority;
            reserve_ = reserve;
            // PacketAction = Action;
        }

        virtual boost::uint32_t length() const
        {
            return Packet::length() + sizeof(sequece_id_)  +sizeof(reserve_)  + sizeof(resource_id_) + sizeof(subpiece_infos_.size()) + subpiece_infos_.size() * sizeof(SubPieceInfo);
        }

        boost::uint16_t sequece_id_;
        boost::uint16_t reserve_;
        RID    resource_id_;
        std::vector<SubPieceInfo> subpiece_infos_;
        boost::uint16_t priority_;
        static const boost::uint16_t INVALID_PRIORITY = 0xFFFF;
        static const boost::uint16_t PUSH_PRIORITY = 100;
        static const boost::uint16_t DEFAULT_PRIORITY = 50;
        static const boost::uint16_t PRIORITY_10 = 10;
        static const boost::uint16_t PRIORITY_20 = 20;
        static const boost::uint16_t PRIORITY_30 = 30;
        static const boost::uint16_t PRIORITY_40 = 40;
    };

    /**
    *@brief  SubPiece 包
    */
    struct SubPiecePacket
        : PeerPacketT<0x56>
    {
        SubPiecePacket()
        {
            sub_piece_content_.reset(new SubPieceContent);
            assert(sub_piece_content_);
        }

        SubPiecePacket(
            boost::uint32_t transaction_id,
            RID rid,
            Guid peer_guid,
            SubPieceInfo sub_piece_info,
            boost::uint16_t sub_piece_length,
            SubPieceBuffer sub_piece_content,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            sub_piece_info_ = sub_piece_info;
            sub_piece_length_ = sub_piece_length;
            sub_piece_content_ = sub_piece_content.GetSubPieceBuffer();
            end_point = endpoint_;
            // PacketAction = Action;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            if (!sub_piece_content_ || !*sub_piece_content_) {
                ar.fail();
                return;
            }
            PeerPacket::serialize(ar);
            ar & sub_piece_info_;
            ar & sub_piece_length_;
            protocol::UdpBuffer & buffer = static_cast<protocol::UdpBuffer &>(
                ar.streambuf());
            buffer.swap_sub_piece_buffer(sub_piece_content_);
            if (Archive::is_saving::value) {
                buffer.commit(sub_piece_length_);
            } else {
                buffer.consume(sub_piece_length_);
            }
        }

        virtual boost::uint32_t length() const
        {
            return PeerPacket::length() + sizeof(sub_piece_info_) + sizeof(sub_piece_length_) + SUB_PIECE_SIZE;
        }

        SubPieceInfo sub_piece_info_;
        boost::uint16_t sub_piece_length_;
        // 不序列化，用特殊方式赋值
        mutable SubPieceContent::pointer sub_piece_content_;
    };

    /**
    *@brief  PeerExchange 包
    */
    struct PeerExchangePacket
        : PeerPacketT<0x57>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
            ar & basic_info_;
            ar & util::serialization::make_sized<boost::uint8_t>(peer_info_);
        }

        PeerExchangePacket()
        {
        }

        PeerExchangePacket(
            boost::uint32_t transaction_id,
            RID rid,
            Guid peer_guid,
            boost::uint8_t basic_info,
            std::vector<CandidatePeerInfo> & candidate_peer_info,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            basic_info_ = basic_info;
            peer_info_.swap(candidate_peer_info);
            end_point = endpoint_;
            // PacketAction = Action;
        }

        virtual boost::uint32_t length() const
        {
            return PeerPacket::length() + sizeof(basic_info_) + peer_info_.size() * sizeof(CandidatePeerInfo);
        }

        inline bool IsRequest() const{ if (basic_info_ % 2 == 0) return true; return false;}

        boost::uint8_t basic_info_;
        std::vector<CandidatePeerInfo> peer_info_;
    };

    /**
    * RIDInfoRequestPacket
    */
    struct RIDInfoRequestPacket
        : PeerPacketT<0x58>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
        }

        RIDInfoRequestPacket(
            boost::uint32_t transaction_id,
            RID rid,
            Guid peer_guid,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            end_point = endpoint_;
            // PacketAction = Action;
        }

        RIDInfoRequestPacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            return PeerPacket::length();
        }
    };

    /**
    * RIDInfoPacket
    */
    struct RIDInfoResponsePacket
        : PeerPacketT<0x59>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
            boost::uint16_t block_count_ = block_md5_.size();
            ar & file_length_;
            ar & block_count_;
            ar & block_size_;
            util::serialization::serialize_collection(ar, block_md5_, block_count_);
            boost::uint16_t peer_version = sequece_id_;
            if (peer_version >= 0x000A) {
                ar & peer_count_info_;
            }
        }

        RIDInfoResponsePacket(boost::uint32_t transaction_id,  Guid peer_guid,
            const RidInfo& rid_info, const PEER_COUNT_INFO& peer_count_info,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_guid_ = peer_guid;
            file_length_ = rid_info.GetFileLength();
            resource_id_ = rid_info.GetRID();
            block_size_ = rid_info.GetBlockSize();
            block_md5_ = rid_info.block_md5_s_;
            peer_count_info_ = peer_count_info;
            end_point = endpoint_;
            // PacketAction = Action;
        }

        RIDInfoResponsePacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t len = PeerPacket::length() + sizeof(file_length_) + sizeof(block_size_) + sizeof(block_md5_.size())
                + block_md5_.size() * sizeof(MD5);
            boost::uint16_t peer_version = sequece_id_;
            if (peer_version >= 0x000A)
            {
                len += sizeof(peer_count_info_);
            }
            return len;
        }

        boost::uint32_t file_length_;
        boost::uint32_t block_size_;
        std::vector<MD5> block_md5_;
        PEER_COUNT_INFO peer_count_info_;
    };

    /**
    * ReportSpeedPacket
    */
    struct ReportSpeedPacket
        : PeerPacketT<0x5A>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            PeerPacket::serialize(ar);
            ar & speed_;
        }

        ReportSpeedPacket(boost::uint32_t transaction_id, RID rid, Guid peer_guid,
            boost::uint32_t speed, boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            resource_id_ = rid;
            peer_guid_ = peer_guid;
            speed_ = speed;
            end_point = endpoint_;
            // PacketAction = Action;
        }

        ReportSpeedPacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            return PeerPacket::length() + sizeof(speed_);
        }

        boost::uint32_t speed_;
    };

    template <typename PacketHandler>
    void register_peer_packet(
        PacketHandler & handler)
    {
        handler.template register_packet<ErrorPacket>();
        handler.template register_packet<ConnectPacket>();
        handler.template register_packet<RequestAnnouncePacket>();
        handler.template register_packet<AnnouncePacket>();
        handler.template register_packet<RequestSubPiecePacket>();
        handler.template register_packet<RequestSubPiecePacketOld>();
        handler.template register_packet<SubPiecePacket>();
        handler.template register_packet<PeerExchangePacket>();
        handler.template register_packet<RIDInfoRequestPacket>();
        handler.template register_packet<RIDInfoResponsePacket>();
        handler.template register_packet<ReportSpeedPacket>();
    }

}

#endif  // _PROTOCOL_PEER_PACKET_H_
