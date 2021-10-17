import struct



class ResponseHeaderStructure():
    FIELD_VERSION = (1, 'B')
    FIELD_CODE = (1, 'H')
    FIELD_PAYLOAD_SIZE = (1, 'I')


CODE_SUCCESSFUL_REGISTRATION = 2000
CODE_CLIENTS_LIST = 2001
CODE_PUBLIC_KEY = 2002
CODE_MSG_SENT = 2003
CODE_MSG_FETCH = 2004
CODE_ERROR = 9000

SUPPORTED_CODES = (CODE_SUCCESSFUL_REGISTRATION, CODE_CLIENTS_LIST,
                    CODE_PUBLIC_KEY, CODE_MSG_SENT, CODE_ERROR,
                    CODE_MSG_FETCH)


class Response():
    def __init__(self, version, code, payload = b""):
        assert type(payload) == bytes
        self.__payload = payload

        assert code in SUPPORTED_CODES
        self.__code = code

        self.__version = version
        self.__raw_data = b""

        self.__header_struct = struct.Struct(self.__get_formatter())


    def __get_formatter(self):
        # we send the data in little endian
        format = "<"

        for field, value in vars(ResponseHeaderStructure).items():
            if field.isupper():
                format += str(value[0]) + value[1]
        
        return format


    def build(self, payload = b""):
        assert type(payload) == bytes
        # Response can be with empty payload
        if payload != b"":
            self.__payload = payload

        payload_size = len(self.__payload)

        header = self.__header_struct.pack(self.__version, self.__code,
                                            payload_size)
        
        payload = struct.pack("<{}s".format(payload_size), self.__payload)

        self.__raw_data = header + payload
        return self.__raw_data


    @property
    def payload(self):
        return self.__payload
    
    @property
    def raw_data(self):
        return self.__raw_data
