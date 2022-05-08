#! /bin/bash

function createFile() {
	dd if=/dev/random of=${1} bs=1K count=${2}
}

alias deleteFile="rm"
alias createDir="mkdir -p"
alias deleteDir="rm -r"
alias changeDir="cd"
alias dir="ls -alh"
alias sum="df -h . && stat -f ." # TODO: replace '.' with mount point
