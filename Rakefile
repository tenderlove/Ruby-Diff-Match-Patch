# -*- ruby -*-
require 'rubygems'
begin
  require 'rake/dsl_definition'
rescue LoadError 
  nil
end
require 'hoe'
require 'rake/extensiontask'

Hoe.plugin :bundler, :rubygems, :doofus, :git

Hoe.spec 'diff_match_patch_native' do
  developer('Elliot Laster', 'elliotlaster@gmail.com')
  self.version = '1.0.1'
  self.readme_file   = 'README.rdoc'
  self.history_file  = 'History.txt'
  self.extra_deps << ['rice', '>= 1.4.2']
  self.extra_dev_deps << ['rake-compiler', '>= 0']

  self.spec_extras = { :extensions => ["ext/diff_match_patch_native/extconf.rb"] }

  Rake::ExtensionTask.new('diff_match_patch_native', spec) do |ext|
    ext.lib_dir = File.join('lib', 'diff_match_patch_native')
  end
end

Rake::Task[:test].prerequisites << :compile
