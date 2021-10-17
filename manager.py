import sqlite3
import logging
from threading import Lock



class Manager():
    def __init__(self, logger_name, db_name = "server.db"):
        self.__db_name = db_name
        self.__connection = None
        self.__lock = Lock()

        self.logger = logging.getLogger(logger_name)
        self.logger.setLevel(logging.DEBUG)
        
        # create handler only once!
        if not self.logger.hasHandlers(): 
            sh = logging.StreamHandler()
            formatter = logging.Formatter("%(name)s - %(levelname)s : %(message)s")
            sh.setFormatter(formatter)
            self.logger.addHandler(sh)


    def init_table(self, table_name, create_query):
        check_tbl_query = "SELECT name FROM sqlite_master WHERE type=\"table\" \
                            AND name=?;"

        rows = self.do_sql(check_tbl_query, [table_name])
        if len(rows) == 0:
            # should always succeed (unless permissions or not enough space)
            self.do_sql(create_query)


    def __create_connection(self):
        self.__connection = sqlite3.connect(self.__db_name)
        self.__connection.text_factory = bytes


    def __cleanup(self):
        if self.__connection != None:
            self.__connection.close()

    
    def do_sql(self, sql, params = []):
        assert type(params) == list

        self.__create_connection()

        try:
            self.__lock.acquire()
            cursor = self.__connection.execute(sql, params)
            self.__connection.commit()
        finally:
            self.__lock.release()

        rows = cursor.fetchall()
        self.__cleanup()

        return rows

    
