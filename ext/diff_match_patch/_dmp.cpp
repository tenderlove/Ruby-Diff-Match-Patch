#include "_dmp.h"
#include <stdio.h>

class rb_diff_match_patch : diff_match_patch<std::string> {

protected:

  Diffs diffsFromRubyArray(Rice::Array &array, bool clearArray=false){
    Diffs diffs;
    size_t arraySize = array.size();
    for (size_t i = 0; i < arraySize; ++i){
      Rice::Array rb_diff;
      if (clearArray) {
        rb_diff = from_ruby<Rice::Array>(array.shift());
      } else {
        rb_diff = from_ruby<Rice::Array>(array[i]);
      }

      Operation op;
      switch (from_ruby<int>(rb_diff[0])) {
        case 1:
          op = INSERT;
          break;
        case 0:
          op = EQUAL;
          break;
        case -1:
          op = DELETE;
          break;
      }
      diffs.push_back(Diff(op, from_ruby<std::string>(rb_diff[1])));

    }
    return diffs;
  }

  Rice::Array rubyArrayFromDiffs(const Diffs &diffs){
    Rice::Array out;
    return rubyArrayFromDiffsWithArray(diffs, out);
  }

  Rice::Array rubyArrayFromDiffsWithArray(Diffs diffs, Rice::Array &out){
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

public:
  rb_diff_match_patch(){}
  
  Rice::Object rb_diff_main(const std::string &text1, const std::string &text2, bool checklines = false){
    Diffs diffs = diff_main(text1, text2, checklines);
    return rubyArrayFromDiffs(diffs);
  }

  float GetDiff_Timeout(){
    return Diff_Timeout;
  }

  void SetDiff_Timeout(float value){
    Diff_Timeout = value;
  }

  float GetDiff_EditCost(){
    return Diff_EditCost;
  }

  void SetDiff_EditCost(float value){
    Diff_EditCost = value;
  }

  Rice::Array rb_diff_cleanupSemantic(Rice::Array array){
    Diffs diffs = diffsFromRubyArray(array, true);
    diff_cleanupSemantic(diffs);
    return rubyArrayFromDiffsWithArray(diffs, array);
  }

  Rice::Array rb_diff_cleanupEfficiency(Rice::Array array){
    Diffs diffs = diffsFromRubyArray(array, true);
    diff_cleanupEfficiency(diffs);
    return rubyArrayFromDiffsWithArray(diffs, array);
  }

};

void register_dmp(){

  Rice::Data_Type< rb_diff_match_patch > rb_cDMP = Rice::define_class< rb_diff_match_patch >("DiffMatchPatch");
  rb_cDMP.define_constructor(Rice::Constructor<rb_diff_match_patch>());
  rb_cDMP.define_method("diff_main", &rb_diff_match_patch::rb_diff_main);
  rb_cDMP.define_method("diff_timeout", &rb_diff_match_patch::GetDiff_Timeout);
  rb_cDMP.define_method("diff_timeout=", &rb_diff_match_patch::SetDiff_Timeout);
  rb_cDMP.define_method("diff_edit_cost", &rb_diff_match_patch::GetDiff_EditCost);
  rb_cDMP.define_method("diff_edit_cost=", &rb_diff_match_patch::SetDiff_EditCost);
  rb_cDMP.define_method("diff_cleanup_semantic!", &rb_diff_match_patch::rb_diff_cleanupSemantic);
  rb_cDMP.define_method("diff_cleanup_efficiency!", &rb_diff_match_patch::rb_diff_cleanupEfficiency);



}