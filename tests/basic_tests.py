from irc_test_base import IRCTester
import time

def run_basic_tests():
	tester = IRCTester()

	def test_basic_connection():
		sock = tester.connect()
		result = "Connection established"
		sock.close()
		return result

	def test_pass():
		# Test wrong password
		sock = tester.connect()
		result = tester.send_message("PASS wrongpassword", sock)
		sock.close()

		# Test empty password
		sock = tester.connect()
		result2 = tester.send_message("PASS ", sock)
		sock.close()

		# Test correct password sequence
		sock = tester.connect()
		result3 = tester.authenticate(sock)
		sock.close()

		return result + result2 + result3

	def test_registration():
		# Test full registration sequence
		sock, responses = tester.get_authenticated_socket("testuser1")
		sock.close()
		return responses

	print("\n=== Starting Basic Connection Tests ===")
	tester.run_test("Basic Connection", test_basic_connection)
	tester.run_test("PASS Tests", test_pass)
	tester.run_test("Registration", test_registration)
	print("=== Basic Connection Tests Complete ===")

if __name__ == "__main__":
	run_basic_tests()
