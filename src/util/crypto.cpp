#include <wspp/util/crypto.hpp>

#include <crypto++/osrng.h>
#include <crypto++/hex.h>
#include <crypto++/pwdbased.h>
#include <crypto++/base64.h>

#include <iostream>
#include <iomanip>

#include <arpa/inet.h>

using namespace std ;

namespace wspp {

string randomBytes(size_t len) {
    using namespace CryptoPP ;
    static AutoSeededRandomPool pool ;
    string bytes ;

    bytes.resize(len, 0) ;

    pool.GenerateBlock(reinterpret_cast<byte *>(&bytes[0]), len);

    return bytes ;
}

string binToHex(const string &src) {
    using namespace CryptoPP ;

    string res ;
    HexEncoder hex(new StringSink(res));

    hex.Put(reinterpret_cast<const byte *>(&src[0]), src.size());
    hex.MessageEnd();
    return res ;
}

string encodeBase64(const string &src)
{
    using namespace CryptoPP ;

    string encoded ;

    StringSource ss(reinterpret_cast<const byte *>(&src[0]), src.size(), true,
        new Base64Encoder(
            new StringSink(encoded)
        )
    );

    encoded.pop_back();

    return encoded ;
}

string decodeBase64(const string &src)
{
    using namespace CryptoPP ;

    string decoded ;

    StringSource ss(reinterpret_cast<const byte *>(&src[0]), src.size(), true,
        new Base64Decoder(
            new StringSink(decoded)
        )
    );

    return decoded ;
}


const size_t salt_length = 16 ;

string passwordHash(const string &password, size_t iterations) {

    using namespace CryptoPP;

    string buffer = randomBytes(salt_length) ;

    // buffer contains salt + key + iterations
    buffer.resize(SHA256::DIGESTSIZE + salt_length + 4) ;

    // generate key from hash and password

    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;

    pbkdf.DeriveKey(
        (byte *)buffer.data() + salt_length, SHA256::DIGESTSIZE,
        0x00,
        (byte *) password.data(), password.size(),
        (byte *) buffer.data(), salt_length,
        iterations
    );

    uint32_t *p = (uint32_t *)(buffer.data() + SHA256::DIGESTSIZE + salt_length) ;

    *p = htonl(iterations) ;

    return buffer ;
}

bool passwordVerify(const string &password, const string &hash) {

    using namespace CryptoPP;

    assert( hash.size() == SHA256::DIGESTSIZE + salt_length + 4 ) ;

    uint32_t *p = (uint32_t *)(hash.data() + SHA256::DIGESTSIZE + salt_length) ;

    size_t iterations = ntohl(*p) ;

    // generate key from hash and password

    char key[SHA256::DIGESTSIZE] ;

    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;

    pbkdf.DeriveKey(
        (byte *)key, SHA256::DIGESTSIZE,
        0x00,
        (byte *) password.data(), password.size(),
        (byte *) hash.data(), salt_length,
        iterations
    );

    // compare key with one stored in hash (avoiding timing issues)

    uint ncount = 0 ;
    for( uint i=0 ; i<SHA256::DIGESTSIZE ; i++ )
        if ( hash.at(salt_length + i) != key[i] ) ncount ++ ;

    return ncount == 0 ;
}

string hashSHA256(const string &src)
{
    std::string digest;
    CryptoPP::SHA256 hash;

    CryptoPP::StringSource ss(reinterpret_cast<const byte *>(&src[0]), src.size(), true,
       new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(digest)));

    return digest;
}



}
