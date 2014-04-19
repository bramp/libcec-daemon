#!/bin/bash

# This script will set up libCEC on Ubuntu.
#
#  Copyright (c) 2011-2012 Pulse-Eight
#
#  Author: Lars Op den Kamp <lars.opdenkamp@pulse-eight.com>
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, you can find it on the World Wide
#  Web at http://www.gnu.org/copyleft/gpl.html, or write to the Free
#  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#

PACKAGE_TYPE="stable"
MIRROR="http://packages.pulse-eight.net/ubuntu"
SUPPORTED_VERSIONS=(hardy lucid maverick natty oneiric precise quantal raring saucy)

if [ ! -f /etc/lsb-release ]; then
  echo "This script only works on an Ubuntu machine"
  exit 1
fi

cat << EOF
This script will install libCEC. It will ask for your password once.

Press CTRL+C to abort.
EOF

source /etc/lsb-release
if [ -z "$DISTRIB_CODENAME" ]; then
  echo "Could not find the distribution codename in /etc/lsb-release.\nPlease enter the distribution codename (e.g. lucid)"
  read DISTRIB_CODENAME
fi

if [ -z "$DISTRIB_CODENAME" ]; then
  echo "No distribution name provided."
  exit 1
else
  supported=0
  for el in "${SUPPORTED_VERSIONS[@]}"; do
    if [[ "$el" == "$DISTRIB_CODENAME" ]]; then supported=1; fi
  done

  if [ $supported -eq 0 ]; then
    echo "Sorry, distribution '$DISTRIB_CODENAME' is not supported."
    exit 1
  fi
fi

echo "Updating apt sources"
sudo apt-get update

echo "Installing public GPG key"
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-key 557482C280AAED87
if [ ! $? -eq 0 ]; then
  echo "ERROR: failed to download the public GPG key"
  exit 1
fi

if [ ! $(grep -e "^deb \${MIRROR} \${DISTRIB_CODENAME} \${PACKAGE_TYPE}" /etc/apt/sources.list.d/xbmc-pulseeight-${PACKAGE_TYPE}.list > /dev/null 2>&1) ] ; then
  echo "Setting up Pulse-Eight's repository"
  cat << EOF | sudo tee /etc/apt/sources.list.d/xbmc-pulseeight-${PACKAGE_TYPE}.list
deb ${MIRROR} ${DISTRIB_CODENAME} ${PACKAGE_TYPE}
EOF
fi

if [ "x$1" == "xtesting" ]; then
  if [ ! $(grep -e "^deb \${MIRROR} \${DISTRIB_CODENAME} testing" /etc/apt/sources.list.d/xbmc-pulseeight-testing.list > /dev/null 2>&1) ] ; then
    echo "Setting up Pulse-Eight's testing repository"
    cat << EOF | sudo tee /etc/apt/sources.list.d/xbmc-pulseeight-testing.list
deb ${MIRROR} ${DISTRIB_CODENAME} testing
EOF
  fi
fi

echo "Updating repositories"
sudo apt-get update

echo "Installing/upgrading libCEC"
sudo apt-get -y install libcec cec-utils libcec-dev cec-firmware-upgrade

exit $?

