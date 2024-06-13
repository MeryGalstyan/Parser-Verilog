#!/bin/bash

[ "$#" -ne 1 ] && { echo "Usage: $0 <verilog_file>"; exit 1; }
[ -e ontology.json ] && rm ontology.json
[ -e ontology_test_back_up.json ] && cp ontology_test_back_up.json ontology.json
./sample_parser "$1"

