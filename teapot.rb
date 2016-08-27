# Teapot v1.2.6 configuration generated at 2016-08-28 01:01:21 +1200

required_version "1.1"

# Project Metadata

define_project 'FAT32 Recovery Tools' do |project|
	project.description = <<-EOF
		FAT32 Recovery Tools description.
	EOF
	
	project.summary = 'FAT32 Recovery Tools short description.'
	project.license = 'MIT License'
	
	# project.add_author 'Samuel Williams', email: 'samuel@oriontransfer.org', website: 'http://www.codeotaku.com'
	# project.website = 'http://teapot.nz/projects/fat32-recovery-tools'
	
	project.version = '0.1.0'
end

# Build Targets

define_target 'fat32-recovery-tools-library' do |target|
	target.build do
		source_root = target.package.path + 'source'
		copy headers: source_root.glob('FAT32RecoveryTools/**/*.{h,hpp}')
		build static_library: 'FAT32RecoveryTools', source_files: source_root.glob('FAT32RecoveryTools/**/*.cpp')
	end
	
	target.depends 'Build/Files'
	target.depends 'Build/Clang'
	
	target.depends :platform
	target.depends 'Language/C++11'

	target.provides 'Library/FAT32RecoveryTools' do
		append linkflags [
			->{install_prefix + 'lib/libFAT32RecoveryTools.a'},
		]
	end
end

define_target 'fat32-recovery-tools-test' do |target|
	target.build do
		run tests: 'FAT32RecoveryTools', source_files: target.package.path.glob('test/FAT32RecoveryTools/**/*.cpp')
	end
	
	target.depends 'Library/UnitTest'
	target.depends 'Library/FAT32RecoveryTools'

	target.provides 'Test/FAT32RecoveryTools'
end

define_target 'fat32-recovery-tools-executable' do |target|
	target.build do
		source_root = target.package.path + 'source'
		
		build executable: 'FAT32RecoveryTools', source_files: source_root.glob('FAT32RecoveryTools.cpp')
	end
	
	target.depends 'Build/Files'
	target.depends 'Build/Clang'
	
	target.depends :platform
	target.depends 'Language/C++11'
	
	target.depends 'Library/FAT32RecoveryTools'
	target.provides 'Executable/FAT32RecoveryTools'
end

# teapot build Run/FAT32RecoveryTools -- $ARGV
define_target 'fat32-recovery-tools-run' do |target|
	target.build do
		run executable: 'FAT32RecoveryTools', arguments: ARGV
	end
	
	target.depends 'Executable/FAT32RecoveryTools'
	target.provides 'Run/FAT32RecoveryTools'
end

# Configurations

define_configuration "fat32-recovery-tools" do |configuration|
	configuration[:source] = "https://github.com/kurocha"

	configuration.require "project"
	# Provides all the build related infrastructure:
	configuration.require 'platforms'
	
	# Provides unit testing infrastructure and generators:
	configuration.require 'unit-test'
	
	# Provides some useful C++ generators:
	configuration.require 'language-cpp-class'
end
