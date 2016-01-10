#!/bin/bash

set -e

CURRENT_DIR=`dirname $0`
pushd ${CURRENT_DIR}/..

function reset_current_directory {
  popd
}
trap reset_current_directory EXIT

rm /usr/local/bin/filesearch
cp build/filesearch /usr/local/bin/filesearch

