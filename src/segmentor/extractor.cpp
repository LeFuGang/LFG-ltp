#include "segmentor/extractor.h"
#include "segmentor/settings.h"
#include "utils/strutils.hpp"
#include "utils/chartypes.hpp"

namespace ltp {
namespace segmentor {

using utility::Template;
using utility::StringVec;

std::vector<Template *> Extractor::templates;

Extractor& Extractor::extractor() {
  static Extractor instance_;
  return instance_;
}

int Extractor::num_templates() {
  extractor();
  return templates.size();
}

Extractor::Extractor() {
  // delimit feature templates--Template将大括号中的字符放入缓存
  //  templates.push_back(new Template("1={c-3}"));
  templates.push_back(new Template("1={c-2}"));
  templates.push_back(new Template("2={c-1}"));
  templates.push_back(new Template("3={c-0}"));
  templates.push_back(new Template("4={c+1}"));
  templates.push_back(new Template("5={c+2}"));
   // templates.push_back(new Template("7={c+3}"));

  //  templates.push_back(new Template("6={c-3}-{c-2}"));
  templates.push_back(new Template("6={c-2}-{c-1}"));
  templates.push_back(new Template("7={c-1}-{c-0}"));
  templates.push_back(new Template("8={c-0}-{c+1}"));
  templates.push_back(new Template("9={c+1}-{c+2}"));
  //  templates.push_back(new Template("13={c+2}-{c+3}"));

  //  templates.push_back(new Template("14={ct-2}"));
  templates.push_back(new Template("14={ct-1}"));
  templates.push_back(new Template("15={ct-0}"));
  templates.push_back(new Template("16={ct+1}"));
  //  templates.push_back(new Template("18={ct+2}"));

  //  templates.push_back(new Template("19={lex0}"));
  templates.push_back(new Template("17={lex1}"));
  templates.push_back(new Template("18={lex2}"));
  templates.push_back(new Template("19={lex3}"));
  //  templates.push_back(new Template("23={lex4}"));

  //  templates.push_back(new Template("24={lexe1}"));
  //  templates.push_back(new Template("25={lexe2}"));
  //  templates.push_back(new Template("26={lexe3}"));
  // templates.push_back(new Template(""));
}
  
Extractor::~Extractor() {                         //解析函数,释放空间,指针
  for (size_t i = 0; i < templates.size(); ++ i) {
    delete templates[i];
  }
}

int Extractor::extract1o(const Instance& inst, int idx,  //idx词的位置
    std::vector<StringVec>& cache) {

  size_t len = inst.size();
  Template::Data data;

#define EQU(x, y) (inst.forms[(x)] == inst.forms[(y)])
#define TYPE(x) (strutils::to_str(inst.chartypes[(x)]&0x07))
 //字符n-gram特征
  // data.set( "c-3", (idx-3 < 0 ? BOS : inst.forms[idx-3]) );//设置key-value键值
  data.set( "c-2", (idx-2 < 0 ? BOS : inst.forms[idx-2]) );//inst.forms向量，存储已处理的词，字符串类型的向量
  data.set( "c-1", (idx-1 < 0 ? BOS : inst.forms[idx-1]) );//里面括号认证是否在inst字符实例内（0，len）
  data.set( "c-0", inst.forms[idx] );
  data.set( "c+1", (idx+1 >= len ? EOS : inst.forms[idx+1]) );
  data.set( "c+2", (idx+2 >= len ? EOS : inst.forms[idx+2]) );
  // data.set( "c+3", (idx+3 >= len ? EOS : inst.forms[idx+3]) );
//字符类别特征
  //  data.set( "ct-2", (idx-2 < 0 ? BOT : TYPE(idx-2)) );
  data.set( "ct-1", (idx-1 < 0 ? BOT : TYPE(idx-1)) );
  data.set( "ct-0", TYPE(idx) );
  data.set( "ct+1", (idx+1 >= len ? EOT : TYPE(idx+1)) );
  //  data.set( "ct+2", (idx+2 >= len ? BOT : TYPE(idx+2)) );
   //字符重复信息特征
  data.set( "lex1", strutils::to_str(inst.lexicon_match_state[idx] & 0x0f));
  data.set( "lex2", strutils::to_str((inst.lexicon_match_state[idx]>>4) & 0x0f));
  data.set( "lex3", strutils::to_str((inst.lexicon_match_state[idx]>>8) & 0x0f));
  //  data.set( "lex3", strutils::to_str((inst.lexicon_match_state[idx]>>12) & 0x0f));
  //  data.set( "lex4", strutils::to_str((inst.lexicon_match_state[idx]>>16) & 0x0f));

#undef TYPE
#undef EQU

  std::string feat;
  feat.reserve(1024);//预分配1024个元素的存储空间
  // render features   将data中的value发送给feat 
  for (size_t i = 0; i < templates.size(); ++ i) {
    templates[i]->render(data, feat);
    cache[i].push_back(feat);
  }
  return 0;
}

}     //  end for namespace segmentor
}     //  end for namespace ltp
