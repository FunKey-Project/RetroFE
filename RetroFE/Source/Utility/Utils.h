/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <string>
#include <vector>

class Utils
{
public:
    static std::string Replace(std::string subject, const std::string& search,
                               const std::string& replace);

    static float ConvertFloat(std::string content);
    static int ConvertInt(std::string content);
    static void NormalizeBackSlashes(std::string &content);
    static std::string GetDirectory(std::string filePath);
    static std::string GetParentDirectory(std::string filePath);
    static std::string GetFileName(std::string filePath);
    static bool FindMatchingFile(std::string prefix, std::vector<std::string> &extensions, std::string &file);
private:
    Utils();
    virtual ~Utils();
};

