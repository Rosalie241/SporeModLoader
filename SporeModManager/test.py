#!/usr/bin/env python3
#
# SporeModManager test.py
#
import os
import sys
import shutil
import zipfile
import argparse
import subprocess
import tempfile
import atexit

#
# Global Variables
#

# argument options
sporemodmanager = ''
verbose         = False
cleanup         = True
network         = False
valgrind        = False

# paths for tests
tests_path       = tempfile.mkdtemp()
corelibs_path    = os.path.join(tests_path, 'CoreLibs')
sporemodapi_file = os.path.join(corelibs_path, 'SporeModAPI.dll')
sporemod_file    = os.path.join(tests_path, 'test.sporemod')
config_file      = os.path.join(tests_path, 'configfile.xml')
modlibs_path     = os.path.join(tests_path, 'ModLibs')
data_path        = os.path.join(tests_path, 'Data')
ep1_path         = os.path.join(tests_path, 'DataEP1')

os_environment  = os.environ.copy()

# global for write_sporemod
write_mod_num   = 0

#
# Helper Functions
#

def cleanup_smm():
	if cleanup:
		shutil.rmtree(tests_path)

def reset_smm():
	if os.path.isfile(config_file):
		os.remove(config_file)

def run_smm(args):
	os_environment["SPOREMODMANAGER_CONFIGFILE"] = str(config_file)
	valgrind_cmd = [ 'valgrind', '--quiet', '--leak-check=full', '--show-leak-kinds=all', '--track-origins=yes', '--error-exitcode=2' ]
	smm_cmd = [ sporemodmanager, '--verbose', '--no-input', f'--corelibs-path={corelibs_path}', f'--modlibs-path={modlibs_path}', f'--data-path={data_path}', f'--ep1-path={ep1_path}' ]
	cmd = [ ]
	if valgrind:
		cmd += valgrind_cmd
	cmd += smm_cmd
	for arg in args:
		cmd.append(arg)
	if verbose:
		print(f'Running {" ".join(cmd)}')
	result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=os_environment)
	if verbose:
		print(f'return code: {result.returncode}')
		if result.stdout != b'':
			print(f'stdout:\n{result.stdout.decode("utf-8").rstrip()}')
		if result.stderr != b'':
			print(f'stderr:\n{result.stderr.decode("utf-8").rstrip()}')
	return result

def write_sporemod(xml, extra = None, createNew = False):
	file = sporemod_file
	if createNew:
		global write_mod_num
		file = os.path.join(tests_path, f'test_{write_mod_num}.sporemod')
		write_mod_num = write_mod_num + 1
	with zipfile.ZipFile(file, mode="w") as archive:
		if xml is not None:
			archive.writestr("ModInfo.xml", xml)
		if extra is not None:
			for list_str in extra:
				archive.writestr(list_str[0], list_str[1])
	return file

def write_sporemodapi_dll(path):
	file = open(path, 'wb')
	# FileVersion 2.5.300
	bytes = b'F\0i\0l\0e\0V\0e\0r\0s\0i\0o\0n\0\0\0\0\0002\0.\0005\0.\000300\0'
	file.write(bytes)
	file.close()

#
# Test Functions
#

# Tests whether help works correctly
def test_help():
	print(f'Running {test_help.__name__}...')
	reset_smm()

	result = run_smm([ 'help' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

# Tests whether version works correctly
def test_version():
	print(f'Running {test_version.__name__}...')
	reset_smm()

	result = run_smm([ 'version' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

# Tests whether install works correctly
def test_install():
	print(f'Running {test_install.__name__}...')
	reset_smm()

	# install mod
	xml = """<mod displayName="test_install_0" 
				unique="test_install_0" 
				description="test_install_0" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# installing the same mod should fail
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# but using --needed should work
	result = run_smm([ 'install', '--needed', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# --update-needed should also work
	result = run_smm([ 'install', '--update-needed', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# check if the <prerequisite /> element works
	xml = """<mod displayName="test_install_1" 
				unique="test_install_1" 
				description="test_install_1" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_install_1_1.dll</prerequisite>
				<prerequisite game="invalid">test_install_1_2.dll</prerequisite>
				<prerequisite game="Spore">test_install_1.package</prerequisite>
				<prerequisite game="spore">test_install_1_lowercase.package</prerequisite>
				<prerequisite game="GalacticAdventures">test_install_1_ep1.package</prerequisite>
				<prerequisite game="galacticAdventures">test_install_1_ep1_mixedcase.package</prerequisite>
			</mod>"""
	files = [
		[ 'test_install_1_1.dll', 'dll' ],
		[ 'test_install_1_2.dll', 'dll' ],
		[ 'test_install_1.package', 'package' ],
		[ 'test_install_1_lowercase.package', 'package' ],
		[ 'test_install_1_ep1.package', 'package_ep1' ],
		[ 'test_install_1_ep1_mixedcase.package', 'package_ep1' ]
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_1_1.dll'))
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_1_2.dll'))
	assert os.path.isfile(os.path.join(data_path, 'test_install_1.package'))
	assert os.path.isfile(os.path.join(data_path, 'test_install_1_lowercase.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_1_ep1.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_1_ep1_mixedcase.package'))

	# check if the <compatFile /> element works
	os.close(os.open(os.path.join(data_path, 'test_install_2_compatfile'), os.O_CREAT))
	os.close(os.open(os.path.join(ep1_path, 'test_install_2_compatfile'), os.O_CREAT))
	xml = """<mod displayName="test_install_2" 
				unique="test_install_2" 
				description="test_install_2" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<compatFile game="spore" compatTargetFileName="test_install_2_compatfile" 
					compatTargetGame="spore">test_install_2_1.package</compatFile>
				<compatFile game="GalacticAdventures" compatTargetFileName="test_install_2_compatfile" 
					compatTargetGame="GalacticAdventures">test_install_2_ep1_1.package</compatFile>
				<compatFile game="spore" compatTargetFileName="test_no_install_2_compatfile" 
					compatTargetGame="spore">test_install_2_2.package</compatFile>
				<compatFile game="GalacticAdventures" compatTargetFileName="test_no_install_2_compatfile" 
					compatTargetGame="GalacticAdventures">test_install_2_ep1_2.package</compatFile>
			</mod>"""
	files = [
		[ 'test_install_2_1.package', 'package' ],
		[ 'test_install_2_2.package', 'package' ],
		[ 'test_install_2_ep1_1.package', 'package_ep1' ],
		[ 'test_install_2_ep1_2.package', 'package_ep1' ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert os.path.isfile(os.path.join(data_path, 'test_install_2_1.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_2_ep1_1.package'))
	assert not os.path.isfile(os.path.join(data_path, 'test_install_2_2.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_2_ep1_2.package'))

	# check if a disabled hasCustomInstaller attribute works
	xml = """<mod displayName="test_install_3"
				unique="test_install_3"
				description="test_install_3"
				installerSystemVersion="1.0.1.1"
				hasCustomInstaller="false" 
				dllsBuild="2.5.20">
				<component unique="test_install_3_component_1" 
					displayName="test_install_3_component_1" 
					description="test_install_3_component_1"
					game="GalacticAdventures" defaultChecked="true">test_install_3_ep1_1.package</component>
				</mod>"""	
	files = [
		[ 'test_install_3_ep1_1.package', 'package_ep1' ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_3_ep1_1.package'))

	# check if the <component /> element works
	xml = """<mod displayName="test_install_4"
				unique="test_install_4"
				description="test_install_4"
				installerSystemVersion="1.0.1.1" 
				hasCustomInstaller="true"
				dllsBuild="2.5.20">
				<component unique="test_install_4_component_1" 
					displayName="test_install_4_component_1" 
					description="test_install_4_component_1"
					game="GalacticAdventures" defaultChecked="true">test_install_4_ep1_1.package</component>
				<component unique="test_install_4_component_2" 
					displayName="test_install_4_component_2" 
					description="test_install_4_component_2"
					game="GalacticAdventures" defaultChecked="false">test_install_4_ep1_2.package</component>
				<component unique="test_install_4_component_3" 
					displayName="test_install_4_component_3" 
					description="test_install_4_component_3"
					game="GalacticAdventures" defaultChecked="true">test_install_4_ep1_3.package</component>
			</mod>"""
	files = [
		[ 'test_install_4_ep1_1.package', 'package_ep1' ],
		[ 'test_install_4_ep1_2.package', 'package_ep1' ],
		[ 'test_install_4_ep1_3.package', 'package_ep1' ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_4_ep1_1.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_4_ep1_2.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_4_ep1_3.package'))

	# check if the <componentGroup /> element works
	xml = """<mod displayName="test_install_5"
				unique="test_install_5"
				description="test_install_4"
				installerSystemVersion="1.0.1.1" 
				hasCustomInstaller="true"
				dllsBuild="2.5.20">
				<componentGroup unique="test_install_5_componentgroup_1" displayName="test_install_5_componentgroup_1">
					<component unique="test_install_5_componentgroup_1_component_1" displayName="test_install_5_componentgroup_1_component_1" description="" game="GalacticAdventures">test_install_5_ep1_1.package</component>
					<component unique="test_install_5_componentgroup_1_component_2" displayName="test_install_5_componentgroup_1_component_2" description="" game="GalacticAdventures" defaultChecked="true">test_install_5_ep1_2.package</component>
					<component unique="test_install_5_componentgroup_1_component_3" displayName="test_install_5_componentgroup_1_component_3" description="" game="GalacticAdventures">test_install_5_ep1_3.package</component>
				</componentGroup>
				<componentGroup unique="test_install_5_componentgroup_2" displayName="test_install_5_componentgroup_2">
					<component unique="test_install_5_componentgroup_2_component_1" displayName="test_install_5_componentgroup_2_component_1" description="" game="GalacticAdventures" defaultChecked="true">test_install_5_ep1_4.package</component>
					<component unique="test_install_5_componentgroup_2_component_2" displayName="test_install_5_componentgroup_2_component_2" description="" game="GalacticAdventures">test_install_5_ep1_5.package</component>
					<component unique="test_install_5_componentgroup_2_component_3" displayName="test_install_5_componentgroup_2_component_3" description="" game="GalacticAdventures">test_install_5_ep1_6.package</component>
				</componentGroup>
			</mod>"""
	files = [
		[ 'test_install_5_ep1_1.package', 'package_ep1' ],
		[ 'test_install_5_ep1_2.package', 'package_ep1' ],
		[ 'test_install_5_ep1_3.package', 'package_ep1' ],
		[ 'test_install_5_ep1_4.package', 'package_ep1' ],
		[ 'test_install_5_ep1_5.package', 'package_ep1' ],
		[ 'test_install_5_ep1_6.package', 'package_ep1' ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_1.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_2.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_3.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_4.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_5.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_6.package'))

	# check if a hasCustomInstaller is enabled when installer version is 1.0.0.0
	xml = """<mod displayName="test_install_6"
				unique="test_install_6"
				description="test_install_6"
				installerSystemVersion="1.0.0.0">
				<component unique="test_install_6_component_1" 
					displayName="test_install_6_component_1" 
					description="test_install_6_component_1"
					game="GalacticAdventures" defaultChecked="true">test_install_6_ep1_1.package</component>
				</mod>"""	
	files = [
		[ 'test_install_6_ep1_1.package', 'package_ep1' ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_6_ep1_1.package'))

	# check if a pre-modinfo.xml mod works
	files = [
		[ 'test_install_7.dll', 'dll' ],
		[ 'test_install_7.txt', 'txt'],
		[ 'test_install_7_ep1.package', 'package_ep1' ],
	]
	write_sporemod(None, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_7.dll'))
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_install_7.txt'))
	assert not os.path.isfile(os.path.join(data_path, 'test_install_7.txt'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_7.txt'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_7_ep1.package'))

	# verify that an invalid dllsBuild doesn't work
	xml = """<mod displayName="test_install_8" 
				unique="test_install_8" 
				description="test_install_8" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="999.999.999">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# verify that file collision with other mods fails
	xml = """<mod displayName="test_install_9" 
				unique="test_install_9" 
				description="test_install_9" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
					<prerequisite>test_install_1_1.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_install_1_1.dll', 'dll' ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# verify that a non-existent file fails
	xml = """<mod displayName="test_install_10" 
				unique="test_install_10" 
				description="test_install_10" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_install_10.dll</prerequisite>
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout != b''
	assert result.stderr != b''

	# verify that an empty modinfo.xml fails
	xml = ''
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# verify that an invalid modinfo.xml fails
	xml = '<mod <prerequisite> />'
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

# Tests whether uninstall works correctly
def test_uninstall():
	print(f'Running {test_uninstall.__name__}...')
	reset_smm()

	# uninstall should fail when no mods have been installed
	result = run_smm([ 'uninstall', '0'])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# install multiple dummy mods
	install_cmd = [ 'install' ]
	for num in range(25):
		xml = f"""<mod displayName="test_uninstall_{num}" 
					unique="test_uninstall_{num}" 
					description="test_uninstall_{num}" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.20">
				</mod>"""
		install_cmd += [ write_sporemod(xml, None, True) ]
	result = run_smm(install_cmd)
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# uninstall with a too high ID shouldn't work
	result = run_smm([ 'uninstall', '26'])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# uninstall with a valid ID should work
	result = run_smm([ 'uninstall', '0'])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# uninstall with a valid range should work
	result = run_smm([ 'uninstall', '0-20'])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# uninstall with a range with numbers + letters shouldn't work
	result = run_smm([ 'uninstall', '0x1-1x0'])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# uninstall with a range with letters shouldn't work
	result = run_smm([ 'uninstall', 'a-b'])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# uninstall with an invalid range shouldn't work
	result = run_smm([ 'uninstall', '0-30'])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# uninstall with another invalid range shouldn't work
	result = run_smm([ 'uninstall', '10-0'])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# uninstall with an insanely big range shouldn't work or crash
	result = run_smm([ 'uninstall', '0-2147483646'])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''


# Tests whether update works correctly
def test_update():
	print(f'Running {test_update.__name__}...')
	reset_smm()

	# updating a non-existant mod should fail
	xml = """<mod displayName="test_update_0" 
				unique="test_update_0" 
				description="test_update_0" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# attempt to install mod
	xml = """<mod displayName="test_update_1" 
				unique="test_update_1" 
				description="test_update_1" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# updating an existing mod should succeed
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''


# Tests whether list-installed works correctly
def test_list_installed():
	print(f'Running {test_list_installed.__name__}...')
	reset_smm()

	# ensure we currently dont have any mods installed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout == b''

	# attempt to install a mod
	xml = """<mod displayName="test_list_installed_0" 
				unique="test_list_installed_0" 
				description="test_list_installed_0" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# ensure we have a mod installed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert b'test_list_installed_0' in result.stdout
	assert result.stderr == b''


	# attempt to install a broken mod
	xml = """<mod displayName="test_list_installed_1" 
				unique="test_list_installed_1" 
				description="test_list_installed_1" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
					<prerequisite>test_list_installed_1.dll</prerequisite>
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout != b''
	assert result.stderr != b''

	# ensure the broken mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert b'test_list_installed_1' not in result.stdout
	assert result.stderr == b''

	# attempt to install multiple mods with
	# a broken one in the middle
	install_cmd = [ 'install' ]
	for num in range(3):
		if num == 1: # invalid file referenced
			xml = f"""<mod displayName="test_list_installed_2_{num}" 
						unique="test_list_installed_2_{num}" 
						description="test_list_installed_2_{num}" 
						installerSystemVersion="1.0.1.1" 
						dllsBuild="2.5.20">
						<prerequisite>test_list_installed_2.dll</prerequisite>
					</mod>"""
		else:
			xml = f"""<mod displayName="test_list_installed_2_{num}" 
						unique="test_list_installed_2_{num}" 
						description="test_list_installed_2_{num}" 
						installerSystemVersion="1.0.1.1" 
						dllsBuild="2.5.20">
					</mod>"""
		install_cmd += [ write_sporemod(xml, None, True) ]
	result = run_smm(install_cmd)
	assert result.returncode == 1
	assert result.stdout != b''
	assert result.stderr != b''

	# ensure the correct mods are listed in list-installed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert b'test_list_installed_2_0' in result.stdout
	assert b'test_list_installed_2_1' not in result.stdout
	assert b'test_list_installed_2_2' not in result.stdout

	# attempt to install another broken mod
	xml = """<mod displayName="test_list_installed_3" 
				unique="test_list_installed_3" 
				description="test_list_installed_3" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="999.999.999">
					<prerequisite>test_list_installed_3.dll</prerequisite>
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# ensure the broken mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert b'test_list_installed_3' not in result.stdout
	assert result.stderr == b''

	# attempt to uninstall a mod
	result = run_smm([ 'uninstall', '0' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# ensure the uninstalled mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert b'test_list_installed_0' not in result.stdout
	assert result.stderr == b''

# Tests whether update-modapi works correctly
def test_update_modapi():
	print(f'Running {test_update_modapi.__name__}...')
	reset_smm()

	# ensure mod with higher dll build required doesn't work
	xml = """<mod displayName="test_update_modapi_0" 
				unique="test_update_modapi_0" 
				description="test_update_modapi_0" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.400">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == b''
	assert result.stderr != b''

	# attempt to update modapi dll
	result = run_smm([ 'update-modapi' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# ensure updating when we dont have a modapi dll works
	os.remove(sporemodapi_file)
	result = run_smm([ 'update-modapi' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr != b''

	# ensure updating when already up-to-date doesn't touch the file
	before_update_mtime = os.path.getmtime(sporemodapi_file)
	result = run_smm([ 'update-modapi' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert before_update_mtime == os.path.getmtime(sporemodapi_file)

	# ensure mod with higher dll build required works
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

#
# main
#

if __name__ == "__main__":
	# register exit handler for cleanup
	atexit.register(cleanup_smm)

	# add argument parser
	parser = argparse.ArgumentParser(description='Runs SporeModManager tests.')
	parser.add_argument('--nocleanup', action='store_false', help="skips cleanup of temporary directory")
	parser.add_argument('--verbose', action='store_true', help='prints command output for each test')
	parser.add_argument('--network', action='store_true', help='runs tests which require network access')
	parser.add_argument('--valgrind', action='store_true', help='runs tests with valgrind')
	parser.add_argument('executable', help='executable to run tests with.')
	args = parser.parse_args()

	# set global variables
	sporemodmanager = args.executable
	verbose 		= args.verbose
	cleanup         = args.nocleanup
	network         = args.network
	valgrind        = args.valgrind

	# create test directories
	os.mkdir(corelibs_path)
	os.mkdir(modlibs_path)
	os.mkdir(data_path)
	os.mkdir(ep1_path)

	# write spore mod api dll
	write_sporemodapi_dll(sporemodapi_file)

	# call tests
	test_help()
	test_version()
	test_install()
	test_uninstall()
	test_update()
	test_list_installed()
	if network:
		test_update_modapi()
