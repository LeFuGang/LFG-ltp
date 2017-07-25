#include <iostream>
#include <string>
#include "ltp/segment_dll.h"

int main(int argc, char * argv[]) {

	std::cout<<"this is"<<std::endl;
	if (argc < 2) {
    std::cerr << "cws [model path] [lexicon_file]" << std::endl;
    return 1;
  }

  void * engine = 0;
  if (argc == 2) {
    engine = segmentor_create_segmentor(argv[1]);
  } else if (argc == 3) {
    engine = segmentor_create_segmentor(argv[1], argv[2]);
  }

  if (!engine) {
    "nihao ";
    return -1;
  }
  std::vector<std::string> words;

  const char * suite[1] = {
    "申万宏源宏观分析师李慧勇则认为，人民币趋势性贬值接近尾声。他称，本轮人民币汇率重估始于2015年8月11日汇改，至目前在岸汇率和离岸人民币对美元即期汇率已累计贬值9.2%左右，人民币有效汇率贬值6.2%左右。虽然未来人民币仍会跟随国内经济而波动，但从大趋势看，企业债务重整已于去年中基本完成，而出口恢复正增长也对人民币形成支撑，人民币趋势性贬值已告结束，未来双向波动将成常态。",
  };

  for (int i = 0; i < 1; ++ i) {
    words.clear();
    int len = segmentor_segment(engine, suite[i], words);
    for (int i = 0; i < len; ++ i) {
      std::cout << words[i];
      if (i+1 == len) std::cout <<std::endl;
      else std::cout<< "|";
    }
  }

  segmentor_release_segmentor(engine);
  return 0;
}
