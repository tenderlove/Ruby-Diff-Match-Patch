#include "_dmp.h"

#include <string>
#include "diff_match_patch-stl/diff_match_patch.h"
#include <ruby/thread.h>

VALUE cPatch;
VALUE cDiffMatchPatch;

#define RB_DEFINE_METHOD(klass, sym, fun, arv) \
  rb_define_method(klass, sym, (ruby_method_vararg *)fun, arv);

/* A patch wrapper that we can heap allocate.  */
class rb_patch_wrapper{
  public:
    dmp::Patch patch;
    rb_patch_wrapper(dmp::Patch &the_patch) : patch(the_patch) {}
};

static void dealloc(void * ctx) { delete reinterpret_cast<dmp *>(ctx); }
static void dealloc_patch(void * ctx) {
  delete reinterpret_cast<rb_patch_wrapper *>(ctx);
}

static VALUE allocate(VALUE klass) {
  dmp * ctx = new dmp();
  return Data_Wrap_Struct(klass, 0, dealloc, ctx);
}

static VALUE rubyArrayFromDiffsWithArray(dmp::Diffs diffs, VALUE out){
  dmp::Diffs::iterator current_diff;
  for (current_diff = diffs.begin(); current_diff != diffs.end(); ++current_diff) {
    VALUE rb_diff = rb_ary_new();
    switch (current_diff->operation){
      case dmp::INSERT:
        rb_ary_push(rb_diff, INT2NUM(1));
        break;
      case dmp::DELETE:
        rb_ary_push(rb_diff, INT2NUM(-1));
        break;
      case dmp::EQUAL:
        rb_ary_push(rb_diff, INT2NUM(0));
        break;
    }
    rb_ary_push(rb_diff, rb_str_new(current_diff->text.c_str(),
                                    current_diff->text.size()));
    rb_ary_push(out, rb_diff);
  }

  return out;
}

dmp::Diffs diffsFromRubyArray(VALUE array){
  dmp::Diffs diffs;
  size_t arraySize = RARRAY_LEN(array);

  for (size_t i = 0; i < arraySize; ++i){
    VALUE rb_diff;
    VALUE diffstr;
    char * c_diffstr;

    rb_diff = RARRAY_AREF(array, i);

    dmp::Operation op;
    switch (NUM2INT(RARRAY_AREF(rb_diff, 0))) {
      case 1:
        op = dmp::INSERT;
        break;
      case 0:
        op = dmp::EQUAL;
        break;
      case -1:
        op = dmp::DELETE;
        break;
    }
    diffstr = RARRAY_AREF(rb_diff, 1);
    c_diffstr = StringValuePtr(diffstr);
    dmp::Diff ddiff = dmp::Diff(op, dmp::string_t(c_diffstr));
    diffs.push_back(ddiff);
  }

  return diffs;
}

struct nogvl_diff_args {
  dmp * ctx;
  const char * text1;
  const char * text2;
  bool lines;
};

static dmp::Diffs * nogvl_diff_main(struct nogvl_diff_args * args) {
  dmp *ctx = args->ctx;

  dmp::Diffs diffs = ctx->diff_main(dmp::string_t(args->text1),
                                    dmp::string_t(args->text2),
                                    args->lines);

  return new dmp::Diffs(diffs);
}

static VALUE rb_diff_main(VALUE self, VALUE text1, VALUE text2, VALUE lines)
{
  dmp * ctx;
  void * rv;
  struct nogvl_diff_args args;
  dmp::Diffs * diffs;
  VALUE list;

  Data_Get_Struct(self, dmp, ctx);

  args.ctx = ctx;
  args.text1 = StringValuePtr(text1);
  args.text2 = StringValuePtr(text2);
  args.lines = RTEST(lines);

  rv = rb_thread_call_without_gvl((void *(*)(void *))nogvl_diff_main,
      (void *)&args, RUBY_UBF_PROCESS, 0);

  diffs = reinterpret_cast<dmp::Diffs *>(rv);

  list = rubyArrayFromDiffsWithArray(*diffs, rb_ary_new());

  delete diffs;

  return list;
}

static VALUE rb_diff_timeout(VALUE self) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);
  return DBL2NUM(ctx->Diff_Timeout);
}

static VALUE rb_set_diff_timeout(VALUE self, VALUE v) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);
  ctx->Diff_Timeout = NUM2DBL(v);

  return v;
}

static VALUE rb_diff_edit_cost(VALUE self) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);
  return DBL2NUM(ctx->Diff_EditCost);
}

static VALUE rb_set_diff_edit_cost(VALUE self, VALUE v) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);
  ctx->Diff_EditCost = NUM2DBL(v);

  return v;
}

static VALUE rb_diff_cleanup_semantic(VALUE self, VALUE list) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Diffs diffs = diffsFromRubyArray(list);
  ctx->diff_cleanupSemantic(diffs);
  return rubyArrayFromDiffsWithArray(diffs, rb_ary_new());
}

static VALUE rb_diff_cleanup_efficiency(VALUE self, VALUE list) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Diffs diffs = diffsFromRubyArray(list);
  ctx->diff_cleanupEfficiency(diffs);
  return rubyArrayFromDiffsWithArray(diffs, rb_ary_new());
}

static VALUE rb_diff_levenshtein(VALUE self, VALUE list) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Diffs diffs = diffsFromRubyArray(list);
  return INT2NUM(ctx->diff_levenshtein(diffs));
}

static VALUE rb_diff_pretty_html(VALUE self, VALUE list) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Diffs diffs = diffsFromRubyArray(list);
  std::string str = ctx->diff_prettyHtml(diffs);

  return rb_str_new(str.c_str(), str.size());
}

static VALUE rb_match_main(VALUE self, VALUE text, VALUE pattern, VALUE loc) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  return INT2NUM(ctx->match_main(dmp::string_t(StringValuePtr(text)),
                                 dmp::string_t(StringValuePtr(pattern)),
                                 NUM2INT(loc)));
}

static VALUE rb_match_threshold(VALUE self) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  return DBL2NUM(ctx->Match_Threshold);
}

static VALUE rb_set_match_threshold(VALUE self, VALUE value) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  ctx->Match_Threshold = NUM2DBL(value);

  return value;
}

static VALUE rb_match_distance(VALUE self) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  return INT2NUM(ctx->Match_Distance);
}

static VALUE rb_set_match_distance(VALUE self, VALUE value) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  ctx->Match_Distance = NUM2INT(value);

  return value;
}

static VALUE rb_patch_delete_threshold(VALUE self) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  return DBL2NUM(ctx->Patch_DeleteThreshold);
}

static VALUE rb_set_patch_delete_threshold(VALUE self, VALUE value) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  ctx->Patch_DeleteThreshold = NUM2DBL(value);

  return value;
}

static VALUE rubyArrayFromPatches(dmp::Patches patches){
  VALUE out = rb_ary_new();
  dmp::Patches::iterator current_patch = patches.begin();

  for (current_patch = patches.begin(); current_patch != patches.end(); ++current_patch){
    rb_patch_wrapper * p = new rb_patch_wrapper(*current_patch);
    VALUE patch = Data_Wrap_Struct(cPatch, 0, dealloc_patch, p);
    rb_ary_push(out, patch);
  }

  return out;
}

static VALUE rb_patch_from_text(VALUE self, VALUE text) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Patches patches = ctx->patch_fromText(dmp::string_t(StringValuePtr(text)));

  return rubyArrayFromPatches(patches);
}

dmp::Patches patchesFromRubyArray(VALUE array){
  dmp::Patches patches;
  for (size_t i = 0; i < RARRAY_LEN(array); ++i) {
    rb_patch_wrapper * wrapper;
    Data_Get_Struct(RARRAY_AREF(array, i), rb_patch_wrapper, wrapper);
    patches.push_back(wrapper->patch);
  }
  return patches;
}

static VALUE rb_patch_to_text(VALUE self, VALUE array) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Patches patches = patchesFromRubyArray(array);
  dmp::string_t str = ctx->patch_toText(patches);

  return rb_str_new(str.c_str(), str.size());
}

static VALUE rb_patch_make_from_texts(VALUE self, VALUE text1, VALUE text2) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Patches patches = ctx->patch_make(dmp::string_t(StringValuePtr(text1)),
                                         dmp::string_t(StringValuePtr(text2)));
  return rubyArrayFromPatches(patches);
}

static VALUE rb_patch_make_from_diffs(VALUE self, VALUE array) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Diffs diffs = diffsFromRubyArray(array);
  dmp::Patches patches = ctx->patch_make(diffs);
  return rubyArrayFromPatches(patches);
}

static VALUE rb_patch_make_from_text_and_diff(VALUE self, VALUE text, VALUE array) {
  dmp * ctx;
  Data_Get_Struct(self, dmp, ctx);

  dmp::Diffs diffs = diffsFromRubyArray(array);
  dmp::Patches patches = ctx->patch_make(dmp::string_t(StringValuePtr(text)),
                                         diffs);
  return rubyArrayFromPatches(patches);
}

static VALUE rb_patch_apply(VALUE self, VALUE patch_array, VALUE text) {
  dmp * ctx;

  VALUE out = rb_ary_new();
  VALUE bool_array = rb_ary_new();
  Data_Get_Struct(self, dmp, ctx);

  dmp::Patches patches = patchesFromRubyArray(patch_array);
  std::pair<std::string, std::vector<bool> > results;

  results = ctx->patch_apply(patches, dmp::string_t(StringValuePtr(text)));
  rb_ary_push(out, rb_str_new(results.first.c_str(), results.first.size()));

  for (size_t i = 0; i < results.second.size(); ++i){
    rb_ary_push(bool_array, results.second[i] ? Qtrue : Qfalse);
  }
  rb_ary_push(out, bool_array);

  return out;
}

static VALUE rb_patch_to_string(VALUE self) {
  rb_patch_wrapper * patch;
  Data_Get_Struct(self, rb_patch_wrapper, patch);

  dmp::Patch the_patch = patch->patch;
  dmp::string_t str = the_patch.toString();

  return rb_str_new(str.c_str(), str.size());
}

void Init_diff_match_patch(){
  cDiffMatchPatch = rb_define_class("DiffMatchPatch", rb_cObject);
  cPatch = rb_define_class_under(cDiffMatchPatch, "Patch", rb_cObject);

  rb_define_alloc_func(cDiffMatchPatch, allocate);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_main", rb_diff_main, 3);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_timeout", rb_diff_timeout, 0);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_timeout=", rb_set_diff_timeout, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_edit_cost", rb_diff_edit_cost, 0);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_edit_cost=", rb_set_diff_edit_cost, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_cleanup_semantic", rb_diff_cleanup_semantic, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_cleanup_efficiency", rb_diff_cleanup_efficiency, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_levenshtein", rb_diff_levenshtein, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "diff_pretty_html", rb_diff_pretty_html, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "match_main", rb_match_main, 3);
  RB_DEFINE_METHOD(cDiffMatchPatch, "match_threshold", rb_match_threshold, 0);
  RB_DEFINE_METHOD(cDiffMatchPatch, "match_threshold=", rb_set_match_threshold, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "match_distance", rb_match_distance, 0);
  RB_DEFINE_METHOD(cDiffMatchPatch, "match_distance=", rb_set_match_distance, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "patch_delete_threshold", rb_patch_delete_threshold, 0);
  RB_DEFINE_METHOD(cDiffMatchPatch, "patch_delete_threshold=", rb_set_patch_delete_threshold, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "patch_from_text", rb_patch_from_text, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "patch_to_text", rb_patch_to_text, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "__patch_make_from_texts__", rb_patch_make_from_texts, 2);
  RB_DEFINE_METHOD(cDiffMatchPatch, "__patch_make_from_diffs__", rb_patch_make_from_diffs, 1);
  RB_DEFINE_METHOD(cDiffMatchPatch, "__patch_make_from_text_and_diff__", rb_patch_make_from_text_and_diff, 2);
  RB_DEFINE_METHOD(cDiffMatchPatch, "patch_apply", rb_patch_apply, 2);

  RB_DEFINE_METHOD(cPatch, "to_string", rb_patch_to_string, 0);
  /*
  rb_cPatch.define_method("to_string", &rb_patch_wrapper::toString);
  rb_cPatch.define_method("is_null?", &rb_patch_wrapper::isNull);
  rb_cPatch.define_method("start_1", &rb_patch_wrapper::start1);
  rb_cPatch.define_method("start_2", &rb_patch_wrapper::start2);
  rb_cPatch.define_method("length_1", &rb_patch_wrapper::length1);
  rb_cPatch.define_method("length_2", &rb_patch_wrapper::length2);
  */
}

#undef dmp
