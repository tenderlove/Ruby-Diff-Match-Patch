#ifndef ruby_DMP_HPP
#define ruby_DMP_HPP

#include <rice/Object.hpp>
#include <rice/Data_Type.hpp>
#include <rice/Array.hpp> 
#include "diff_match_patch-stl/diff_match_patch.h"
#include <string>
#include <rice/Constructor.hpp>

class rb_diff_match_patch : diff_match_patch<std::string> {
public:
  rb_diff_match_patch(){}
  Rice::Object rb_diff_main(const std::string &text1, const std::string &text2, bool checklines = true){
    Rice::Array out;
    Diffs diffs = diff_main(text1, text2, checklines);
    Diffs::iterator current_diff;
    for (current_diff = diffs.begin(); current_diff != diffs.end(); ++current_diff) {
      Rice::Array rb_diff;
      switch (current_diff->operation){
        case INSERT:
          rb_diff.push(1);
          break;
        case DELETE:
          rb_diff.push(-1);
          break;
        case EQUAL:
          rb_diff.push(0);
          break;
      }
      rb_diff.push(current_diff->text);
      out.push(rb_diff);
    }
      
    return out;
  }

};

void register_dmp(){


  Rice::Data_Type< rb_diff_match_patch > rb_cDMP = Rice::define_class< rb_diff_match_patch >("DiffMatchPatch");
  rb_cDMP.define_constructor(Rice::Constructor<rb_diff_match_patch>());
  rb_cDMP.define_method("diff_main", &rb_diff_match_patch::rb_diff_main);

}

#endif