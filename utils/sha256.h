// //////////////////////////////////////////////////////////
// sha256.h
// Copyright (c) 2014,2015 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#pragma once

#include "staticstring.hpp"

#include <string>

// define fixed size integer types
#ifdef _MSC_VER
// Windows
typedef unsigned __int8  uint8_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
// GCC
#include <stdint.h>
#endif


/// compute SHA256 hash
/** Usage:
    SHA256 sha256;
    std::string myHash  = sha256("Hello World");     // std::string
    std::string myHash2 = sha256("How are you", 11); // arbitrary data, 11 bytes

    // or in a streaming fashion:

    SHA256 sha256;
    while (more data available)
      sha256.add(pointer to fresh data, number of new bytes);
    std::string myHash3 = sha256.getHash();
  */
class SHA256 //: public Hash
{
public:
  /// split into 64 byte blocks (=> 512 bits), hash is 32 bytes long
  enum { BlockSize = 512 / 8, HashBytes = 32 };

  /// same as reset()
  SHA256();

  /// compute SHA256 of a memory block
  std::string operator()(const void* data, size_t numBytes);
  /// compute SHA256 of a string, excluding final zero
  std::string operator()(const std::string& text);

  /// add arbitrary number of bytes
  void add(const void* data, size_t numBytes);

  /// return latest hash as 64 hex characters
  std::string getHash();
  /// return latest hash as bytes
  void getHash(unsigned char buffer[HashBytes]);

  /// restart
  void reset();

private:
  /// process 64 bytes
  void processBlock(const void* data);
  /// process everything left in the internal buffer
  void processBuffer();

  /// size of processed data in bytes
  uint64_t m_numBytes;
  /// valid bytes in m_buffer
  size_t   m_bufferSize;
  /// bytes not processed yet
  uint8_t  m_buffer[BlockSize];

  enum { HashValues = HashBytes / 4 };
  /// hash, stored as integers
  uint32_t m_hash[HashValues];
};

std::string sha256(const std::string& s);

template<unsigned N>
StaticString<2*SHA256::HashBytes+1> sha256(const StaticString<N>& input)
{    
    static SHA256 sha;    
    sha.reset();
    sha.add( (unsigned char*)input.c_str(), input.length());
    
    static unsigned char digest[SHA256::HashBytes];
    memset(digest,0,SHA256::HashBytes);
    sha.getHash(digest);

    static char str[2*SHA256::HashBytes+1];
    str[2*SHA256::HashBytes] = 0;
    
    for (int i = 0; i < SHA256::HashBytes; i++)
    {
        static const char dec2hex[16+1] = "0123456789abcdef";
        str[i * 2]     = dec2hex[(digest[i] >> 4) & 15];
        str[i * 2 + 1] = dec2hex[ digest[i]       & 15];
    }        
    
    return StaticString<2*SHA256::HashBytes+1>(str);
}