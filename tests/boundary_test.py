from irc_test_base import IRCTester
import time

def test_boundary_messages():
    tester = IRCTester()
    sock = tester.connect()

    # Authenticate first using the tester helper
    tester.authenticate(sock)

    # Join test channel
    tester.send_message("JOIN #test", sock)
    time.sleep(0.5)  # Wait for channel join to process

    # Test different message sizes
    boundary_messages = [
        "PRIVMSG #test :" + "X"*1022 + "\r\n",  # Just under 1024
        "PRIVMSG #test :" + "X"*1024 + "\r\n",  # Exactly 1024
        "PRIVMSG #test :" + "X"*1026 + "\r\n"   # Just over 1024
    ]

    def send_exact_bytes(data, chunk_size):
        try:
            # Send message in chunks
            for i in range(0, len(data), chunk_size):
                chunk = data[i:i + chunk_size]
                sock.send(chunk.encode())
                time.sleep(0.1)

            # Try to receive a response
            sock.settimeout(1.0)  # Set timeout for receive
            response = sock.recv(1024)
            print(f"Server response: {response.decode('utf-8', 'ignore')}")

        except Exception as e:
            print(f"Error during send/receive: {e}")

    # Test each boundary case
    for msg in boundary_messages:
        print(f"\nSending message of length {len(msg)}")
        send_exact_bytes(msg, 1023)
        time.sleep(0.5)

    sock.close()

if __name__ == "__main__":
    print("=== Starting Boundary Tests ===")
    test_boundary_messages()
    print("=== Boundary Tests Complete ===")
