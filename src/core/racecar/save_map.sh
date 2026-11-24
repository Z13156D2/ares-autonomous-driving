#!/bin/bash
set -e
map_name="${1:-mymap_6}"
rosrun map_server map_saver -f "$(rospack find racecar)/map/${map_name}"
