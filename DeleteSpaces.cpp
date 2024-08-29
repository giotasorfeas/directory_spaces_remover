#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <unistd.h>

bool isDirectory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

std::string processName(const std::string& name) {
    std::string newName;
    bool afterSpecialChar = false;

    for (size_t i = 0; i < name.length(); ++i) {
        if (name[i] == ' ') {
            if (afterSpecialChar) {
                continue;
            } else {
                newName += '_';
            }
        } else {
            newName += name[i];
        }
        afterSpecialChar = (name[i] == '.' || name[i] == ')');
    }

    return newName;
}

void iterateAndProcess(const std::string& directoryPath, std::ofstream& logFile) {
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir) {
        std::cerr << "Error: Unable to open directory " << directoryPath << std::endl;
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == "..") {
            continue;
        }

        std::string fullPath = directoryPath + "/" + entryName;
        std::string newName = processName(entryName);

        if (newName != entryName) {
            std::string newFullPath = directoryPath + "/" + newName;
            if (rename(fullPath.c_str(), newFullPath.c_str()) != 0) {
                std::cerr << "Error: Unable to rename " << fullPath << " to " << newFullPath << std::endl;
                perror("rename");
            } else {
                logFile << "Renamed: " << fullPath << " -> " << newFullPath << std::endl;
            }
        }

        if (isDirectory(fullPath)) {
            iterateAndProcess(fullPath, logFile);
        }
    }
    closedir(dir);
}

int main() {
    std::string directoryPath;
    std::cout << "Enter the directory path: ";
    std::getline(std::cin, directoryPath);

    if (isDirectory(directoryPath)) {
        std::ofstream logFile("changes.txt");
        if (!logFile) {
            std::cerr << "Error: Unable to open changes.txt for writing." << std::endl;
            return 1;
        }
        iterateAndProcess(directoryPath, logFile);
        logFile.close();
    } else {
        std::cerr << "Invalid directory path: " << directoryPath << std::endl;
    }

    return 0;
}