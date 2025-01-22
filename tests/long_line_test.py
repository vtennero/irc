# fixed_long_line_test.py
import socket
import time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 6667))

# Send authentication
s.send(b"PASS password123\r\n")
s.send(b"NICK test\r\n")
s.send(b"USER test 0 * :Test\r\n")

# Send a very long line without \r\n
long_msg = b"PRIVMSG #test :" + b"A" * 8192
print(f"Sending message of length {len(long_msg)}")
s.send(long_msg)

# Wait a bit then send the line ending
time.sleep(1)
s.send(b"\r\n")

# Try to receive response
try:
    response = s.recv(1024)
    print(f"Server response: {response.decode('utf-8', 'ignore')}")
except Exception as e:
    print(f"Error receiving response: {e}")
