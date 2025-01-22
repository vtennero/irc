# boundary_test.py
import socket
import time

def send_exact_bytes(sock, data, chunk_size):
    try:
        for i in range(0, len(data), chunk_size):
            chunk = data[i:i + chunk_size]
            sock.send(chunk.encode())
            time.sleep(0.1)
        # Try to receive a response
        response = sock.recv(1024)
        print(f"Server response: {response.decode('utf-8', 'ignore')}")
    except Exception as e:
        print(f"Error during send/receive: {e}")

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 6667))

msgs = [
    "PASS password123\r\n",
    "NICK test\r\n",
    "USER test 0 * :Test\r\n",
    "PRIVMSG #test :" + "X"*1022 + "\r\n",  # Just under 1024
    "PRIVMSG #test :" + "X"*1024 + "\r\n",  # Exactly 1024
    "PRIVMSG #test :" + "X"*1026 + "\r\n"   # Just over 1024
]

for msg in msgs:
    print(f"\nSending message of length {len(msg)}")
    send_exact_bytes(s, msg, 1023)
    time.sleep(0.5)
