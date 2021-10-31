import logging
import struct
import uuid
import request
import response
from config import *
from client_manager import ClientManager
from message_manager import MessageManager




class RequestHandler():
    def __init__(self, client_mgr, messages_mgr):
        self.__client_mgr = client_mgr
        self.__messages_mgr = messages_mgr

        # Initialize internal logger
        self.__init_logger()
        self.__requests_handlers = {
            request.CODE_REGISTER : self.__register_user,
            request.CODE_LIST_CLIENTS: self.__list_clients,
            request.CODE_SEND_MSG: self.__send_msg,
            request.CODE_FETCH_PUBLIC_KEY: self.__get_pubkey,
            request.CODE_FETCH_MESSAGES: self.__fetch_messages
            }


    def handle(self, request_obj):
        code = request_obj.request_code

        # Another chekc for the request code
        if code not in request.SUPPORTED_CODES:
            self.__log_debug("Recevied bad request code")
            return RequestHandler.get_err_response()

        self.__log_debug("Handling request code: {}".format(code))

        return self.__requests_handlers[code](request_obj.payload , request_obj.client_id)
        

    def __register_user(self, params_raw, *args):
        payload_size = ClientManager.NAME_SIZE + ClientManager.PUBLIC_KEY_SIZE

        if len(params_raw) != payload_size:
            self.__log_debug("Invalid payload for client registration req.")
            return RequestHandler.get_err_response()

        client_name = params_raw[: ClientManager.NAME_SIZE].strip(b"\x11")
        if type(client_name) is bytes:
            client_name = client_name.decode("utf-8")

        #Return error response if client exists
        if self.__client_mgr.is_client_exist(client_name):
            self.__log_debug("Client {} already exists!".format(client_name))
            return RequestHandler.get_err_response()
        
        # generate new uuid
        client_id = uuid.uuid4()
        client_id = client_id.bytes_le
        
        public_key = params_raw[ClientManager.NAME_SIZE :].strip(b'\x11')
        public_key.decode("utf-8")

        self.__client_mgr.add_client(client_id, client_name, public_key)

        response_obj = response.Response(SUPPORTED_VERSION,
                         response.CODE_SUCCESSFUL_REGISTRATION)
        response_obj.build(payload = client_id)
        
        return response_obj


    
    def __list_clients(self, params_raw, *args):
        resp_payload = b""

        clients_list = self.__client_mgr.fetch_all()

        for client in clients_list:
            client_id, client_name = client[0], client[1]
            # concat UUID and name ( name should be 255 bytes and UUID 16)
            if len(client_id) < ClientManager.CLIENT_ID_SIZE:
                client_id += b"\x00" * (ClientManager.CLIENT_ID_SIZE - len(client_id))

            if len(client_name) < ClientManager.NAME_SIZE:
                client_name += b"\x00" * (ClientManager.NAME_SIZE - len(client_name))

            resp_payload += client_id + client_name

        response_obj = response.Response(SUPPORTED_VERSION, 
                                            response.CODE_CLIENTS_LIST)
        response_obj.build(resp_payload)

        return response_obj


    def __send_msg(self, params_raw, *args):
        request_min_size = (ClientManager.CLIENT_ID_SIZE + \
                                request.FIELD_SIZE_MSG_TYPE_SIZE + \
                                request.FIELD_SIZE_MSG_CONTENT_SIZE)
        
        self.__log_debug("Sendmsg: {}".format(params_raw))
        # Sanity check
        if len(params_raw) < request_min_size:
            self.__log_debug("Received invalid message request")
            return RequestHandler.get_err_response()

        client_id = params_raw[: ClientManager.CLIENT_ID_SIZE]
        if not self.__client_mgr.is_id_valid(client_id):
            self.__log_critical("attempt to send message to \
                                     nonexistent client")
            return RequestHandler.get_err_response()

        # validate content size
        content_size_offset = request.FIELD_SIZE_MSG_TYPE_SIZE + ClientManager.CLIENT_ID_SIZE
        content_size = params_raw[content_size_offset : content_size_offset + \
                                    request.FIELD_SIZE_MSG_CONTENT_SIZE]

        content_size = struct.unpack(">I", content_size)[0]
        if len(params_raw[request_min_size :]) != content_size:
            self.__log_critical("attempt to send message with bad size {}".format(content_size))
            return RequestHandler.get_err_response()

        # args always has the client id
        client_src = args[0]
        client_dst = client_id

        #validate msg type
        msg_type_offset = ClientManager.CLIENT_ID_SIZE
        msg_type = params_raw[msg_type_offset : msg_type_offset + \
                                 request.FIELD_SIZE_MSG_TYPE_SIZE]
        
        msg_type = struct.unpack(">B", msg_type)[0]
        if msg_type not in request.SUPPORTED_MSG_TYPES:
            self.__log_critical("attempt to send unsupported msg type")
            return RequestHandler.get_err_response()

        msg_id = self.__messages_mgr.add_message(client_dst,
                                                client_src, msg_type,
                                                params_raw[request_min_size :])
        if msg_id == None:
            return RequestHandler.get_err_response()

        response_obj = response.Response(SUPPORTED_VERSION, response.CODE_MSG_SENT)
        msg_id = msg_id.to_bytes(MessageManager.MSG_ID_SIZE, byteorder = "little")
        response_obj.build(client_dst + msg_id)
        return response_obj
        

    def __get_pubkey(self, params_raw, *args):
        if len(params_raw) != ClientManager.CLIENT_ID_SIZE:
            self.__log_critical("attempt to get pubkey with bad id")
            return RequestHandler.get_err_response()

        client_id = params_raw
        pubkey = self.__client_mgr.get_pub_key(client_id)

        # we send the public key as we get it (even if no public key for id)
        response_obj = response.Response(SUPPORTED_VERSION,
                                         response.CODE_PUBLIC_KEY)
        response_obj.build(pubkey)
        return response_obj

            

    def __fetch_messages(self, params_raw , *args):
        # we always get the client id as parameter
        client_id = args[0]

        messages = self.__messages_mgr.fetch_messages(client_id)

        payload = b""
        for message in messages:
            payload += message

        response_obj = response.Response(SUPPORTED_VERSION, 
                                        response.CODE_MSG_FETCH)
        response_obj.build(payload)

        return response_obj
        
    

    @staticmethod
    def get_err_response():
        # should always succeed
        resp = response.Response(SUPPORTED_VERSION, response.CODE_ERROR)
        return resp


    def __init_logger(self):
        self.__logger = logging.getLogger(self.__class__.__name__)
        self.__logger.setLevel(logging.DEBUG)
        
        # create handler only once!
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

