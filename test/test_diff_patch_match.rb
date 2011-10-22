require "test/unit"
require File.join(File.dirname(File.expand_path(__FILE__)), '../', 'lib', 'diff_match_patch')

class TestDiffMatchPatch < Test::Unit::TestCase

  def setup
    @dmp = DiffMatchPatch.new
  end

  def test_diff_main
    assert @dmp.diff_main("", "", false).empty?
    equality_result = [[0, "abc"]]
    assert_equal equality_result, @dmp.diff_main("abc", "abc", false)
    simple_insertion_result = [[0, "ab"], [1, "123"], [0, "c"]]
    assert_equal simple_insertion_result, @dmp.diff_main("abc", "ab123c", false)
  end

end
