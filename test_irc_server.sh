#!/bin/bash

# test_irc_server.sh
echo "IRC Server Test Suite"
echo "====================\n"

SERVER="localhost"
PORT=6667
PASSWORD="password123"

test_fragmented_message() {
    echo "Test 1: Fragmented Message"
    {
        echo -n "PASS $PASSWORD\r"
        sleep 1
        echo -n "\n"
        sleep 1
        echo -n "NICK test"
        sleep 1
        echo -n "user\r\n"
        sleep 1
        echo -n "USER test"
        sleep 1
        echo " 0 * :Testing User\r\n"
    } | nc $SERVER $PORT
    echo "Fragmented message test completed\n"
}

test_message_flooding() {
    echo "Test 2: Message Flooding"
    {
        echo "PASS $PASSWORD\r\n"
        echo "NICK flooder\r\n"
        echo "USER flood 0 * :Flood Test\r\n"
        for i in {1..100}; do
            echo "PRIVMSG #test :Flood message $i with some padding data to make it longer and test buffer handling\r\n"
        done
    } | nc $SERVER $PORT
    echo "Message flooding test completed\n"
}

test_null_bytes() {
    echo "Test 3: Null Byte Handling"
    python3 -c "
import socket
s=socket.socket()
s.connect(('$SERVER',$PORT))
s.send(b'PASS $PASSWORD\r\n')
s.send(b'NICK nulltest\r\nUSER test 0 * :Test\r\nPRIVMSG #test :Hello\x00World\r\n')
s.close()
"
    echo "Null byte test completed\n"
}

# Run all tests
test_fragmented_message
sleep 2
test_message_flooding
sleep 2
test_null_bytes

echo "All tests completed"
