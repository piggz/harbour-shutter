#!/bin/bash
. debian/vars
set -e
set -x
rm -rf ${CURDIR}/debian/tmp
# >> install pre
# << install pre
%qmake5_install
# >> install post
# << install post
desktop-file-install --delete-original       \
  --dir ${CURDIR}/debian/tmp/usr/share/applications             \
   ${CURDIR}/debian/tmp/usr/share/applications/*.desktop
