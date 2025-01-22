import threading
import time
from irc_test_base import IRCTester

def run_stress_tests():
    tester = IRCTester()

    def test_large_messages():
        sock, _ = tester.get_authenticated_socket("testuser4")
        responses = []

        # Test large message
        large_msg = "A" * 8192
        responses.extend(tester.send_message(f"PRIVMSG #channel :{large_msg}", sock))

        # Test very large message
        very_large_msg = "A" * 100000
        responses.extend(tester.send_message(f"PRIVMSG #channel :{very_large_msg}", sock))

        sock.close()
        return responses

    def test_special_characters():
        responses = []

        # Test Unicode username
        sock, auth_resp = tester.get_authenticated_socket("用户")
        responses.extend(auth_resp)
        sock.close()

        # Test emoji in username
        sock, auth_resp = tester.get_authenticated_socket("test👾user")
        responses.extend(auth_resp)
        sock.close()

        # Test Unicode channel
        sock, _ = tester.get_authenticated_socket("testuser5")
        responses.extend(tester.send_message("JOIN #チャンネル", sock))
        sock.close()

        return responses

    def test_multiple_connections():
        responses = []
        def connection_thread(thread_num):
            try:
                sock, auth_resp = tester.get_authenticated_socket(f"test{thread_num}")
                responses.extend(auth_resp)
                time.sleep(0.1)
                sock.close()
            except Exception as e:
                print(f"Thread {thread_num} error: {e}")

        threads = []
        for i in range(10):  # Reduced to 10 simultaneous connections for stability
            t = threading.Thread(target=connection_thread, args=(i,))
            threads.append(t)
            t.start()
            time.sleep(0.1)  # Small delay between thread starts

        for t in threads:
            t.join()

        return responses

    print("\n=== Starting Stress Tests ===")
    tester.run_test("Large Messages", test_large_messages)
    tester.run_test("Special Characters", test_special_characters)
    print("Testing multiple connections (this may take a moment)...")
    tester.run_test("Multiple Connections", test_multiple_connections)
    print("=== Stress Tests Complete ===")

if __name__ == "__main__":
    run_stress_tests()
