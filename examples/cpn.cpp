#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "ltp/segment_dll.h"
#include "ltp/postag_dll.h"
#include "ltp/ner_dll.h"
using namespace std;

int main()
{
  void *cws_engine = segmentor_create_segmentor("../ltp_data/cws.model");

  // if (!cws_engine) {
  //   std::cerr << "failed to load model" << std::endl;
  //   return -1;
  // }

  void *pos_engine = postagger_create_postagger("./pos_1.model");
  if (!pos_engine)
  {
    std::cerr << "failed to load model" << std::endl;
    return -1;
  }

  void *ner_engine = ner_create_recognizer("../ltp_data/ner.model");
  if (!ner_engine)
  {
    std::cerr << "failed to load model" << std::endl;
    return -1;
  }

  std::vector<std::string> words;

  std::ifstream infile("cpn.txt"); //输入

  std::ofstream pos_outfile;
  std::ofstream ner_outfile;
  pos_outfile.open("pos.txt"); //输出
  ner_outfile.open("ner.txt"); //输出
 
  std::string content;
  std::string line;

  while (std::getline(infile, line))
  {
    content = line;

    istringstream is(content);
    std::string str;
    words.clear();
    while (is >> str)
    {
      words.push_back(str);
    }
    // words.clear();

    // segmentor_segment(cws_engine, content, words);
    // int len = segmentor_segment(cws_engine, content, words);
    // for (int i = 0; i < len; ++i)
    // {
    //   outfile << words[i];
    //   // std::cout << words[i];
    //   if (i + 1 == len)
    //     outfile << std::endl;
    //   else
    //     outfile << " ";
    // }
    ///////////////////

    std::vector<std::string> postags;
    postags.clear();

    postagger_postag(pos_engine, words, postags);

    for (int i = 0; i < postags.size(); ++ i) {
        pos_outfile<<words[i]<<"_"<<postags[i];
        if (i == postags.size() - 1) pos_outfile<<""<<std::endl;
        else pos_outfile<<" ";

        // std::cout << words[i] << "_" << postags[i];
        // if (i == postags.size() - 1) std::cout << std::endl;
        // else std::cout << " ";
    }

    //////////////////////

    std::vector<std::string>  tags;
    tags.clear();

    ner_recognize(ner_engine, words, postags, tags);

      for (int i = 0; i < tags.size(); ++ i) {
            ner_outfile<<words[i]<<"/"<<postags[i]<<"#"<<tags[i];
            if (i == tags.size() - 1) ner_outfile<<""<<std::endl;
            else ner_outfile<<" ";
        std::cout << words[i] << "\t" << postags[i] << "\t" << tags[i] << std::endl;
      }
  }
  pos_outfile.close();
  ner_outfile.close();

  // segmentor_release_segmentor(cws_engine);
  postagger_release_postagger(pos_engine);
  ner_release_recognizer(ner_engine);
  return 0;
}
