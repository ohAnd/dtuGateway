import requests

input_url = "https://raw.githubusercontent.com/suaveolent/hoymiles-wifi/main/hoymiles_wifi/const.py"
output_file = "include/dtuConst.h"

# Download the input file
response = requests.get(input_url)
if response.status_code == 200:
    input_lines = response.text.split('\n')
else:
    print(f"Failed to download input file from {input_url}")
    exit()

output_lines = []
output_lines.append("// constants converted from python constants file: " + input_url + "\n//")

for line in input_lines:
    if '=' in line:
        # If the line contains an '=', it might define either a byte sequence or a numeric constant
        parts = line.split('=')
        name = parts[0].strip()
        value = parts[1].strip()
        if value.startswith('b"') and value.endswith('"'):
            # This is a byte sequence
            bytes_str = value.lstrip('b').strip().strip('"')
            hex_values = bytes_str.split('\\x')[1:]
            # Convert hex values to integers
            int_values = [int(value, 16) for value in hex_values]
            # Construct Arduino C++ line
            output_line = f"const byte {name}[] = {{{','.join([f'0x{value:02x}' for value in int_values])}}};"
            output_lines.append(output_line)
        elif value.isdigit():
            # This is a numeric constant
            output_line = f"#define {name} {value}"
            output_lines.append(output_line)
        else:
            # Treat as a comment
            output_lines.append(f"// {line}")
    else:
        # Treat as a comment
        output_lines.append(f"// {line}")

# Write output to file
with open(output_file, 'w') as file:
    file.write('\n'.join(output_lines))

print(f"Output written to {output_file}")
