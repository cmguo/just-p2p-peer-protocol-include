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
    //查询guid对应的tracker分组的算法
    inline uint32_t GetGuidMod(const Guid& guid, uint32_t mod)
    {
        boost::uint64_t buf[2];
        memcpy(&buf, &guid.data(), sizeof(guid.data()));
        buf[1] = framework::system::BytesOrder::little_endian_to_host_longlong(buf[1]);
        return static_cast<uint32_t> (buf[1] % mod);
    }

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

        ListPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const RID& resource_id,
            const std::vector<CandidatePeerInfo>& candidate_peer_info,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;
            response.peer_infos_=candidate_peer_info;
            end_point = endpoint_;
            IsRequest = 0;
        }

        //这个构造函数需要另外设置peer_infos_
        ListPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const RID& resource_id,            
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;           
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
            boost::uint32_t stun_peer_ip,
            boost::uint16_t stun_peer_udpport,
            boost::uint32_t stun_detected_ip,
            boost::uint16_t stun_detected_udp_port,
            std::vector<boost::uint32_t> real_ips,
            std::vector<REPORT_RESOURCE_STRUCT> resource_ids,
            boost::uint8_t nat_type,
            boost::uint8_t upload_priority,
            boost::uint8_t idle_time_in_mins,
            boost::int32_t upload_bandwidth_kbs,
            boost::int32_t upload_limit_kbs,
            boost::int32_t upload_speed_kbs,
            boost::asio::ip::udp::endpoint endpoint_,
            boost::uint16_t internal_tcp_port = 0,
            boost::uint16_t upnp_tcp_port = 0)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.peer_guid_ = peer_guid;
            request.local_resource_count_ = local_resource_count;
            request.server_resource_count_ = server_resource_count;
            request.udp_port_ = udp_port;
            request.internal_tcp_port_ = internal_tcp_port;
            request.upnp_tcp_port_ = upnp_tcp_port;
            request.stun_peer_ip_ = stun_peer_ip;
            request.stun_peer_udp_port_ = stun_peer_udpport;
            request.stun_detected_ip_ = stun_detected_ip;
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
                ar & request.reversed_;
                ar & request.internal_tcp_port_;
                ar & request.udp_port_;
                ar & request.upnp_tcp_port_;
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

                if (Archive::is_loading::value && peer_version_ < PEER_VERSION_V11)
                {
                    request.stun_detected_ip_ = 0;
                }
                else
                {
                    ar & request.stun_detected_ip_;
                }

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
            boost::uint8_t  reversed_;
            boost::uint16_t  internal_tcp_port_;
            boost::uint16_t udp_port_;
            boost::uint16_t upnp_tcp_port_;
            boost::uint32_t stun_peer_ip_;
            boost::uint16_t stun_peer_udp_port_;
            boost::uint16_t stun_detected_udp_port_;
            boost::int32_t upload_bandwidth_kbs_; // 上传带宽
            boost::int32_t upload_limit_kbs_;     // 当前上传限速
            boost::int32_t upload_speed_kbs_;     // 当前上传速度
            std::vector<boost::uint32_t> real_ips_;
            std::vector<REPORT_RESOURCE_STRUCT> resource_ids_;
            boost::uint32_t stun_detected_ip_;

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

    struct InternalCommandPacket
//        : Packet
//        , PacketT<0x38>
          : public ServerPacketT<0x38>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if(IsRequest)
            {
                ar & magic_number_;
                ar & command_id_;
                ar & reserved_;
            }
        }

        InternalCommandPacket()
        {
            reserved_ = 0;
        }

        InternalCommandPacket(
            boost::uint32_t transaction_id,
            boost::uint16_t magic_number,
            boost::uint16_t command_id,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            magic_number_ = magic_number;
            command_id_ = command_id;
            end_point = endpoint_;
            reserved_ = 0;
            IsRequest = 1;
        }

        InternalCommandPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            end_point = endpoint_;
            IsRequest = 0;
        }

        boost::uint16_t magic_number_;
        boost::uint16_t command_id_;
        boost::uint32_t reserved_;
    };

    //ck添加，查询一个peer拥有的rid，注意，这里不是所有的rid，因为一个peer的不同rid是在不同组的tracker上的。
    struct QueryPeerResourcesPacket
        : public ServerPacketT<0x39>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                ar & request.peer_guid_;
            }
            else{
                ar & response.peer_guid_;
                // ar & response.resources_;
                ar & util::serialization::make_sized<boost::uint8_t>(response.resources_);
            }
        }

        QueryPeerResourcesPacket()
        {
            // IsRequest = 0;
        }

        // request
        QueryPeerResourcesPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            const Guid& peer_guid,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.peer_guid_ = peer_guid;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        QueryPeerResourcesPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const Guid& peer_guid,
            const std::vector<RID>& resources,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.peer_guid_ = peer_guid;
            response.resources_ = resources;
            end_point = endpoint_;
            IsRequest = 0;

        }
        struct Request {
            Guid peer_guid_;
        } request;
        struct Response {
            Guid peer_guid_;
            std::vector<RID> resources_;           
        } response;

    };

    struct QueryTrackerStatisticPacket
        : public ServerPacketT<0x3a>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                //请求没有包体
            }
            else{
                // ar & response.tracker_statistic_;				
                ar & util::serialization::make_sized<boost::uint16_t>(response.tracker_statistic_);
            }
        }

        QueryTrackerStatisticPacket()
        {
            // IsRequest = 0;
        }

        // request
        QueryTrackerStatisticPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        QueryTrackerStatisticPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const std::string& tracker_statistic,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.tracker_statistic_ = tracker_statistic;
            end_point = endpoint_;
            IsRequest = 0;

        }
        struct Request {			
        } request;
        struct Response {
            //用一个string作为回复，简单，易扩展，一了百了。
            std::string tracker_statistic_;
        } response;

    };


    struct ListTcpPacket
        : public ServerPacketT<0x3b>
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

        ListTcpPacket()
        {
            // IsRequest = 1;
        }

        // request
        ListTcpPacket(
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
        ListTcpPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const RID& resource_id,
            const std::vector<CandidatePeerInfo>& candidate_peer_info,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;
            response.peer_infos_ = candidate_peer_info;
            end_point = endpoint_;
            IsRequest = 0;
        }
        ListTcpPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const RID& resource_id,            
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;
            //response.peer_infos_ = candidate_peer_info;
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


    //和list不同的是，这里制定了客户端的ip，而不是从socket里获取
    struct ListWithIpPacket
        : public ServerPacketT<0x3c>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {

            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & request.resource_id_;
                ar & request.peer_guid_;
                ar & request.request_peer_count_;
                ar & request.reqest_ip_;
            } else{
                ar & response.resource_id_;
                ar & util::serialization::make_sized<boost::uint16_t>(response.peer_infos_);
            }
        }

        ListWithIpPacket()
        {
            // IsRequest = 1;
        }

        // request
        ListWithIpPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            RID resource_id,
            Guid peer_guid,
            boost::uint16_t request_peer_count,
            boost::uint32_t reqest_ip,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.resource_id_ = resource_id;
            request.peer_guid_ = peer_guid;
            request.request_peer_count_ = request_peer_count;
            request.reqest_ip_ = reqest_ip;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        ListWithIpPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const RID& resource_id,
            const std::vector<CandidatePeerInfo>& candidate_peer_info,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;
            response.peer_infos_ = candidate_peer_info;
            end_point = endpoint_;
            IsRequest = 0;
        }

        ListWithIpPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const RID& resource_id,            
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;            
            end_point = endpoint_;
            IsRequest = 0;
        }

        struct Request {
            RID resource_id_;
            Guid peer_guid_;
            boost::uint16_t request_peer_count_;
            boost::uint32_t reqest_ip_;
        } request;
        struct Response {
            RID resource_id_;
            std::vector<CandidatePeerInfo> peer_infos_;
        } response;
    };

    //和list不同的是，这里制定了客户端的ip，而不是从socket里获取
    struct ListTcpWithIpPacket
        : public ServerPacketT<0x3d>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {

            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & request.resource_id_;
                ar & request.peer_guid_;
                ar & request.request_peer_count_;
                ar & request.reqest_ip_;
            } else{
                ar & response.resource_id_;
                ar & util::serialization::make_sized<boost::uint16_t>(response.peer_infos_);
            }
        }

        ListTcpWithIpPacket()
        {
            // IsRequest = 1;
        }

        // request
        ListTcpWithIpPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            RID resource_id,
            Guid peer_guid,
            boost::uint16_t request_peer_count,
            boost::uint32_t reqest_ip,
            boost::asio::ip::udp::endpoint endpoint_)
        {
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            request.resource_id_ = resource_id;
            request.peer_guid_ = peer_guid;
            request.request_peer_count_ = request_peer_count;
            request.reqest_ip_ = reqest_ip;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        ListTcpWithIpPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const RID& resource_id,
            const std::vector<CandidatePeerInfo>& candidate_peer_info,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;
            response.peer_infos_ = candidate_peer_info;
            end_point = endpoint_;
            IsRequest = 0;
        }
        ListTcpWithIpPacket(
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const RID& resource_id,           
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            response.resource_id_ = resource_id;
            end_point = endpoint_;
            IsRequest = 0;
        }



        struct Request {
            RID resource_id_;
            Guid peer_guid_;
            boost::uint16_t request_peer_count_;
            boost::uint32_t reqest_ip_;
        } request;
        struct Response {
            RID resource_id_;
            std::vector<CandidatePeerInfo> peer_infos_;
        } response;
    };



    template <typename PacketHandler>
    void register_tracker_packet(
        PacketHandler & handler)
    {
        handler.template register_packet<ListPacket>();

        // CommitPacket内核不会发送，这里没有删除是因为考虑不影响服务器
        handler.template register_packet<CommitPacket>();

        // KeepAlivePacket内核不会发送，这里没有删除是因为考虑不影响服务器
        handler.template register_packet<KeepAlivePacket>();

        handler.template register_packet<LeavePacket>();
        handler.template register_packet<ReportPacket>();

        // QueryPeerCountPacket内核不会发送，这里没有删除是因为考虑不影响服务器
        handler.template register_packet<QueryPeerCountPacket>();

        // QueryBatchPeerCountPacket内核不会发送，这里没有删除是因为考虑不影响服务器
        handler.template register_packet<QueryBatchPeerCountPacket>();

        // QueryBatchPeerCountPacket内核不会发送，加到这里是为了让UdpServer能够处理
        handler.template register_packet<InternalCommandPacket>();

        //查问题使用的命令，内核不会发送
        handler.template register_packet<QueryPeerResourcesPacket>();

        //查询tracker 统计的后台命令
        handler.template register_packet<QueryTrackerStatisticPacket>();

        //查询tcp的端口
        handler.template register_packet<ListTcpPacket>();
        //带客户端ip，查询资源，用于tracker网管
        handler.template register_packet<ListWithIpPacket>();
        //带客户端ip，查询资源，用于tracker代理
        handler.template register_packet<ListTcpWithIpPacket>();
    }

}

#endif  // _PROTOCOL_TRACKET_PACKET_H_
