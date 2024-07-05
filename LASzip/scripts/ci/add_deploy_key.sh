#!/bin/bash

if [ -n "$encrypted_de53628fc98a_key" ]; then
    echo "decrypting SSH for github login"
    openssl aes-256-cbc -K $encrypted_de53628fc98a_key -iv $encrypted_de53628fc98a_iv -in scripts/ci/pdaldocs-private.key.enc -out scripts/ci/pdaldocs-private.key -d
#    openssl aes-256-cbc -K $encrypted_de53628fc98a_key -iv $encrypted_de53628fc98a_iv -in scripts/ci/pdaldocs-private.key.enc -out ~\/.ssh/pdal-docs/id_rsa -d

    cp scripts/ci/pdaldocs-private.key ~/.ssh/id_rsa
    rm scripts/ci/pdaldocs-private.key
    chmod 600 ~/.ssh/id_rsa
    echo -e "Host *\n\tStrictHostKeyChecking no\n" > ~/.ssh/config
fi;


