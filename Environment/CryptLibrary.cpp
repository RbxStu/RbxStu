//
// Created by Dottik on 28/4/2024.
//

#include "CryptLibrary.hpp"

#include <Utilities.hpp>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/blowfish.h>
#include <cryptopp/hex.h>
#include <cryptopp/md4.h>
#include <cryptopp/md5.h>
#include <cryptopp/modes.h>
#include <cryptopp/ripemd.h>
#include <cryptopp/sha.h>
#include <cryptopp/sha3.h>
#include <lua.h>
#include <lualib.h>

std::unique_ptr<CryptoPP::HashTransformation> to_hash_from_string(const std::string &szHashAlgorithm) {

    if (szHashAlgorithm == "sha1") {
        return std::make_unique<CryptoPP::SHA1>();
    }
    if (szHashAlgorithm == "sha224") {
        return std::make_unique<CryptoPP::SHA224>();
    }
    if (szHashAlgorithm == "sha256") {
        return std::make_unique<CryptoPP::SHA256>();
    }
    if (szHashAlgorithm == "sha384") {
        return std::make_unique<CryptoPP::SHA384>();
    }
    if (szHashAlgorithm == "sha512") {
        return std::make_unique<CryptoPP::SHA512>();
    }
    if (szHashAlgorithm == "sha3-224") {
        return std::make_unique<CryptoPP::SHA3_224>();
    }
    if (szHashAlgorithm == "sha3-256") {
        return std::make_unique<CryptoPP::SHA3_256>();
    }
    if (szHashAlgorithm == "sha3-224") {
        return std::make_unique<CryptoPP::SHA3_224>();
    }
    if (szHashAlgorithm == "sha3-384") {
        return std::make_unique<CryptoPP::SHA3_384>();
    }
    if (szHashAlgorithm == "sha3-512") {
        return std::make_unique<CryptoPP::SHA3_512>();
    }
    if (szHashAlgorithm == "ripemd128") {
        return std::make_unique<CryptoPP::RIPEMD128>();
    }
    if (szHashAlgorithm == "ripemd160") {
        return std::make_unique<CryptoPP::RIPEMD160>();
    }
    if (szHashAlgorithm == "ripemd256") {
        return std::make_unique<CryptoPP::RIPEMD256>();
    }
    if (szHashAlgorithm == "ripemd320") {
        return std::make_unique<CryptoPP::RIPEMD320>();
    }
    return std::make_unique<CryptoPP::SHA256>();
}

std::unique_ptr<CryptoPP::SymmetricCipher> to_cypher_from_string(const std::string &szMode, const bool bIsEncryption) {
    if (bIsEncryption) {
        if (szMode == "aes-cbc" || szMode == "cbc" || szMode == "aes_cbc") {
            return std::make_unique<CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption>();
        }
        if (szMode == "aes-ecb" || szMode == "ecb" || szMode == "aes_ecb") {
            return std::make_unique<CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption>();
        }
        if (szMode == "aes-ctr" || szMode == "ctr" || szMode == "aes_ctr") {
            return std::make_unique<CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption>();
        }
        if (szMode == "aes-cfb" || szMode == "cfb" || szMode == "aes_cfb") {
            return std::make_unique<CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption>();
        }
        if (szMode == "aes-ofb" || szMode == "ofb" || szMode == "aes_ofb") {
            return std::make_unique<CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption>();
        }
        if (szMode == "bf-cbc" || szMode == "blowfish-cbc" || szMode == "blowfish_cbc") {
            return std::make_unique<CryptoPP::CBC_Mode<CryptoPP::Blowfish>::Encryption>();
        }
        if (szMode == "bf-cfb" || szMode == "blowfish-cfb" || szMode == "blowfish_cfb") {
            return std::make_unique<CryptoPP::CFB_Mode<CryptoPP::Blowfish>::Encryption>();
        }
        if (szMode == "bf-ofb" || szMode == "blowfish-ofb" || szMode == "blowfish_ofb") {
            return std::make_unique<CryptoPP::OFB_Mode<CryptoPP::Blowfish>::Encryption>();
        }
    } else {

        if (szMode == "aes-cbc" || szMode == "cbc" || szMode == "aes_cbc") {
            return std::make_unique<CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption>();
        }
        if (szMode == "aes-ecb" || szMode == "ecb" || szMode == "aes_ecb") {
            return std::make_unique<CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption>();
        }
        if (szMode == "aes-ctr" || szMode == "ctr" || szMode == "aes_ctr") {
            return std::make_unique<CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption>();
        }
        if (szMode == "aes-cfb" || szMode == "cfb" || szMode == "aes_cfb") {
            return std::make_unique<CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption>();
        }
        if (szMode == "aes-ofb" || szMode == "ofb" || szMode == "aes_ofb") {
            return std::make_unique<CryptoPP::OFB_Mode<CryptoPP::AES>::Decryption>();
        }
        if (szMode == "bf-cbc" || szMode == "blowfish-cbc" || szMode == "blowfish_cbc") {
            return std::make_unique<CryptoPP::CBC_Mode<CryptoPP::Blowfish>::Decryption>();
        }
        if (szMode == "bf-cfb" || szMode == "blowfish-cfb" || szMode == "blowfish_cfb") {
            return std::make_unique<CryptoPP::CFB_Mode<CryptoPP::Blowfish>::Decryption>();
        }
        if (szMode == "bf-ofb" || szMode == "blowfish-ofb" || szMode == "blowfish_ofb") {
            return std::make_unique<CryptoPP::OFB_Mode<CryptoPP::Blowfish>::Decryption>();
        }
    }

    if (bIsEncryption) // Default to aes-cbc
        return std::make_unique<CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption>();
    else
        return std::make_unique<CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption>();
}

int crypt_base64encode(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);

    CryptoPP::Base64Encoder _base64Encoder{};

    _base64Encoder.Put(reinterpret_cast<const byte *>(s), strlen(s));
    _base64Encoder.MessageEnd();

    std::string final{};
    final.resize(_base64Encoder.MaxRetrievable());

    _base64Encoder.Get(const_cast<byte *>(reinterpret_cast<const byte *>(final.c_str())), final.size());

    lua_pushstring(L, final.c_str());
    return 1;
}

int crypt_base64decode(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);

    CryptoPP::Base64Decoder _base64Decoder{};

    _base64Decoder.Put(reinterpret_cast<const byte *>(s), strlen(s));
    _base64Decoder.MessageEnd();

    std::string final{};
    final.resize(_base64Decoder.MaxRetrievable());

    _base64Decoder.Get(const_cast<byte *>(reinterpret_cast<const byte *>(final.c_str())), final.size());

    lua_pushstring(L, final.c_str());
    return 1;
}

int crypt_encrypt(lua_State *L) {
    const char *plain = luaL_checkstring(L, 1);
    const char *key = luaL_checkstring(L, 2);
    std::string IV = luaL_optstring(L, 3, "NADAAAAAA");
    if (strcmp(IV.c_str(), "NADAAAAAA") == 0)
        IV = Module::Utilities::get_singleton()->get_random_string(16);
    std::string algorithm = luaL_optstring(L, 4, "aes-cbc");


    std::ranges::transform(algorithm, algorithm.begin(), tolower);

    const auto cryptCypher = to_cypher_from_string(algorithm, true);

    cryptCypher->SetKeyWithIV(reinterpret_cast<const CryptoPP::byte *>(key), strlen(key),
                              reinterpret_cast<const CryptoPP::byte *>(IV.c_str()));

    // Decrypt.
    std::string decrypted;
    CryptoPP::StreamTransformationFilter decryptor(*cryptCypher, new CryptoPP::StringSink(decrypted));
    decryptor.Put(reinterpret_cast<const CryptoPP::byte *>(plain), strlen(plain));
    decryptor.MessageEnd();


    std::string encodedData{};
    CryptoPP::StringSource(IV, true, // NOLINT(*-unused-raii)
                           new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encodedData)) // Base64Encoder
    );

    std::string encoded;
    CryptoPP::StringSource(encodedData, true, // NOLINT(*-unused-raii)
                           new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded)));

    lua_pushstring(L, encoded.c_str());
    lua_pushstring(L, IV.c_str());
    return 2;
}

int crypt_decrypt(lua_State *L) {
    const char *encrypted = luaL_checkstring(L, 1);
    const char *key = luaL_checkstring(L, 2);
    const char *IV = luaL_checkstring(L, 3);
    std::string algorithm = luaL_checkstring(L, 4);

    std::ranges::transform(algorithm, algorithm.begin(), tolower);

    const auto cryptCypher = to_cypher_from_string(algorithm, true);


    cryptCypher->SetKeyWithIV(reinterpret_cast<const CryptoPP::byte *>(key), strlen(key),
                              reinterpret_cast<const CryptoPP::byte *>(IV));

    // Decrypt the ciphertext
    std::string decrypted;
    CryptoPP::StreamTransformationFilter decryptor(*cryptCypher, new CryptoPP::StringSink(decrypted));
    decryptor.Put(reinterpret_cast<const CryptoPP::byte *>(encrypted), strlen(encrypted));
    decryptor.MessageEnd();

    lua_pushstring(L, decrypted.c_str());
    return 1;
}

int crypt_hash(lua_State *L) {
    const char *data = luaL_checkstring(L, 1);
    std::string algorithm = luaL_optstring(L, 2, "sha256");

    std::ranges::transform(algorithm, algorithm.begin(), tolower);

    const auto hashAlgorithm = to_hash_from_string(algorithm);

    std::string finalHashed{};
    CryptoPP::StringSource( // NOLINT(*-unused-raii)
            data, true,
            new CryptoPP::HashFilter(*hashAlgorithm,
                                     new CryptoPP::HexEncoder(new CryptoPP::StringSink(finalHashed), false)));

    lua_pushstring(L, finalHashed.c_str());
    return 1;
}

int crypt_generatekey(lua_State *L) {

    auto str = Module::Utilities::get_singleton()->get_random_string(32);
    CryptoPP::Base64Encoder _base64Encoder{};

    _base64Encoder.Put(reinterpret_cast<const byte *>(str.c_str()), str.size());
    _base64Encoder.MessageEnd();

    std::string final{};
    final.resize(_base64Encoder.MaxRetrievable());

    _base64Encoder.Get(const_cast<byte *>(reinterpret_cast<const byte *>(final.c_str())), final.size());

    lua_pushstring(L, final.c_str());
    return 1;
}

void CryptoLibrary::register_environment(lua_State *L) {
    static const luaL_Reg reg[] = {
            {"base64decode", crypt_base64decode},
            {"base64_decode", crypt_base64decode},

            {"base64encode", crypt_base64encode},
            {"base64_encode", crypt_base64encode},

            {"decrypt", crypt_decrypt},
            {"encrypt", crypt_decrypt},

            {"hash", crypt_hash},
            {"generatekey", crypt_generatekey},
            {nullptr, nullptr},
    };

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, reg);
    lua_pop(L, 1);

    lua_newtable(L);
    luaL_register(L, nullptr, reg);
    lua_setreadonly(L, -1, true);
    lua_setfield(L, LUA_GLOBALSINDEX, "crypt");

    lua_pop(L, 1);
}
