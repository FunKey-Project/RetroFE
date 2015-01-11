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

#include "Launcher.h"
#include "../Collection/Item.h"
#include "../Utility/Log.h"
#include "../Database/Configuration.h"
#include "../Utility/Utils.h"
#include "../RetroFE.h"
#include "../SDL.h"
#include <cstdlib>
#include <locale>
#include <sstream>
#include <fstream>
#ifdef WIN32
#include <windows.h>
#include <cstring>
#endif

Launcher::Launcher(RetroFE &p, Configuration &c)
    : Config(c)
    , RetroFEInst(p)
{
}

bool Launcher::Run(std::string collection, Item *collectionItem)
{
    std::string launcherName = collectionItem->GetLauncher();
    std::string executablePath;
    std::string selectedItemsDirectory;
    std::string selectedItemsPath;
    std::string currentDirectory;
    std::string extensions;
    std::string matchedExtension;
    std::string args;

    if(!GetLauncherExecutable(executablePath, currentDirectory, launcherName))
    {
        Logger::Write(Logger::ZONE_ERROR, "Launcher", "Failed to find launcher executable (launcher: " + launcherName + " executable: " + executablePath + ")");
        return false;
    }
    if(!GetExtensions(extensions, collection))
    {
        Logger::Write(Logger::ZONE_ERROR, "Launcher", "No file extensions configured for collection \"" + collection + "\"");
        return false;
    }
    if(!GetCollectionDirectory(selectedItemsDirectory, collection))
    {
        Logger::Write(Logger::ZONE_ERROR, "Launcher", "Could not find files in directory \"" + selectedItemsDirectory + "\" for collection \"" + collection + "\"");
        return false;
    }
    if(!GetLauncherArgs(args, launcherName))
    {
        Logger::Write(Logger::ZONE_ERROR, "Launcher", "No launcher arguments specified for launcher " + launcherName);
        return false;
    }
    if(!FindFile(selectedItemsPath, matchedExtension, selectedItemsDirectory, collectionItem->GetName(), extensions))
    {
        // FindFile() prints out diagnostic messages for us, no need to print anything here
        return false;
    }
    args = ReplaceVariables(args,
                            selectedItemsPath,
                            collectionItem->GetName(),
                            collectionItem->GetFileName(),
                            selectedItemsDirectory,
                            collection);

    executablePath = ReplaceVariables(executablePath,
                                      selectedItemsPath,
                                      collectionItem->GetName(),
                                      collectionItem->GetFileName(),
                                      selectedItemsDirectory,
                                      collection);

    currentDirectory = ReplaceVariables(currentDirectory,
                                        selectedItemsPath,
                                        collectionItem->GetName(),
                                        collectionItem->GetFileName(),
                                        selectedItemsDirectory,
                                        collection);

    if(!ExecuteCommand(executablePath, args, currentDirectory))
    {
        Logger::Write(Logger::ZONE_ERROR, "Launcher", "Failed to launch.");
        return false;
    }

    return true;
}

std::string Launcher::ReplaceVariables(std::string str,
                                       std::string itemFilePath,
                                       std::string itemName,
                                       std::string itemFilename,
                                       std::string itemDirectory,
                                       std::string itemCollectionName)
{
    str = Utils::Replace(str, "%ITEM_FILEPATH%", itemFilePath);
    str = Utils::Replace(str, "%ITEM_NAME%", itemName);
    str = Utils::Replace(str, "%ITEM_FILENAME%", itemFilename);
    str = Utils::Replace(str, "%ITEM_DIRECTORY%", itemDirectory);
    str = Utils::Replace(str, "%ITEM_COLLECTION_NAME%", itemCollectionName);
    str = Utils::Replace(str, "%RETROFE_PATH%", Configuration::GetAbsolutePath());
#ifdef WIN32
    str = Utils::Replace(str, "%RETROFE_EXEC_PATH%", Configuration::GetAbsolutePath() + "/RetroFE.exe");
#else
    str = Utils::Replace(str, "%RETROFE_EXEC_PATH%", Configuration::GetAbsolutePath() + "/RetroFE");
#endif

    return str;
}

bool Launcher::ExecuteCommand(std::string executable, std::string args, std::string currentDirectory)
{
    bool retVal = false;
    std::string executionString = "\"" + executable + "\" " + args;

    Logger::Write(Logger::ZONE_INFO, "Launcher", "Attempting to launch: " + executionString);
    Logger::Write(Logger::ZONE_INFO, "Launcher", "     from within folder: " + currentDirectory);

    //todo: use delegation instead of depending on knowing the RetroFE class (tie to an interface)
    RetroFEInst.LaunchEnter();

#ifdef WIN32
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    char applicationName[256];
    char currDir[256];
    memset(&applicationName, 0, sizeof(applicationName));
    memset(&startupInfo, 0, sizeof(startupInfo));
    memset(&processInfo, 0, sizeof(processInfo));
    strncpy(applicationName, executionString.c_str(), sizeof(applicationName));
    strncpy(currDir, currentDirectory.c_str(), sizeof(currDir));
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.wShowWindow = SW_SHOWDEFAULT;

    if(!CreateProcess(NULL, applicationName, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo))
#else
    if(system(executionString.c_str()) != 0)
#endif
    {
        Logger::Write(Logger::ZONE_ERROR, "Launcher", "Failed to run: " + executable);
    }

    else
    {
#ifdef WIN32
        while(WAIT_OBJECT_0 != MsgWaitForMultipleObjects(1, &processInfo.hProcess, FALSE, INFINITE, QS_ALLINPUT))
        {
            MSG msg;
            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);
            }
        }

        // result = GetExitCodeProcess(processInfo.hProcess, &exitCode);
        CloseHandle(processInfo.hProcess);
#endif
        retVal = true;
    }

    Logger::Write(Logger::ZONE_INFO, "Launcher", "Completed");
    RetroFEInst.LaunchExit();

    return retVal;
}

bool Launcher::GetLauncherName(std::string &launcherName, std::string collection)
{
    std::string launcherKey = "collections." + collection + ".launcher";

    // find the launcher for the particular item
    if(!Config.GetProperty(launcherKey, launcherName))
    {
        std::stringstream ss;

        ss << "Launch failed. Could not find a configured launcher for collection \""
           << collection
           << "\" (could not find a property for \""
           << launcherKey
           << "\")";

        Logger::Write(Logger::ZONE_ERROR, "Launcher", ss.str());

        return false;
    }

    std::stringstream ss;
    ss        << "collections."
              << collection
              << " is configured to use launchers."
              << launcherName
              << "\"";

    Logger::Write(Logger::ZONE_DEBUG, "Launcher", ss.str());

    return true;
}



bool Launcher::GetLauncherExecutable(std::string &executable, std::string &currentDirectory, std::string launcherName)
{
    std::string executableKey = "launchers." + launcherName + ".executable";

    if(!Config.GetProperty(executableKey, executable))
    {
        return false;
    }

    std::string currentDirectoryKey = "launchers." + launcherName + ".currentDirectory";
    currentDirectory = Utils::GetDirectory(executable);

    Config.GetProperty(currentDirectoryKey, currentDirectory);

    return true;
}

bool Launcher::GetLauncherArgs(std::string &args, std::string launcherName)
{
    std::string argsKey = "launchers." + launcherName + ".arguments";

    if(!Config.GetProperty(argsKey, args))
    {
        Logger::Write(Logger::ZONE_ERROR, "Launcher", "No arguments specified for: " + argsKey);

        return false;
    }
    return true;
}

bool Launcher::GetExtensions(std::string &extensions, std::string collection)
{
    std::string extensionsKey = "collections." + collection + ".list.extensions";

    if(!Config.GetProperty(extensionsKey, extensions))
    {
        Logger::Write(Logger::ZONE_ERROR, "Launcher", "No extensions specified for: " + extensionsKey);
        return false;
    }

    extensions = Utils::Replace(extensions, " ", "");
    extensions = Utils::Replace(extensions, ".", "");

    return true;
}

bool Launcher::GetCollectionDirectory(std::string &directory, std::string collection)
{
    std::string itemsPathKey = "collections." + collection + ".list.path";
    std::string itemsPathValue;

    // find the items path folder (i.e. ROM path)
    if(!Config.GetPropertyAbsolutePath(itemsPathKey, itemsPathValue))
    {
        directory = "";
    }
    else
    {
        directory += itemsPathValue + "/";
    }

    return true;
}

bool Launcher::FindFile(std::string &foundFilePath, std::string &foundFilename, std::string directory, std::string filenameWithoutExtension, std::string extensions)
{
    std::string extension;
    bool fileFound = false;
    std::stringstream ss;
    ss << extensions;

    while(!fileFound && std::getline(ss, extension, ',') )
    {
        std::string selectedItemsPath = directory + filenameWithoutExtension + "." + extension;
        std::ifstream f(selectedItemsPath.c_str());

        if (f.good())
        {
            std::stringstream ss;

            ss        <<"Checking to see if \""
                      << selectedItemsPath << "\" exists  [Yes]";

            fileFound = true;

            Logger::Write(Logger::ZONE_INFO, "Launcher", ss.str());

            foundFilePath = selectedItemsPath;
            foundFilename = extension;
        }
        else
        {
            std::stringstream ss;

            ss        << "Checking to see if \""
                      << selectedItemsPath << "\" exists  [No]";

            Logger::Write(Logger::ZONE_WARNING, "Launcher", ss.str());
        }

        f.close();
    }

    // get the launchers executable

    if(!fileFound)
    {
        std::stringstream ss;
        ss        <<"Could not find any files with the name \""
                  << filenameWithoutExtension << "\" in folder \""
                  << directory;

        Logger::Write(Logger::ZONE_ERROR, "Launcher", ss.str());

    }

    return fileFound;
}

