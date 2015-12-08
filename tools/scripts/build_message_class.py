#!/usr/bin/python
import re
import os
import sys
import textwrap
import datetime
from google.protobuf.descriptor_pb2 import FileDescriptorSet, FieldOptions

from pygments import highlight
from pygments.lexers import CppLexer
from pygments.formatters import Terminal256Formatter

# Add our cwd to the path so we can import generated python protobufs
# And extend our options with our MessageOptions
sys.path.append(os.getcwd())
from MessageOptions_pb2 import pointer, PointerType
FieldOptions.RegisterExtension(pointer)
PointerType = dict(PointerType.items())

def indent(str, len=4):
    return '\n'.join([(' ' * len) + l for l in str.splitlines()])

def to_camel_case(snake_str):
    components = snake_str.split('_')
    # We capitalize the first letter of each component except the first one
    # with the 'title' method and join them together.
    return components[0] + "".join(x.title() for x in components[1:])

class Field:

    # Static map_types field
    map_types = {}

    def __init__(self, f, context):
        self.name = f.name
        self.number = f.number

        if f.type in [ f.TYPE_MESSAGE, f.TYPE_ENUM ]:
            self.type = f.type_name
        else:
            # Work out what primitive type we have
            self.type = {
                f.TYPE_DOUBLE   : 'double',
                f.TYPE_FLOAT    : 'float',
                f.TYPE_INT64    : 'int64',
                f.TYPE_UINT64   : 'uint64',
                f.TYPE_INT32    : 'int32',
                f.TYPE_FIXED64  : 'fixed64',
                f.TYPE_FIXED32  : 'fixed32',
                f.TYPE_BOOL     : 'bool',
                f.TYPE_STRING   : 'string',
                f.TYPE_GROUP    : 'group', # TODO SOMETHING
                f.TYPE_BYTES    : 'bytes',
                f.TYPE_UINT32   : 'uint32',
                f.TYPE_SFIXED32 : 'sfixed32',
                f.TYPE_SFIXED64 : 'sfixed64',
                f.TYPE_SINT32   : 'sint32',
                f.TYPE_SINT64   : 'sint64'
            }[f.type]

        # If we have one later we can use it
        self.default_value = f.default_value

        # Now we add custom type information
        self.repeated = f.label == f.LABEL_REPEATED
        self.pointer = f.options.Extensions[pointer]

    def get_cpp_type(self):

        cpp_type = self.type
        is_map = False

        vector_regex = re.compile(r'^\.messages\.([fiu]?)vec([2-4]?)$');
        matrix_regex = re.compile(r'^\.messages\.([fiu]?)mat([2-4]{0,2})$');

        # Vector types map to armadillo
        if vector_regex.match(cpp_type):
            r = vector_regex.match(cpp_type)
            cpp_type = '::arma::{}vec{}'.format(r.group(1), r.group(2));
        elif matrix_regex.match(cpp_type):
            r = matrix_regex.match(cpp_type)
            cpp_type = '::arma::{}mat{}'.format(r.group(1), r.group(2));

        # Transform and rotation types map to the Transform classes
        elif cpp_type == '.messages.Transform2D':
            cpp_type = '::utility::math::matrix::Transform2D'
        elif cpp_type == '.messages.Transform3D':
            cpp_type = '::utility::math::matrix::Transform3D'
        elif cpp_type == '.messages.Rotation2D':
            cpp_type = '::utility::math::matrix::Rotation2D'
        elif cpp_type == '.messages.Rotation3D':
            cpp_type = '::utility::math::matrix::Rotation3D'

        # Timestamps and durations map to real time/duration classes
        elif cpp_type == '.google.protobuf.Timestamp':
            cpp_type = '::NUClear::clock::time_point'
        elif cpp_type == '.google.protobuf.Duration':
            cpp_type = '::NUClear::clock::duration'

        # Struct types map to YAML nodes
        elif cpp_type == '.google.protobuf.Struct':
            cpp_type = '::YAML::Node'

        # Standard types get mapped to their appropriate type
        elif cpp_type in [ 'double', 'float', 'bool' ]:
            pass # double and float and bool are fine as is
        elif cpp_type in [ 'int64', 'sint64', 'sfixed64' ]:
            cpp_type = 'int64_t'
        elif cpp_type in [ 'uint64', 'fixed64' ]:
            cpp_type = 'uint64_t'
        elif cpp_type in [ 'int32', 'sint32', 'sfixed32' ]:
            cpp_type = 'int32_t'
        elif cpp_type in [ 'uint32', 'fixed32' ]:
            cpp_type = 'uint32_t'
        elif cpp_type in [ 'string' ]:
            cpp_type = '::std::string'
        elif cpp_type in [ 'bytes' ]:
            cpp_type = '::std::vector<char>'

        # Check if it is a map field
        elif cpp_type in self.map_types:
            is_map = True
            cpp_type = '::std::map<{}, {}>'.format(self.map_types[cpp_type][0].get_cpp_type(), self.map_types[cpp_type][1].get_cpp_type())

        # Otherwise we assume it's a normal type and let it work out its scoping
        else:
            cpp_type = '::'.join(cpp_type.split('.'));

        # If we are using a pointer type do the manipulation here
        if self.pointer == PointerType['RAW']:
            cpp_type = '{}*'.format(cpp_type)
        elif self.pointer == PointerType['SHARED']:
            cpp_type = '::std::shared_ptr<{}>'.format(cpp_type)
        elif self.pointer == PointerType['UNIQUE']:
            cpp_type = '::std::unique_ptr<{}>'.format(cpp_type)

        # If it's a repeated field, and not a map, it's a vector
        if self.repeated and not is_map:
            cpp_type = '::std::vector<{}>'.format(cpp_type)

        return cpp_type


    def generate_cpp_header(self):

        cpp_type = self.get_cpp_type()

        if self.default_value:
            return '{} {} = {};'.format(cpp_type, to_camel_case(self.name), self.default_value)
        else:
            return '{} {};'.format(cpp_type, to_camel_case(self.name))

    def __repr__(self):
        return "%r" % (self.__dict__)


class Message:
    def __init__(self, m, context):
        self.name = m.name
        self.fqn = '{}.{}'.format(context.fqn, self.name)
        self.enums = [Enum(e, self) for e in m.enum_type]

        # Get all the submessages that are not map entries
        self.submessages = [Message(n, self) for n in m.nested_type if not n.options.map_entry]
        self.oneofs = []

        for n in m.nested_type:
            if n.options.map_entry:
                Field.map_types['{}.{}'.format(self.fqn, n.name)] = (Field(n.field[0], self), Field(n.field[1], self))

        # TODO WORK OUT ONEOF
        # TODO WORK OUT m.options.map_type if true this is a map from
        # m.field[0] to m.field[1] type


        # All fields that are not a part of oneof
        self.fields = [Field(f, self) for f in m.field if f.oneof_index == 0]

        # m.name is the name of the message
        # m.field contains the fields
        # m.extension contains any exntesions (error don't use)
        # m.nested_type contains submessages
        # m.enum_type contains held enum types
        # m.extension_range contains extension ranges
        # m.oneof_decl contaions all one_of declarations
        # m.options contains options (message_set_wire_format no_standard_descriptor_accessor deprecated map_entry uninterpreted_option)
        # m.reserved_range and m.reserved_name contains names/field numbers not to use


    def generate_cpp_header(self):

        # Make our value pairs
        fields = indent('\n'.join(['{}'.format(v.generate_cpp_header()) for v in self.fields]))
        enums = indent('\n\n'.join([e.generate_cpp_header() for e in self.enums]))
        submessages = indent('\n\n'.join([m.generate_cpp_header() for m in self.submessages]))

        template = textwrap.dedent("""\
            struct {name} {{
            {enums}
            {submessages}
            {fields}
            }};
            """);

        # TODO
        # {name}() : defaultvalues {{}}
        # {name}(const YAML::Node& node) {{ CONVERT }}
        # {name}(const proto::{FQN}& proto) {{ CONVERT }}
        # {name}(const PyObject* pyobj) {{ CONVERT }}

        # operator YAML::Node() {{ CONVERT TO YAML NODE }}
        # operator proto::{FQN}() {{ CONVERT TO PROTOBUF }}
        # operator PyObject* pyobj() {{ WRAP IN A PYOBJECT }}

        return template.format(
            name=self.name,
            enums=enums,
            submessages=submessages,
            fields=fields
        )

    def __repr__(self):
        return "%r" % (self.__dict__)

class Enum:
    def __init__(self, e, context):
        self.name = e.name
        self.fqn = '{}.{}'.format(context.fqn, self.name)
        self.values = [(v.name, v.number) for v in e.value]
        # e.name contains the name of the enum
        # e.value is a list of enum values
        # e.options is a set of enum options (allow_alias, deprecated, list of uninterpreted options)
        # e.value[].name is the name of the constant
        # e.value[].number is the number assigned
        # e.value[].options is a set of enum options (deprecated, list of uninterpreted_option)

    def generate_cpp_header(self):

        # Make our value pairs
        values = indent('\n'.join(['{} = {}'.format(v[0], v[1]) for v in self.values]), 8)

        # Make our switch statement pairs
        switches = indent('\n'.join(['case Value::{}: return "{}";'.format(v[0], v[0]) for v in self.values]), 12)

        # Make our if chain
        if_chain = indent('\n'.join(['if (str == "{}") value = Value::{};'.format(v[0], v[0]) for v in self.values]), 8)

        # Add our commas
        values = ',\n'.join([v for v in values.splitlines()])

        # Make our fancy enums
        template = textwrap.dedent("""\
            struct {name} {{
                enum Value {{
            {values}
                }};
                Value value;

                {name}(const Value& value)
                : value(value) {{}}

                {name}(const std::string& str) {{
            {if_chain}
                    throw std::runtime_error("String did not match any enum for {name}");
                }}

                inline operator Value() const {{
                    return value;
                }}

                inline operator std::string() const {{
                    switch(value) {{
            {switches}
                        default:
                            throw std::runtime_error("enum {name}'s value is corrupt, unknown value stored");
                    }}
                }}
            }};
            """)

        return template.format(
            name=self.name,
            values=values,
            if_chain=if_chain,
            switches=switches
        )

    def __repr__(self):
        return "%r" % (self.__dict__)


class File:
    def __init__(self, f):
        self.package = f.package
        self.name = f.name
        self.fqn = '.{}'.format(self.package)
        self.dependencies = [d for d in f.dependency]
        self.enums = [Enum(e, self) for e in f.enum_type]
        self.messages = [Message(m, self) for m in f.message_type]

        # f.fileoptions contains many things including unrecognised options (java_package java_outer_classname java_multiple_files java_generate_equals_and_hash java_string_check_utf8 optimize_for go_package cc_generic_services java_generic_services py_generic_services deprecated cc_enable_arenas objc_class_prefix csharp_namespace javanano_use_deprecated_package uninterpreted_option)
        # f.syntax tells if it is proto2 or proto3

    def generate_cpp_header(self):

        define = '{}_H'.format('_'.join([s.upper() for s in self.name[:-6].strip().split('/')]))
        parts = self.package.split('.')
        ns_open = '\n'.join(['namespace {} {{'.format(x) for x in parts])
        ns_close = '\n'.join('}' * len(parts))
        enums = indent('\n\n'.join([e.generate_cpp_header() for e in self.enums]))
        messages = indent('\n\n'.join([m.generate_cpp_header() for m in self.messages]))

        # By default include some useful headers
        includes = set([
            '1<cstdint>',
            '2<string>',
            '2<map>',
            '2<vector>',
            '2<memory>'
        ])

        # We use a dirty hack here of putting a priority on each header
        # to make the includes be in a better order
        for d in self.dependencies:
            if d in ['messages/Vector.proto', 'messages/Matrix.proto']:
                includes.add('3<armadillo>')
            if d in ['messages/Transform.proto']:
                includes.add('4"utility/math/matrix/Transform2D.h"')
                includes.add('4"utility/math/matrix/Transform3D.h"')
            if d in ['messages/Rotation.proto']:
                includes.add('4"utility/math/matrix/Rotation2D.h"')
                includes.add('4"utility/math/matrix/Rotation3D.h"')
            elif d in ['google/protobuf/timestamp.proto', 'google/protobuf/duration.proto']:
                includes.add('3<nuclear_bits/clock.hpp>')
            elif d in ['google/protobuf/struct.proto']:
                includes.add('3<yaml-cpp/yaml.h>')
            else:
                includes.add('4"{}"'.format(d[:-6] + '.h'))
        # Don't forget to remove the first character
        includes = '\n'.join(['#include {}'.format(i[1:]) for i in sorted(list(includes))])

        template = textwrap.dedent("""\
            #ifndef {define}
            #define {define}

            {includes}

            {openNamespace}

                // Enum Definitions
            {enums}

                // Message Definitions
            {messages}

            {closeNamespace}

            #endif  // {define}
            """)

        return template.format(
            define=define,
            includes=includes,
            openNamespace=ns_open,
            enums=enums,
            messages=messages,
            closeNamespace=ns_close
        )

        return template


    def generate_cpp_impl(self):
        return '#include "{}"'.format(self.name[:-6] + '.h')

    def __repr__(self):
        return "%r" % (self.__dict__)


base_file = sys.argv[1]

with open('{}.pb'.format(base_file), 'r') as f:
    # Load the descriptor protobuf file
    d = FileDescriptorSet()
    d.ParseFromString(f.read())

    # Check that there is only one file
    assert(len(d.file) == 1)

    b = File(d.file[0])

    with open('{}.cpp'.format(base_file), 'w') as f:
        f.write(b.generate_cpp_impl())

    with open('{}.h'.format(base_file), 'w') as f:
        f.write(b.generate_cpp_header())

    print highlight(b.generate_cpp_header(), CppLexer(), Terminal256Formatter())
    # TODO generate smart enums
    # TODO Generate cpp -> yaml
    # TODO Generate yaml -> cpp
    # TODO Generate cpp -> proto
    # TODO Generate proto -> cpp
    # TODO Generate Python interface


# Basically you need to make a struct for each of the messages here

# You also need to make a conversion operator and cast operator to go to->from protobufs

# if you encounter some special types (vec* mat*) you need to use armadillo to store them
# Everything else should be made up of their basic types as normal

# TODO need a way to grab custom accessor functions here


# TODO we also want to write python wrappers for the classes here
