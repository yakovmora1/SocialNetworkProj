from datetime import datetime
import sys
from manager import Manager



class ClientManager(Manager):
    CLIENT_ID_SIZE = 16
    PUBLIC_KEY_SIZE = 160
    NAME_SIZE = 255


    def __init__(self):
        self.__table_name = "clients"

        super().__init__(self.__class__.__name__)

        sql_create = "CREATE TABLE {}(ID varchar({}) PRIMARY KEY NOT NULL, Name varchar({}),\
                        PublicKey varchar({}), LastSeen date);".format(self.__table_name,
                        self.CLIENT_ID_SIZE, self.NAME_SIZE, self.PUBLIC_KEY_SIZE)

        try:
            self.init_table(self.__table_name, sql_create)
        except:
            self.__log_exception("Failed to initiate clients manager")
            # we exit because its a crucial exception
            sys.exit()
            
    
    
    def add_client(self, client_id, client_name, public_key):
        query = "INSERT INTO {} VALUES(?, ?, ?, {});".format(datetime.now())
        params = [client_id, client_id, public_key]
        
        try:
            self.do_sql(query, params)
        except:
            self.__log_exception("Failed to add client")


    def get_pub_key(self, client_id):
        query = "SELECT PublicKey FROM {} WHERE ID = ?;".format(self.__table_name)
        params = [client_id]

        # should always succeed
        rows = self.do_sql(query, params)

        if len(rows) > 0:
            return rows[0]
        # no pubkey found so return empty one
        return b""

        
    def is_client_exist(self, client_name):
        query = "SELECT * FROM {} WHERE Name = ?;".format(self.__table_name) 
        params = [client_name]

        # shuold always succeed
        rows = self.do_sql(query, params)

        return len(rows) > 0


    def is_id_valid(self, client_id):
        query = "SELECT * FROM {} WHERE ID = ?;".format(self.__table_name) 
        params = [client_id]

        # shuold always succeed
        rows = self.do_sql(query, params)

        return len(rows) > 0


    def fetch_all(self):
        query = "SELECT ID, Name FROM {}".format(self.__table_name)
        rows = []

        try:
            rows = self.do_sql(query)
        except:
            self.__log_exception("Failed to fetch clients")
        
        return rows


    @property
    def number_of_clients(self):
        query = "SELECT * FROM {}".format(self.__table_name)

        try:
            self.do_sql(query)
        except:
            self.__log_debug("Failed to get number of clients")


    # Wrappers for logging functionality
    def __log_debug(self, msg):
        self.logger.debug(msg)
    
    def __log_info(self, msg):
        self.logger.info(msg)
    
    def __log_critical(self, msg):
        self.logger.critical(msg)
    
    def __log_exception(self, msg):
        self.logger.exception(msg)

