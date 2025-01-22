import time
from irc_test_base import IRCTester

def run_message_tests():
    tester = IRCTester()

    def test_channel_operations():
        # Get authenticated connection
        sock, _ = tester.get_authenticated_socket("testuser2")

        responses = []
        # Test joining channel
        responses.extend(tester.send_message("JOIN #testchannel", sock))
        time.sleep(0.5)

        # Test sending message to channel
        responses.extend(tester.send_message("PRIVMSG #testchannel :Hello World", sock))
        time.sleep(0.5)

        # Test multiple channel join
        responses.extend(tester.send_message("JOIN #channel1,#channel2,#channel3", sock))

        sock.close()
        return responses

    def test_malformed_messages():
        # Get authenticated connection
        sock, _ = tester.get_authenticated_socket("testuser3")

        responses = []
        # Test partial commands
        responses.extend(tester.send_message("PR", sock))
        responses.extend(tester.send_message("PRIV", sock))

        # Test split commands
        responses.extend(tester.send_message("PRIVMSG\n#channel", sock))

        sock.close()
        return responses

    print("\n=== Starting Message Tests ===")
    tester.run_test("Channel Operations", test_channel_operations)
    tester.run_test("Malformed Messages", test_malformed_messages)
    print("=== Message Tests Complete ===")

if __name__ == "__main__":
    run_message_tests()
