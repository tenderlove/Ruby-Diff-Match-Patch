#ifndef ruby_DMP_HPP
#define ruby_DMP_HPP

#ifdef __cplusplus
#include <string>
#include "diff_match_patch-stl/diff_match_patch.h"
#endif

#include <ruby.h>
#include <string.h>

#define dmp diff_match_patch<std::string>
typedef VALUE (ruby_method_vararg)(...);

void register_dmp();

#endif
