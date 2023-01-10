#!/bin/bash
clang-format -style=file -i `find src -name "*.c"`
clang-format -style=file -i `find src -name "*.h"`
