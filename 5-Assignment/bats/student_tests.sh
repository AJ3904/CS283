#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Multiple Pipes" {
    run "./dsh" <<EOF
ls | grep dshlib | sort
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dshlib.cdshlib.hdsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Empty Pipe Input" {
    run "./dsh" <<EOF
cat /dev/null | grep dshlib.c
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>Commandfailedwitherrorcode:1dsh3>cmdloopreturned1"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Pipe to echo" {
    run "./dsh" <<EOF
echo "Hello, dsh!" | grep dsh
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="Hello,dsh!dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Invalid Command in Pipe" {
    run "./dsh" <<EOF
ls | non_existent_cmd | grep dshlib.c
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>Commandfailedwitherrorcode:2dsh3>cmdloopreturned2"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Too many pipe commands" {
    run "./dsh" <<EOF
echo "1" | cat | cat | cat | cat | cat | cat | cat | cat | cat
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>error:pipinglimitedto8commandscmdloopreturned-2"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}