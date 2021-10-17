import struct
import logging
import config


MSG_TYPE_GET_SYM_KEY = 1
MSG_TYPE_SEND_SYM_KEY = 2
MSG_TYPE_SEND_TXT_MSG = 3
MSG_TYPE_SEND_FILE = 4
SUPPORTED_MSG_TYPES = [MSG_TYPE_GET_SYM_KEY, MSG_TYPE_SEND_FILE,
                        MSG_TYPE_SEND_SYM_KEY, MSG_TYPE_SEND_TXT_MSG]

# Message request fields' sizes
FIELD_SIZE_MSG_TYPE_SIZE = 1
FIELD_SIZE_MSG_CONTENT_SIZE = 4

CODE_REGISTER = 1000
CODE_LIST_CLIENTS = 1001
CODE_FETCH_PUBLIC_KEY = 1002
CODE_SEND_MSG = 1003
CODE_FETCH_MESSAGES = 1004
SUPPORTED_CODES = (CODE_REGISTER, CODE_LIST_CLIENTS, CODE_SEND_MSG,
                    CODE_FETCH_MESSAGES, CODE_FETCH_PUBLIC_KEY)


class RequestHeaderStructure():
    FIELD_CLIENT_ID = (16, 'B')
    FIELD_CODE = (1, 'H')
    FIELD_VERSION = (1, 'B')
    FIELD_PAYLOAD_SIZE = (1, 'I')


class Request():
    # header size in bytes
    HEADER_SIZE = 23
    MAX_PAYLOAD_SIZE = 0x7fffffff


    def __init__(self, raw_data = b""):
        assert type(raw_data) == bytes
        self.__init_logger(self.__class__.__name__)

        self.__raw_data = raw_data
        self.__supported_version = config.SUPPORTED_VERSION
        self.is_valid = False
              
        self.__header_struct = struct.Struct(format = self.__get_formatter())
        self.__header = {}
        # important for the header decoding
        self.__init_header()
        
        if raw_data != b"":
            if self.decode_header(raw_data) and self.decode_payload(raw_data):
                self.is_valid = True
        

    def __get_formatter(self):
        # We are parsing little endian data from the network
        formatter = "<"

        for field, value in vars(RequestHeaderStructure).items():
            if field.isupper():
                formatter += str(value[0]) + value[1]
        
        self.__log_debug("Formatter created: {}".format(formatter))
        return formatter


    # Decode only the header
    def decode_header(self, raw_data):
        assert type(raw_data) == bytes

        if len(raw_data) < Request.HEADER_SIZE:
            self.__log_debug("Invalied header size")
            return False

        header = raw_data[: Request.HEADER_SIZE]
        self.__parse_header(header)

        if self.__header["FIELD_VERSION"] != self.__supported_version:
            self.__log_debug("Unsupported request version")
            return False

        # check if payload size is not huge
        if self.__header["FIELD_PAYLOAD_SIZE"] > Request.MAX_PAYLOAD_SIZE:
            self.__log_debug("Payload size unsupported")
            return False

        if self.__header["FIELD_CODE"] not in SUPPORTED_CODES:
            self.__log_debug("Invalid request code")
            return False
 
        return True
        

    def decode_payload(self, raw_data):
        assert type(raw_data) == bytes
        assert len(raw_data) < Request.HEADER_SIZE

        if len(raw_data) > Request.MAX_PAYLOAD_SIZE:
            self.__log_info("Received payload of unsupported size")
            return False

        # validate payload size (TODO: should be done in a function)
        if len(raw_data[Request.HEADER_SIZE :]) != self.__header["FIELD_PAYLOAD_SIZE"]:
            self.__log_debug("Invalid payload size")
            return False

        self.__raw_data = raw_data

        return True


    def __init_header(self):
        fields = [field for field in vars(RequestHeaderStructure) if field.isupper()]

        for field in fields:
            self.__header.update({field : 0})


    def __parse_header(self, header):
        unpacked_data = self.__header_struct.unpack(header)

        fields = [field for field in vars(RequestHeaderStructure) if field.isupper()]

        for index, field in enumerate(fields):
            self.__header[field] = unpacked_data[index]

        self.__log_debug("Parsed header: {}".format(self.__header))


    @property
    def client_id(self):
        return self.__header["FIELD_CLIENT_ID"]

    @property
    def client_version(self):
        return self.__header["FIELD_VERSION"]

    @property
    def request_code(self):
        return self.__header["FIELD_CODE"]

    @property
    def payload(self):
        return self.__raw_data[Request.HEADER_SIZE :]

    @property
    def raw_data(self):
        return self.__raw_data
    
    @property
    def payload_size(self):
        return self.__header["FIELD_PAYLOAD_SIZE"]
    
 
    def __init_logger(self, logger_name):
        self.__logger = logging.getLogger(logger_name)
        self.__logger.setLevel(logging.DEBUG)

        # create handler only once!
        if not self.__logger.hasHandlers(): 
            sh = logging.StreamHandler()
            formatter = logging.Formatter("%(name)s - %(levelname)s - %(message)s")
            sh.setFormatter(formatter)
            self.__logger.addHandler(sh)


    def __log_debug(self, msg):
        self.__logger.debug(msg)
    
    def __log_info(self, msg):
        self.__logger.info(msg)
    
    def __log_critical(self, msg):
        self.__logger.critical(msg)
    
    def __log_exception(self, msg):
        self.__logger.exception(msg)