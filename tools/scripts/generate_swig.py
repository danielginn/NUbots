#!/usr/bin/python
#
# File:   generate_swig.py
# Authors:
#   Trent Houliston <trent@houliston.me>
#
import os
import sys
import re
import textwrap

# Ensure we have specified a name
if sys.argv[1]:
    input_file = sys.argv[1]
else:
    print 'You must specify an input header'
    sys.exit(1)

# Ensure we've got at least one module
if sys.argv[2]:
    output_file = sys.argv[2]
else:
    print 'You must specify an output file'
    sys.exit(1)

imports = []

# Read the input file looking for #include lines
with open(input_file, 'r') as file:

    # Get our relative path to this file and it's directory
    rel = os.path.relpath(input_file)
    dirname = os.path.dirname(rel)

    # Make some regular expressions to find include statements
    sys_re = re.compile(r'#include\s+<([^>]+)>')
    usr_re = re.compile(r'#include\s+"([^"]+)"')

    for line in file:

        # This is a system header
        if sys_re.match(line):
            h = sys_re.match(line).group(1)

            # If it's armadillo we use armanpy
            if h == 'armadillo':
                # imports.append('armanpy.i')
                pass

        # This is a user header
        elif usr_re.match(line):
            h = usr_re.match(line).group(1)

            if h.startswith('messages') or h.startswith('utility'):
                if not h.endswith('.pb.h'):
                    imports.append('{}{}'.format(re.sub(r'(?:\.h|\.hpp)$', '', h), '.i'))
            elif os.sep not in h:
                imports.append('{}{}{}{}'.format(dirname, os.sep, re.sub(r'(?:\.h|\.hpp)$', '', h), '.i'))
            else:
                print h
                System.exit(1)

with open(output_file, 'w') as file:

    imports = '\n'.join(['    %import {}'.format(i) for i in imports])

    content = textwrap.dedent("""\
        %module shared
        {imports}
        %{{
        /* Includes the header in the wrapper code */
        #include "{header}"
        %}}

        /* Parse the header file to generate wrappers */
        %include "{header}"
        """)

    file.write(content.format(imports=imports, header=rel))
