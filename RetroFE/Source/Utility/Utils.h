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
class Utils
{
public:
    static std::string Replace(std::string subject, const std::string& search,
                               const std::string& replace);

    static float ConvertFloat(std::string content);
    static int ConvertInt(std::string content);
    static void ReplaceSlashesWithUnderscores(std::string &content);
    static std::string GetDirectory(std::string filePath);
    static std::string GetParentDirectory(std::string filePath);
    static std::string GetFileName(std::string filePath);
    static bool FindMatchingFile(std::string prefix, std::vector<std::string> &extensions, std::string &file);
    static std::string ToLower(std::string str);
    static std::string UppercaseFirst(std::string str);

    //todo: there has to be a better way to do this
    static std::string CombinePath(std::list<std::string> &paths);
    static std::string CombinePath(std::string path1, std::string path2);
    static std::string CombinePath(std::string path1, std::string path2, std::string path3);
    static std::string CombinePath(std::string path1, std::string path2, std::string path3, std::string path4);
    static std::string CombinePath(std::string path1, std::string path2, std::string path3, std::string path4, std::string path5);
   
#ifdef WIN32
    static const char PathSeparator = '\\';
#else
    static const char PathSeparator = '/';
#endif

private:
    Utils();
    virtual ~Utils();
};

