/* This file is part of RetroFE.
 *
 * RetroFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RetroFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RetroFE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Utils.h"
#include "../Database/Configuration.h"
#include "Log.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <dirent.h>
#include <locale>
#include <list>
#include <linux/vt.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/keyboard.h>


/* The following devices are the same on all systems.  */
#define CURRENT_TTY "/dev/tty"
#define DEV_CONSOLE "/dev/console"
/*Linux, normal names */
# define CURRENT_VC "/dev/tty0"


Utils::Utils()
{
}

Utils::~Utils()
{
}

std::string Utils::toLower(std::string str)
{
    for(unsigned int i=0; i < str.length(); ++i)
    {
        std::locale loc;
        str[i] = std::tolower(str[i], loc);
    }

    return str;
}

std::string Utils::uppercaseFirst(std::string str)
{
    if(str.length() > 0)
    {
        std::locale loc;
        str[0] = std::toupper(str[0], loc);
    }

    return str;
}
std::string Utils::filterComments(std::string line)
{
    size_t position;

    // strip out any comments
    if((position = line.find("#")) != std::string::npos)
    {
        line = line.substr(0, position);
    }
    // unix only wants \n. Windows uses \r\n. Strip off the \r for unix.
    line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );
    
    return line;
}

std::string Utils::combinePath(std::list<std::string> &paths)
{
    std::list<std::string>::iterator it = paths.begin();
    std::string path;

    if(it != paths.end())
    {
        path += *it;
        it++;
    }

    while(it != paths.end())
    {
        path += Utils::pathSeparator;
        path += *it;
        it++;
    }

    return path;
}

std::string Utils::combinePath(std::string path1, std::string path2)
{
    std::list<std::string> paths;
    paths.push_back(path1);
    paths.push_back(path2);
    return combinePath(paths);
}

std::string Utils::combinePath(std::string path1, std::string path2, std::string path3)
{
    std::list<std::string> paths;
    paths.push_back(path1);
    paths.push_back(path2);
    paths.push_back(path3);
    return combinePath(paths);
}

std::string Utils::combinePath(std::string path1, std::string path2, std::string path3, std::string path4)
{
    std::list<std::string> paths;
    paths.push_back(path1);
    paths.push_back(path2);
    paths.push_back(path3);
    paths.push_back(path4);
    return combinePath(paths);
}
std::string Utils::combinePath(std::string path1, std::string path2, std::string path3, std::string path4, std::string path5)
{
    std::list<std::string> paths;
    paths.push_back(path1);
    paths.push_back(path2);
    paths.push_back(path3);
    paths.push_back(path4);
    paths.push_back(path5);
    return combinePath(paths);
}


bool Utils::findMatchingFile(std::string prefix, std::vector<std::string> &extensions, std::string &file)
{
    for(unsigned int i = 0; i < extensions.size(); ++i)
    {
        std::string temp = prefix + "." + extensions[i];
        temp = Configuration::convertToAbsolutePath(Configuration::isUserLayout_?Configuration::userPath:Configuration::absolutePath, temp);

        std::ifstream f(temp.c_str());

        if (f.good())
        {
            file = temp;
            return true;
        }
        //printf("In %s, file %s not found\n", __func__, temp.c_str());
    }

    return false;
}


std::string Utils::replace(
    std::string subject,
    const std::string& search,
    const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}


float Utils::convertFloat(std::string content)
{
    float retVal = 0;
    std::stringstream ss;
    ss << content;
    ss >> retVal;

    return retVal;
}

int Utils::convertInt(std::string content)
{
    int retVal = 0;
    std::stringstream ss;
    ss << content;
    ss >> retVal;

    return retVal;
}

void Utils::replaceSlashesWithUnderscores(std::string &content)
{
    std::replace(content.begin(), content.end(), '\\', '_');
    std::replace(content.begin(), content.end(), '/', '_');
}


std::string Utils::getDirectory(std::string filePath)
{

    std::string directory = filePath;

    const size_t last_slash_idx = filePath.rfind(pathSeparator);
    if (std::string::npos != last_slash_idx)
    {
        directory = filePath.substr(0, last_slash_idx);
    }

    return directory;
}

std::string Utils::getParentDirectory(std::string directory)
{
    size_t last_slash_idx = directory.find_last_of(pathSeparator);
    if(directory.length() - 1 == last_slash_idx)
    {
        directory = directory.erase(last_slash_idx, directory.length()-1);
        last_slash_idx = directory.find_last_of(pathSeparator);
    }

    if (std::string::npos != last_slash_idx)
    {
        directory = directory.erase(last_slash_idx, directory.length());
    }

    return directory;
}


std::string Utils::getFileName(std::string filePath)
{

    /** Declared static to be kept in memory even after this function's scope */
    static std::string filename;
    filename = filePath;

    const size_t last_slash_idx = filePath.rfind(pathSeparator);
    if (std::string::npos != last_slash_idx)
    {
        filename = filePath.erase(0, last_slash_idx+1);
    }

    return filename;
}


std::string Utils::removeExtension(std::string filePath)
{

    /** Declared static to be kept in memory even after this function's scope */
    static std::string filename;
    filename = filePath;

    const size_t lastPoint = filename.find_last_of("."); 
    if (std::string::npos != lastPoint)
    {
        filename = filePath.substr(0, lastPoint);
    }

    return filename;
}


std::string Utils::trimEnds(std::string str)
{
    // strip off any initial tabs or spaces
    size_t trimStart = str.find_first_not_of(" \t");

    if(trimStart != std::string::npos)
    {
        size_t trimEnd = str.find_last_not_of(" \t");

        str = str.substr(trimStart, trimEnd - trimStart + 1);
    }

    return str;
}
 
bool Utils::IsPathExist(const std::string &s)
{
    struct stat buffer;
    return (stat (s.c_str(), &buffer) == 0);
}

bool Utils::executeRawPath(const char *shellCmd)
{
    bool retVal = false;

    Logger::write(Logger::ZONE_INFO, "Utils", "Attempting to launch: " + std::string(shellCmd));

    std::string executionString = "exec " + std::string(shellCmd);
    printf("Running: %s\n", executionString.c_str());
    if(system(executionString.c_str()) != 0)
    {
        Logger::write(Logger::ZONE_ERROR, "Utils", "Failed to run: " + std::string(shellCmd));
    }
    else
    {
        retVal = true;
    }

    Logger::write(Logger::ZONE_INFO, "Utils", "Completed");

    return retVal;
}

bool Utils::rootfsWritable()
{
    bool retVal = false;
    Logger::write(Logger::ZONE_DEBUG, "Utils", "Making rootfs writable with " + std::string(SHELL_CMD_ROOTFS_RW));
    retVal = executeRawPath(SHELL_CMD_ROOTFS_RW);
    return retVal;
}

bool Utils::rootfsReadOnly()
{
    bool retVal = false;
    Logger::write(Logger::ZONE_DEBUG, "Utils", "Making rootfs read only with " + std::string(SHELL_CMD_ROOTFS_RO));
    retVal = executeRawPath(SHELL_CMD_ROOTFS_RO);
    return retVal;
}

int Utils::termfix(uint32_t ttyId){
    // Init tty file path
    char ttyFilePath[100];
    sprintf(ttyFilePath, "/dev/tty%d", ttyId);

    // Open tty
    int fd = open(ttyFilePath, O_RDWR, 0);

    // Unlock virtual terminal
    int res = ioctl(fd, VT_UNLOCKSWITCH, 1);
    if(res != 0)
    {
        printf("ioctl VT_UNLOCKSWITCH failed");
        return res;
    }

    // Set Virtual terminal mode text (not graphical)
    ioctl(fd, KDSETMODE, KD_TEXT);
    if(res != 0)
    {
        printf("ioctl KDSETMODE failed");
        return res;
    }

    //printf("Success\n");
    return res;
}

int Utils::open_a_console(const char *fnam)
{
    int fd;

    /* try read-write */
    fd = open(fnam, O_RDWR);

    /* if failed, try read-only */
    if (fd < 0 && errno == EACCES)
        fd = open(fnam, O_RDONLY);

    /* if failed, try write-only */
    if (fd < 0 && errno == EACCES)
        fd = open(fnam, O_WRONLY);

    return fd;
}

/*
 * Get an fd for use with kbd/console ioctls.
 * We try several things because opening /dev/console will fail
 * if someone else used X (which does a chown on /dev/console).
 */
int Utils::get_console_fd_or_die(void)
{
    static const char *const console_names[] = {
        DEV_CONSOLE, CURRENT_VC, CURRENT_TTY
    };

    int fd;

    for (fd = 2; fd >= 0; fd--) {
        int fd4name;
        int choice_fd;
        char arg;

        fd4name = open_a_console(console_names[fd]);
 chk_std:
        choice_fd = (fd4name >= 0 ? fd4name : fd);

        arg = 0;
        if (ioctl(choice_fd, KDGKBTYPE, &arg) == 0)
            return choice_fd;
        if (fd4name >= 0) {
            fd4name = -1;
            goto chk_std;
        }
    }

    printf("can't open console");
    /*return fd; - total failure */
}


int Utils::getVTid(){
    struct vt_stat vtstat;

    vtstat.v_active = 0;
    ioctl(get_console_fd_or_die(), VT_GETSTATE, &vtstat);
    //printf("Active VT: %d\n", vtstat.v_active);

    return vtstat.v_active;
}
