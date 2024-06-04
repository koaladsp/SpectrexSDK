import os

from enum import Enum


class ShaderType(Enum):
    Vertex = 1
    Geometry = 2
    Fragment = 3


output = ""

output += "#pragma once\n\n"
output += "namespace Plugin::Shaders {\n\n"

for filename in os.listdir("."):
    shader_type = None

    if "vert" in filename:
        shader_type = ShaderType.Vertex

        print("Vertex shader ", filename)
    elif "geom" in filename:
        shader_type = ShaderType.Geometry

        print("Geometry shader ", filename)
    elif "frag" in filename:
        shader_type = ShaderType.Fragment

        print("Fragment shader ", filename)
    else:
        continue

    name = os.path.splitext(filename)[0] + shader_type.name

    with open(filename, "r") as file:
        file_content = file.read()

        output += "/* {} */\n\n".format(name)

        output += 'const char* {} = R"(\n'.format(name)
        output += file_content
        output += ')";\n\n'

output += "} // namespace Plugin::Shaders"

with open("Shaders.h", "a+") as file:
    file.seek(0)

    # only overwrite if content is not the same
    # keeps the library from being recompiled
    if file.read() != output:
        file.truncate(0)
        file.seek(0)

        file.write(output)
