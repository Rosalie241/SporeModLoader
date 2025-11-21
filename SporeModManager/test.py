#!/usr/bin/env python3
#
# SporeModManager test.py
#
import os
import shutil
import zipfile
import argparse
import subprocess
import tempfile
import atexit
import uuid

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
mods_path        = os.path.join(tests_path, 'mods')
mods2_path       = os.path.join(tests_path, 'mods2')
corelibs_path    = os.path.join(tests_path, 'CoreLibs')
sporemodapi_file = os.path.join(corelibs_path, 'SporeModAPI.dll')
sporemod_file    = os.path.join(mods_path, 'test.sporemod')
package_file     = os.path.join(mods_path, 'test_package.package')
invalid_file     = os.path.join(mods_path, 'test.invalid')
package_file_2   = os.path.join(mods2_path, 'test_package.package')
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

	global write_mod_num
	write_mod_num = 0

def run_smm(args):
	os_environment["SPOREMODMANAGER_CONFIGFILE"] = str(config_file)
	valgrind_cmd = [ 'valgrind', '--quiet', '--leak-check=full', '--show-leak-kinds=all', '--track-origins=yes', '--error-exitcode=2' ]
	smm_cmd = [ sporemodmanager, '--verbose', '--no-input', f'--corelibs-path={corelibs_path}', f'--modlibs-path={modlibs_path}', f'--data-path={data_path}', f'--ep1-path={ep1_path}' ]
	cmd = [ ]
	if valgrind:
		cmd += valgrind_cmd
	cmd += smm_cmd
	cmd += args
	if verbose:
		print(f'Running {" ".join(cmd)}')
	result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=os_environment, text=True)
	if verbose:
		print(f'return code: {result.returncode}')
		if result.stdout != '':
			print(f'stdout:\n{result.stdout.rstrip()}')
		if result.stderr != '':
			print(f'stderr:\n{result.stderr.rstrip()}')
	return result

def write_sporemod(xml = None, extra = None, createNew = False, invalidZip = False):
	file = sporemod_file
	if createNew:
		global write_mod_num
		file = os.path.join(mods_path, f'test_{write_mod_num}.sporemod')
		write_mod_num = write_mod_num + 1
	if invalidZip:
		mod_file = open(file, 'wb')
		mod_file.write(b'invalidZip')
		mod_file.close()
	else:
		with zipfile.ZipFile(file, mode="w") as archive:
			if xml is not None:
				archive.writestr("ModInfo.xml", xml)
			if extra is not None:
				for list_str in extra:
					archive.writestr(list_str[0], list_str[1])
	return file

def write_package(path):
	with open(path, 'wb') as file:
		file.write(b'package')

def write_invalid(path):
	with open(path, 'wb') as file:
		file.write(b'invalid')

def write_sporemodapi_dll(path):
	with open(path, 'wb') as file:
		# FileVersion 2.5.300
		bytes = b'F\0i\0l\0e\0V\0e\0r\0s\0i\0o\0n\0\0\0\0\0002\0.\0005\0.\000300\0'
		file.write(bytes)

def check_file_contents(path, content):
	with open(path, 'r') as file:
		file_content = file.read()
		if verbose:
			print(f'expected file contents: {content}\nactual file contents: {file_content}')
		return file_content == content

#
# Test Functions
#

# Tests whether help works correctly
def test_help():
	print(f'Running {test_help.__name__}...')
	reset_smm()

	result = run_smm([ 'help' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

# Tests whether version works correctly
def test_version():
	print(f'Running {test_version.__name__}...')
	reset_smm()

	result = run_smm([ 'version' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

# Tests whether install works correctly
def test_install():
	print(f'Running {test_install.__name__}...')
	reset_smm()

	# installing mod with invalid extension should fail
	result = run_smm([ 'install', invalid_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test.invalid'))
	assert not os.path.isfile(os.path.join(data_path, 'test.invalid'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test.invalid'))

	# install package mod
	result = run_smm([ 'install', package_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(ep1_path, 'test_package.package'))

	# installing sporemod that isn't a zip should fail
	write_sporemod(invalidZip=True)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# install sporemod mod
	xml = """<mod displayName="test_install_0" 
				unique="test_install_0" 
				description="test_install_0" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# installing the same mod should fail
	result = run_smm([ 'install', package_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# but using --needed should work
	result = run_smm([ 'install', '--needed', package_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	result = run_smm([ 'install', '--needed', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# --update-needed should also work
	result = run_smm([ 'install', '--update-needed', package_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	result = run_smm([ 'install', '--update-needed', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

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
		[ 'test_install_1_1.dll', str(uuid.uuid4()) ],
		[ 'test_install_1_2.dll', str(uuid.uuid4()) ],
		[ 'test_install_1.package', str(uuid.uuid4()) ],
		[ 'test_install_1_lowercase.package', str(uuid.uuid4()) ],
		[ 'test_install_1_ep1.package', str(uuid.uuid4()) ],
		[ 'test_install_1_ep1_mixedcase.package', str(uuid.uuid4()) ]
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_1_1.dll'))
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_1_2.dll'))
	assert os.path.isfile(os.path.join(data_path, 'test_install_1.package'))
	assert os.path.isfile(os.path.join(data_path, 'test_install_1_lowercase.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_1_ep1.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_1_ep1_mixedcase.package'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])
	assert check_file_contents(os.path.join(modlibs_path, files[1][0]), files[1][1])
	assert check_file_contents(os.path.join(data_path, files[2][0]), files[2][1])
	assert check_file_contents(os.path.join(data_path, files[3][0]), files[3][1])
	assert check_file_contents(os.path.join(ep1_path, files[4][0]), files[4][1])
	assert check_file_contents(os.path.join(ep1_path, files[5][0]), files[5][1])

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
		[ 'test_install_2_1.package', str(uuid.uuid4()) ],
		[ 'test_install_2_2.package', str(uuid.uuid4()) ],
		[ 'test_install_2_ep1_1.package', str(uuid.uuid4()) ],
		[ 'test_install_2_ep1_2.package', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(data_path, 'test_install_2_1.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_2_ep1_1.package'))
	assert not os.path.isfile(os.path.join(data_path, 'test_install_2_2.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_2_ep1_2.package'))
	assert check_file_contents(os.path.join(data_path, files[0][0]), files[0][1])
	assert check_file_contents(os.path.join(ep1_path, files[2][0]), files[2][1])

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
		[ 'test_install_3_ep1_1.package', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
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
		[ 'test_install_4_ep1_1.package', str(uuid.uuid4()) ],
		[ 'test_install_4_ep1_2.package', str(uuid.uuid4()) ],
		[ 'test_install_4_ep1_3.package', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_4_ep1_1.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_4_ep1_2.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_4_ep1_3.package'))
	assert check_file_contents(os.path.join(ep1_path, files[0][0]), files[0][1])
	assert check_file_contents(os.path.join(ep1_path, files[2][0]), files[2][1])

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
		[ 'test_install_5_ep1_1.package', str(uuid.uuid4()) ],
		[ 'test_install_5_ep1_2.package', str(uuid.uuid4()) ],
		[ 'test_install_5_ep1_3.package', str(uuid.uuid4()) ],
		[ 'test_install_5_ep1_4.package', str(uuid.uuid4()) ],
		[ 'test_install_5_ep1_5.package', str(uuid.uuid4()) ],
		[ 'test_install_5_ep1_6.package', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_1.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_2.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_3.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_4.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_5.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_5_ep1_6.package'))
	assert check_file_contents(os.path.join(ep1_path, files[1][0]), files[1][1])
	assert check_file_contents(os.path.join(ep1_path, files[3][0]), files[3][1])

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
		[ 'test_install_6_ep1_1.package', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_6_ep1_1.package'))
	assert check_file_contents(os.path.join(ep1_path, files[0][0]), files[0][1])

	# check if a pre-modinfo.xml mod works
	files = [
		[ 'test_install_7.dll', str(uuid.uuid4()) ],
		[ 'test_install_7.txt', str(uuid.uuid4()) ],
		[ 'test_install_7_ep1.package', str(uuid.uuid4()) ],
	]
	write_sporemod(None, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_7.dll'))
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_install_7.txt'))
	assert not os.path.isfile(os.path.join(data_path, 'test_install_7.txt'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_7.txt'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_7_ep1.package'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])
	assert check_file_contents(os.path.join(ep1_path, files[2][0]), files[2][1])

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
	assert result.stdout == ''
	assert result.stderr != ''

	# verify that file collision with other mods fails
	xml = """<mod displayName="test_install_9" 
				unique="test_install_9" 
				description="test_install_9" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_install_1_1.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_install_1_1.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

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
	assert result.stdout != ''
	assert result.stderr != ''

	# verify that an empty modinfo.xml fails
	xml = ''
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# verify that an invalid modinfo.xml fails
	xml = '<mod <prerequisite> />'
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# verify that 2 sporemods with the same unique name install correctly,
	# the latter one of them should be skipped
	xml1 = """<mod displayName="test_install_11" 
				unique="test_install_11" 
				description="test_install_11" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_install_11_1.dll</prerequisite>
			</mod>"""
	xml2 = """<mod displayName="test_install_11" 
				unique="test_install_11" 
				description="test_install_11" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_install_11_2.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_install_11_1.dll', str(uuid.uuid4()) ],
		[ 'test_install_11_2.dll', str(uuid.uuid4()) ],
	]
	result = run_smm([ 'install', write_sporemod(xml1, files, True), write_sporemod(xml2, files, True) ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_11_1.dll'))
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_install_11_2.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# verify that duplicate filenames are skipped
	xml = """<mod displayName="test_install_12" 
				unique="test_install_12" 
				description="test_install_12" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_install_12.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_install_12.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file, sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_12.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

# Tests whether uninstall works correctly
def test_uninstall():
	print(f'Running {test_uninstall.__name__}...')
	reset_smm()

	# uninstall should fail when no mods have been installed
	result = run_smm([ 'uninstall', '0'])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# install multiple dummy mods
	install_cmd = [ 'install' ]
	for num in range(25):
		xml = f"""<mod displayName="test_uninstall_{num:02}" 
					unique="test_uninstall_{num:02}" 
					description="test_uninstall_{num:02}" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.20">
					<prerequisite>test_uninstall_{num:02}.dll</prerequisite>
				</mod>"""
		files = [
			[ f'test_uninstall_{num:02}.dll', str(uuid.uuid4()) ],
		]
		install_cmd += [ write_sporemod(xml, files, True) ]
	result = run_smm(install_cmd)
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	for num in range(25):
		assert os.path.isfile(os.path.join(modlibs_path, f'test_uninstall_{num:02}.dll'))

	result = run_smm([ 'install', package_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# uninstall with a too high ID shouldn't work
	result = run_smm([ 'uninstall', '26' ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# uninstall with a valid ID should work
	result = run_smm([ 'uninstall', '0' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert not os.path.isfile(os.path.join(ep1_path, 'test_package.package'))

	# uninstall with a valid range should work
	result = run_smm([ 'uninstall', '0-20' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	for num in range(21):
		assert not os.path.isfile(os.path.join(modlibs_path, f'test_uninstall_{num:02}.dll'))

	# uninstall with a range with numbers + letters shouldn't work
	result = run_smm([ 'uninstall', '0x1-1x0' ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# uninstall with a range with letters shouldn't work
	result = run_smm([ 'uninstall', 'a-b' ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# uninstall with an invalid range shouldn't work
	result = run_smm([ 'uninstall', '0-30'])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# uninstall with another invalid range shouldn't work
	result = run_smm([ 'uninstall', '10-0'])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# uninstall with an insanely big range shouldn't work or crash
	result = run_smm([ 'uninstall', '0-2147483646' ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''


# Tests whether update works correctly
def test_update():
	print(f'Running {test_update.__name__}...')
	reset_smm()

	# updating mod with invalid extension should fail
	result = run_smm([ 'update', invalid_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test.invalid'))
	assert not os.path.isfile(os.path.join(data_path, 'test.invalid'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test.invalid'))

	# updating a sporemod that isn't a zip should fail
	write_sporemod(invalidZip=True)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# updating a non-existant package mod should fail
	result = run_smm([ 'update', package_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# updating a non-existant sporemod mod should fail
	xml = """<mod displayName="test_update_0" 
				unique="test_update_0" 
				description="test_update_0" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# attempt to install package mod
	result = run_smm([ 'install', package_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# attempt to install sporemod mod
	xml = """<mod displayName="test_update_1" 
				unique="test_update_1" 
				description="test_update_1" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_update_1_1.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_update_1_1.dll', str(uuid.uuid4()) ],
		[ 'test_update_1_2.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_update_1_1.dll'))
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_update_1_2.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# updating an existing mod should succeed
	result = run_smm([ 'update', package_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# ensure that updating updates the files correctly
	xml = xml.replace('test_update_1_1.dll', 'test_update_1_2.dll')
	write_sporemod(xml, files)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_update_1_1.dll'))
	assert os.path.isfile(os.path.join(modlibs_path, 'test_update_1_2.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[1][0]), files[1][1])

	# verify that 2 sporemods with the same unique name update correctly,
	# the latter one of them should be skipped
	xml1 = """<mod displayName="test_update_1" 
				unique="test_update_1" 
				description="test_update_1" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_update_1_1.dll</prerequisite>
			  </mod>"""
	xml2 = """<mod displayName="test_update_1" 
				unique="test_update_1" 
				description="test_update_1" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_update_1_2.dll</prerequisite>
			  </mod>"""
	result = run_smm([ 'update', write_sporemod(xml1, files, True), write_sporemod(xml2, files, True) ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_update_1_1.dll'))
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_update_1_2.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# verify that 2 package mods with the same unique name update correctly
	result = run_smm([ 'update', package_file, package_file_2 ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# updating an existing mod by giving the same path twice should work
	result = run_smm([ 'update', package_file, package_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	result = run_smm([ 'update', sporemod_file, sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

# Tests whether list-installed works correctly
def test_list_installed():
	print(f'Running {test_list_installed.__name__}...')
	reset_smm()

	# ensure we currently dont have any mods installed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout == ''

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
	assert result.stdout != ''
	assert result.stderr == ''
	result = run_smm([ 'install', package_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# ensure we have a mod installed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert 'test_list_installed_0' in result.stdout
	assert 'test_package' in result.stdout
	assert result.stderr == ''

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
	assert result.stdout != ''
	assert result.stderr != ''

	# ensure the broken mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert 'test_list_installed_1' not in result.stdout
	assert result.stderr == ''

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
		install_cmd += [ write_sporemod(xml, createNew=True) ]
	result = run_smm(install_cmd)
	assert result.returncode == 1
	assert result.stdout != ''
	assert result.stderr != ''

	# ensure the correct mods are listed in list-installed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert 'test_list_installed_2_0' in result.stdout
	assert 'test_list_installed_2_1' not in result.stdout
	assert 'test_list_installed_2_2' not in result.stdout

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
	assert result.stdout == ''
	assert result.stderr != ''

	# ensure the broken mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert 'test_list_installed_3' not in result.stdout
	assert result.stderr == ''

	# attempt to uninstall a mod
	result = run_smm([ 'uninstall', '0' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# ensure the uninstalled mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert 'test_list_installed_0' not in result.stdout
	assert result.stderr == ''

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
	assert result.stdout == ''
	assert result.stderr != ''

	# attempt to update modapi dll
	result = run_smm([ 'update-modapi' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# ensure updating when we dont have a modapi dll works
	os.remove(sporemodapi_file)
	result = run_smm([ 'update-modapi' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr != ''

	# ensure updating when already up-to-date doesn't touch the file
	before_update_mtime = os.path.getmtime(sporemodapi_file)
	result = run_smm([ 'update-modapi' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert before_update_mtime == os.path.getmtime(sporemodapi_file)

	# ensure mod with higher dll build required works
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

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
	os.mkdir(mods_path)
	os.mkdir(mods2_path)
	os.mkdir(corelibs_path)
	os.mkdir(modlibs_path)
	os.mkdir(data_path)
	os.mkdir(ep1_path)

	# write spore mod api dll and package file
	write_sporemodapi_dll(sporemodapi_file)
	write_package(package_file)
	write_package(package_file_2)
	write_invalid(invalid_file)

	# call tests
	test_help()
	test_version()
	test_install()
	test_uninstall()
	test_update()
	test_list_installed()
	if network:
		test_update_modapi()
