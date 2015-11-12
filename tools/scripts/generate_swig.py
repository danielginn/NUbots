#!/usr/bin/python
#
# File:   generate_swig.py
# Authors:
#   Trent Houliston <trent@houliston.me>
#
import sys
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

with open(output_file, 'w') as file:

    content = textwrap.dedent("""\
        %module example
        %{{
        /* Includes the header in the wrapper code */
        #include "messages/{header}"
        %}}

        /* Parse the header file to generate wrappers */
        %include "messages/{header}"
        """)

    file.write(content.format(header=input_file))
