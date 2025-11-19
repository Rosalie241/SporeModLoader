#!/usr/bin/env python3
#
# SporeModManager gen_rev_header.py
#
import os
import subprocess
import argparse

#
# Helper Functions
#

def run_git(args):
	cmd = [ 'git' ]
	cmd += args
	result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
	return result


def generate_header(path, revision):
	# only update header when revision
	# isn't already in the header
	if os.path.isfile(path):
		with open(path, 'r') as header:
			text = header.read()
			if revision in text:
				return

	# write out simple header
	with open(path, 'w') as header:
		header.write('#ifndef REVISION_H\n')
		header.write('#define REVISION_H\n\n')
		header.write(f'#define REVISION_STR \"{revision}\"\n\n')
		header.write('#endif // REVISION_H\n')

#
# main
#
if __name__ == "__main__":
	# add argument parser
	parser = argparse.ArgumentParser(description='Generates revision header.')
	parser.add_argument('path', help='path to generated header.')
	args = parser.parse_args()

	revision = 'unknown'
	result = run_git([ 'describe', '--always' ])
	if result.returncode == 0 and result.stdout != '':
		revision = result.stdout.rstrip()
	else:
		print('Failed to run git command, falling back to unknown revision')

	generate_header(args.path, revision)
