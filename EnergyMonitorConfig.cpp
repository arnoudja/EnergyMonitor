
#include "EnergyMonitorConfig.h"

#include <fstream>
#include <algorithm>
#include <cctype>

using namespace std;

namespace
{
    const string cConfigFilename = "energymonitor.conf";

    const int cMaxLineSize = 128;
    const char cCommentCharacter = '#';
    const string cSplitCharacters = " \t";

    const string cPictureOutputConfigName = "PictureOutput1";
}

bool EnergyMonitorConfig::isTestMode() const
{
    return !(getPictureOutputFilename().empty());
}

string  EnergyMonitorConfig::getPictureOutputFilename() const
{
    return getValue(cPictureOutputConfigName);
}

string EnergyMonitorConfig::getValue(const string& key) const
{
    string keyLower(key);
    transform(keyLower.begin(), keyLower.end(), keyLower.begin(), ::tolower);

    TConfigValuesMap::const_iterator iter = myConfigValues.find(keyLower);

    return (iter != myConfigValues.end()) ? iter->second : string();
}

void EnergyMonitorConfig::readConfig()
{
    ifstream configFile(cConfigFilename.c_str());

    if (configFile.is_open())
    {
        while (!configFile.eof())
        {
            char line[cMaxLineSize + 1];
            line[cMaxLineSize] = '\0';

            configFile.getline(line, cMaxLineSize);

            string key, value;
            if (splitLine(line, key, value))
            {
                transform(key.begin(), key.end(), key.begin(), ::tolower);
                myConfigValues[key] = value;
            }
        }
    }
}

bool EnergyMonitorConfig::splitLine(const string& line, string& key, string& value)
{
    size_t keyStartPos = line.find_first_not_of(cSplitCharacters);

    size_t keyEndPos = string::npos;
    if (keyStartPos != string::npos)
    {
        keyEndPos = line.find_first_of(cSplitCharacters, keyStartPos);
    }

    size_t valueStartPos = string::npos;
    if (keyEndPos != string::npos)
    {
        valueStartPos = line.find_first_not_of(cSplitCharacters, keyEndPos);
    }

    size_t valueEndPos = string::npos;
    if (valueStartPos != string::npos)
    {
        valueEndPos = line.find_last_not_of(cSplitCharacters);
    }

    bool result = false;
    if (valueEndPos != string::npos && valueEndPos >= valueStartPos &&
        line.at(keyStartPos) != cCommentCharacter)
    {
        key = line.substr(keyStartPos, keyEndPos - keyStartPos);
        value = line.substr(valueStartPos, valueEndPos - valueStartPos + 1);

        result = true;
    }

    return result;
}
