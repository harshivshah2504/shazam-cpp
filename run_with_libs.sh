#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib:/home/vscode/mongo-driver/mongo-cxx-driver/build/src/mongocxx:/home/vscode/mongo-driver/mongo-cxx-driver/build/src/bsoncxx:$LD_LIBRARY_PATH
exec "$@"
