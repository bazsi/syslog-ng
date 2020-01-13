#!/bin/sh

EXTRA_FILES_DIR=dbld/images/helpers/extra-files

mkdir -p ${EXTRA_FILES_DIR} || true
cp packaging/debian/control ${EXTRA_FILES_DIR}/packaging-debian-control
