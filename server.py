from threading import Thread, main_thread
from os import path
import socket
import logging
import threading
from client_manager import ClientManager
from message_manager import MessageManager
from request import Request
from request_handler import RequestHandler


class Server():
    MAX_PARALLEL_CONNECTIONS = 10
    MAX_PACKET_SIZE = 512


    def __init__(self, port_info = "port.info"):
        self.__config_file = port_info
        self.__connection_workers = []
        self.__client_manager = ClientManager()
        self.__message_manager = MessageManager() 
        self.__server_sock = None

        # initialize the socket and start listening
        self.__init_socket(Server.MAX_PARALLEL_CONNECTIONS)

        # initialize the logger for debugging
        self.__init_logger()


    def loop(self):
        # endless loop of accepting connections
        while True:
            connection_socket, connection_addr = self.__server_sock.accept()
            self.__log_info("New Connection from {}".format(connection_addr, ))

            self.__spawn_worker(connection_socket)

        # Cleanup all the resources including the worker threads        
        self.__cleanup()


    def __spawn_worker(self, connection_socket):
        try:
            worker = threading.Thread(target = self.__worker_handle_connection,
                                    args = (connection_socket,))
            worker.start()
        except:
            self.__log_exception("Failed to spawn new worker")
            return

        self.__log_debug("New worker thread started: {}".format(worker.ident))
        self.__connection_workers.append(worker)

        
    def __worker_handle_connection(self, connection_socket):
        data_received = connection_socket.recv(Request.HEADER_SIZE)
        self.__log_debug("Received : {}".format(data_received))
        if len(data_received) == 0:
            self.__log_debug("Connection ended unexpectedly")
            return
        
        payload_size = len(data_received)
        request = Request()

        if not request.decode_header(data_received):
            self.__log_info("Malformed packet received - worker ends")
            resp = RequestHandler.get_err_response()
            connection_socket.send(resp.build())
            return
        
        # continue to receive data
        payload = data_received[Request.HEADER_SIZE :]
        while  payload_size < request.payload_size:
            data_received = connection_socket.recv(Server.MAX_PACKET_SIZE)
            payload += data_received

            if len(data_received) < Server.MAX_PACKET_SIZE:
                #No more data to receive (its a stateless protocol!)
                break
        
        if not request.decode_payload(payload):
            self.__log_info("Malformed packet received - worker ends")
            resp = RequestHandler.get_err_response()
            connection_socket.send(resp.build())
            return

        # Handle the request
        request_handler = RequestHandler(self.__client_manager, self.__message_manager)
        request_handler.handle(request)


    def __cleanup(self):
        self.__log_debug("Close connection workers")
        #server_thread = threading.current_thread()
        for thread in self.__connection_workers:
            if thread.is_alive():
                thread.join()


    def __delete__(self):
        pass


    def __get_server_port(self):
        if not path.isfile(self.__config_file):
            raise Exception("Failed to find server's config file")

        with open(self.__config_file, "r") as f:
            line = f.readline()

            if line == b"":
                raise Exception("Failed to interpret port number")

            line = line.strip()
            if not line.isnumeric():
                raise Exception("Invalid port specifed in {}".format( \
                                self.__config_file))

            return int(line)



    def __init_socket(self, max_connections):
        self.__server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__server_sock.bind(("localhost", self.__get_server_port()))
        self.__server_sock.listen(max_connections)


    def __init_logger(self):
        self.__logger = logging.getLogger(self.__class__.__name__)
        self.__logger.setLevel(logging.DEBUG)
        if not self.__logger.hasHandlers():
            sh = logging.StreamHandler()
            formatter = logging.Formatter("%(name)s - %(levelname)s - %(message)s")
            sh.setFormatter(formatter)
            self.__logger.addHandler(sh)


    # Wrappers for logging functionality
    def __log_debug(self, msg):
        self.__logger.debug(msg)
    
    def __log_info(self, msg):
        self.__logger.info(msg)
    
    def __log_critical(self, msg):
        self.__logger.critical(msg)
    
    def __log_exception(self, msg):
        self.__logger.exception(msg)



if __name__ == "__main__":
    s = Server()
    s.loop()