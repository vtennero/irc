import socket
import time
from typing import List, Optional, Tuple

class IRCTester:
	def __init__(self, host: str = '127.0.0.1', port: int = 6667, password: str = 'password123'):
		self.host = host
		self.port = port
		self.password = password

	def connect(self) -> socket.socket:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.connect((self.host, self.port))
		return sock

	def authenticate(self, sock: socket.socket, nick: str = "testuser") -> List[str]:
		"""Authenticate with the server using PASS and NICK commands"""
		responses = []
		responses.extend(self.send_message(f"PASS {self.password}", sock))
		time.sleep(0.1)  # Small delay between commands
		responses.extend(self.send_message(f"NICK {nick}", sock))
		time.sleep(0.1)
		responses.extend(self.send_message(f"USER {nick} 0 * :Real Name", sock))
		time.sleep(0.5)  # Give server time to process registration
		return responses

	def get_authenticated_socket(self, nick: str = "testuser") -> Tuple[socket.socket, List[str]]:
		"""Get a new socket and authenticate it"""
		sock = self.connect()
		responses = self.authenticate(sock, nick)
		return sock, responses

	def send_message(self, message: str, sock: Optional[socket.socket] = None,
					wait_time: float = 0.5, expect_response: bool = True) -> List[str]:
		"""Send a message to the IRC server and optionally wait for response"""
		close_socket = False
		if sock is None:
			sock = self.connect()
			close_socket = True

		try:
			sock.send(f"{message}\r\n".encode())
			responses = []

			if expect_response:
				time.sleep(wait_time)  # Give server time to respond

				# Set socket to non-blocking to read all available data
				sock.setblocking(0)
				try:
					while True:
						try:
							data = sock.recv(4096).decode('utf-8', errors='ignore')
							if not data:
								break
							responses.append(data)
						except BlockingIOError:
							break
						except socket.error:
							break
				finally:
					sock.setblocking(1)

			if close_socket:
				sock.close()

			return responses

		except Exception as e:
			print(f"Error sending message: {e}")
			if close_socket:
				try:
					sock.close()
				except:
					pass
			return []

	def run_test(self, test_name: str, test_func, *args, **kwargs):
		"""Run a test and print its result"""
		print(f"\n=== Running Test: {test_name} ===")
		try:
			result = test_func(*args, **kwargs)
			if isinstance(result, list):
				print(f"Test completed. Responses:")
				for r in result:
					print(f"  {r.strip()}")
			else:
				print(f"Test completed: {result if result else 'No response'}")
		except Exception as e:
			print(f"Test failed: {e}")
