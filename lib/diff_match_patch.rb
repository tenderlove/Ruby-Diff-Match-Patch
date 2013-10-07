require 'diff_match_patch_native'

class DiffMatchPatch
  VERSION = '1.0.1'

  def patch_make(*args)
    case
    when args[0].kind_of?(String) && args[1].kind_of?(String)
      __patch_make_from_texts__(args[0], args[1])
    when args[0].kind_of?(String) && args[1].kind_of?(Array)
      __patch_make_from_text_and_diff__(args[0], args[1])
    when args[0].kind_of?(Array) && args[1].nil?
      __patch_make_from_diffs__(args[0])
    else
      raise ArgumentError.new "patch_make accepts two strings, a string and a diff, or a set of diffs only"
    end
  end

end
