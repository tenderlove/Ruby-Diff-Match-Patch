require "test/unit"
require File.join(File.dirname(File.expand_path(__FILE__)), '../', 'lib', 'diff_match_patch')

class TestDiffMatchPatch < Test::Unit::TestCase

  def setup
    @dmp = DiffMatchPatch.new
  end

  def test_simple_diff_main_scenarios
    assert @dmp.diff_main("", "", false).empty?
  
    equality_result = [[0, "abc"]]
    assert_equal equality_result, @dmp.diff_main("abc", "abc", false)
  
    simple_insertion_result = [[0, "ab"], [1, "123"], [0, "c"]]
    assert_equal simple_insertion_result, @dmp.diff_main("abc", "ab123c", false)
  
    simple_deletion_result = [[0, "a"], [-1,"123"], [0,"bc"]]
    assert_equal simple_deletion_result, @dmp.diff_main("a123bc", "abc", false)
    
    two_insertion_result = [[0, "a"], [1, "123"], [0, "b"], [1, "456"], [0, "c"]]
    assert_equal two_insertion_result, @dmp.diff_main("abc", "a123b456c", false)

    two_deletion_result = [[0, "a"], [-1, "123"], [0, "b"], [-1, "456"], [0, "c"]]
    assert_equal two_deletion_result, @dmp.diff_main("a123b456c", "abc", false)
  end

  def test_should_get_and_set_diff_timeout
    dmp = DiffMatchPatch.new # Use a new one so there's no interference
    assert dmp.diff_timeout.instance_of?(Float)
    assert_equal 1.0, dmp.diff_timeout

    dmp.diff_timeout = 0

    assert dmp.diff_timeout.instance_of?(Float)
    assert_equal 0, dmp.diff_timeout
  end

  def test_real_diff_main_scenarios
    # Turn off the timeout
    @dmp.diff_timeout = 0

    simple_case_1_result = [[-1, "a"], [1, "b"]]
    assert_equal simple_case_1_result, @dmp.diff_main("a", "b", false)

    simple_case_2_result = [[-1, "Apple"], [1, "Banana"], [0, "s are a"], [1, "lso"], [0, " fruit."]]
    assert_equal simple_case_2_result, @dmp.diff_main("Apples are a fruit.", "Bananas are also fruit.", false)
  
    # TODO: Case 3 from diff_match_patch.cpp

    overlap_1_result = [[-1, "1"], [0, "a"], [-1, "y"], [0, "b"], [-1, "2"], [1, "xab"]]
    assert_equal overlap_1_result, @dmp.diff_main("1ayb2", "abxab", false)

    overlap_2_result = [[1, "xaxcx"], [0, "abc"], [-1, "y"]]
    assert_equal overlap_2_result, @dmp.diff_main("abcy", "xaxcxabc", false)

    overlap_3_result = [[-1, "ABCD"], [0, "a"], [-1, "="], [1, "-"], [0, "bcd"], [-1, "="], [1, "-"], [0, "efghijklmnopqrs"], [-1, "EFGHIJKLMNOefg"]]
    assert_equal overlap_3_result, @dmp.diff_main("ABCDa=bcd=efghijklmnopqrsEFGHIJKLMNOefg", "a-bcd-efghijklmnopqrs", false)

    large_equality_result = [[1, " "], [0, "a"], [1, "nd"], [0, " [[Pennsylvania]]"], [-1, " and [[New"]]
    assert_equal large_equality_result, @dmp.diff_main("a [[Pennsylvania]] and [[New", " and [[Pennsylvania]]", false)

    # TODO: Timeout test

    # Test the linemode speedup.
    # Must be long to pass the 100 char cutoff.
    a = "1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n";
    b = "abcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\n";
    assert_equal(@dmp.diff_main(a, b, true), @dmp.diff_main(a, b, false));

    a = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
    b = "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghij";
    assert_equal(@dmp.diff_main(a, b, true), @dmp.diff_main(a, b, false));
  end

end
