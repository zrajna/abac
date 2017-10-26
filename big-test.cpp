
#include "cabac.h"
#include "math.h"
#include "zlib.h"

#define BYTE unsigned char
//#define TEST_SIZE 1048576u // 1M
#define TEST_SIZE 16384u
//#define TEST_SIZE 1024u
//#define TEST_SIZE 256u

int GetMaxCompressedLen( int nLenSrc ) 
{
    int n16kBlocks = (nLenSrc+16383) / 16384; // round up any fraction of a block
    return ( nLenSrc + 6 + (n16kBlocks*5) );
}
int CompressData( const BYTE* abSrc, int nLenSrc, BYTE* abDst, int nLenDst )
{
    z_stream zInfo ={0};
    zInfo.total_in=  zInfo.avail_in=  nLenSrc;
    zInfo.total_out= zInfo.avail_out= nLenDst;
    zInfo.next_in= (BYTE*)abSrc;
    zInfo.next_out= abDst;

    int nErr, nRet= -1;
    nErr= deflateInit( &zInfo, Z_DEFAULT_COMPRESSION ); // zlib function
    if ( nErr == Z_OK ) {
        nErr= deflate( &zInfo, Z_FINISH );              // zlib function
        if ( nErr == Z_STREAM_END ) {
            nRet= zInfo.total_out;
        }
    }
    deflateEnd( &zInfo );    // zlib function
    return( nRet );
}
int UncompressData( const BYTE* abSrc, int nLenSrc, BYTE* abDst, int nLenDst )
{
    z_stream zInfo ={0};
    zInfo.total_in=  zInfo.avail_in=  nLenSrc;
    zInfo.total_out= zInfo.avail_out= nLenDst;
    zInfo.next_in= (BYTE*)abSrc;
    zInfo.next_out= abDst;

    int nErr, nRet= -1;
    nErr= inflateInit( &zInfo );               // zlib function
    if ( nErr == Z_OK ) {
        nErr= inflate( &zInfo, Z_FINISH );     // zlib function
        if ( nErr == Z_STREAM_END ) {
            nRet= zInfo.total_out;
        }
    }
    inflateEnd( &zInfo );   // zlib function
    return( nRet ); // -1 or len of output
}

evx::uint8 test_kernel(evx::uint32 value)
{
    return evx::uint8(value % 4);
    //return evx::uint8(value % 16);
    //return evx::uint8(value % 64);
}

void test_basic_gzip_rt()
{
    evx_msg("GZIP:");

    evx::uint32 testSize = TEST_SIZE;
    evx::uint32 testSizeBytes = testSize >> 3;
    
    BYTE *a = (BYTE*) malloc(testSizeBytes);
    BYTE *b = (BYTE*) malloc(GetMaxCompressedLen(testSizeBytes));

    for (evx::uint32 i = 0; i < testSizeBytes; ++i)
        a[i] = test_kernel(i);

    evx_msg("raw size: %i bits", testSize);

    int cLength = CompressData(a, testSizeBytes, b, GetMaxCompressedLen(testSizeBytes));
    evx_msg("encoded size: %i bits", cLength << 3);

    BYTE *c = (BYTE*) malloc(testSizeBytes);
    int fLength = UncompressData(b, cLength, c, testSizeBytes);

    for (int i = 0; i < fLength; ++i)
        if (test_kernel(i) != c[i]) {
            evx_err("Data integrity check failure.");
            return;
        }

    evx_msg("test completed successfully.");
}

void test_basic_cabac_rt()
{
    evx_msg("ABAC:");

    evx::uint32 testSize = TEST_SIZE;
    evx::uint32 testSizeBytes = testSize >> 3;
    evx::entropy_coder coder;
    evx::bitstream a((evx::uint32) testSize);
    evx::bitstream b((evx::uint32) testSize);

    for (evx::uint32 i = 0; i < testSizeBytes; ++i)
        a.write_byte(test_kernel(i));

    evx::uint32 raw_size = a.query_occupancy();
    evx_msg("raw size: %i bits", raw_size);

    if (raw_size != testSize) {
        evx_err("Failed to write data to the source bitstream.");
        return;
    }

    coder.encode(&a, &b);
    evx_msg("encoded size: %i bits", b.query_occupancy());

    evx::bitstream c((evx::uint32) testSize);
    coder.decode(raw_size, &b, &c);

    for (evx::uint32 i = 0; i < c.query_byte_occupancy(); ++i)
        if (test_kernel(i) != c.query_data()[i]) {
            evx_err("Data integrity check failure.");
            return;
        }

    evx_msg("test completed successfully.");
}

int main() 
{
    test_basic_cabac_rt();
    test_basic_gzip_rt();
	return 0;
}
