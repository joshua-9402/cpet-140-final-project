//
// Created by Josh Literal on 11/11/25.
//

#ifndef CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H
#define CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H

#include <string>

class cryptography {
    public:
        static std::string encryptData(const std::string& input);
        static std::string decryptData(const std::string& input);
        static std::string hashData(const std::string& input);
};


#endif //CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H