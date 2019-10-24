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
#pragma once

#include <string>
#include <vector>
#include <list>
#include <stdint.h>
class Utils
{
public:
    static std::string replace(std::string subject, const std::string& search,
                               const std::string& replace);

    static float convertFloat(std::string content);
    static int convertInt(std::string content);
    static void replaceSlashesWithUnderscores(std::string &content);
    static std::string getDirectory(std::string filePath);
    static std::string getParentDirectory(std::string filePath);
    static std::string getFileName(std::string filePath);
    static bool findMatchingFile(std::string prefix, std::vector<std::string> &extensions, std::string &file);
    static std::string toLower(std::string str);
    static std::string uppercaseFirst(std::string str);
    static std::string filterComments(std::string line);
    static std::string trimEnds(std::string str);

    //todo: there has to be a better way to do this
    static std::string combinePath(std::list<std::string> &paths);
    static std::string combinePath(std::string path1, std::string path2);
    static std::string combinePath(std::string path1, std::string path2, std::string path3);
    static std::string combinePath(std::string path1, std::string path2, std::string path3, std::string path4);
    static std::string combinePath(std::string path1, std::string path2, std::string path3, std::string path4, std::string path5);

    static int termfix(uint32_t ttyId);
    static int getVTid();
   
#ifdef WIN32
    static const char pathSeparator = '\\';
#else
    static const char pathSeparator = '/';
#endif

private:
    Utils();
    virtual ~Utils();
    static int open_a_console(const char *fnam);
    static int get_console_fd_or_die(void);
};

