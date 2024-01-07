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

sporemodmanager = ''
verbose         = False

sporemod_path   = tempfile.mkdtemp()
config_path     = tempfile.mkdtemp()
sporemod_file   = os.path.join(sporemod_path, 'test.sporemod')
config_file     = os.path.join(config_path, 'configfile.xml')
modlibs_path    = tempfile.mkdtemp()
data_path       = tempfile.mkdtemp()
ep1_path        = tempfile.mkdtemp()


os_environment  = os.environ.copy()

#
# Helper Functions
#

def cleanup_smm():
	shutil.rmtree(sporemod_path)
	shutil.rmtree(config_path)
	shutil.rmtree(modlibs_path)
	shutil.rmtree(data_path)
	shutil.rmtree(ep1_path)

def reset_smm():
	if os.path.isfile(config_file):
		os.remove(config_file)

def run_smm(args):
	os_environment["SPOREMODMANAGER_CONFIGFILE"] = str(config_file)
	cmd = [sporemodmanager, '--verbose', '--no-input', f'--modlibs-path={modlibs_path}', f'--data-path={data_path}', f'--ep1-path={ep1_path}']
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

def write_sporemod(xml, extra = None):
	with zipfile.ZipFile(sporemod_file, mode="w") as archive:
		if xml is not None:
			archive.writestr("ModInfo.xml", xml)
		if extra is not None:
			for list_str in extra:
				archive.writestr(list_str[0], list_str[1])

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
	assert result.returncode != 0
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
		[ 'test_install_7_ep1.package', 'package_ep1' ],
	]
	write_sporemod(None, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_7.dll'))
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
	assert result.returncode != 0
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
	assert result.returncode != 0
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
	assert result.returncode != 0
	assert result.stdout != b''
	assert result.stderr != b''

	# verify that an empty modinfo.xml fails
	xml = ''
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

# Tests whether uninstall works correctly
def test_uninstall():
	print(f'Running {test_uninstall.__name__}...')
	reset_smm()

	# uninstall should fail when no mods have been installed
	result = run_smm([ 'uninstall', '0'])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# install a dummy mods
	# TODO: rewrite this to use a single install command
	for num in range(25):
		xml = f"""<mod displayName="test_uninstall_{num}" 
					unique="test_uninstall_{num}" 
					description="test_uninstall_{num}" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.20">
				</mod>"""
		write_sporemod(xml)
		result = run_smm([ 'install', sporemod_file ])
		assert result.returncode == 0
		assert result.stdout != b''
		assert result.stderr == b''

	# uninstall with a too high ID shouldn't work
	result = run_smm([ 'uninstall', '26'])
	assert result.returncode != 0
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

	# uninstall with an invalid range shouldn't work
	result = run_smm([ 'uninstall', '0-30'])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# uninstall with another invalid range shouldn't work
	result = run_smm([ 'uninstall', '10-0'])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# uninstall with an insanely big range shouldn't work or crash
	result = run_smm([ 'uninstall', '0-2147483646'])
	assert result.returncode != 0
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
	assert result.returncode != 0
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
	assert result.returncode != 0
	assert result.stdout != b''
	assert result.stderr != b''

	# ensure the broken mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert b'test_list_installed_1' not in result.stdout
	assert result.stderr == b''

	# attempt to install another broken mod
	xml = """<mod displayName="test_list_installed_2" 
				unique="test_list_installed_2" 
				description="test_list_installed_2" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="999.999.999">
					<prerequisite>test_list_installed_2.dll</prerequisite>
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# ensure the broken mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert b'test_list_installed_2' not in result.stdout
	assert result.stderr == b''

	# attempt to uninstall a mod
	result = run_smm([ 'uninstall', '0' ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# ensure the uninstalled mod isn't listed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout == b''
	assert b'test_list_installed_0' not in result.stdout
	assert result.stderr == b''

#
# main
#

if __name__ == "__main__":
	# register exit handler for cleanup
	atexit.register(cleanup_smm)

	# add argument parser
	parser = argparse.ArgumentParser(description='Runs SporeModManager tests.')
	parser.add_argument('--verbose', action='store_true', help='prints command output for each test.')
	parser.add_argument('executable', help='executable to run tests with.')
	args = parser.parse_args()

	# set global variables
	sporemodmanager = args.executable
	verbose 		= args.verbose

	# call tests
	test_help()
	test_version()
	test_install()
	test_uninstall()
	test_update()
	test_list_installed()
