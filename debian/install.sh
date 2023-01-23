#!/bin/bash
. debian/vars
set -e
set -x
rm -rf ${CURDIR}/debian/tmp

INSTALL_ROOT="${CURDIR}/debian/tmp" make install

desktop-file-install --delete-original       \
  --dir ${CURDIR}/debian/tmp/usr/share/applications             \
   ${CURDIR}/debian/tmp/usr/share/applications/*.desktop
