#! /usr/bin/python3
import os
import collections

parser_result = {}

package_list = {}
proto_str = ""
is_find_package = False
is_find_syntax  = False
variable_name = "proto_code"
def generate_lua_file():
    with open('../src/lua/proto/code.lua', 'w+', encoding='utf-8') as out:
        out.write('''----------------------------------------------------------------------------------
-- don't edit it, generated from .proto files by tools
----------------------------------------------------------------------------------

''')
        out.write(variable_name + " = [[\n" + proto_str + "\n]]")


def parse_proto(f):
    global proto_str, is_find_package, is_find_syntax
    print('parse file:' + f)
    with open(f, 'r', encoding='utf-8') as inp:
        for line in inp.readlines():
                pos = line.find('package')
                if pos >= 0:
                    if is_find_package == False:
                        proto_str += line
                        is_find_package = True
                    continue
                pos = line.find('syntax')
                if pos >= 0:
                    if is_find_syntax == False:
                        proto_str += line
                        is_find_syntax = True
                    continue
                pos = line.find('import')
                if pos >= 0:
                    continue
                proto_str += line

for root, dirs, files in os.walk('../src/proto/'):
    for f in files:
        if f.endswith('.proto'):
            parse_proto(os.path.join(root, f))
generate_lua_file()
