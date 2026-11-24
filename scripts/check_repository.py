#!/usr/bin/env python3
"""Validate the organized source tree without requiring a ROS installation."""

import ast
import csv
import math
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parent.parent
EXPECTED_PACKAGES = {
    'racecar', 'multi_goal', 'find_obj', 'encoder_driver', 'racecar_driver',
    'hi226_imu', 'imu_launch', 'ls01b_v2', 'serial_imu', 'serial',
    'rf2o_laser_odometry', 'rplidar_ros', 'imu_complementary_filter',
    'imu_filter_madgwick', 'rviz_imu_plugin',
}


def main():
    errors = []
    packages = {}
    for manifest in sorted((ROOT / 'src').rglob('package.xml')):
        try:
            name = ET.parse(manifest).getroot().findtext('name')
            if name in packages:
                errors.append('Duplicate ROS package: ' + str(name))
            packages[name] = manifest.parent
        except ET.ParseError as exc:
            errors.append(str(manifest.relative_to(ROOT)) + ': ' + str(exc))
    if set(packages) != EXPECTED_PACKAGES:
        errors.append('Unexpected package set: ' + str(set(packages) ^ EXPECTED_PACKAGES))

    xml_files = list((ROOT / 'src').rglob('*.xml')) + list((ROOT / 'src').rglob('*.launch'))
    for path in xml_files:
        try:
            ET.parse(path)
        except ET.ParseError as exc:
            errors.append(str(path.relative_to(ROOT)) + ': ' + str(exc))

    python_files = list((ROOT / 'src/core').rglob('*.py')) + list((ROOT / 'src/drivers').rglob('*.py'))
    python_files += list((ROOT / 'scripts').glob('*.py'))
    for path in python_files:
        try:
            ast.parse(path.read_bytes(), filename=str(path.relative_to(ROOT)))
        except (SyntaxError, UnicodeError) as exc:
            errors.append(str(path.relative_to(ROOT)) + ': ' + str(exc))

    installed_entries = [
        'src/core/multi_goal/scripts/goal_loop.py',
        'src/core/multi_goal/scripts/getpoints.py',
        'src/drivers/encoder_driver/src/Encoder_vel.py',
        'src/core/racecar/scripts/camera.py',
        'src/core/racecar/src/racecar_teleop.py',
        'src/core/racecar/src/racecar_joy.py',
        'src/core/racecar/src/ESC_calibration.py',
    ]
    for relative in installed_entries:
        path = ROOT / relative
        if not path.read_bytes().startswith(b'#!/usr/bin/env python3\n'):
            errors.append('Missing Python 3 shebang: ' + relative)
        if '/home/racecar/' in path.read_text(encoding='utf-8'):
            errors.append('Original machine path remains: ' + relative)

    maps = list((ROOT / 'src/core/racecar/map').glob('*.yaml'))
    for path in maps:
        text = path.read_text(encoding='utf-8')
        match = re.search(r'^image:\s*(\S+)\s*$', text, flags=re.M)
        if not match or Path(match.group(1)).is_absolute():
            errors.append('Missing or absolute map image path: ' + path.name)
            continue
        image = path.parent / match.group(1)
        if not image.is_file():
            errors.append('Missing map image: ' + str(image.relative_to(ROOT)))
        elif not image.read_bytes().startswith(b'P5\n'):
            errors.append('Unexpected PGM header: ' + image.name)

    routes = list((ROOT / 'src/core/multi_goal/waypoints').rglob('*.csv'))
    if not routes:
        errors.append('No route CSV files found')
    for path in routes:
        try:
            with path.open(encoding='utf-8', newline='') as stream:
                rows = list(csv.reader(stream))
            if not rows:
                raise ValueError('empty route')
            for line, row in enumerate(rows, 1):
                if len(row) != 4 or not all(math.isfinite(float(v)) for v in row):
                    raise ValueError('expected four finite numbers at line ' + str(line))
        except (ValueError, UnicodeError, csv.Error) as exc:
            errors.append(str(path.relative_to(ROOT)) + ': ' + str(exc))

    for relative in [
        'src/core/racecar/src/lib/libracecar_controller.a',
        'src/drivers/racecar_driver/lib/libracecar_driver.a',
    ]:
        path = ROOT / relative
        if not path.is_file() or not path.read_bytes().startswith(b'!<arch>\n'):
            errors.append('Missing or invalid static archive: ' + relative)

    # Check relative Markdown links; external links and Mermaid are not executed.
    for path in [ROOT / 'README.md', ROOT / 'README.zh-CN.md', *sorted((ROOT / 'docs').glob('*.md'))]:
        for target in re.findall(r'\]\(([^)]+)\)', path.read_text(encoding='utf-8')):
            if '://' not in target and not target.startswith('#'):
                if not (path.parent / target.split('#', 1)[0]).exists():
                    errors.append('Broken link in ' + str(path.relative_to(ROOT)) + ': ' + target)

    for path in (ROOT / 'src').rglob('.git'):
        errors.append('Nested Git metadata: ' + str(path.relative_to(ROOT)))

    if errors:
        for error in errors:
            print('FAIL:', error)
        return 1
    print('PASS: {} ROS packages; {} XML/launch files; {} Python files; {} maps; {} routes.'.format(
        len(packages), len(xml_files), len(python_files), len(maps), len(routes)))
    print('Static validation only. ROS compilation and vehicle tests are still required.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
