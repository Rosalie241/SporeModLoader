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
import unittest

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
# Helper classes
#

class SporeModManagerTest(unittest.TestCase):
	def setUp(self):
		# setup variables
		self.smmPath = tempfile.mkdtemp()
		self.modsPath = os.path.join(self.smmPath, 'mods')
		self.corelibsPath = os.path.join(self.smmPath, 'CoreLibs')
		self.modLibsPath = os.path.join(self.smmPath, 'ModLibs')
		self.dataPath = os.path.join(self.smmPath, 'Data')
		self.dataEP1Path = os.path.join(self.smmPath, 'DataEP1')
		self.packageFile = os.path.join(self.smmPath, 'test.package')
		self.invalidFile = os.path.join(self.smmPath, 'test.invalid')
		self.writeModNum = 0

		# create directories
		os.mkdir(self.modsPath)
		os.mkdir(self.corelibsPath)
		os.mkdir(self.modLibsPath)
		os.mkdir(self.dataPath)
		os.mkdir(self.dataEP1Path)

		# write files
		self.writeSporeModAPIDLL(os.path.join(self.corelibsPath, 'SporeModAPI.dll'))
		self.writePackage(self.packageFile)
		self.writePackage(self.invalidFile)

	def tearDown(self):
		if self.smmPath != None:
			shutil.rmtree(self.smmPath)

	def checkFileContents(self, path, content):
		with open(path, 'r') as file:
			file_content = file.read()
			if verbose:
				print(f'expected file contents: {content}\nactual file contents: {file_content}')
			return file_content == content

	def writeFile(self, path, contents):
		with open(path, 'wb') as file:
			file.write(contents)

	def writeSporeModAPIDLL(self, path):
		self.writeFile(path, b'F\0i\0l\0e\0V\0e\0r\0s\0i\0o\0n\0\0\0\0\0002\0.\0005\0.\000300\0')

	def writePackage(self, path):
		self.writeFile(path, b'package')

	def writeSporemod(self, xml = None, extra = None, invalidZip = False):
		file = os.path.join(self.modsPath, f'test_{self.writeModNum}.sporemod')
		self.writeModNum = self.writeModNum + 1
		if invalidZip:
			modFile = open(file, 'wb')
			modFile.write(b'invalidZip')
			modFile.close()
		else:
			with zipfile.ZipFile(file, mode="w") as archive:
				if xml is not None:
					archive.writestr("ModInfo.xml", xml)
				if extra is not None:
					for list_str in extra:
						archive.writestr(list_str[0], list_str[1])
		return file

	def runSMM(self, args):
		os_environment["SPOREMODMANAGER_CONFIGFILE"] = str(os.path.join(self.modsPath, 'ConfigFile.xml'))
		os_environment["ASAN_OPTIONS"] = str('exitcode=2:abort_on_error=false')
		valgrind_cmd = [ 'valgrind', '--quiet', '--leak-check=full', '--show-leak-kinds=all', '--track-origins=yes', '--error-exitcode=2' ]
		smm_cmd = [ sporemodmanager, '--verbose', '--no-input',
					f'--corelibs-path={self.corelibsPath}', f'--modlibs-path={self.modLibsPath}',
					f'--data-path={self.dataPath}', f'--ep1-path={self.dataEP1Path}' ]
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

#
# Tests
#

class BasicCommandlineTests(SporeModManagerTest):
	def test_help(self):
		result = self.runSMM([ 'help' ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')

	def test_version(self):
		result = self.runSMM([ 'version' ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')

class InstallTests(SporeModManagerTest):
	def test_install_invalid_extension(self):
		result = self.runSMM([ 'install', self.invalidFile ])
		self.assertTrue(result.returncode != 0)
		self.assertTrue(result.stdout == '')
		self.assertTrue(result.stderr != '')
		self.assertTrue(not os.path.isfile(os.path.join(self.modLibsPath, 'test.invalid')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataPath, 'test.invalid')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'test.invalid')))

	def test_install_package(self):
		result = self.runSMM([ 'install', self.packageFile ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test.package')))

	def test_install_package_duplicate(self):
		result = self.runSMM([ 'install', self.packageFile ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test.package')))

		# installing the same mod should fail
		result = self.runSMM([ 'install', self.packageFile ])
		assert result.returncode == 1
		assert result.stdout == ''
		assert result.stderr != ''

		# but using --needed should work
		result = self.runSMM([ 'install', '--needed', self.packageFile ])
		assert result.returncode == 0
		assert result.stdout != ''
		assert result.stderr == ''

		# --update-needed should also work
		result = self.runSMM([ 'install', '--update-needed', self.packageFile ])
		assert result.returncode == 0
		assert result.stdout != ''
		assert result.stderr == ''


	def test_install_sporemod_invalid_zip(self):
		result = self.runSMM([ 'install', self.writeSporemod(invalidZip=True) ])
		self.assertTrue(result.returncode == 1)
		self.assertTrue(result.stdout == '')
		self.assertTrue(result.stderr != '')

	def test_install_sporemod_no_modinfo(self):
		files = [
			[ 'test_install_sporemod_no_modinfo.dll', str(uuid.uuid4()) ],
			[ 'test_install_sporemod_no_modinfo.txt', str(uuid.uuid4()) ],
			[ 'test_install_sporemod_no_modinfo_ep1.package', str(uuid.uuid4()) ],
		]
		result = self.runSMM([ 'install', self.writeSporemod(None, files) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(os.path.isfile(os.path.join(self.modLibsPath, 'test_install_sporemod_no_modinfo.dll')))
		self.assertTrue(not os.path.isfile(os.path.join(self.modLibsPath, 'test_install_sporemod_no_modinfo.txt')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataPath, 'test_install_sporemod_no_modinfo.txt')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_sporemod_no_modinfo.txt')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_sporemod_no_modinfo_ep1.package')))
		self.assertTrue(self.checkFileContents(os.path.join(self.modLibsPath, files[0][0]), files[0][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[2][0]), files[2][1]))

	def test_install_sporemod_modinfo(self):
		xml = """<mod displayName="test_install_modinfo" 
					unique="test_install_modinfo" 
					description="test_install_modinfo" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.20">
				</mod>"""
		result = self.runSMM([ 'install', self.writeSporemod(xml) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')

	def test_install_sporemod_modinfo_duplicate(self):
		xml = """<mod displayName="test_install_sporemod_modinfo_duplicate" 
					unique="test_install_sporemod_modinfo_duplicate" 
					description="test_install_0" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.20">
				</mod>"""
		sporemod = self.writeSporemod(xml)
		result = self.runSMM([ 'install', sporemod ])
		assert result.returncode == 0
		assert result.stdout != ''
		assert result.stderr == ''

		# installing the same mod should fail
		result = self.runSMM([ 'install', sporemod ])
		assert result.returncode == 1
		assert result.stdout == ''
		assert result.stderr != ''

		# but using --needed should work
		result = self.runSMM([ 'install', '--needed', sporemod ])
		assert result.returncode == 0
		assert result.stdout != ''
		assert result.stderr == ''

		# --update-needed should also work
		result = self.runSMM([ 'install', '--update-needed', sporemod ])
		assert result.returncode == 0
		assert result.stdout != ''
		assert result.stderr == ''

	def test_install_sporemod_modinfo_prerequisite(self):
		xml = """<mod displayName="test_install_sporemod_modinfo_prerequisite_1" 
					unique="test_install_sporemod_modinfo_prerequisite_1" 
					description="test_install_sporemod_modinfo_prerequisite_1" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.20">
					<prerequisite>test_install_1.dll</prerequisite>
					<prerequisite game="invalid">test_install_2.dll</prerequisite>
					<prerequisite game="Spore">test_install_1.package</prerequisite>
					<prerequisite game="spore">test_install_lowercase.package</prerequisite>
					<prerequisite game="GalacticAdventures">test_install_ep1.package</prerequisite>
					<prerequisite game="galacticAdventures">test_install_ep1_mixedcase.package</prerequisite>
				</mod>"""
		files = [
			[ 'test_install_1.dll', str(uuid.uuid4()) ],
			[ 'test_install_2.dll', str(uuid.uuid4()) ],
			[ 'test_install_1.package', str(uuid.uuid4()) ],
			[ 'test_install_lowercase.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1_mixedcase.package', str(uuid.uuid4()) ]
		]
		result = self.runSMM([ 'install', self.writeSporemod(xml, files) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(os.path.isfile(os.path.join(self.modLibsPath, 'test_install_1.dll')))
		self.assertTrue(os.path.isfile(os.path.join(self.modLibsPath, 'test_install_2.dll')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataPath, 'test_install_1.package')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataPath, 'test_install_lowercase.package')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1.package')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_mixedcase.package')))
		self.assertTrue(self.checkFileContents(os.path.join(self.modLibsPath, files[0][0]), files[0][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.modLibsPath, files[1][0]), files[1][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataPath, files[2][0]), files[2][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataPath, files[3][0]), files[3][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[4][0]), files[4][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[5][0]), files[5][1]))

	def test_install_sporemod_modinfo_compatFile(self):
		self.writeFile(os.path.join(self.dataPath, 'compatfile_test_1.package'), b'compatfile')
		self.writeFile(os.path.join(self.dataEP1Path, 'compatfile_test_2.package'), b'compatfile')
		xml = """<mod displayName="test_install_sporemod_modinfo_compatFile" 
					unique="test_install_sporemod_modinfo_compatFile" 
					description="test_install_sporemod_modinfo_compatFile" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.20">
					<compatFile game="spore" compatTargetFileName="compatfile_test_1.package" 
						compatTargetGame="spore">compatfile_1.package</compatFile>
					<compatFile game="GalacticAdventures" compatTargetFileName="compatfile_test_2.package" 
						compatTargetGame="GalacticAdventures">compatfile_1_ep1.package</compatFile>
					<compatFile game="spore" compatTargetFileName="no_compatfile" 
						compatTargetGame="spore">compatfile_2.package</compatFile>
					<compatFile game="GalacticAdventures" compatTargetFileName="no_compatfile" 
						compatTargetGame="GalacticAdventures">compatfile_2_ep1.package</compatFile>
				</mod>"""
		files = [
			[ 'compatfile_1.package', str(uuid.uuid4()) ],
			[ 'compatfile_1_ep1.package', str(uuid.uuid4()) ],
			[ 'compatfile_2.package', str(uuid.uuid4()) ],
			[ 'compatfile_2_ep1.package', str(uuid.uuid4()) ],
		]
		result = self.runSMM([ 'install', self.writeSporemod(xml, files) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(os.path.isfile(os.path.join(self.dataPath, 'compatfile_1.package')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'compatfile_1_ep1.package')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataPath, 'compatfile_2.package')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'compatfile_2_ep1.package')))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataPath, files[0][0]), files[0][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[1][0]), files[1][1]))

	def test_install_sporemod_modinfo_component(self):
		xml = """<mod displayName="test_install_sporemod_modinfo_component"
					unique="test_install_sporemod_modinfo_component"
					description="test_install_sporemod_modinfo_component"
					installerSystemVersion="1.0.1.1" 
					hasCustomInstaller="true"
					dllsBuild="2.5.20">
					<component unique="test_install_component_1" 
						displayName="test_install_component_1" 
						description="test_install_component_1"
						game="GalacticAdventures" defaultChecked="true">test_install_ep1_1.package</component>
					<component unique="test_install_component_2" 
						displayName="test_install_component_2" 
						description="test_install_component_2"
						game="GalacticAdventures" defaultChecked="false">test_install_ep1_2.package</component>
					<component unique="test_install_component_3" 
						displayName="test_install_component_3" 
						description="test_install_component_3"
						game="GalacticAdventures" defaultChecked="true">test_install_ep1_3.package</component>
				</mod>"""
		files = [
			[ 'test_install_ep1_1.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1_2.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1_3.package', str(uuid.uuid4()) ],
		]
		result = self.runSMM([ 'install', self.writeSporemod(xml, files) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_1.package')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_2.package')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_3.package')))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[0][0]), files[0][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[2][0]), files[2][1]))

	def test_install_sporemod_modinfo_componentgroup(self):
		xml = """<mod displayName="test_install_sporemod_modinfo_componentgroup"
					unique="test_install_sporemod_modinfo_componentgroup"
					description="test_install_sporemod_modinfo_componentgroup"
					installerSystemVersion="1.0.1.1" 
					hasCustomInstaller="true"
					dllsBuild="2.5.20">
					<componentGroup unique="test_install_componentgroup_1" displayName="test_install_componentgroup_1">
						<component unique="test_install_componentgroup_1_component_1" displayName="test_install_componentgroup_1_component_1" description="" game="GalacticAdventures">test_install_ep1_1.package</component>
						<component unique="test_install_componentgroup_1_component_2" displayName="test_install_componentgroup_1_component_2" description="" game="GalacticAdventures" defaultChecked="true">test_install_ep1_2.package</component>
						<component unique="test_install_componentgroup_1_component_3" displayName="test_install_componentgroup_1_component_3" description="" game="GalacticAdventures">test_install_ep1_3.package</component>
					</componentGroup>
					<componentGroup unique="test_install_componentgroup_2" displayName="test_install_componentgroup_2">
						<component unique="test_install_componentgroup_2_component_1" displayName="test_install_componentgroup_2_component_1" description="" game="GalacticAdventures" defaultChecked="true">test_install_ep1_4.package</component>
						<component unique="test_install_componentgroup_2_component_2" displayName="test_install_componentgroup_2_component_2" description="" game="GalacticAdventures">test_install_ep1_5.package</component>
						<component unique="test_install_componentgroup_2_component_3" displayName="test_install_componentgroup_2_component_3" description="" game="GalacticAdventures">test_install_ep1_6.package</component>
					</componentGroup>
				</mod>"""
		files = [
			[ 'test_install_ep1_1.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1_2.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1_3.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1_4.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1_5.package', str(uuid.uuid4()) ],
			[ 'test_install_ep1_6.package', str(uuid.uuid4()) ],
		]
		result = self.runSMM([ 'install', self.writeSporemod(xml, files) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_1.package')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_2.package')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_3.package')))
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_4.package')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_5.package')))
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'test_install_ep1_6.package')))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[1][0]), files[1][1]))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[3][0]), files[3][1]))

	def test_install_sporemod_modinfo_has_custom_installer(self):
		xml = """<mod displayName="test_install_sporemod_modinfo_has_custom_installer_1"
					unique="test_install_sporemod_modinfo_has_custom_installer_1"
					description="test_install_sporemod_modinfo_has_custom_installer_1"
					installerSystemVersion="1.0.1.1"
					hasCustomInstaller="false" 
					dllsBuild="2.5.20">
					<component unique="component" 
						displayName="component" 
						description="component"
						game="GalacticAdventures" defaultChecked="true">component_1.package</component>
					</mod>"""	
		files = [
			[ 'component_1.package', str(uuid.uuid4()) ],
		]
		result = self.runSMM([ 'install', self.writeSporemod(xml, files) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(not os.path.isfile(os.path.join(self.dataEP1Path, 'component_1.package')))

		# check if a hasCustomInstaller is enabled when installer version is 1.0.0.0
		xml = """<mod displayName="test_install_sporemod_modinfo_has_custom_installer_2"
					unique="test_install_sporemod_modinfo_has_custom_installer_2"
					description="test_install_sporemod_modinfo_has_custom_installer_2"
					installerSystemVersion="1.0.0.0">
					<component unique="component" 
						displayName="component" 
						description="component"
						game="GalacticAdventures" defaultChecked="true">component_2.package</component>
					</mod>"""
		files = [
			[ 'component_2.package', str(uuid.uuid4()) ],
		]
		result = self.runSMM([ 'install', self.writeSporemod(xml, files) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(os.path.isfile(os.path.join(self.dataEP1Path, 'component_2.package')))
		self.assertTrue(self.checkFileContents(os.path.join(self.dataEP1Path, files[0][0]), files[0][1]))

	def test_install_sporemod_modinfo_dlls_build(self):
		xml = """<mod displayName="test_install_sporemod_modinfo_dlls_build_1" 
					unique="test_install_sporemod_modinfo_dlls_build_1" 
					description="test_install_sporemod_modinfo_dlls_build_1" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="999.999.999">
				</mod>"""
		result = self.runSMM([ 'install', self.writeSporemod(xml) ])
		self.assertTrue(result.returncode == 1)
		self.assertTrue(result.stdout == '')
		self.assertTrue(result.stderr != '')

		xml = """<mod displayName="test_install_sporemod_modinfo_dlls_build_2" 
					unique="test_install_sporemod_modinfo_dlls_build_2" 
					description="test_install_sporemod_modinfo_dlls_build_2" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.100.1.1">
				</mod>"""
		result = self.runSMM([ 'install', self.writeSporemod(xml) ])
		self.assertTrue(result.returncode == 1)
		self.assertTrue(result.stdout == '')
		self.assertTrue(result.stderr != '')

		xml = """<mod displayName="test_install_sporemod_modinfo_dlls_build_3" 
					unique="test_install_sporemod_modinfo_dlls_build_3" 
					description="test_install_sporemod_modinfo_dlls_build_3" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.100">
				</mod>"""
		result = self.runSMM([ 'install', self.writeSporemod(xml) ])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')

class UninstallTests(SporeModManagerTest):
	def test_uninstall_no_mods(self):
		result = self.runSMM([ 'uninstall', '0'])
		self.assertTrue(result.returncode == 1)
		self.assertTrue(result.stdout == '')
		self.assertTrue(result.stderr != '')


	def test_uninstall_mods(self):
		install_cmd = [ 'install' ]
		xml = f"""<mod displayName="test_uninstall_1" 
					unique="test_uninstall_1" 
					description="test_uninstall_1" 
					installerSystemVersion="1.0.1.1" 
					dllsBuild="2.5.20">
					<prerequisite>test_uninstall_1.dll</prerequisite>
				</mod>"""
		files = [
			[ f'test_uninstall_1.dll', str(uuid.uuid4()) ],
		]
		result = self.runSMM([ 'install', self.writeSporemod(xml, files)])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(os.path.isfile(os.path.join(self.modLibsPath, f'test_uninstall_1.dll')))

		result = self.runSMM([ 'uninstall', '0'])
		self.assertTrue(result.returncode == 0)
		self.assertTrue(result.stdout != '')
		self.assertTrue(result.stderr == '')
		self.assertTrue(not os.path.isfile(os.path.join(self.modLibsPath, f'test_uninstall_1.dll')))

	def test_uninstall_invalid_number(self):
		result = self.runSMM([ 'uninstall', '0x0'])
		self.assertTrue(result.returncode == 1)
		self.assertTrue(result.stdout == '')
		self.assertTrue(result.stderr != '')

		result = self.runSMM([ 'uninstall', 'a'])
		self.assertTrue(result.returncode == 1)
		self.assertTrue(result.stdout == '')
		self.assertTrue(result.stderr != '')



#
# Test Functions
#

# Tests whether install works correctly
def test_install():
	print(f'Running {test_install.__name__}...')
	reset_smm()

	# install sporemod mod

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
	result = run_smm([ 'install', '--update-needed', write_sporemod(xml1, files, True), write_sporemod(xml2, files, True) ])
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
	result = run_smm([ 'install', '--update-needed', sporemod_file, sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_12.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# install sporemod mod
	xml = """<mod displayName="test_install_13" 
				unique="test_install_13" 
				description="test_install_13" 
				installerSystemVersion="1.0.1.1" 
				dllsBuild="2.5.20">
				<prerequisite>test_install_13.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_install_13.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_13.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# verify that --needed doesn't update the files
	mtime_file = os.path.join(modlibs_path, 'test_install_13.dll')
	before_install_mtime = os.path.getmtime(mtime_file)
	new_files = [
		[ 'test_install_13.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, new_files)
	result = run_smm([ 'install', '--needed', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_13.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])
	assert before_install_mtime == os.path.getmtime(mtime_file)

	# verify that --update-needed updates the files
	result = run_smm([ 'install', '--update-needed', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_install_13.dll'))
	assert check_file_contents(os.path.join(modlibs_path, new_files[0][0]), new_files[0][1])
	assert before_install_mtime != os.path.getmtime(mtime_file)


# Tests whether uninstall works correctly
def test_uninstall():
	print(f'Running {test_uninstall.__name__}...')
	reset_smm()

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

# Tests whether dependencies work correctly
def test_depends():
	print(f'Running {test_depends.__name__}...')
	reset_smm()

	# install dummy dependency mod
	xml = """<mod displayName="test_depends_1" 
				unique="test_depends_1" 
				description="test_depends_1"
				version="1.0.2"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_1.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_1.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_1.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# verify that the version attribute doesn't accept negative values
	xml = """<mod displayName="test_depends_2" 
				unique="test_depends_2" 
				description="test_depends_2" 
				version="1.0.-1"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_2.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_2.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_depends_2.dll'))

	# verify that the version attribute doesn't accept less than 2 numbers
	xml = """<mod displayName="test_depends_3" 
				unique="test_depends_3" 
				description="test_depends_3" 
				version="1"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_3.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_3.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_depends_3.dll'))

	# verify that the version attribute doesn't accept more than 4 numbers
	xml = """<mod displayName="test_depends_4" 
				unique="test_depends_4" 
				description="test_depends_4" 
				version="1.0.0.0.0"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_4.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_4.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_depends_4.dll'))

	# verify that the depends attribute works with an installed mod
	xml = """<mod displayName="test_depends_5" 
				unique="test_depends_5" 
				description="test_depends_5" 
				version="1.0.3"
				depends="test_depends_1"
				dependsVersions="1.0.2"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_5.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_5.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_5.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# verify that the depends attribute doesn't work when the version doesn't match
	xml = """<mod displayName="test_depends_6" 
				unique="test_depends_6" 
				description="test_depends_6" 
				depends="test_depends_1"
				dependsVersions="1.0.4"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_6.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_6.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_depends_6.dll'))

	# verify that the depends and dependsVersions attribute works with multiple valid entries
	xml = """<mod displayName="test_depends_7" 
				unique="test_depends_7" 
				description="test_depends_7" 
				depends="test_depends_1?test_depends_5"
				dependsVersions="1.0.2?1.0.3"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_7.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_7.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_7.dll'))

	# verify that the depends and dependsVersions attribute doesn't work with an invalid version
	# entry within multiple entries
	xml = """<mod displayName="test_depends_8" 
				unique="test_depends_8" 
				description="test_depends_8" 
				depends="test_depends_1?test_depends_5"
				dependsVersions="1.0.2?1.0.4"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_8.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_8.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_depends_8.dll'))

	# verify that the depends attribute doesn't work with a non-existent mod
	xml = """<mod displayName="test_depends_9" 
				unique="test_depends_9" 
				description="test_depends_9" 
				depends="test_depends_none"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_9.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_9.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_depends_9.dll'))

	# verify that the depends attribute doesn't work with multiple non-existent mods
	xml = """<mod displayName="test_depends_10" 
				unique="test_depends_10" 
				description="test_depends_10" 
				depends="test_depends_none_1?test_depends_none_2"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_10.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_10.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_depends_10.dll'))

	# verify that depends attribute is only validated for installerSystemVersion >= 1.0.1.3
	xml = """<mod displayName="test_depends_11" 
				unique="test_depends_11" 
				description="test_depends_11" 
				depends="test_depends_12"
				installerSystemVersion="1.0.1.2" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_11.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_11.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'install', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_11.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# verify that we can install a dependency alongside the mod in a single install
	xml1 = """<mod displayName="test_depends_12_1"
				unique="test_depends_12_1"
				description="test_depends_12_1"
				depends="test_depends_12_2"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_12_1.dll</prerequisite>
			</mod>"""
	xml2 = """<mod displayName="test_depends_12_2"
				unique="test_depends_12_2"
				description="test_depends_12_2"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_12_2.dll</prerequisite>
			</mod>"""
	files1 = [
		[ 'test_depends_12_1.dll', str(uuid.uuid4()) ],
	]
	files2 = [
		[ 'test_depends_12_2.dll', str(uuid.uuid4()) ],
	]
	result = run_smm([ 'install', write_sporemod(xml1, files1, True), write_sporemod(xml2, files2, True) ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_12_1.dll'))
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_12_2.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files1[0][0]), files1[0][1])
	assert check_file_contents(os.path.join(modlibs_path, files2[0][0]), files2[0][1])

	# verify that we cannot uninstall a mod which is a dependency of another mod
	result = run_smm([ 'uninstall', '3' ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''

	# verify that we can uninstall a mod when all dependencies are uninstalled
	result = run_smm([ 'uninstall', '5' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	result = run_smm([ 'uninstall', '4' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	result = run_smm([ 'uninstall', '0' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	# verify that we can uninstall mods alongside dependencies in one go
	result = run_smm([ 'uninstall', '1', '2' ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''

	#
	# verify that updating a dependency behaves like expected
	#

	# first we install the dependency and dependant
	xml1 = """<mod displayName="test_depends_13_1"
				unique="test_depends_13_1"
				description="test_depends_13_1"
				version="1.0.0"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_1.dll</prerequisite>
			</mod>"""
	xml2 = """<mod displayName="test_depends_13_2"
				unique="test_depends_13_2"
				description="test_depends_13_2"
				depends="test_depends_13_1"
				dependsVersions="1.0.0"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_2.dll</prerequisite>
			</mod>"""
	files1 = [
		[ 'test_depends_13_1.dll', str(uuid.uuid4()) ],
	]
	files2 = [
		[ 'test_depends_13_2.dll', str(uuid.uuid4()) ],
	]
	result = run_smm([ 'install', write_sporemod(xml1, files1, True), write_sporemod(xml2, files2, True) ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_1.dll'))
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_2.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files1[0][0]), files1[0][1])
	assert check_file_contents(os.path.join(modlibs_path, files2[0][0]), files2[0][1])

	# now check if updating the dependency works with a valid version
	xml = """<mod displayName="test_depends_13_1"
				unique="test_depends_13_1"
				description="test_depends_13_1"
				version="1.0.1"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_3.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_13_3.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_3.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# check if updating the dependency doesn't work with a too low version
	xml = """<mod displayName="test_depends_13_1"
				unique="test_depends_13_1"
				description="test_depends_13_1"
				version="0.0.9"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_4.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_13_4.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 1
	assert result.stdout == ''
	assert result.stderr != ''
	assert not os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_4.dll'))

	# check if updating the dependency alongside dependant works
	xml1 = """<mod displayName="test_depends_13_1"
				unique="test_depends_13_1"
				description="test_depends_13_1"
				version="0.0.9"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_5.dll</prerequisite>
			</mod>"""
	xml2 = """<mod displayName="test_depends_13_2"
				unique="test_depends_13_2"
				description="test_depends_13_2"
				depends="test_depends_13_1"
				dependsVersions="0.0.9"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_6.dll</prerequisite>
			</mod>"""
	files1 = [
		[ 'test_depends_13_5.dll', str(uuid.uuid4()) ],
	]
	files2 = [
		[ 'test_depends_13_6.dll', str(uuid.uuid4()) ],
	]
	result = run_smm([ 'update', write_sporemod(xml1, files1, True), write_sporemod(xml2, files2, True) ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_5.dll'))
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_6.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files1[0][0]), files1[0][1])
	assert check_file_contents(os.path.join(modlibs_path, files2[0][0]), files2[0][1])

	# check if removing the version requirement from the dependant when updating works
	xml = """<mod displayName="test_depends_13_2"
				unique="test_depends_13_2"
				description="test_depends_13_2"
				depends="test_depends_13_1"
				installerSystemVersion="1.0.1.3"
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_7.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_13_7.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_7.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# verify that changing the dependency version from the dependant makes using a dependency
	# without version work with the update command
	xml = """<mod displayName="test_depends_13_1"
				unique="test_depends_13_1"
				description="test_depends_13_1"
				installerSystemVersion="1.0.1.3" 
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_8.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_13_8.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_8.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

	# verify that updating a dependency without version to with version works
	xml = """<mod displayName="test_depends_13_1"
				unique="test_depends_13_1"
				description="test_depends_13_1"
				installerSystemVersion="1.0.1.3"
				version="1.0.0"
				dllsBuild="2.5.20">
				<prerequisite>test_depends_13_9.dll</prerequisite>
			</mod>"""
	files = [
		[ 'test_depends_13_9.dll', str(uuid.uuid4()) ],
	]
	write_sporemod(xml, files)
	result = run_smm([ 'update', sporemod_file ])
	assert result.returncode == 0
	assert result.stdout != ''
	assert result.stderr == ''
	assert os.path.isfile(os.path.join(modlibs_path, 'test_depends_13_9.dll'))
	assert check_file_contents(os.path.join(modlibs_path, files[0][0]), files[0][1])

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

if __name__ == '__main__':
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

	unittest.main(argv=['first-arg-is-ignored'], verbosity=2)
