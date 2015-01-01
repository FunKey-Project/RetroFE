/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <string>

class Configuration;
class Item;
class RetroFE;

class Launcher
{
public:
    Launcher(RetroFE *p);
    bool Run(std::string collection, Item *collectionItem);

private:
    std::string ReplaceString(
        std::string subject,
        const std::string &search,
        const std::string &replace);

    bool GetLauncherName(std::string &launcherName, std::string collection);
    bool GetLauncherExecutable(std::string &executable, std::string &currentDirectory, std::string launcherName);
    bool GetLauncherArgs(std::string &args, std::string launcherName);
    bool GetExtensions(std::string &extensions, std::string launcherName);
    bool GetCollectionDirectory(std::string &directory, std::string collection);
    bool ExecuteCommand(std::string executable, std::string arguments, std::string currentDirectory);
    bool FindFile(std::string &foundFilePath, std::string &foundFilename, std::string directory, std::string filenameWithoutExtension, std::string extensions);
    std::string ReplaceVariables(std::string str,
                                 std::string itemFilePath,
                                 std::string itemName,
                                 std::string itemFilename,
                                 std::string itemDirectory,
                                 std::string itemCollectionName);

    Configuration *Config;
    RetroFE *RetroFEInst;
};
