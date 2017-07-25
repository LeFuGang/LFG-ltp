#include "config.h"
#include "utils/logging.hpp"
#include "utils/strvec.hpp"
#include "utils/strutils.hpp"
#include "segmentor/segmentor.h"
#include "segmentor/instance.h"
#include "segmentor/extractor.h"
#include "segmentor/options.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>    //  std::sort
#include <functional>   //  std::greater

namespace ltp {
namespace segmentor {

using framework::ViterbiFeatureContext;
using framework::ViterbiScoreMatrix;
using utility::StringVec;
using utility::SmartMap;
using math::FeatureVector;
using math::SparseVec;
using strutils::trim;

const std::string Segmentor::model_header = "otcws";

Segmentor::Segmentor(): model(0) {}
Segmentor::~Segmentor() { if (model) { delete model; model = 0; } }

//特征提取
void Segmentor::extract_features(const Instance& inst,
    Model* mdl,
    ViterbiFeatureContext* ctx,
    bool create) const {
  size_t N = Extractor::num_templates();//提取特征函数的数量
  size_t T = mdl->num_labels();//标签的个数

  std::vector< StringVec > cache;
  std::vector< int > cache_again;

  cache.resize(N);
  size_t L = inst.size();

  if (ctx) {
    // allocate the uni_features
    ctx->uni_features.resize(L, T);
    ctx->uni_features = 0;
  }

  for (size_t pos = 0; pos < L; ++ pos) {//词
    for (size_t n = 0; n < N; ++ n) { cache[n].clear(); }//特征函数的个数
    cache_again.clear();

    Extractor::extract1o(inst, pos, cache);//特征函数的提取进入cache，特征函数对应的各个位置对应的词
    for (size_t tid = 0; tid < cache.size(); ++ tid) {
      for (size_t itx = 0; itx < cache[tid].size(); ++ itx) {
        if (create) { mdl->space.retrieve(tid, cache[tid][itx], true); }

        int idx = mdl->space.index(tid, cache[tid][itx]);
        if (idx >= 0) { cache_again.push_back(idx); }
      }
    }

    size_t num_feat = cache_again.size();
    if (num_feat > 0 && ctx) {
      size_t t = 0;
      int * idx = new int[num_feat];
      for (size_t j = 0; j < num_feat; ++ j) {
        idx[j] = cache_again[j];
      }

      ctx->uni_features[pos][t] = new FeatureVector;
      ctx->uni_features[pos][t]->n = num_feat;
      ctx->uni_features[pos][t]->val = 0;
      ctx->uni_features[pos][t]->loff = 0;
      ctx->uni_features[pos][t]->idx = idx;

      for (t = 1; t < T; ++ t) {
        ctx->uni_features[pos][t] = new FeatureVector;
        ctx->uni_features[pos][t]->n = num_feat;
        ctx->uni_features[pos][t]->idx = idx;
        ctx->uni_features[pos][t]->val = 0;
        ctx->uni_features[pos][t]->loff = t;
      }
    }
  }
}
//匹配词典中的词
void Segmentor::build_lexicon_match_state(
    const std::vector<const Model::lexicon_t*>& lexicons,
    Instance* inst) const {
  // cache lexicon features.
  size_t len = inst->size();//处理后，词料的大小
  if (inst->lexicon_match_state.size()) { return; }
  inst->lexicon_match_state.resize(len, 0);

  // perform the maximum forward match algorithm--向前最大匹配算法
  for (size_t i = 0; i < len; ++ i) {
    std::string word; word.reserve(32);
    for (size_t j = i; j<i+5 && j < len; ++ j) {//距离为5
      word = word + inst->forms[j];

      bool found = false;
      for (size_t k = 0; k < lexicons.size(); ++ k) {
        if (lexicons[k]->get(word.c_str())) { found = true; break; }//词典匹配词汇
      }

      if (!found) { continue; }
      int l = j+1-i;

      if (l > (inst->lexicon_match_state[i] & 0x0F)) {
        inst->lexicon_match_state[i] &= 0xfff0;
        inst->lexicon_match_state[i] |= l;
      }

      if (l > ((inst->lexicon_match_state[j]>>4) & 0x0F)) {
        inst->lexicon_match_state[j] &= 0xff0f;
        inst->lexicon_match_state[j] |= (l<<4);
      }

      for (size_t k = i+1; k < j; ++k) {
        if (l>((inst->lexicon_match_state[k]>>8) & 0x0F)) {
          inst->lexicon_match_state[k] &= 0xf0ff;
          inst->lexicon_match_state[k] |= (l<<8);
        }
      }
    }
  }
}
//计算发射和转移概率
void Segmentor::calculate_scores(const Instance& inst,
    const Model& mdl,
    const ViterbiFeatureContext& ctx,//维特比算法解码文本
    bool avg,
    ViterbiScoreMatrix* scm) {//维特比算法的概率矩阵
  size_t L = inst.size();//预处理后的、词的大小
  size_t T = mdl.num_labels();//T表示标签的数量

  scm->resize(L, T, NEG_INF);//设置概率矩阵的属性

  for (size_t i = 0; i < L; ++ i) {
    for (size_t t = 0; t < T; ++ t) {
      FeatureVector * fv = ctx.uni_features[i][t];
      if (!fv) {
        continue;
      }
      scm->set_emit(i, t, mdl.param.dot(ctx.uni_features[i][t], avg));//设置发射矩阵(词->标签)的概率
    }
  }

  for (size_t pt = 0; pt < T; ++ pt) {
    for (size_t t = 0; t < T; ++ t) {
      int idx = mdl.space.index(pt, t);
      scm->set_tran(pt, t, mdl.param.dot(idx, avg));//转移概率(标签->标签)
    }
  }
}

void Segmentor::calculate_scores(const Instance& inst,
    const Model& bs_mdl,
    const Model& mdl,
    const ViterbiFeatureContext& bs_ctx,//解码文本
    const ViterbiFeatureContext& ctx,
    bool avg,
    ViterbiScoreMatrix* scm) {
  size_t len = inst.size();
  size_t L = model->num_labels();

  scm->resize(len, L, NEG_INF);

  for (size_t i = 0; i < len; ++ i) {
    for (size_t l = 0; l < L; ++ l) {
      FeatureVector* fv = bs_ctx.uni_features[i][l];
      if (!fv) { continue; }

      double score;
      if(!avg) {
        score = bs_mdl.param.dot(fv, false);
      } else {
        //flush baseline_mdl and mdl time to the same level
        score = bs_mdl.param.predict(fv, mdl.param.last() - bs_mdl.param.last());
      }

      fv = ctx.uni_features[i][l];
      if (!fv) {
        continue;
      }
      scm->set_emit(i, l, score + mdl.param.dot(ctx.uni_features[i][l], avg));//设置发射概率
    }
  }

  for (size_t pl = 0; pl < L; ++ pl) {
    for (size_t l = 0; l < L; ++ l) {
      double score;

      int idx = bs_mdl.space.index(pl, l);
      if(!avg) {
        score = bs_mdl.param.dot(idx, false);
      } else {
        score = bs_mdl.param.predict(idx, mdl.param.last() - bs_mdl.param.last());
      }

      idx = mdl.space.index(pl, l);
      scm->set_tran(pl, l, score + mdl.param.dot(idx, avg));//设置转移概率
    }
  }
}
//得到词汇，chars词/tagsidex词的标签号码0/1/2/3/-b/i/e/s、words存储分好的词汇
void Segmentor::build_words(const std::vector<std::string>& chars,
    const std::vector<int>& tagsidx,
    std::vector<std::string>& words) {
  words.clear();
  int len = chars.size();
  if (len == 0) { return; }

  // should check the tagsidx size
  std::string word = chars[0];
  for (int i = 1; i < len; ++ i) {
    int tag = tagsidx[i];
    if (tag == __b_id__ || tag == __s_id__) { // b, s判断是否是词首或单个词
      words.push_back(word);
      word = chars[i];
    } else {
      word += chars[i];//将一个词汇组合在一起
    }
  }

  words.push_back(word);//最后一个词汇
}
//导入外部词典
void Segmentor::load_lexicon(const char* filename, Model::lexicon_t* lexicon) const {
  std::ifstream ifs(filename);
  if (!ifs.good()) { return; }//good函数判断流是否正常
  std::string line;
  while (std::getline(ifs, line)) {
    trim(line);//去除空格和换行
    std::string form = line.substr(0, line.find_first_of(" \t"));
    lexicon->set(form.c_str(), true);
  }
  INFO_LOG("loaded %d lexicon entries", lexicon->size());
}

}     //  end for namespace segmentor
}     //  end for namespace ltp
