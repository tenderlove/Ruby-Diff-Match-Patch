#include <rice/Module.hpp>
#include "_dmp.h"

extern "C"

void Init_diff_match_patch() {
  RUBY_TRY {
    register_dmp();
  } RUBY_CATCH
}