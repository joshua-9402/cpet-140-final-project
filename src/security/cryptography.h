#ifndef CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H
#define CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H

#include <vector>
#include <string>

namespace cryptography {
    bool checkSodium();
    std::string hashKey(const std::string &key, int hashLen);
    std::vector<unsigned char> generateKey(size_t keyBits);
    std::string toHex(const std::vector<unsigned char>& key);
    std::string saltKey(const std::string& key);
    bool encryptFile(const std::string &filePath, const std::vector<unsigned char> &key);
    bool decryptFile(const std::string &filePath, const std::vector<unsigned char> &key, std::string* errorMsg = nullptr);
};


#endif //CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H