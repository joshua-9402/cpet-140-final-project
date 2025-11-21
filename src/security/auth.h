#ifndef CPET_140_FINAL_PROJECT_AUTH_H
#define CPET_140_FINAL_PROJECT_AUTH_H

#include <string>


class auth {
    public:
        static bool testAuth(const std::string& username, const std::string& password);

        static bool testDeployAuth(const std::string &username, const std::string &password);

        static bool adminAuth(const std::string& username, const std::string& password);

        static bool basicAuth(const std::string &username, const std::string &password);
};


#endif //CPET_140_FINAL_PROJECT_AUTH_H