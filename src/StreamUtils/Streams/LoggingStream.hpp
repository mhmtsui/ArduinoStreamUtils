// StreamUtils - github.com/bblanchon/ArduinoStreamUtils
// Copyright Benoit Blanchon 2019
// MIT License

#pragma once

#include "../Policies/ReadLoggingPolicy.hpp"
#include "../Policies/WriteLoggingPolicy.hpp"
#include "StreamProxy.hpp"

namespace StreamUtils {

struct LoggingStream : StreamProxy<ReadLoggingPolicy, WriteLoggingPolicy> {
  LoggingStream(Stream& upstream, Stream& log)
      : StreamProxy<ReadLoggingPolicy, WriteLoggingPolicy>(
            upstream, ReadLoggingPolicy{log}, WriteLoggingPolicy{log}) {}
};

}  // namespace StreamUtils