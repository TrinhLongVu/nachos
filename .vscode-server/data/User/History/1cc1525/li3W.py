import socket

# create a socket object
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# get the local machine name
host = socket.gethostname()

# set a port number for the server to listen on
port = 1234

# bind the socket object to the host and port
server_socket.bind((host, port))

# start listening for incoming connections
server_socket.listen(1)

# wait for a connection
print('Waiting for a connection...')
client_socket, address = server_socket.accept()
print('Connected by', address)

# send a welcome message to the client
message = 'Thank you for connecting!'
client_socket.send(message.encode())

# close the connection
client_socket.close()
