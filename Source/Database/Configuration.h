/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <string>
#include <map>
#include <vector>

class Configuration
{
public:
   Configuration();
   virtual ~Configuration();

   // gets the global configuration
	bool Import(std::string keyPrefix, std::string file);

	bool GetProperty(std::string key, std::string &value);
	bool GetProperty(std::string key, int &value);
	bool GetProperty(std::string key, bool &value);
	void GetChildKeyCrumbs(std::string parent, std::vector<std::string> &children);
	void SetProperty(std::string key, std::string value);
   bool PropertyExists(std::string key);
   bool PropertyPrefixExists(std::string key);
	bool GetPropertyAbsolutePath(std::string key, std::string &value);
	static void SetAbsolutePath(std::string absolutePath);
	static std::string GetAbsolutePath();
	static std::string ConvertToAbsolutePath(std::string prefix, std::string path);
	bool IsVerbose() const;
	void SetVerbose(bool verbose);
	bool IsRequiredPropertiesSet();

private:
	bool ParseLine(std::string keyPrefix, std::string line, int lineCount);
	std::string TrimEnds(std::string str);
	typedef std::map<std::string, std::string> PropertiesType;
	typedef std::pair<std::string, std::string> PropertiesPair;
	bool Verbose;

	static std::string AbsolutePath;
	PropertiesType Properties;
};
