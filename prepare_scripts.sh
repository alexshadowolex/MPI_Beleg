#!/bin/bash

# A script to pepare the other scripts with dos2unix and chmod 
echo "Starting dos2unix"
dos2unix *.sh

echo "Starting chmod"
chmod +x *.sh