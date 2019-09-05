#ifndef LLARP_PATH_IHOPHANDLER_HPP
#define LLARP_PATH_IHOPHANDLER_HPP

#include <crypto/types.hpp>
#include <util/types.hpp>
#include <crypto/encrypted_frame.hpp>
#include <messages/relay.hpp>
#include <vector>

#include <memory>

struct llarp_buffer_t;

namespace llarp
{
  struct AbstractRouter;

  namespace routing
  {
    struct IMessage;
  }

  namespace path
  {
    struct IHopHandler
    {
      using TrafficEvent_t = std::pair< std::vector< byte_t >, TunnelNonce >;
      using TrafficQueue_t = std::vector< TrafficEvent_t >;

      virtual ~IHopHandler() = default;

      virtual bool
      Expired(llarp_time_t now) const = 0;

      virtual bool
      ExpiresSoon(llarp_time_t now, llarp_time_t dlt) const = 0;

      /// send routing message and increment sequence number
      virtual bool
      SendRoutingMessage(const routing::IMessage& msg, AbstractRouter* r) = 0;

      // handle data in upstream direction
      bool
      HandleUpstream(const llarp_buffer_t& X, const TunnelNonce& Y,
                     AbstractRouter*)
      {
        m_UpstreamQueue.emplace_back();
        auto& pkt = m_UpstreamQueue.back();
        pkt.first.resize(X.sz);
        std::copy_n(X.base, X.sz, pkt.first.begin());
        pkt.second = Y;
        return true;
      }

      // handle data in downstream direction
      bool
      HandleDownstream(const llarp_buffer_t& X, const TunnelNonce& Y,
                       AbstractRouter*)
      {
        m_DownstreamQueue.emplace_back();
        auto& pkt = m_DownstreamQueue.back();
        pkt.first.resize(X.sz);
        std::copy_n(X.base, X.sz, pkt.first.begin());
        pkt.second = Y;
        return true;
      }

      /// return timestamp last remote activity happened at
      virtual llarp_time_t
      LastRemoteActivityAt() const = 0;

      virtual bool
      HandleLRSM(uint64_t status, std::array< EncryptedFrame, 8 >& frames,
                 AbstractRouter* r) = 0;

      uint64_t
      NextSeqNo()
      {
        return m_SequenceNum++;
      }
      virtual void
      FlushQueues(AbstractRouter* r) = 0;

     protected:
      uint64_t m_SequenceNum = 0;
      TrafficQueue_t m_UpstreamQueue;
      TrafficQueue_t m_DownstreamQueue;

      virtual void
      UpstreamWork(TrafficQueue_t queue, AbstractRouter* r) = 0;

      virtual void
      DownstreamWork(TrafficQueue_t queue, AbstractRouter* r) = 0;

      virtual void
      HandleAllUpstream(std::vector< RelayUpstreamMessage > msgs,
                        AbstractRouter* r) = 0;
      virtual void
      HandleAllDownstream(std::vector< RelayDownstreamMessage > msgs,
                          AbstractRouter* r) = 0;
    };

    using HopHandler_ptr = std::shared_ptr< IHopHandler >;
  }  // namespace path
}  // namespace llarp
#endif
