#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>

#include "ltp/ner_dll.h"
#include "ner_process.h"
using namespace std;

int main() {
    void * engine = ner_create_recognizer("../ltp_data/ner.model");
  if (!engine) {
    std::cerr << "failed to load model" << std::endl;
    return -1;
  }

  std::ifstream infile("train1.pos");
  std::string content;
  std::string line;
    
       
  std::ofstream outfile;
  outfile.open("train1.ner");
  while (std::getline(infile, line)){
        std::vector<std::string> words;
        std::vector<std::string> postags;
        std::vector<std::string>  tags;
       content=line;
        istringstream is(content);
       std::string str;
       words.clear();
 while(is>>str)
  {
    int index=str.find("_");
    words.push_back(str.substr(0,index));
    postags.push_back(str.substr(index+1));
  }

  ner_recognize(engine, words, postags, tags);
 
  //加入实体字典
  //ner_process(words,tags);
  
  for (int i = 0; i < tags.size(); ++ i) {
    outfile<<words[i]<<"/"<<postags[i]<<"#"<<tags[i];
        if (i == tags.size() - 1) outfile<<""<<std::endl;
        else outfile<<" ";
    // std::cout << words[i] << "\t" << postags[i] << "\t" << tags[i] << std::endl;
  }
  
}
  ner_release_recognizer(engine);
  return 0;
}

