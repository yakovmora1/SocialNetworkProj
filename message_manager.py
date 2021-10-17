import sys
import random
from manager import Manager


# TODO: make singleton
class MessageManager(Manager):
    CLIENT_DST_SIZE = 16
    CLIENT_SRC_SIZE = 16
    MSG_ID_SIZE = 4


    def __init__(self):
        self.__table_name = "messages"

        super().__init__(self.__class__.__name__)

        sql_create = "CREATE TABLE {}(ID int  PRIMARY KEY NOT NULL, ToClient varchar({}),\
                        FromClient varchar({}), Type char, Content BLOB);".format(self.__table_name,
                        self.CLIENT_DST_SIZE, self.CLIENT_SRC_SIZE)

        try:
            self.init_table(self.__table_name, sql_create)
        except:
            self.__log_exception("Failed to initiate message manager")
            sys.exit()
            
    # Wrappers for logging functionality
    def __log_debug(self, msg):
        self.logger.debug(msg)
    
    def __log_info(self, msg):
        self.logger.info(msg)
    
    def __log_critical(self, msg):
        self.logger.critical(msg)
    
    def __log_exception(self, msg):
        self.logger.exception(msg)


    def add_message(self, client_dst, client_src, msg_type, content= ""):
        #sqlite will auto-generate ID
        query = "INSERT INTO {} VALUES(?, ?, ?, ?, ?);".format(self.__table_name)

        # low probability for msg with same id
        msg_id = random.randint(1, 0xffffffff)
        params = [msg_id, client_dst, client_src, msg_type, content]
    
        try:
            self.do_sql(query, params)
        except:
            self.__log_debug("Falied to add new message")
            return None

        return msg_id
        
    
    def fetch_messages(self, client_id):
        self.__log_debug("Client {} requested all messages".format(client_id))

        query = "SELECT Content FROM {} WHERE ToClient = ?;".format(client_id)
        params = [client_id]
        rows = []

        try:
            rows = self.do_sql(query, params)
        except:
            self.__log_debug("Failed to fetch messages for client")
        
        return rows

        

