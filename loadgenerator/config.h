#ifndef __CONFIG_H_
#define __CONFIG_H_

#include<string>
#include<map>
#include<vector>

typedef std::pair<std::string, std::string> BenchConfig;
typedef std::map<std::string, std::string> BenchConfigMap;
typedef std::map<std::string, BenchConfigMap> BenchConfigMaps; 
typedef std::vector<std::string> BenchConfigKeys;
typedef std::vector<std::pair<std::string, std::string>> BenchConfigSectionKeys;
typedef std::vector<std::string> BenchConfigSections;

class BenchConfigParser {
  public:
    bool parse(const char *filename);

    BenchConfigMaps & getConfigs() { return m_configs; }
    BenchConfigMap & getSectionConfigs(const char *section) { return m_configs[section]; }

    const char *getConfig(const char *section, const char *key, const char *value);

    void getConfigKeys(BenchConfigSectionKeys & skeys);
    void getConfigKeys(const char *section, BenchConfigKeys & keys);
    void getSections(BenchConfigSections & sections);

  public:
    static std::string& trim(std::string &s);

  private:
    BenchConfigMaps m_configs;
};


#endif /* __CONFIG_H_ */

/* vim: set ts=4 sw=4 : */
