/******************************************************************************
*
* Copyright (c) 2012 PPLive Inc.  All rights reserved
* 
* NatCheckPacket.h
* 
* Description: 和nat类型检测服务器进行交互的命令
* 
* --------------------
* 2012-07-18, kelvinchen create
* --------------------
******************************************************************************/

#ifndef _PROTOCOL_NAT_CHECK_PACKET_H_
#define _PROTOCOL_NAT_CHECK_PACKET_H_

#include "protocol/ServerPacket.h"

namespace protocol
{
    //------请求natcheckserver用同一个ip同一个port返回看到的请求者的endpoint------
    //Request：querytimes，localip，localport
    //Response： querytimes,detectip,detectport,sendserverip,sendserverport

    struct NatCheckSameRoutePacket
        : public ServerPacketT<0x81>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & query_times_;
                ar & request.local_ip_;
                ar & request.local_port_;
                
            } else {      
                ar & query_times_;
                ar & response.detected_ip_;
                ar & response.detect_udp_port_;   
                ar & response.send_nc_ip_;
                ar & response.send_nc_port_;
            }
        }

        NatCheckSameRoutePacket()
        {
            // IsRequest = 0;
        }

        // request
        NatCheckSameRoutePacket(            
            boost::uint16_t query_times,
            boost::uint32_t local_ip,
            boost::uint16_t local_port,
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            const boost::asio::ip::udp::endpoint& endpoint_
            )
        {
            request.local_ip_ = local_ip;
            request.local_port_ = local_port;
            query_times_ = query_times;
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        NatCheckSameRoutePacket(
            boost::uint16_t query_times,
            boost::uint32_t detected_ip,
            boost::uint16_t detected_udp_port,
            boost::uint32_t send_nc_ip,
            boost::uint16_t send_nc_port,
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            query_times_=query_times;
            response.detected_ip_ = detected_ip;
            response.detect_udp_port_ = detected_udp_port;
            response.send_nc_ip_ = send_nc_ip;
            response.send_nc_port_ = send_nc_port;
            end_point = endpoint_;
            IsRequest = 0;
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacketT::length();            
            if (IsRequest) 
            {
                length += sizeof(query_times_) + sizeof(request.local_ip_) + sizeof(request.local_port_);
            } else {      
                length += sizeof(query_times_) + sizeof(response.detected_ip_)+ sizeof(response.detect_udp_port_)
                    +sizeof(response.send_nc_ip_) + sizeof(response.send_nc_port_);
            }
            return length;
        }

         //表示是第几次查询了，可以用来统计重发的次数
        boost::uint16_t query_times_;

        struct Request {
            boost::uint32_t local_ip_;
            boost::uint16_t local_port_;   
            
        } request;
        struct Response {
            boost::uint32_t detected_ip_;
            boost::uint16_t detect_udp_port_;
            boost::uint32_t send_nc_ip_;
            boost::uint16_t send_nc_port_;
        } response;

    };

   //------请求natcheckserver用同一个ip不同的port返回看到的请求者的endpoint------
//    Request：querytimes，localip，localport
//    Response： querytimes,detectip,detectport, sendserverip, sendserverport,receiveserverport


    struct NatCheckDiffPortPacket
        : public ServerPacketT<0x82>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & query_times_;
                ar & request.local_ip_;
                ar & request.local_port_;
                
            } else {      
                ar & query_times_;
                ar & response.detected_ip_;
                ar & response.detect_udp_port_;   
                ar & response.send_nc_ip_;
                ar & response.send_nc_port_;
                ar & response.recv_nc_port_;
            }
        }

        NatCheckDiffPortPacket()
        {
            // IsRequest = 0;
        }

        // request
        NatCheckDiffPortPacket(            
            boost::uint16_t query_times,
            boost::uint32_t local_ip,
            boost::uint16_t local_port,
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            const boost::asio::ip::udp::endpoint& endpoint_
            )
        {
            request.local_ip_ = local_ip;
            request.local_port_ = local_port;
            query_times_ = query_times;
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        NatCheckDiffPortPacket(
            boost::uint16_t query_times,
            boost::uint32_t detected_ip,
            boost::uint16_t detected_udp_port,
            boost::uint32_t send_nc_ip,
            boost::uint16_t send_nc_port,
            boost::uint16_t recv_nc_port,
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            query_times_ = query_times;
            response.detected_ip_ = detected_ip;
            response.detect_udp_port_ = detected_udp_port;
            response.send_nc_ip_ = send_nc_ip;
            response.send_nc_port_ = send_nc_port;
            response.recv_nc_port_ = recv_nc_port;
            end_point = endpoint_;
            IsRequest = 0;
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacketT::length();            
            if (IsRequest) 
            {
                length += sizeof(query_times_) + sizeof(request.local_ip_) + sizeof(request.local_port_);
            } else {      
                length += sizeof(query_times_) + sizeof(response.detected_ip_)+ sizeof(response.detect_udp_port_)
                    +sizeof(response.send_nc_ip_) + sizeof(response.send_nc_port_) + sizeof(response.recv_nc_port_);
            }
            return length;
        }

         //表示是第几次查询了，可以用来统计重发的次数
        boost::uint16_t query_times_;

        struct Request {
            boost::uint32_t local_ip_;
            boost::uint16_t local_port_;   
            
        } request;
        struct Response {
            boost::uint32_t detected_ip_;
            boost::uint16_t detect_udp_port_;
            boost::uint32_t send_nc_ip_;
            boost::uint16_t send_nc_port_;
            boost::uint16_t recv_nc_port_;
        } response;

    };

  
    //------请求natcheckserver用不同的ip，不同的port，返回看到的请求者的endpoint------
    //Request：querytimes，localip，localport
    //Response： querytimes,detectip,detectport, sendserverip, sendserverport,receiveserverip, receiveserverport

    struct NatCheckDiffIpPacket
        : public ServerPacketT<0x83>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & query_times_;
                ar & request.local_ip_;
                ar & request.local_port_;
                
            } else {      
                ar & query_times_;
                ar & response.detected_ip_;
                ar & response.detect_udp_port_;    
                ar & response.send_nc_ip_;
                ar & response.send_nc_port_; 
                ar & response.recv_nc_ip_; 
                ar & response.recv_nc_port_;
            }
        }

        NatCheckDiffIpPacket()
        {
            // IsRequest = 0;
        }

        // request
        NatCheckDiffIpPacket(            
            boost::uint16_t query_times,
            boost::uint32_t local_ip,
            boost::uint16_t local_port,
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            const boost::asio::ip::udp::endpoint& endpoint_
            )
        {
            request.local_ip_ = local_ip;
            request.local_port_ = local_port;
            query_times_ = query_times;
            transaction_id_ = transaction_id;
            peer_version_ = peer_version;
            end_point = endpoint_;
            IsRequest = 1;
        }

        // response
        NatCheckDiffIpPacket(
            boost::uint16_t query_times,
            boost::uint32_t detected_ip,
            boost::uint16_t detected_udp_port,
            boost::uint32_t send_nc_ip,
            boost::uint16_t send_nc_port,
            boost::uint16_t recv_nc_ip,
            boost::uint16_t recv_nc_port,
            boost::uint32_t transaction_id,
            boost::uint8_t error_code,
            const boost::asio::ip::udp::endpoint& endpoint_)
        {
            transaction_id_ = transaction_id;
            error_code_ = error_code;
            query_times_=query_times;
            response.detected_ip_ = detected_ip;
            response.detect_udp_port_ = detected_udp_port;
            response.send_nc_ip_ = send_nc_ip;
            response.send_nc_port_ = send_nc_port;
            response.recv_nc_ip_ = recv_nc_ip;
            response.recv_nc_port_ = recv_nc_port;
            end_point = endpoint_;
            IsRequest = 0;
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacketT::length();            
            if (IsRequest) 
            {
                length += sizeof(query_times_) + sizeof(request.local_ip_) + sizeof(request.local_port_);
            } else {      
                length += sizeof(query_times_) + sizeof(response.detected_ip_)+ sizeof(response.detect_udp_port_)
                    +sizeof(response.send_nc_ip_) + sizeof(response.send_nc_port_)
                    +sizeof(response.recv_nc_port_) + sizeof(response.recv_nc_ip_);
            }
            return length;
        }

         //表示是第几次查询了，可以用来统计重发的次数
        boost::uint16_t query_times_;

        struct Request {
            boost::uint32_t local_ip_;
            boost::uint16_t local_port_;   
            
        } request;
        struct Response {
            boost::uint32_t detected_ip_;
            boost::uint16_t detect_udp_port_;
            boost::uint32_t send_nc_ip_;
            boost::uint16_t send_nc_port_;
            boost::uint32_t recv_nc_ip_;
            boost::uint16_t recv_nc_port_;
        } response;

    };


    template <typename PacketHandler>
    void register_natcheck_packet(
        PacketHandler & handler)
    {
        handler.template register_packet<NatCheckSameRoutePacket>();
        handler.template register_packet<NatCheckDiffPortPacket>();
        handler.template register_packet<NatCheckDiffIpPacket>();
    }

}

#endif  // _PROTOCOL_NAT_CHECK_PACKET_H_
