#!/bin/bash

# Fetch the current UNIX timestamp
current_time=$(date +%s)

# Save the timestamp to a temporary file
echo $current_time > /tmp/current_time.txt