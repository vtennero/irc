# concurrent_test.py
import socket
import threading
import time

def stress_client(client_id):
	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect(('localhost', 6667))

		# Guest
		s.send(f"PASS password123\r\n".encode())
		s.send(f"NICK test{client_id}\r\n".encode())
		s.send(f"USER test{client_id} 0 * :Test User\r\n".encode())

		# Send fragmented messages
		for i in range(50):
			msg = f"PRIVMSG #test :Message {i} from client {client_id}"
			for char in msg:
				s.send(char.encode())
				time.sleep(0.01)
			s.send(b"\r\n")

	except Exception as e:
		print(f"Client {client_id} error: {e}")
	finally:
		s.close()

# Launch 10 concurrent clients
threads = []
for i in range(10):
	t = threading.Thread(target=stress_client, args=(i,))
	threads.append(t)
	t.start()
	time.sleep(0.1)  # Slight delay between client starts

for t in threads:
	t.join()
