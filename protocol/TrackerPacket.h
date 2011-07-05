//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------


/**
* @file
* @brief Peer和IndexServer相关协议包相关的基类
*/

#ifndef _PROTOCOL_TRACKET_PACKET_H_
#define _PROTOCOL_TRACKET_PACKET_H_

#include "protocol/ServerPacket.h"

#include <util/serialization/stl/vector.h>

namespace protocol
{
    /**
    *@brief Peer发向TrackerServer的 List 包和TrackerServer回给Peer的 List 包
    */
    struct ListPacket
        : public ServerPacketT<0x31>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {

            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & request.resource_id_;
                ar & request.peer_guid_;
                ar & request.request_peer_count_;
            } else{
                ar & response.resource_id_;
                ar & util::serialization::make_sized<boost::uint16_t>(response.peer_infos_);
            }
        }

        ListPacket()
        {
            // IsRequest = 1;
        }

        // request
        ListPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            RID resource_id,
            Guid peer_guid,
            boost::uint16_t request_peer_count,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.resource_id_ = resource_id;
            request.peer_guid_ = peer_guid;
            request.request_peer_count_ = request_peer_count;
            end_point = endpoint_;
            IsRequest = 1;

        }

        // response
        ListPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            RID resource_id,
            std::vector<CandidatePeerInfo> candidate_peer_info,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;
            response.peer_infos_ = candidate_peer_info;
            end_point = endpoint_;
            IsRequest = 0;
        }

        struct Request {
            RID resource_id_;
            Guid peer_guid_;
            boost::uint16_t request_peer_count_;
        } request;
        struct Response {
            RID resource_id_;
            std::vector<CandidatePeerInfo> peer_infos_;
        } response;
    };

    /**
    *@brief Peer发向TrackerServer的 Commit 包和TrackerServer回给Peer的 Commit 包
    */
    struct CommitPacket
        : public ServerPacketT<0x32>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & request.peer_guid_;
                ar & request.udp_port_;
                ar & request.tcp_port_;
                ar & request.stun_peer_ip_;
                ar & request.stun_peer_udp_port_;
                ar & util::serialization::make_sized<boost::uint8_t>(request.real_ips_);
                ar & util::serialization::make_sized<boost::uint8_t>(request.rids_);
                if (PEER_VERSION >= 0x00000002) {
                    ar & request.stun_detect_udp_port_;
                }
            } else {
                ar & response.keep_alive_interval_;
                ar & response.detected_ip_;
                ar & response.detected_udp_port_;
            }
        }
        CommitPacket()
        {
            // IsRequest = 0;
        }

        // request
        CommitPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            Guid peer_guid,
            boost::uint16_t udp_port,
            boost::uint16_t tcp_port,
            boost::uint32_t stun_peer_ip,
            boost::uint16_t stun_peer_udpport,
            std::vector<boost::uint32_t> real_ips,
            std::vector<RID> resource_ids,
            boost::uint16_t stun_detected_udp_port,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.peer_guid_ = peer_guid;
            request.udp_port_ = udp_port;
            request.tcp_port_ = tcp_port;
            request.stun_peer_ip_ = stun_peer_ip;
            request.stun_peer_udp_port_ = stun_peer_udpport;
            request.real_ips_ = real_ips;
            request.rids_ = resource_ids;
            request.stun_detect_udp_port_ = stun_detected_udp_port;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        CommitPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            boost::uint16_t keepalive_interval,
            boost::uint32_t detected_ip,
            boost::uint16_t detected_udp_port,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.keep_alive_interval_ = keepalive_interval;
            response.detected_ip_ = detected_ip;
            response.detected_udp_port_ = detected_udp_port;
            end_point = endpoint_;
            IsRequest = 0;
        }

        struct Request {
            Guid peer_guid_;
            boost::uint16_t udp_port_;
            boost::uint16_t tcp_port_;
            boost::uint32_t stun_peer_ip_;
            boost::uint16_t stun_peer_udp_port_;
            // boost::uint8_t real_ip_count_;
            std::vector<boost::uint32_t> real_ips_;
            // boost::uint8_t rid_count_;
            std::vector<RID> rids_;
            boost::uint16_t stun_detect_udp_port_;
        } request;
        struct Response {
            boost::uint16_t keep_alive_interval_;
            boost::uint32_t detected_ip_;
            boost::uint16_t detected_udp_port_;
        } response;

    };

    /**
    *@brief Peer发向TrackerServer的 KeepAlive 包和TrackerServer回给Peer的 KeepAlive 包
    */
    struct KeepAlivePacket
        : public ServerPacketT<0x33>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest){
                ar & request.peer_guid_;
                ar & request.stun_peer_ip_;
                ar & request.stun_peer_udp_port_;
                ar & request.stun_detect_udp_port_;
            }else{
                ar & response.keep_alive_interval_;
                ar & response.detected_ip_;
                ar & response.detected_udp_port_;
                ar & response.resource_count_;
            }
        }

        KeepAlivePacket()
        {
            // IsRequest = 0;
        }

        // request
        KeepAlivePacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            Guid peer_guid,
            boost::uint32_t stun_peer_ip,
            boost::uint16_t stun_peer_udpport,
            boost::uint16_t stun_detected_udp_port,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.peer_guid_ = peer_guid;
            request.stun_peer_ip_ = stun_peer_ip;
            request.stun_peer_udp_port_ = stun_peer_udpport;
            request.stun_detect_udp_port_ = stun_detected_udp_port;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        KeepAlivePacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            boost::uint16_t keepalive_interval,
            boost::uint32_t detected_ip,
            boost::uint16_t detected_udp_port,
            boost::uint16_t resource_count,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.keep_alive_interval_ = keepalive_interval;
            response.detected_ip_ = detected_ip;
            response.detected_udp_port_ = detected_udp_port;
            response.resource_count_ = resource_count;
            end_point = endpoint_;
            IsRequest = 0;
        }

    public :
        struct Request {
            Guid peer_guid_;
            boost::uint32_t stun_peer_ip_;
            boost::uint16_t stun_peer_udp_port_;
            boost::uint16_t stun_detect_udp_port_;
        } request;
        struct Response {
            boost::uint16_t keep_alive_interval_;
            boost::uint32_t detected_ip_;
            boost::uint16_t detected_udp_port_;
            boost::uint16_t resource_count_;
        } response;
    };

    /**
    *@brief Peer发向TrackerServer的 Leave 包
    */
    struct LeavePacket
        : public ServerPacketT<0x34>
    {
        LeavePacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            Guid peer_guid,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            peer_guid_ = peer_guid;
            end_point = endpoint_;
            IsRequest = 1;
        }

        LeavePacket()
        {
            IsRequest = 1;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            ar & peer_guid_;
        }
        Guid peer_guid_;
    };

    /**
    *@brief Peer发向TrackerServer的 Report 包和TrackerServer回给Peer的 KeepAlive 包
    */
    struct ReportPacket
        : public ServerPacketT<0x35>
    {
        ReportPacket()
        {
        }

        // request
        ReportPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            Guid peer_guid,
            boost::uint16_t local_resource_count,
            boost::uint16_t server_resource_count,
            boost::uint16_t udp_port,
            boost::uint16_t peer_version1,
            boost::uint32_t stun_peer_ip,
            boost::uint16_t stun_peer_udpport,
            boost::uint16_t stun_detected_udp_port,
            std::vector<boost::uint32_t> real_ips,
            std::vector<REPORT_RESOURCE_STRUCT> resource_ids,
            boost::uint8_t nat_type,
            boost::uint8_t upload_priority,
            boost::uint8_t idle_time_in_mins,
            boost::int32_t upload_bandwidth_kbs,
            boost::int32_t upload_limit_kbs,
            boost::int32_t upload_speed_kbs,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.peer_guid_ = peer_guid;
            request.local_resource_count_ = local_resource_count;
            request.server_resource_count_ = server_resource_count;
            request.udp_port_ = udp_port;
            request.peer_version1_ = peer_version1;
            request.stun_peer_ip_ = stun_peer_ip;
            request.stun_peer_udp_port_ = stun_peer_udpport;
            request.stun_detected_udp_port_ = stun_detected_udp_port;
            request.real_ips_ = real_ips;
            request.resource_ids_ = resource_ids;
            request.peer_nat_type_ = nat_type;
            request.upload_priority_ = upload_priority;
            request.idle_time_in_mins_ = idle_time_in_mins;
            request.upload_bandwidth_kbs_ = upload_bandwidth_kbs;
            request.upload_limit_kbs_ = upload_limit_kbs;
            request.upload_speed_kbs_ = upload_speed_kbs;
            end_point = endpoint_;
            IsRequest = 1;
        }
        // response
        ReportPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            boost::uint16_t keepalive_interval,
            boost::uint32_t detected_ip,
            boost::uint16_t detected_udp_port,
            boost::uint16_t resource_count,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.keep_alive_interval_ = keepalive_interval;
            response.detected_ip_ = detected_ip;
            response.detected_udp_port_ = detected_udp_port;
            response.resource_count_ = resource_count;
            end_point = endpoint_;
            IsRequest = 0;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest){
                ar & request.peer_guid_;
                ar & request.local_resource_count_;
                ar & request.server_resource_count_;
                ar & request.peer_nat_type_;
                ar & request.upload_priority_;
                ar & request.idle_time_in_mins_;
                ar & framework::container::make_array(request.reversed_, 3);
                ar & request.udp_port_;
                ar & request.peer_version1_;
                ar & request.stun_peer_ip_;
                ar & request.stun_peer_udp_port_;
                ar & request.stun_detected_udp_port_;

                if (Archive::is_loading::value && peer_version_ < PEER_VERSION_V6)
                {
                    // 旧版本协议中没有这几个字段，反序列化的时候设置为-1
                    request.upload_bandwidth_kbs_ = -1;
                    request.upload_limit_kbs_ = -1;
                    request.upload_speed_kbs_ = -1;
                }
                else
                {
                    ar & request.upload_bandwidth_kbs_;
                    ar & request.upload_limit_kbs_;
                    ar & request.upload_speed_kbs_;
                }

                ar & util::serialization::make_sized<boost::uint8_t>(request.real_ips_);
                ar & util::serialization::make_sized<boost::uint8_t>(request.resource_ids_);
            }else{
                ar & response.keep_alive_interval_;
                ar & response.detected_ip_;
                ar & response.detected_udp_port_;
                ar & response.resource_count_;
            }
        }

        struct Request {
            Guid peer_guid_;
            boost::uint16_t local_resource_count_;
            boost::uint16_t server_resource_count_;
            boost::uint8_t  peer_nat_type_;
            boost::uint8_t  upload_priority_;
            boost::uint8_t  idle_time_in_mins_;
            boost::uint8_t  reversed_[3];
            boost::uint16_t udp_port_;
            // peer的版本号，ServerPacket的公共头中已经有peer_version_字段，所以这里命名为peer_version1_
            boost::uint16_t peer_version1_;
            boost::uint32_t stun_peer_ip_;
            boost::uint16_t stun_peer_udp_port_;
            boost::uint16_t stun_detected_udp_port_;
            boost::int32_t upload_bandwidth_kbs_; // 上传带宽
            boost::int32_t upload_limit_kbs_;     // 当前上传限速
            boost::int32_t upload_speed_kbs_;     // 当前上传速度
            std::vector<boost::uint32_t> real_ips_;
            std::vector<REPORT_RESOURCE_STRUCT> resource_ids_;

        } request;
        struct Response {
            boost::uint16_t keep_alive_interval_;
            boost::uint32_t detected_ip_;
            boost::uint16_t detected_udp_port_;
            boost::uint16_t resource_count_;
        } response;
    };

    /**
    * @brief Peer向TrackerServer查询资源拥有的PeerCount 和 Tracker向Peer发送查询结果
    */
    struct QueryPeerCountPacket
        : public ServerPacketT<0x36>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                ar & request.resource_id_;
            }
            else{
                ar & response.resource_id_;
                ar & response.peer_count_;
            }
        }

        QueryPeerCountPacket()
        {
            // IsRequest = 0;
        }

        // request
        QueryPeerCountPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            Guid resource_id,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.resource_id_ = resource_id;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        QueryPeerCountPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            Guid resource_id,
            boost::uint32_t peer_count,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;
            response.peer_count_ = peer_count;
            end_point = endpoint_;
            IsRequest = 0;

        }
        struct Request {
            Guid resource_id_;
        } request;
        struct Response {
            Guid resource_id_;
            boost::uint32_t peer_count_;
        } response;

    };

    struct QueryBatchPeerCountPacket
        : public ServerPacketT<0x37>
    {
        struct RESOURCE_PEER_COUNT_INFO
        {
            RID resource_id_;
            boost::uint32_t peer_count_;

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & resource_id_ & peer_count_;
            }
        };

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & util::serialization::make_sized<boost::uint8_t>(request.resources_);
            } else {
                ar & util::serialization::make_sized<boost::uint8_t>(response.resource_peer_counts_info_);
            }
        }

        QueryBatchPeerCountPacket()
        {
            // IsRequest = 0;
        }

        // request
        QueryBatchPeerCountPacket(
            boost::uint32_t transaction_id,
            const std::vector<RID>& resource_ids,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            request.resources_ = resource_ids;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        QueryBatchPeerCountPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            end_point = endpoint_;
            IsRequest = 0;
        }
        QueryBatchPeerCountPacket(
            boost::uint32_t transaction_id,
            const std::vector<RESOURCE_PEER_COUNT_INFO>& resource_peer_counts,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            response.resource_peer_counts_info_ = resource_peer_counts;
            end_point = endpoint_;
            IsRequest = 0;
        }


        struct Request {
            std::vector<RID> resources_;
        } request;
        struct Response {
            std::vector<RESOURCE_PEER_COUNT_INFO> resource_peer_counts_info_;
        } response;
    };


    template <typename PacketHandler>
    void register_tracker_packet(
        PacketHandler & handler)
    {
        handler.template register_packet<ListPacket>();
        handler.template register_packet<CommitPacket>();
        handler.template register_packet<KeepAlivePacket>();
        handler.template register_packet<LeavePacket>();
        handler.template register_packet<ReportPacket>();
        handler.template register_packet<QueryPeerCountPacket>();
        handler.template register_packet<QueryBatchPeerCountPacket>();
    }

}

#endif  // _PROTOCOL_TRACKET_PACKET_H_