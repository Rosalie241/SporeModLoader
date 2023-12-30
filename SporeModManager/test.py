#!/usr/bin/env python3
#
# SporeModManager test.py
#
import os
import sys
import shutil
import zipfile
import subprocess
import tempfile
import atexit

#
# Global Variables
#

sporemodmanager = sys.argv[1]
sporemod_file   = os.path.join(tempfile.mkdtemp(), 'test.sporemod')
config_file     = os.path.join(tempfile.mkdtemp(), 'configfile.xml')
modlibs_path    = tempfile.mkdtemp()
data_path       = tempfile.mkdtemp()
ep1_path        = tempfile.mkdtemp()

os_environment  = os.environ.copy()

#
# Helper Functions
#

def cleanup_smm():
	if os.path.isfile(config_file):
		os.remove(config_file)

	if os.path.isfile(sporemod_file):
		os.remove(sporemod_file)

	shutil.rmtree(modlibs_path)
	shutil.rmtree(data_path)
	shutil.rmtree(ep1_path)

def reset_smm():
	if os.path.isfile(config_file):
		os.remove(config_file)

def run_smm(args):
	os_environment["SPOREMODMANAGER_CONFIGFILE"] = str(config_file)
	cmd = [sporemodmanager, f'--modlibs-path={modlibs_path}', f'--data-path={data_path}', f'--ep1-path={ep1_path}']
	for arg in args:
		cmd.append(arg)
	return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=os_environment)

def write_sporemod(xml, extra = None):
	with zipfile.ZipFile(sporemod_file, mode="w") as archive:
		if xml != '':
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

# Tests whether list-installed works correctly
def test_list_installed():
	print(f'Running {test_list_installed.__name__}...')
	reset_smm()

	# ensure we currently dont have any mods installed
	result = run_smm([ 'list-installed' ])
	assert result.returncode == 0
	assert result.stdout == b''

	# attempt to install a dummy mod
	xml = """<mod displayName="test_list_installed" 
				unique="test_list_installed" 
				description="test_install_sporemod_duplicate" 
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
	assert b'test_list_installed' in result.stdout
	assert result.stderr == b''

# Tests whether installing duplicates works correctly
def test_install_sporemod_duplicate():
	print(f'Running {test_install_sporemod_duplicate.__name__}...')
	reset_smm()

	xml = """<mod displayName="test_install_sporemod_duplicate" 
				unique="test_install_sporemod_duplicate" 
				description="test_install_sporemod_duplicate" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
			</mod>"""

	write_sporemod(xml)

	# install mod
	result = run_smm([ 'install', '-v', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# installing the same mod should fail
	result = run_smm([ 'install', '-v', sporemod_file ])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# but using --needed should work
	result = run_smm([ 'install', '--needed', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout == b''
	assert result.stderr != b''

	# --update-needed should also work
	result = run_smm([ 'install', '--update-needed', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

# Tests whether installing sporemods works correctly
def test_install_sporemod():
	print(f'Running {test_install_sporemod.__name__}...')
	reset_smm()

	# check if the basic <prerequisite /> element works
	xml = """<mod displayName="test_install_sporemod_0" 
				unique="test_install_sporemod_0" 
				description="test_install_sporemod_0" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_install_sporemod.dll</prerequisite>
				<prerequisite game="CoreSpore">test_install_sporemod.package</prerequisite>
				<prerequisite game="GalacticAdventures">test_install_sporemod_ep1.package</prerequisite>
			</mod>"""
	files = [
		[ 'test_install_sporemod.dll', 'dll' ],
		[ 'test_install_sporemod.package', 'package' ],
		[ 'test_install_sporemod_ep1.package', 'package_ep1' ]
	]
	write_sporemod(xml, files)

	result = run_smm([ 'install', '-v', sporemod_file ])

	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_sporemod.dll'))
	assert os.path.isfile(os.path.join(data_path, 'test_install_sporemod.package'))
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_sporemod_ep1.package'))

	# now check if <compatFile /> works
	os.close(os.open(os.path.join(ep1_path, 'test_install_sporemod_compatfile'), os.O_CREAT))
	xml = """<mod displayName="test_install_sporemod_1" 
				unique="test_install_sporemod_1" 
				description="test_install_sporemod_1" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<compatFile game="GalacticAdventures" compatTargetFileName="test_install_sporemod_compatfile" 
					compatTargetGame="GalacticAdventures">test_install_sporemod_ep1_1.package</compatFile>
				<compatFile game="GalacticAdventures" compatTargetFileName="test_no_install_compatfile" 
					compatTargetGame="GalacticAdventures">test_install_sporemod_ep1_2.package</compatFile>
			</mod>"""
	files = [
		[ 'test_install_sporemod_ep1_1.package', 'package_ep1' ],
		[ 'test_install_sporemod_ep1_2.package', 'package_ep1' ],
	]
	write_sporemod(xml, files)

	result = run_smm([ 'install', '-v', sporemod_file ])

	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''
	assert os.path.isfile(os.path.join(ep1_path, 'test_install_sporemod_ep1_1.package'))
	assert not os.path.isfile(os.path.join(ep1_path, 'test_install_sporemod_ep1_2.package'))

	# verify that an invalid dllsBuild doesn't work
	xml = """<mod displayName="test_install_sporemod_2" 
				unique="test_install_sporemod_2" 
				description="test_install_sporemod_2" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="9.9.999">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])

	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

# Tests whether installing sporemods with invalid modinfo.xml works correctly
def test_install_sporemod_invalid_modinfo():
	print(f'Running {test_install_sporemod_invalid_modinfo.__name__}...')
	reset_smm()

	# no modinfo.xml should fail
	xml = ''
	write_sporemod(xml)
	result = run_smm([ 'install', '-v', sporemod_file ])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# empty modinfo.xml should also fail
	xml = " "
	write_sporemod(xml)
	result = run_smm([ 'install', '-v', sporemod_file ])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# TODO: add more validation to SporeModManager
	# and then add more tests here

# Tests whether uninstall works correctly
def test_uninstall():
	print(f'Running {test_uninstall.__name__}...')
	reset_smm()

	# uninstall should fail when no mods have been installed
	result = run_smm([ 'uninstall', '0'])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# install a dummy mod
	xml = """<mod displayName="test_uninstall" 
				unique="test_uninstall" 
				description="test_uninstall" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
			</mod>"""
	write_sporemod(xml)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

	# uninstall with a too high ID shouldn't work
	result = run_smm([ 'uninstall', '1'])
	assert result.returncode != 0
	assert result.stdout == b''
	assert result.stderr != b''

	# uninstall with a valid ID should work
	result = run_smm([ 'uninstall', '0'])
	assert result.returncode == 0
	assert result.stdout != b''
	assert result.stderr == b''

# register exit handler for cleanup
atexit.register(cleanup_smm)

#
# Test Function Calls
#

test_help()
test_version()
test_list_installed()
test_install_sporemod()
test_install_sporemod_duplicate()
test_install_sporemod_invalid_modinfo()
test_uninstall()
