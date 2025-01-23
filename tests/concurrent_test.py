from irc_test_base import IRCTester
import threading
import time
import socket

class ConcurrentTester(IRCTester):
    def stress_client(self, client_id):
        sock = None
        try:
            # Get authenticated socket using parent class method
            sock, _ = self.get_authenticated_socket(f"test{client_id}")
            if not sock:
                raise ConnectionError("Failed to create socket")

            # Send fragmented messages
            for i in range(50):
                msg = f"PRIVMSG #test :Message {i} from client {client_id}"
                # Send character by character
                for char in msg:
                    sock.send(char.encode())
                    time.sleep(0.01)
                sock.send(b"\r\n")

        except (socket.error, ConnectionError) as e:
            print(f"Client {client_id} error: {e}")
        finally:
            if sock:
                try:
                    sock.close()
                except:
                    pass

    def run_concurrent_test(self, num_clients=10):
        threads = []
        for i in range(num_clients):
            t = threading.Thread(target=self.stress_client, args=(i,))
            threads.append(t)
            t.start()
            time.sleep(0.1)  # Slight delay between client starts

        for t in threads:
            t.join()

if __name__ == "__main__":
    tester = ConcurrentTester(host='localhost', port=6667, password='password123')
    tester.run_concurrent_test()
