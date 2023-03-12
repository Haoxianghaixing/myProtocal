import socket

def server_program():
	#get the hostname
	host = socket.gethostname()
	print(host)
	port = 5000
	try:
		server_socket = socket.socket()
		print("create socket succ!")

		server_socket.bind((host, port)) #bind host and port together
		print("bind socket succ!")

		server_socket.listen()
		print("listen succ!")
	except:
		print("init socket error!")

	conn, addr = server_socket.accept() #accept new connection
	print("Connection from:" + str(addr))

	while True:
		data = conn.recv(1024).decode()
		print("data from client: " + str(data))
		data = input('->')
		if not data:
			break
		conn.send(data.encode()) #respond client

	conn.close()
	print("end of service")

if __name__ == '__main__':
	server_program()