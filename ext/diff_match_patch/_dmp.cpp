#include "_dmp.h"


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

};

void register_dmp(){

  Rice::Data_Type< rb_diff_match_patch > rb_cDMP = Rice::define_class< rb_diff_match_patch >("DiffMatchPatch");
  rb_cDMP.define_constructor(Rice::Constructor<rb_diff_match_patch>());
  rb_cDMP.define_method("diff_main", &rb_diff_match_patch::rb_diff_main);
  rb_cDMP.define_method("diff_timeout", &rb_diff_match_patch::GetDiff_Timeout);
  rb_cDMP.define_method("diff_timeout=", &rb_diff_match_patch::SetDiff_Timeout);
  rb_cDMP.define_method("diff_edit_cost", &rb_diff_match_patch::GetDiff_EditCost);
  rb_cDMP.define_method("diff_edit_cost=", &rb_diff_match_patch::SetDiff_EditCost);


}