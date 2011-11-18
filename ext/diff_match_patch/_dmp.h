#ifndef ruby_DMP_HPP
#define ruby_DMP_HPP

#include <rice/Object.hpp>
#include <rice/Data_Type.hpp>
#include <rice/Array.hpp> 
#include "diff_match_patch-stl/diff_match_patch.h"
#include <string>
#include <rice/Constructor.hpp>

#define dmp diff_match_patch<std::string>


void register_dmp();

#endif