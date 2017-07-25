#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include "ner_process.h"

using namespace std;
int ner_process(std::vector<std::string> &words,
                std::vector<std::string> &tags)
{
  std::vector<std::string> tags_temp;
  // words1.swap(words);
  map<string, string> dict_map;
  map<string, string>::iterator iter;
  std::ifstream infile("dict_ner.txt");
  std::string ner_key;
  std::string ner_value;
  std::string line;
  while (std::getline(infile, line))
  {
    int index = line.find(" ");
    ner_key = line.substr(0, index);
    ner_value = line.substr(index + 1);
    dict_map[ner_key] = ner_value;

    // std::cout << ner_key << "--" << ner_value << std::endl;
  }

  
  bool is_match;
  //词典匹配
  for (int i = 0; i < words.size();)
  {
    is_match = false;
    int base = i;
    std::string word;
    //距离位5的搜索匹配
    for (int j = 0; j <= 5; j++)
    {
      word += words[base + j];
      std::cout <<"word:"<<word<< std::endl;
      if (dict_map.find(word) != dict_map.end())
      {
        is_match = true;
        if (j == 0) //第一个词位实体
        {
          tags[base] = "S-" + dict_map[word];
          break;
        }

        for (int k = base; k <= base + j; k++) //大于1的匹配，重新打标签
        {
          if (k == base)
          {
            tags[k] = "B-" + dict_map[word];
             std::cout << "tags:" << tags[k] << std::endl;
          }
          else if (k == base + j)
          {
            tags[k] = "E-" + dict_map[word];
          }
          else
            tags[k] = "I-" + dict_map[word];
        }
        i = base + j + 1;
        break;
      }
    }
    if (!is_match)
      i++;
  }

 for (int i = 0; i < tags.size(); i++)
  {
    std::cout << "for:" << tags[i] << std::endl;
  }

  return 0;
}
