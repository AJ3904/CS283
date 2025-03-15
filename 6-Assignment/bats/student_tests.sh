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

@test "Local: Custom exit" {
    run ./dsh <<EOF
exit
EOF

    [ "$status" -eq 0 ]
}

@test "Local: Custom cd" {
    run ./dsh <<EOF
cd /tmp
pwd
exit
EOF

    echo "Output: $output"
    [[ "$output" == *"/tmp"* ]]
    [ "$status" -eq 0 ]
}

@test "Local: Dragon" {
    run ./dsh <<EOF
dragon
EOF

    echo "Output: $output"
    expected_output=$(cat <<EOL
                                                                        @%%%%                       
                                                                     %%%%%%                         
                                                                    %%%%%%                          
                                                                 % %%%%%%%           @              
                                                                %%%%%%%%%%        %%%%%%%           
                                       %%%%%%%  %%%%@         %%%%%%%%%%%%@    %%%%%%  @%%%%        
                                  %%%%%%%%%%%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%%%%%%%%%%%%          
                                %%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%%%%%%% %%%%%%%%%%%%%%%           
                               %%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%     %%%            
                             %%%%%%%%%%%%%%%%%%%%%%%%%%%%@ @%%%%%%%%%%%%%%%%%%        %%            
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%                
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@%%%%%%@              
      %%%%%%%%@           %%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%      %%                
    %%%%%%%%%%%%%         %%@%%%%%%%%%%%%           %%%%%%%%%%% %%%%%%%%%%%%      @%                
  %%%%%%%%%%   %%%        %%%%%%%%%%%%%%            %%%%%%%%%%%%%%%%%%%%%%%%                        
 %%%%%%%%%       %         %%%%%%%%%%%%%             %%%%%%%%%%%%@%%%%%%%%%%%                       
%%%%%%%%%@                % %%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%                     
%%%%%%%%@                 %%@%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  
%%%%%%%@                   %%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              
%%%%%%%%%%                  %%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%  
%%%%%%%%%@                   @%%%%%%%%%%%%%%         %%%%%%%%%%%%@ %%%% %%%%%%%%%%%%%%%%%   %%%%%%%%
%%%%%%%%%%                  %%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%% %%%%%%%%%
%%%%%%%%%@%%@                %%%%%%%%%%%%%%%%@       %%%%%%%%%%%%%%     %%%%%%%%%%%%%%%%%%%%%%%%  %%
 %%%%%%%%%%                  % %%%%%%%%%%%%%%@        %%%%%%%%%%%%%%   %%%%%%%%%%%%%%%%%%%%%%%%%% %%
  %%%%%%%%%%%%  @           %%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%% 
   %%%%%%%%%%%%% %%  %  %@ %%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%% 
    %%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%           @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%%%%%% 
     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              %%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%   
      @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  %%%%%%%%%%%%%%%%%%%%%%%%%               
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%%%%%%  %%%%%%%          
           %%%%%%%%%%%%%%%%%%%%%%%%%%                           %%%%%%%%%%%%%%%  @%%%%%%%%%         
              %%%%%%%%%%%%%%%%%%%%           @%@%                  @%%%%%%%%%%%%%%%%%%   %%%        
                  %%%%%%%%%%%%%%%        %%%%%%%%%%                    %%%%%%%%%%%%%%%    %         
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%            
                %%%%%%%%%%%%%%%%%%%%%%%%%%  %%%% %%%                      %%%%%%%%%%  %%%@          
                     %%%%%%%%%%%%%%%%%%% %%%%%% %%                          %%%%%%%%%%%%%@          
                                                                                 %%%%%%%@       
EOL
)
    [[ "$output" == *"$expected_output"* ]]
    [ "$status" -eq 0 ]
}

@test "Local: Non-existant command" {
    run ./dsh <<EOF
not_exists
exit
EOF

    echo "Output: $output"
    [[ "$output" == *"Command failed with error code: 2"* ]]
    [ "$status" -eq 0 ]  # Shell should continue after command not found
}

@test "Local: Command with multiple pipes" {
    run ./dsh <<EOF
ls | grep '\.c$' | wc -l
exit
EOF

    echo "Output: $output"
    [[ "$output" == *"5"* ]]
    [ "$status" -eq 0 ]
}

@test "Server startup and shutdown" {
    TEST_PORT=25123
    
    # Start server in background
    ./dsh -s -p $TEST_PORT > server_output.tmp 2>&1 &
    SERVER_PID=$!

    sleep 3
    
    # Verify server is running
    if ! ps -p $SERVER_PID > /dev/null; then
        echo "ERROR: Server failed to start"
        cat server_output.tmp
        return 1
    fi

    # Kill Server
    if ps -p $SERVER_PID > /dev/null; then
        echo "Server didn't respond to SIGTERM, using SIGKILL"
        kill -9 $SERVER_PID
        sleep 2
    fi
    
    # Verify server is stopped
    if ps -p $SERVER_PID > /dev/null; then
        echo "ERROR: Server failed to shut down even with SIGKILL"
        ps -f $SERVER_PID
        return 1
    else
        echo "Server successfully shut down"
    fi
    
    # Clean up
    rm -f server_output.tmp
}

@test "Connect client to server" {
    TEST_PORT=25884
    
    pkill -f "dsh -s -p $TEST_PORT" || true
    sleep 1
    
    # Start server in background
    ./dsh -s -p $TEST_PORT > server_output.log 2>&1 &
    SERVER_PID=$!
    
    sleep 3
    
    # Connect to server
    echo "echo 'Testing connection'" | ./dsh -c 127.0.0.1 -p $TEST_PORT > client_output.log 2>&1
    cat client_output.log
    
    # Check if connection worked
    if grep -q "Testing connection" client_output.log; then
        output=$(cat client_output.log)
    else
        output="None of the client connection attempts succeeded"
    fi
    
    # Output for debugging
    echo "Final output: $output"
    
    # Clean up
    kill $SERVER_PID || true
    sleep 2
    rm -f server_output.log client_output.log
    
    # Check if output contains success message
    [[ "$output" == *"Testing connection"* ]]
}

@test "Cannot connect to non existent server" {
    OUTPUT=$(./dsh -c localhost -p 65432 2>&1 || true)
    STATUS_CODE=$?
    
    echo "Output: $OUTPUT"
    echo "Status code: $STATUS_CODE"
    
    # Check for connection failure
    [[ "$OUTPUT" == *"cmd loop returned -52"* ]]
    [[ "$STATUS_CODE" == 0 ]]
}

@test "Remote: Single command" {
    
    # Start server in background with a unique port
    TEST_PORT=12346
    ./dsh -s -p $TEST_PORT &
    SERVER_PID=$!

    sleep 1
    
    # Run command
    run ./dsh -c -p $TEST_PORT <<EOF
echo "hello world"
exit
EOF

    # Stop the server
    kill $SERVER_PID
    
    echo "Output: $output"
    [[ "$output" == *"hello world"* ]]
    [ "$status" -eq 0 ]
}

@test "Remote: Dragon" {
    
    # Start server in background with a unique port
    TEST_PORT=12346
    ./dsh -s -p $TEST_PORT &
    SERVER_PID=$!

    sleep 1
    
    # Run command
    run ./dsh -c -p $TEST_PORT <<EOF
dragon
exit
EOF

    # Stop the server
    kill $SERVER_PID
    
    echo "Output: $output"
    expected_output=$(cat <<EOL
                                                                        @%%%%                       
                                                                     %%%%%%                         
                                                                    %%%%%%                          
                                                                 % %%%%%%%           @              
                                                                %%%%%%%%%%        %%%%%%%           
                                       %%%%%%%  %%%%@         %%%%%%%%%%%%@    %%%%%%  @%%%%        
                                  %%%%%%%%%%%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%%%%%%%%%%%%          
                                %%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%%%%%%% %%%%%%%%%%%%%%%           
                               %%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%     %%%            
                             %%%%%%%%%%%%%%%%%%%%%%%%%%%%@ @%%%%%%%%%%%%%%%%%%        %%            
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%                
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@%%%%%%@              
      %%%%%%%%@           %%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%      %%                
    %%%%%%%%%%%%%         %%@%%%%%%%%%%%%           %%%%%%%%%%% %%%%%%%%%%%%      @%                
  %%%%%%%%%%   %%%        %%%%%%%%%%%%%%            %%%%%%%%%%%%%%%%%%%%%%%%                        
 %%%%%%%%%       %         %%%%%%%%%%%%%             %%%%%%%%%%%%@%%%%%%%%%%%                       
%%%%%%%%%@                % %%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%                     
%%%%%%%%@                 %%@%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  
%%%%%%%@                   %%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              
%%%%%%%%%%                  %%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%  
%%%%%%%%%@                   @%%%%%%%%%%%%%%         %%%%%%%%%%%%@ %%%% %%%%%%%%%%%%%%%%%   %%%%%%%%
%%%%%%%%%%                  %%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%% %%%%%%%%%
%%%%%%%%%@%%@                %%%%%%%%%%%%%%%%@       %%%%%%%%%%%%%%     %%%%%%%%%%%%%%%%%%%%%%%%  %%
 %%%%%%%%%%                  % %%%%%%%%%%%%%%@        %%%%%%%%%%%%%%   %%%%%%%%%%%%%%%%%%%%%%%%%% %%
  %%%%%%%%%%%%  @           %%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%% 
   %%%%%%%%%%%%% %%  %  %@ %%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%% 
    %%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%           @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%%%%%% 
     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              %%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%   
      @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  %%%%%%%%%%%%%%%%%%%%%%%%%               
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%%%%%%  %%%%%%%          
           %%%%%%%%%%%%%%%%%%%%%%%%%%                           %%%%%%%%%%%%%%%  @%%%%%%%%%         
              %%%%%%%%%%%%%%%%%%%%           @%@%                  @%%%%%%%%%%%%%%%%%%   %%%        
                  %%%%%%%%%%%%%%%        %%%%%%%%%%                    %%%%%%%%%%%%%%%    %         
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%            
                %%%%%%%%%%%%%%%%%%%%%%%%%%  %%%% %%%                      %%%%%%%%%%  %%%@          
                     %%%%%%%%%%%%%%%%%%% %%%%%% %%                          %%%%%%%%%%%%%@          
                                                                                 %%%%%%%@       
EOL
)
    [[ "$output" == *"$expected_output"* ]]
    [ "$status" -eq 0 ]
}

@test "Remote: Custom cd" {
    
    # Start server in background with a unique port
    TEST_PORT=12348
    ./dsh -s -p $TEST_PORT &
    SERVER_PID=$!

    sleep 1
    
    # Run cd command
    run ./dsh -c -p $TEST_PORT <<EOF
cd /tmp
pwd
exit
EOF

    # Stop the server
    kill $SERVER_PID
    
    echo "Output: $output"
    [[ "$output" == *"/tmp"* ]]
    [ "$status" -eq 0 ]
}

@test "Remote: stop-server" {
    # Start server in background with a unique port
    TEST_PORT=12349
    ./dsh -s -p $TEST_PORT &
    SERVER_PID=$!
    
    sleep 2
    
    # Verify server is running
    ps -p $SERVER_PID > /dev/null || {
        echo "Server failed to start properly"
        return 1
    }
    
    # Run stop-server command
    echo "stop-server" | ./dsh -c localhost -p $TEST_PORT

    sleep 3
    
    # Check if server process still exists
    if ps -p $SERVER_PID > /dev/null; then
        echo "ERROR: Server process $SERVER_PID still running!"
        kill -9 $SERVER_PID
        return 1
    else
        echo "Server successfully terminated"
    fi
}

@test "Remote: Non-existant command" {
    
    # Start server in background with a unique port
    TEST_PORT=12346
    ./dsh -s -p $TEST_PORT &
    SERVER_PID=$!

    sleep 1
    
    # Run command
    run ./dsh -c -p $TEST_PORT <<EOF
not_exists
exit
EOF

    # Stop the server
    kill $SERVER_PID
    
    echo "Output: $output"
    [[ "$output" == *"not_exists: command not found"* ]]
    [ "$status" -eq 0 ]
}

@test "Remote: Execute pipes" {
    
    # Start server in background with a unique port
    TEST_PORT=12347
    ./dsh -s -p $TEST_PORT &
    SERVER_PID=$!

    sleep 1
    
    # Run command with pipes
    run ./dsh -c -p $TEST_PORT <<EOF
ls | wc -l
exit
EOF

    # Stop the server
    kill $SERVER_PID
    
    echo "Output: $output"
    [[ "$output" == *"12"* ]]
    [ "$status" -eq 0 ]
}
