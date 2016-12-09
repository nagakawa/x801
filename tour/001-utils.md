## A Tour of the Experiment801 Codebase

Part 1: `utils.h`

`utils.h` is a header file in the Base module; all of its things reside in the
`x801::base` namespace (see the style guide for more information).

### Endian-Independent Integer Handling

The header provides facilities to read and write integers with C++ streams.
It takes care of endian problems by using `portable_endian.h`, which defines
the `endian.h` functions for different platforms (or imports the header if
it's available). You can use these functions as such:

    uint32_t thing = x801::base::readInt<uint32_t>(stream1);
    x801::base::writeInt<uint32_t>(stream2, thing);

### Constructing `std::string` Objects for Binary Data

There is a (templated) function to create a `std::string` object from an
array of characters. This can be used if there might be terminating nulls
in the middle of the string (using the usual constructor can leave out
everything after the first null character), since the `""s` syntax was added
only in C++14.

### ZLib Wrappers

Since the basic ZLib functions are clunky to work with, there are two wrapper
functions to make it easier. Note that these are a little kludgy since you
have to translate between streams and buffers a lot. I might have to work on
streamlining the wrappers. Anyway, the signatures are:

    int readZipped(
      std::istream& f,    // file handle to read the compressed data from
      char*& block,       // a reference to store the pointer to the buffer
                          // of decompressed data. This must be deallocated
                          // with C's free().
      uint32_t& amtReadC, // a reference that will store the number of
                          // characters read from the compressed data...
      uint32_t& amtReadU  // ... and the number of characters that
                          // decompresses to
    );
    int writeZipped(
      std::ostream& f,      // file stream to write compressed data to
      const char* block,    // pointer to the uncompressed data
      uint32_t len,         // the size of this data, in bytes
      uint32_t& amtWrittenC // a reference that will store the number of
                            // bytes written to the stream.
    );
