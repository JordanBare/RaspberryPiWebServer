//
// Created by Jordan Bare on 7/12/18.
//

#include <cereal/archives/portable_binary.hpp>
#include <fstream>
#include "CredentialsManager.h"

void CredentialsManager::checkForCredentialsFile() {
    /*
    std::string credFileName = mCredentialsDirectory + "loginInfo.txt";
    std::ifstream credentialsInFile(credFileName);
    if(!credentialsInFile.is_open()){
        std::string userName;
        std::string password;
        std::cout << "Enter the Username: " << std::endl;
        std::cin >> userName;
        std::cout << "Enter the Password: " << std::endl;
        std::cin >> password;
        std::unique_ptr credentials = std::make_unique<Credentials>(userName, password);
        OPENSSL_cleanse(&userName, userName.length());
        OPENSSL_cleanse(&password, password.length());
        std::ofstream credentialsOutFile(credFileName);
        if(credentialsOutFile.is_open()){
            cereal::PortableBinaryOutputArchive outputArchive(credentialsOutFile);
            outputArchive(*credentials);
            credentialsOutFile.close();
        }
    } else {
        credentialsInFile.close();
    }
     */
}

CredentialsManager::CredentialsManager(const std::string &folderRoot):mCredentialsDirectory(folderRoot) {
    //checkForCredentialsFile();
}

bool CredentialsManager::compareCredentials(const std::string &body) {
    std::string usrAttribute = "usr=";
    std::string pwdAttribute = "&pwd=";
    unsigned long usrLoc = body.find(usrAttribute);
    unsigned long pwdLoc = body.find(pwdAttribute);
    unsigned long csrfLoc = body.find("&_csrf=");
    std::string user = body.substr(usrLoc + usrAttribute.length(), pwdLoc - (usrLoc + usrAttribute.length()));
    std::string password = body.substr(pwdLoc + pwdAttribute.length(), csrfLoc - (pwdLoc + pwdAttribute.length()));

    /*
    std::unique_ptr<Credentials> credentials = std::make_unique<Credentials>();
    std::ifstream credentialsFile(mCredentialsDirectory + "loginInfo.txt");
    if(credentialsFile.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(credentialsFile);
        inputArchive(*credentials);
        credentialsFile.close();
    }
    if(!credentials->compareUserName(user)){
        return false;
    }
    else if(!credentials->comparePassword(password)){
        return false;
    }
     */

    return password == "password";
}
