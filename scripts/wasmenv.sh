#!/usr/bin/env bash
set -e
#EMSDK_VERSION="tot-upstream"
EMSDK_VERSION="latest"

ensure_repo() {
  local url name
  local "${@}"

  git -C ${name} pull || git clone ${url} ${name}
}

ensure_emscripten() {
  ensure_repo url='https://github.com/emscripten-core/emsdk.git' name='emsdk'
  pushd 'emsdk'
  ./emsdk update-tags
  ./emsdk install ${EMSDK_VERSION}
  ./emsdk activate ${EMSDK_VERSION}
  source ./emsdk_env.sh
  popd
}

main() {
  ensure_emscripten
}

main