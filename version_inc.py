import re
import json
import os
from datetime import datetime
from datetime import date

version_h = os.getcwd() + '\include/version.h'
version_json = os.getcwd() + '\include/version.json'

version_string = ""

# Get current version string from the header file
with open(version_h, 'r') as file:
    for line in file:
        if "VERSION" in line:
            match = re.match(r'^(.*[^ ]) +([^ ]+) *$', line)
            if match:
                version_string = match.group(2)
                version_string = version_string.replace('"', '')

print(f"get current version: {version_string} from {version_h}")

major, minor, patch = map(int, version_string.split('.'))

# Increment to the new version string
patch += 1
version_string_new = f"{major}.{minor}.{patch}"

print(f"calc new version: {version_string_new}")

# datetime_now = datetime.now().strftime("%b %d %Y - %H:%M:%S
datetime_now = datetime.now().strftime("%d.%m.%Y - %H:%M:%S")

# Update the header file
with open(version_h, 'w', encoding='ascii') as file:
    file.write(f'#define VERSION "{version_string_new}"\n')
    file.write(f'#define BUILDTIME "{str(datetime_now)}"\n')
    file.write(f'#define BUILDTIMESTAMP "{int((datetime.now()).timestamp())}"')

# Update the JSON file
with open(version_json, 'r') as file:
    json_data = json.load(file)

json_data['version'] = version_string_new
json_data['versiondate'] = datetime_now

# Save the updated JSON data back to the file
with open(version_json, 'w', encoding='ascii') as file:
    json.dump(json_data, file, ensure_ascii=False, indent=4)
