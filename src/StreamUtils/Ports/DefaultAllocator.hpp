// StreamUtils - github.com/bblanchon/ArduinoStreamUtils
// Copyright Benoit Blanchon 2019-2020
// MIT License

#pragma once

namespace StreamUtils {

#include <stdlib.h>  // malloc, free, size_t
#define USE_STATIC_MEM
#define STATIC_SIZE 256
static char _static_buf[STATIC_SIZE+1];
//#include <stdlib.h>  // malloc, free, size_t

struct DefaultAllocator {
  void* allocate(size_t n) {
#ifdef USE_STATIC_MEM
    if (n > STATIC_SIZE){
      return NULL;
    }else{
      return _static_buf;
    }
#else
    return malloc(n);
#endif
  }

  void deallocate(void* p) {
#ifndef USE_STATIC_MEM    
    free(p);
#endif
  }
};

}  // namespace StreamUtils