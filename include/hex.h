#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

namespace ot_mini {

// "3a5f9c" gibi bir hex string'i byte dizisine çevirir.
// Başarılıysa true döner, aByteLen'e kaç byte yazıldığını koyar.
// aMaxBytes buffer taşmasını önlemek için üst sınırdır.
bool HexStringToBytes(const std::string &aHex, uint8_t *aBytes, size_t aMaxBytes, size_t &aByteLen);

// Bir byte dizisini "3a5f9c" formatında hex string'e çevirir.
std::string BytesToHexString(const uint8_t *aBytes, size_t aLength);

} // namespace ot_mini
