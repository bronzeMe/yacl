// Copyright 2019 Ant Group Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>

#include "brpc/channel.h"
#include "brpc/server.h"
#include "bthread/bthread.h"
#include "bthread/condition_variable.h"

#include "yacl/link/ssl_options.h"
#include "yacl/link/transport/channel.h"

namespace org::interconnection::link {
class PushRequest;
class PushResponse;
}  // namespace org::interconnection::link

namespace yacl::link::transport {

class InterconnectionLink : public TransportLink {
 public:
  struct Options {
    uint32_t http_timeout_ms = 10 * 1000;          // 10 seconds
    uint32_t http_max_payload_bytes = 512 * 1024;  // 512k bytes
    std::string channel_protocol;
    std::string channel_connection_type;
    uint32_t max_retry = 3;
    uint32_t retry_interval_ms = 1000;
    bool aggressive_retry = true;
  };
  static Options MakeOptions(Options& default_opt, uint32_t http_timeout_ms,
                             uint32_t http_max_payload_size,
                             const std::string& brpc_channel_protocol,
                             const std::string& brpc_channel_connection_type,
                             uint32_t retry_count, uint32_t retry_interval,
                             bool aggressive_retry);

  InterconnectionLink(size_t self_rank, size_t peer_rank, Options options)
      : TransportLink(self_rank, peer_rank), options_(std::move(options)) {}
  void SetMaxBytesPerChunk(size_t bytes) override {
    options_.http_max_payload_bytes = bytes;
  }
  size_t GetMaxBytesPerChunk() override {
    return options_.http_max_payload_bytes;
  }
  std::unique_ptr<::google::protobuf::Message> PackMonoRequest(
      const std::string& key, ByteContainerView value) override;
  std::unique_ptr<::google::protobuf::Message> PackChunkedRequest(
      const std::string& key, ByteContainerView value, size_t offset,
      size_t total_length) override;
  void UnpackMonoRequest(const ::google::protobuf::Message& request,
                         std::string* key, ByteContainerView* value) override;
  void UnpackChunckRequest(const ::google::protobuf::Message& request,
                           std::string* key, ByteContainerView* value,
                           size_t* offset, size_t* total_length) override;
  void FillResponseOk(const ::google::protobuf::Message& request,
                      ::google::protobuf::Message* response) override;
  void FillResponseError(const ::google::protobuf::Message& request,
                         ::google::protobuf::Message* response) override;
  bool IsChunkedRequest(const ::google::protobuf::Message& request) override;
  bool IsMonoRequest(const ::google::protobuf::Message& request) override;

 protected:
  Options options_;
};

}  // namespace yacl::link::transport
