require 'mkmf'

dmp_stl_path = File.join(File.dirname(File.expand_path(__FILE__)), '..', '..', 'src', 'include')
$INCFLAGS << " " << "-I#{dmp_stl_path}".quote

create_makefile("diff_match_patch_native")
